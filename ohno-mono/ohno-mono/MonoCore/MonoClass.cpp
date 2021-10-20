#include <iostream>
#include <cassert>

#include "MonoCore/MonoManager.h"
#include "MonoCore/MonoClass.h"

namespace ohno
{
	MonoClass::MonoClass(::MonoClass* rawClass)
		: mClass{ rawClass }
		, mName{ mono_class_get_name(mClass) }
	{
		std::cout << "|--" << mName << std::endl;

		LoadAllMethods();
		LoadAllFields();
	}

	MonoClass::~MonoClass()
	{
		mMethods.clear();
		mFields.clear();
	}

	::MonoObject* MonoClass::CreateInstance(::MonoObject* instance, void* args[], size_t num) const
	{
		assert(instance != nullptr);

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

	bool MonoClass::isSubClassOf(::MonoClass* monoClass) const
	{
		if (monoClass == nullptr)
			return false;

		return mono_class_is_subclass_of(mClass, monoClass, true);
	}

	::MonoObject* MonoClass::InvokeMethod(const char* name, ::MonoObject* instance, void** params, size_t numParams) const
	{
		auto iter = mMethods.find(name);

		if (iter != mMethods.end() && iter->second->GetNumParam() == numParams)
		{
			return iter->second->Invoke(instance, params);
		}

		return nullptr;
	}

	void MonoClass::LoadAllMethods()
	{
		std::cout << "|----" << "Methods" << std::endl;

		void* methodIter = nullptr;
		::MonoMethod* curClassMethod = mono_class_get_methods(mClass, &methodIter);

		while (curClassMethod != nullptr)
		{
			std::unique_ptr<MonoMethod> methodPtr = std::make_unique<MonoMethod>(curClassMethod);
			mMethods[methodPtr->GetMethodName()] = std::move(methodPtr);

			curClassMethod = mono_class_get_methods(mClass, &methodIter);
		}
	}

	void MonoClass::LoadAllFields()
	{
		std::cout << "|----" << "Fields" << std::endl;

		void* fieldIter = nullptr;
		::MonoClassField* curClassField = mono_class_get_fields(mClass, &fieldIter);

		while (curClassField != nullptr)
		{
			std::shared_ptr<MonoClassField> field = std::make_shared<MonoClassField>(curClassField);
			mFields[field->GetFieldName()] = std::move(field);

			curClassField = mono_class_get_fields(mClass, &fieldIter);
		}
	}

	const char* MonoClass::GetClassName() const
	{
		return mName;
	}

	::MonoClass* MonoClass::GetRawClass() const
	{
		return mClass;
	}

	const MonoClassField* MonoClass::GetField(const char* fieldName) const
	{
		return mFields.find(fieldName)->second.get();
	}

	const MonoMethod* MonoClass::GetMethod(const char* methodName, size_t numParam) const
	{
		auto it = mMethods.find(methodName);

		while (it != mMethods.end())
		{
			if (it->second->GetNumParam() == numParam)
			{
				return it->second.get();
			}
		}

		return nullptr;
	}
}