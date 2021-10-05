#pragma once

#include <memory>
#include <unordered_map>

#include "MonoCore/MonoPrereq.h"

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

		void CompileAssembly(const char* csprojPath);
		void LoadAssembly(const char* dllPath);

		[[nodiscard]] MonoDomain* ScriptDomain() const;
		
	private: //Private Functions
		void UnloadAllAssembly();

	private: //Private data members
		//Name of file
		//std::unordered_map<const char*, std::unique_ptr<MonoAssembly>> mAssemblies{};

		//Mono Pointers
		MonoDomain* mRootDomain{};
		MonoDomain* mScriptDomain{};
	};
}
