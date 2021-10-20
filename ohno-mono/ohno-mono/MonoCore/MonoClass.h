#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "MonoCore/MonoPrereq.h"
#include "MonoCore/MonoMethod.h"
#include "MonoCore/MonoClassField.h"

namespace ohno
{
	class MonoClass
	{
	public:
		MonoClass(::MonoClass* rawClass);
		~MonoClass();

		::MonoObject* CreateInstance(::MonoObject* instance = nullptr, void* args[] = nullptr, size_t num = 0) const;

		::MonoObject* InvokeMethod(const char* name,
								   ::MonoObject* instance = nullptr,
								   void** params = nullptr,
								   size_t numParams = 0) const;

		const MonoClassField* GetField(const char* fieldName) const;
		const MonoMethod* GetMethod(const char* methodName, size_t numParam = 0) const;

		[[nodiscard]] bool isSubClassOf(::MonoClass* monoClass) const;
		[[nodiscard]] const char* GetClassName() const;
		[[nodiscard]] ::MonoClass* GetRawClass() const;

	private:
		void LoadAllMethods();
		void LoadAllFields();

	private:
		::MonoClass* mClass{ nullptr };
		const char*  mName{ nullptr };

		std::unordered_map<std::string, std::unique_ptr<MonoMethod>>     mMethods{};
		std::unordered_map<std::string, std::shared_ptr<MonoClassField>> mFields{};
	};
}