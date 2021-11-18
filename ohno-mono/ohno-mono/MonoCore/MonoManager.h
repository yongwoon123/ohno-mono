#pragma once

#include <memory>
#include <filesystem>
#include <unordered_map>

#include "MonoCore/MonoPrereq.h"
#include "MonoCore/OhnoAssembly.h"

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

		const OhnoClass* GetClass(const char* monoClassName);

		[[nodiscard]] ::MonoDomain* ScriptDomain() const;

		std::vector<const OhnoClass*> GetInheritedClass(const OhnoClass* myClass) const;

	private: //Private Functions
		void UnloadAllAssembly();

	private: //Private data members
		//Name of file
		std::unordered_map<std::string, std::unique_ptr<OhnoAssembly>> mAssemblies{};

		//Mono Pointers
		::MonoDomain* mRootDomain{};
		::MonoDomain* mScriptDomain{};
	};
}
