#pragma once

#include <unordered_map>

#include "MonoCore/MonoPrereq.h"
#include "MonoCore/OhnoClass.h"

namespace ohno
{
	class OhnoAssembly
	{
	public://Public Functions

		//Constructor
		OhnoAssembly(const std::string& filePath);

		//Destructor
		~OhnoAssembly();

		const OhnoClass* GetClass(const char* monoClassName);

	private:
		//Helper to load all classes
		void LoadAllClass();

		//Mono pointers
		::MonoAssembly* mAssembly{ nullptr };
		::MonoImage* mImage{ nullptr };

		std::unordered_map<std::string, std::unique_ptr<OhnoClass>> mClasses{};
	};
}