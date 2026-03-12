#pragma once

#include "MonoCore/MonoPrereq.h"

namespace ohno
{
    // OhnoClassField wraps a single C# field (::MonoClassField*).
    // Provides typed get/set access and metadata queries (name, type, access modifier).
    // Constructed by OhnoClass::LoadAllFields() - do not create directly.
    class OhnoClassField
    {
    public:
        OhnoClassField(const MonoManager& mm, ::MonoClassField* monoClassField);

        // Read the field value from objInstance into outputValue.
        // outputValue must point to a buffer large enough for the field's type.
        void Get(::MonoObject* objInstance, void* outputValue) const;

        // Write inputValue into the field on objInstance.
        // inputValue must point to the value (same layout rules as Get).
        void Set(::MonoObject* objInstance, void* inputValue) const;

        // Returns the field value as a ::MonoObject*. Use for reference types and arrays.
        ::MonoObject* GetMonoObject(::MonoObject* objInstance) const;

        // Returns the field name (e.g. "Health").
        const char* GetFieldName() const;

        // Returns the type class name (e.g. "Int32", "Vector3").
        const char* GetClassName() const;

        // Returns the C# access modifier as a string ("public", "private", etc.).
        const char* GetAccess() const;

        // Returns the access modifier as a MonoMemberAccess enum value.
        MonoMemberAccess GetAccessEnum() const;

        // Returns true if the field is an array type.
        bool GetIsArray() const;

        // Returns true if the field is a struct.
        bool GetIsStruct() const;

    private:
        friend std::ostream& operator<<(std::ostream& cout, const OhnoClassField& rhs);

        ::MonoClassField* mField     {};
        std::string       mFieldName {};

        ::MonoType*  mType      {};
        ::MonoClass* mClass     {};
        std::string  mClassName {};

        // Raw access flag bits masked against MONO_FIELD_ATTR_FIELD_ACCESS_MASK.
        uint32_t mFlags {};

        bool mIsArray  {};
        bool mIsStruct {};

        const MonoManager& mMonoManagerRef;
    };
}
