#pragma once

#include <unordered_map>

#include "MonoCore/MonoPrereq.h"

namespace ohno
{
	class MonoAssembly
	{
	public:
		MonoAssembly(const std::string& filePath);
		~MonoAssembly();

		[[nodiscard]] bool GetLoaded();

	private:
		void LoadAllClass();

		bool mIsLoaded{ false };

		//Mono pointers
		::MonoAssembly* mAssembly{ nullptr };
		::MonoImage* mImage{ nullptr };

		//std::map<str, std::shared_ptr<MonoClass>> mClasses{};
	};
}