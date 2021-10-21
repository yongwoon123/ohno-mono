#include <iostream>
#include <cassert>

#include "MonoCore/MonoManager.h"
#include "MonoCore/OhnoClass.h"

namespace ohno
{
	OhnoClass::OhnoClass(::MonoClass* rawClass)
		: mClass{ rawClass }
		, mName{ mono_class_get_name(mClass) }
	{
		std::cout << "|--" << mName << std::endl;

		LoadAllMethods();
		LoadAllFields();
	}

	OhnoClass::~OhnoClass()
	{
		mMethods.clear();
		mFields.clear();
	}

	::MonoObject* OhnoClass::CreateInstance(::MonoObject* instance, void* args[], size_t num) const
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

	bool OhnoClass::isSubClassOf(::MonoClass* monoClass) const
	{
		if (monoClass == nullptr)
			return false;

		return mono_class_is_subclass_of(mClass, monoClass, true);
	}

	::MonoObject* OhnoClass::InvokeMethod(const char* name, ::MonoObject* instance, void** params, size_t numParams) const
	{
		auto iter = mMethods.find(name);

		if (iter != mMethods.end() && iter->second->GetNumParam() == numParams)
		{
			return iter->second->Invoke(instance, params);
		}

		return nullptr;
	}

	void OhnoClass::LoadAllMethods()
	{
		std::cout << "|----" << "Methods" << std::endl;

		void* methodIter = nullptr;
		::MonoMethod* curClassMethod = mono_class_get_methods(mClass, &methodIter);

		while (curClassMethod != nullptr)
		{
			std::unique_ptr<OhnoMethod> methodPtr = std::make_unique<OhnoMethod>(curClassMethod);
			mMethods[methodPtr->GetMethodName()] = std::move(methodPtr);

			curClassMethod = mono_class_get_methods(mClass, &methodIter);
		}
	}

	void OhnoClass::LoadAllFields()
	{
		std::cout << "|----" << "Fields" << std::endl;

		void* fieldIter = nullptr;
		::MonoClassField* curClassField = mono_class_get_fields(mClass, &fieldIter);

		while (curClassField != nullptr)
		{
			std::shared_ptr<OhnoClassField> field = std::make_shared<OhnoClassField>(curClassField);
			mFields[field->GetFieldName()] = std::move(field);

			curClassField = mono_class_get_fields(mClass, &fieldIter);
		}
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
		return mFields.find(fieldName)->second.get();
	}

	const OhnoMethod* OhnoClass::GetMethod(const char* methodName, size_t numParam) const
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