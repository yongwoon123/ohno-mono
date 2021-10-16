#include <iostream>
#include <filesystem>

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
		for (const auto& dir : stdfs::directory_iterator{ GetDllOutputPath() })
		{
			const stdfs::path& path = dir.path();

			if (path.extension() == ".dll")
			{
				LoadAssembly(path.string());
			}
		}
	}

	void MonoManager::LoadAssembly(const std::string& assemblyPath)
	{
		std::cout << "\nLoading Assembly: " << assemblyPath << std::endl;
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
	}

	MonoDomain* MonoManager::ScriptDomain() const
	{
		return mScriptDomain;
	}
}