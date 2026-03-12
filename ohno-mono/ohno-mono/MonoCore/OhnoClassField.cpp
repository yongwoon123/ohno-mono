#include <iostream>

#include "MonoCore/MonoManager.h"
#include "MonoCore/OhnoClassField.h"

namespace ohno
{
    OhnoClassField::OhnoClassField(const MonoManager& mm, ::MonoClassField* monoClassField)
        : mField          { monoClassField }
        , mFieldName      { mono_field_get_name(mField) }
        , mType           { mono_field_get_type(mField) }
        , mClass          { mono_class_from_mono_type(mType) }
        , mClassName      { mono_class_get_name(mClass) }
        , mFlags          { mono_field_get_flags(mField) }
        , mIsArray        { mClassName.find("[]") != std::string::npos }
        , mIsStruct       { (bool)mono_type_is_struct(mType) }
        , mMonoManagerRef { mm }
    {
    }

    void OhnoClassField::Get(::MonoObject* objInstance, void* outputValue) const
    {
        mono_field_get_value(objInstance, mField, outputValue);
    }

    void OhnoClassField::Set(::MonoObject* objInstance, void* inputValue) const
    {
        mono_field_set_value(objInstance, mField, inputValue);
    }

    ::MonoObject* OhnoClassField::GetMonoObject(::MonoObject* objInstance) const
    {
        return mono_field_get_value_object(mMonoManagerRef.CurrentDomain(), mField, objInstance);
    }

    const char* OhnoClassField::GetFieldName() const
    {
        return mFieldName.c_str();
    }

    const char* OhnoClassField::GetClassName() const
    {
        return mClassName.c_str();
    }

    const char* OhnoClassField::GetAccess() const
    {
        switch (mFlags & MONO_FIELD_ATTR_FIELD_ACCESS_MASK)
        {
            case MONO_FIELD_ATTR_PRIVATE:        return "private";
            case MONO_FIELD_ATTR_FAM_AND_ASSEM:  return "protected internal";
            case MONO_FIELD_ATTR_ASSEMBLY:       return "internal";
            case MONO_FIELD_ATTR_FAMILY:         return "protected";
            case MONO_FIELD_ATTR_PUBLIC:         return "public";
            default:                             return "unknown";
        }
    }

    MonoMemberAccess OhnoClassField::GetAccessEnum() const
    {
        switch (mFlags & MONO_FIELD_ATTR_FIELD_ACCESS_MASK)
        {
            case MONO_FIELD_ATTR_PRIVATE:       return MonoMemberAccess::Private;
            case MONO_FIELD_ATTR_FAM_AND_ASSEM: return MonoMemberAccess::ProtectedInternal;
            case MONO_FIELD_ATTR_ASSEMBLY:      return MonoMemberAccess::Internal;
            case MONO_FIELD_ATTR_FAMILY:        return MonoMemberAccess::Protected;
            case MONO_FIELD_ATTR_PUBLIC:        return MonoMemberAccess::Public;
            default:                            return MonoMemberAccess::Private;
        }
    }

    bool OhnoClassField::GetIsArray() const
    {
        return mIsArray;
    }

    bool OhnoClassField::GetIsStruct() const
    {
        return mIsStruct;
    }

    std::ostream& operator<<(std::ostream& cout, const OhnoClassField& rhs)
    {
        cout << "|----------" << rhs.GetAccess() << " " << rhs.mClassName << " " << rhs.mFieldName;
        return cout;
    }
}
