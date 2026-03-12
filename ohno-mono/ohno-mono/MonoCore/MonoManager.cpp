#include <iostream>

#include "MonoCore/MonoManager.h"

namespace ohno
{
    namespace stdfs = std::filesystem;

    // Paths to the bundled Mono runtime - relative to the executable's working directory.
    // Passed to mono_set_dirs() so Mono can find its standard libraries and config.
    static const char* etc = "../External/mono/etc";
    static const char* lib = "../External/mono/lib";

    // .bat wrapper for the bundled msbuild compiler.
    // Using a .bat allows system() to invoke it without Mono being on the system PATH.
    static const char* compiler = R"(External\mono\bin\msbuild.bat)";

    // Output directory for compiled .dll files, relative to the running executable.
    static const char* outputDir = "MonoDll";
    static const char* dllName   = "Assembly-CSharp.dll"; // Must match the C# project assembly name.

    // Absolute path to the msbuild compiler script.
    // Computed once via a static local. parent_path() steps up from the exe directory
    // to the solution root where External/ lives.
    const stdfs::path& GetFullCompilerPath()
    {
        static const stdfs::path fullCompilerPath { stdfs::current_path().parent_path() / compiler };
        return fullCompilerPath;
    }

    // Absolute path to the MonoDll/ output directory, relative to the running executable.
    const stdfs::path& GetDllOutputPath()
    {
        static const stdfs::path dllOutput { stdfs::current_path() / outputDir };
        return dllOutput;
    }

    MonoManager::~MonoManager()
    {
        UnloadAllAssembly();

        // mono_jit_cleanup shuts down the Mono JIT and releases the root domain.
        // Only call this when the process is exiting - it cannot be re-initialized afterwards.
        if (mRootDomain != nullptr)
        {
            mono_jit_cleanup(mRootDomain);
        }
    }

    MonoManager& MonoManager::Get()
    {
        static MonoManager instance;
        return instance;
    }

    void MonoManager::Reload(const std::string& csprojPath)
    {
        std::cout << "[Reload] Unloading script domain...\n";
        UnloadAllAssembly();
        std::cout << "[Reload] Recompiling C#...\n";
        CompileAssembly(csprojPath);
        std::cout << "[Reload] Loading assembly...\n";
        LoadAssembly();
    }

    void MonoManager::Init()
    {
        mono_set_dirs(lib, etc);

#ifdef _DEBUG
        mono_debug_init(MONO_DEBUG_FORMAT_MONO);
#endif // _DEBUG

        mono_config_parse(nullptr);

        // "v4.0.30319" targets .NET 4.x / Mono 4.5 API.
        // The root domain stays alive for the entire process and cannot be unloaded.
        mRootDomain = mono_jit_init_version("ohno-mono", "v4.0.30319");
    }

    void MonoManager::CompileAssembly(const std::string& csprojPath)
    {
        std::cout << "Compiling " << csprojPath << "\n";

        // Outer quotes wrap the whole command so Windows treats it as a single shell command.
        std::string command { "\"\"" + GetFullCompilerPath().string() + "\" " };

#ifdef _DEBUG
        command += "/p:Configuration=Debug /p:Platform=x64 /p:OutputPath=\"" + GetDllOutputPath().string() + "\" ";
#else
        command += "/p:Configuration=Release /p:Platform=x64 /p:OutputPath = \"" + GetDllOutputPath().string() + "\" ";
#endif
        command += "\"" + csprojPath + "\"" + "\"";

        // system() is used here for simplicity.
        // Consider a dedicated build thread so compilation does not block the main loop.
        system(command.c_str());
    }

    void MonoManager::LoadAssembly()
    {
        if (mScriptDomain == nullptr)
        {
            // The domain name is just a label used for debugging.
            // mono_domain_set activates it so subsequent Mono calls target this domain.
            const std::string outputDirStr { (stdfs::current_path() / outputDir).string() };
            mScriptDomain = mono_domain_create_appdomain(const_cast<char*>(outputDirStr.c_str()), nullptr);
            mono_domain_set(mScriptDomain, true);
        }

        for (const auto& dir : stdfs::directory_iterator { GetDllOutputPath() })
        {
            const stdfs::path& path = dir.path();

            if (path.extension() == ".dll")
            {
                LoadAssembly(path);
            }
        }

        // LoadDependencies triggers method and inherited field loading for every class.
        // Deferred until all assemblies are loaded so cross-assembly type references resolve correctly.
        std::ranges::for_each(mAssemblies, [] (const auto& assem)->void
            {
                assem.second->LoadDependencies();
            });
    }

    void MonoManager::LoadAssembly(const std::filesystem::path& assemblyPath)
    {
        // Keyed by filename stem (e.g. "Assembly-CSharp" from Assembly-CSharp.dll).
        const std::string& fileName = assemblyPath.stem().string();
        mAssemblies[fileName] = std::make_unique<OhnoAssembly>(*this, assemblyPath.string());
    }

    void MonoManager::UnloadAllAssembly()
    {
        if (mScriptDomain != nullptr)
        {
            // Must switch to the root domain before unloading the script domain -
            // unloading the currently active domain is not permitted.
            mono_domain_set(mono_get_root_domain(), true);

            // Allow managed finalizers up to 2000ms to complete before forcing teardown.
            mono_domain_finalize(mScriptDomain, 2000);

            // Destroys all managed objects and frees JIT-compiled code.
            // Any ::MonoObject* or ::MonoClass* from this domain is now invalid.
            ::MonoObject* exception = nullptr;
            mono_domain_try_unload(mScriptDomain, &exception);

            mono_gc_collect(mono_gc_max_generation());

            mScriptDomain = nullptr;
        }

        mAssemblies.clear();
    }

    ::MonoObject* MonoManager::CreateInstance(const char* monoClassName, void** args, size_t num)
    {
        const OhnoClass* classPtr = GetClass(monoClassName);

        if (classPtr)
        {
            return classPtr->CreateInstance(args, num);
        }

        return nullptr;
    }

    const OhnoClass* MonoManager::GetClass(const char* monoClassName)
    {
        const OhnoClass* ret = nullptr;

        for (const auto& [assemblyName, assembly] : mAssemblies)
        {
            ret = assembly->GetClass(monoClassName);

            if (ret)
            {
                return ret;
            }
        }

        return ret;
    }

    std::vector<const OhnoClass*> MonoManager::GetInheritedClass(const OhnoClass* myClass) const
    {
        std::vector<const OhnoClass*> allInheriClass = {};

        std::ranges::for_each(mAssemblies, [&allInheriClass, &myClass] (const auto& entry)->void
            {
                auto temp = entry.second->GetInheritedClass(myClass);
                allInheriClass.insert(allInheriClass.end(), temp.begin(), temp.end());
            });

        return allInheriClass;
    }

    ::MonoDomain* MonoManager::ScriptDomain() const
    {
        return mScriptDomain;
    }

    ::MonoDomain* MonoManager::CurrentDomain() const
    {
        return mScriptDomain;
    }
}
