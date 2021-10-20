#include <iostream>

#include "MonoCore/MonoManager.h"

namespace ohno
{
	namespace stdfs = std::filesystem;

	static const char* etc = "../External/mono/etc";
	static const char* lib = "../External/mono/lib";
	static const char* compiler = R"(External\mono\bin\xbuild)";

	static const char* outputDir = "MonoDll";
	static const char* dllName = "Assembly-CSharp.dll"; // Check the Sample C# Project assembly name

	const stdfs::path& GetFullCompilerPath()
	{
		static const stdfs::path fullCompilerPath{ stdfs::current_path().parent_path() / compiler };
		return fullCompilerPath;
	}

	const stdfs::path& GetDllOutputPath()
	{
		static const stdfs::path dllOutput{ stdfs::current_path() / outputDir };
		return dllOutput;
	}

	MonoManager::~MonoManager()
	{
		UnloadAllAssembly();

		if (mRootDomain != nullptr)
		{
			mono_jit_cleanup(mRootDomain);
		}
	}

	void MonoManager::Init()
	{
		std::cout << "Mono Init" << std::endl;

		mono_set_dirs(lib, etc);

#ifdef _DEBUG
		mono_debug_init(MONO_DEBUG_FORMAT_MONO);
#endif // _DEBUG

		mono_config_parse(nullptr);

		mRootDomain = mono_jit_init_version("ohno-mono", "v4.0.30319");
	}

	void MonoManager::CompileAssembly(const std::string& csprojPath)
	{
		std::cout << "Compiling " << csprojPath << std::endl;

		std::string command{ "\"\"" + GetFullCompilerPath().string() + "\" " };

#ifdef _DEBUG
		command += "/p:Configuration=Debug /p:Platform=x64 /p:OutputPath=\"" + GetDllOutputPath().string() + "\" ";
#else
		command += "/p:Configuration=Release /p:Platform=x64 /p:OutputPath = \"" + GetDllOutputPath().string() + "\" ";
#endif
		command += "\"" + csprojPath + "\"";

		system(command.c_str());
	}

	void MonoManager::LoadAssembly()
	{
		if (mScriptDomain == nullptr)
		{
			const std::string outputDirStr{ (stdfs::current_path() / outputDir).string() };
			mScriptDomain = mono_domain_create_appdomain(const_cast<char*>(outputDirStr.c_str()), nullptr);
			mono_domain_set(mScriptDomain, true);

			//AssertSystem::Assert(mScriptDomain, "Cannot create script app domain.");
		}

		for (const auto& dir : stdfs::directory_iterator{ GetDllOutputPath() })
		{
			const stdfs::path& path = dir.path();

			if (path.extension() == ".dll")
			{
				LoadAssembly(path);
			}
		}
	}

	void MonoManager::LoadAssembly(const std::filesystem::path& assemblyPath)
	{
		std::cout << assemblyPath.filename().string() << std::endl;

		const std::string& fileName = assemblyPath.stem().string();
		mAssemblies[fileName] = std::make_unique<MonoAssembly>(assemblyPath.string());
	}

	void MonoManager::UnloadAllAssembly()
	{
		if (mScriptDomain != nullptr)
		{
			mono_domain_set(mono_get_root_domain(), true);
			mono_domain_finalize(mScriptDomain, 2000);

			::MonoObject* exception = nullptr;
			mono_domain_try_unload(mScriptDomain, &exception);

			mono_gc_collect(mono_gc_max_generation());

			mScriptDomain = nullptr;
		}

		mAssemblies.clear();
	}

	::MonoObject* MonoManager::CreateInstance(const char* monoClassName, void** args, size_t num)
	{
		const MonoClass* classPtr = GetClass(monoClassName);

		if (classPtr)
		{
			auto* obj = mono_object_new(mScriptDomain, classPtr->GetRawClass());

			return classPtr->CreateInstance(
				obj,
				args,
				num
			);
		}

		return nullptr;
	}

	const MonoClass* MonoManager::GetClass(const char* monoClassName)
	{
		const MonoClass* ret = nullptr;

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

	MonoDomain* MonoManager::ScriptDomain() const
	{
		return mScriptDomain;
	}
}