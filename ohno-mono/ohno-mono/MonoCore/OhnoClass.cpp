#include <iostream>
#include <cassert>

#include "MonoCore/MonoManager.h"
#include "MonoCore/OhnoClass.h"

namespace ohno
{
	OhnoClass::OhnoClass(const MonoManager& mm, ::MonoClass* rawClass)
		: mClass { rawClass }
		, mName { mono_class_get_name(mClass) }
		, monoManagerRef { mm }
	{
		LoadAllFields();
	}

	OhnoClass::~OhnoClass()
	{
		mMethods.clear();
		mFields.clear();
	}

	::MonoObject* OhnoClass::CreateInstance(void* args[], size_t num) const
	{
		::MonoObject* instance = mono_object_new(monoManagerRef.ScriptDomain(), mClass);

		if (args == nullptr)
		{
			mono_runtime_object_init(instance);
		}
		else
		{
			InvokeMethod(".ctor", instance, args, num);
		}

		return instance;
	}

	bool OhnoClass::isSubClassOf(::MonoClass* monoClass) const
	{
		if (monoClass == nullptr)
			return false;

		return mono_class_is_subclass_of(mClass, monoClass, true);
	}

	::MonoObject* OhnoClass::InvokeMethod(const char* name, ::MonoObject* instance, void** params, size_t numParams) const
	{
		const auto it = std::ranges::find_if(mMethods, [&name, &numParams] (const auto& method)->bool
			{
				return std::strcmp(name, method->GetMethodName()) == 0
					&& method->GetNumParam() == numParams;
			});

		return  it == mMethods.end() ? nullptr : (*it)->Invoke(instance, params);
	}

	void OhnoClass::AddInternalCall(const std::string& name, const void* method) const
	{
		const std::string fullMethodName = std::string { mName } + "::" + name;
		mono_add_internal_call(fullMethodName.c_str(), method);
	}

	void OhnoClass::LoadAllMethods()
	{
		void* methodIter = nullptr;
		::MonoMethod* curClassMethod = mono_class_get_methods(mClass, &methodIter);

		while (curClassMethod != nullptr)
		{
			mMethods.emplace_back(std::make_unique<OhnoMethod>(curClassMethod));
			curClassMethod = mono_class_get_methods(mClass, &methodIter);
		}
	}

	void OhnoClass::LoadAllFields()
	{
		void* fieldIter = nullptr;
		::MonoClassField* curClassField = mono_class_get_fields(mClass, &fieldIter);

		while (curClassField != nullptr)
		{
			mFields.emplace_back(std::make_unique<OhnoClassField>(curClassField));
			curClassField = mono_class_get_fields(mClass, &fieldIter);
		}
	}

	void OhnoClass::LoadAllInheritedFields()
	{
		auto allInheritedClasses = monoManagerRef.GetInheritedClass(this);

		std::ranges::for_each(allInheritedClasses, [this] (const auto& klass)->void
			{
				auto& vec = mInheritedFields[klass->GetClassName()];

				std::ranges::for_each(klass->mFields, [&vec] (const auto& field)->void
					{
						vec.emplace_back(field.get());
					});
			});
	}

	const char* OhnoClass::GetClassName() const
	{
		return mName;
	}

	::MonoClass* OhnoClass::GetRawClass() const
	{
		return mClass;
	}

	const OhnoClassField* OhnoClass::GetField(const char* fieldName) const
	{
		const auto it = std::ranges::find_if(mFields, [&fieldName] (const auto& field)->bool
			{
				return std::strcmp(fieldName, field->GetFieldName()) == 0;
			});

		return it == mFields.end() ? nullptr : (*it).get();
	}

	const OhnoMethod* OhnoClass::GetMethod(const char* methodName, size_t numParam) const
	{
		const auto it = std::ranges::find_if(mMethods, [&methodName, &numParam] (const auto& method)->bool
			{
				return std::strcmp(methodName, method->GetMethodName()) == 0
					&& method->GetNumParam() == numParam;
			});

		return  it == mMethods.end() ? nullptr : (*it).get();;
	}

	std::ostream& operator<<(std::ostream& cout, const OhnoClass& rhs)
	{
		cout << "|------Fields\n";

		std::ranges::for_each(rhs.mFields, [&cout] (const auto& field)->void
			{
				cout << "|--------" << field->GetFieldName() << "\n";
				cout << *field << "\n";
			});

		cout << "|------Inherited Fields\n";

		std::ranges::for_each(rhs.mInheritedFields, [&cout] (const auto& fieldVec)->void
			{
				std::ranges::for_each(fieldVec.second, [&fieldVec, &cout] (const auto& field)->void
					{
						cout << "|--------" << fieldVec.first << "::" << field->GetFieldName() << "\n";
						cout << *field << "\n";
					});
			});

		cout << "|------Methods\n";

		std::ranges::for_each(rhs.mMethods, [&cout] (const auto& method)->void
			{
				cout << "|--------" << method->GetMethodName() << "\n";
				cout << *method << "\n";
			});

		return cout;
	}
}