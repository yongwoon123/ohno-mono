#pragma once

#include <memory>
#include <filesystem>
#include <unordered_map>

#include "MonoCore/MonoPrereq.h"
#include "MonoCore/MonoAssembly.h"

namespace ohno
{
	class MonoManager
	{
	public: //Public Functions

		//Constructor
		MonoManager() = default;

		//Destructor
		~MonoManager();

		//Init function called only once
		void Init();

		void CompileAssembly(const std::string& csprojPath);

		//Loads assembly compiled by the compile assembly function
		void LoadAssembly();

		//Loads specfic assembly
		void LoadAssembly(const std::filesystem::path& assemblyPath);

		::MonoObject* CreateInstance(const char* monoClassName, void** args = nullptr, size_t num = 0);
		const MonoClass* GetClass(const char* monoClassName);

		[[nodiscard]] MonoDomain* ScriptDomain() const;

	private: //Private Functions
		void UnloadAllAssembly();

	private: //Private data members
		//Name of file
		std::unordered_map<std::string, std::unique_ptr<MonoAssembly>> mAssemblies{};

		//Mono Pointers
		MonoDomain* mRootDomain{};
		MonoDomain* mScriptDomain{};
	};
}
