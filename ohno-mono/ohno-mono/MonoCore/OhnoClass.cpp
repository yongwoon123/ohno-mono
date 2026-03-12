#include <iostream>
#include <cassert>

#include "MonoCore/MonoManager.h"
#include "MonoCore/OhnoClass.h"

namespace ohno
{
    OhnoClass::OhnoClass(const MonoManager& mm, ::MonoClass* rawClass)
        : mClass          { rawClass }
        , mName           { mono_class_get_name(mClass) }
        , mMonoManagerRef { mm }
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
        // mono_object_new allocates the managed object but does NOT call any constructor.
        ::MonoObject* instance = mono_object_new(mMonoManagerRef.ScriptDomain(), mClass);

        if (args == nullptr)
        {
            // mono_runtime_object_init finds and invokes the default .ctor automatically.
            mono_runtime_object_init(instance);
        }
        else
        {
            // ".ctor" is the metadata name Mono uses for all constructors.
            // Mono selects the overload whose parameter count matches num.
            InvokeMethod(".ctor", instance, args, num);
        }

        return instance;
    }

    bool OhnoClass::isSubClassOf(::MonoClass* monoClass) const
    {
        if (monoClass == nullptr)
            return false;

        // The third argument (true) includes interface implementations in addition to class inheritance.
        return mono_class_is_subclass_of(mClass, monoClass, true);
    }

    ::MonoObject* OhnoClass::InvokeMethod(const char* name, ::MonoObject* instance, void** params, size_t numParams) const
    {
        const auto it = std::ranges::find_if(mMethods, [&name, &numParams] (const auto& method)->bool
            {
                return std::strcmp(name, method->GetMethodName()) == 0
                    && method->GetNumParam() == numParams;
            });

        return it == mMethods.end() ? nullptr : (*it)->Invoke(instance, params);
    }

    ::MonoObject* OhnoClass::InvokeVirtualMethod(const char* name, ::MonoObject* instance) const
    {
        // Look up the method on this class, then resolve the actual override for the given
        // instance via mono_object_get_virtual_method (walks the vtable).
        ::MonoMethod* method = mono_class_get_method_from_name(mClass, name, 0);
        if (method == nullptr) return nullptr;
        ::MonoMethod* virtualMethod = mono_object_get_virtual_method(instance, method);
        ::MonoObject* exception = nullptr;
        return mono_runtime_invoke(virtualMethod, instance, nullptr, &exception);
    }

    void OhnoClass::AddInternalCall(const std::string& name, const void* method) const
    {
        // Mono requires the full "ClassName::MethodName" format for internal call registration.
        const std::string fullMethodName = std::string { mName } + "::" + name;
        mono_add_internal_call(fullMethodName.c_str(), method);
    }

    void OhnoClass::LoadAllMethods()
    {
        // mono_class_get_methods iterates with an opaque iterator - pass nullptr to start.
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
            mFields.emplace_back(std::make_unique<OhnoClassField>(mMonoManagerRef, curClassField));
            curClassField = mono_class_get_fields(mClass, &fieldIter);
        }
    }

    void OhnoClass::LoadAllInheritedFields()
    {
        auto allInheritedClasses = mMonoManagerRef.GetInheritedClass(this);

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

        return it == mMethods.end() ? nullptr : (*it).get();
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
