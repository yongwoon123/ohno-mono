#pragma once

#include <unordered_map>

#include "MonoCore/MonoPrereq.h"
#include "MonoCore/MonoClass.h"

namespace ohno
{
	class MonoAssembly
	{
	public://Public Functions

		//Constructor
		MonoAssembly(const std::string& filePath);

		//Destructor
		~MonoAssembly();

		const MonoClass* GetClass(const char* monoClassName);

	private:
		//Helper to load all classes
		void LoadAllClass();

		//Mono pointers
		::MonoAssembly* mAssembly{ nullptr };
		::MonoImage* mImage{ nullptr };

		std::unordered_map<std::string, std::unique_ptr<MonoClass>> mClasses{};
	};
}