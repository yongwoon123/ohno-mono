#include "MonoCore/OhnoArray.h"

#include "MonoCore/MonoManager.h"
#include "MonoCore/OhnoString.h"

namespace ohno
{
    OhnoArray::OhnoArray(const MonoManager& mm, ::MonoClass* klass, uint32_t size)
        : mRawClass       { klass }
        , mMonoManagerRef { mm }
    {
        mArray = mono_array_new(mm.CurrentDomain(), klass, static_cast<uintptr_t>(size));
    }

    OhnoArray::OhnoArray(const MonoManager& mm, ::MonoArray* existing)
        : mArray          { existing }
        , mRawClass       { nullptr }
        , mMonoManagerRef { mm }
    {
        // mono_class_get_element_class strips the "[]" wrapper from the array type.
        ::MonoClass* arrClass = mono_object_get_class(reinterpret_cast<::MonoObject*>(existing));
        mRawClass = mono_class_get_element_class(arrClass);
    }

    uint32_t OhnoArray::Size() const
    {
        return static_cast<uint32_t>(mono_array_length(mArray));
    }

    ::MonoArray* OhnoArray::GetInternal() const
    {
        return mArray;
    }

    ::MonoClass* OhnoArray::GetRawClass() const
    {
        return mRawClass;
    }

    uint8_t* OhnoArray::GetArrayAddr(::MonoArray* arr, int size, uintptr_t index)
    {
        return reinterpret_cast<uint8_t*>(mono_array_addr_with_size(arr, size, index));
    }

    // --- String specializations ---
    // Strings are reference types. memcpy would store a raw GC pointer into the array slot,
    // bypassing the write barrier - the GC could then move the object and leave a dangling pointer.
    // mono_array_set fires the write barrier correctly.

    template<>
    std::string OhnoArray::Get<std::string>(uint32_t index) const
    {
        ::MonoString* monoStr = mono_array_get(mArray, ::MonoString*, index);
        return OhnoString::MonoToString(monoStr);
    }

    template<>
    void OhnoArray::Set<std::string>(uint32_t index, const std::string& value)
    {
        ::MonoString* monoStr = OhnoString::StringToMono(mMonoManagerRef, value);
        mono_array_set(mArray, ::MonoString*, index, monoStr);
    }
}
