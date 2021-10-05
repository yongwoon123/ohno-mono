#include <iostream>

#include "MonoCore/MonoManager.h"

namespace ohno
{
	static const char* etc = "../External/mono/etc";
	static const char* lib = "../External/mono/lib";
	static const char* compiler = R"(External\mono\bin)";

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
		mono_set_dirs(lib, etc);

#ifdef _DEBUG
		mono_debug_init(MONO_DEBUG_FORMAT_MONO);
#endif // _DEBUG

		mono_config_parse(nullptr);

		mRootDomain = mono_jit_init_version("ohno-mono", "v4.0.30319");

		std::cout << "Mono Init" << std::endl;
	}

	void MonoManager::CompileAssembly(const char* csprojPath)
	{
	}

	void MonoManager::LoadAssembly(const char* path)
	{
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