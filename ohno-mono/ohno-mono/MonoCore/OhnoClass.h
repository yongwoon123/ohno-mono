#pragma once

#include <string>
#include <memory>
#include <map>
#include <unordered_map>

#include "MonoCore/MonoPrereq.h"
#include "MonoCore/OhnoMethod.h"
#include "MonoCore/OhnoClassField.h"

namespace ohno
{
	class OhnoClass
	{
	public:
		OhnoClass(const MonoManager& mm, ::MonoClass* rawClass);
		~OhnoClass();

		::MonoObject* CreateInstance(void* args[] = nullptr, size_t num = 0) const;

		::MonoObject* InvokeMethod(const char* name,
								   ::MonoObject* instance = nullptr,
								   void** params = nullptr,
								   size_t numParams = 0) const;

		void AddInternalCall(const std::string& name, const void* method) const;

		const OhnoClassField* GetField(const char* fieldName) const;
		const OhnoMethod* GetMethod(const char* methodName, size_t numParam = 0) const;

		[[nodiscard]] bool isSubClassOf(::MonoClass* monoClass) const;
		[[nodiscard]] const char* GetClassName() const;
		[[nodiscard]] ::MonoClass* GetRawClass() const;

	private:
		friend OhnoAssembly;
		friend std::ostream& operator<<(std::ostream& cout, const OhnoClass& rhs);

		void LoadAllMethods();
		void LoadAllFields();
		void LoadAllInheritedFields();

	private:
		::MonoClass* mClass { nullptr };
		const char* mName { nullptr };

		std::vector<std::unique_ptr<OhnoClassField>> mFields {};
		std::map<std::string, std::vector<const OhnoClassField*>> mInheritedFields {};

		std::vector<std::unique_ptr<OhnoMethod>> mMethods {};

		const MonoManager& monoManagerRef;
	};
}