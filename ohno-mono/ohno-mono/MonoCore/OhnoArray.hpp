#pragma once

#include <cstring>

#include "MonoCore/MonoManager.h"

// Template method bodies for OhnoArray.
// Included at the bottom of OhnoArray.h - do not include this file directly.

namespace ohno
{
    template <typename T>
    OhnoArray::OhnoArray(const MonoManager& mm, const std::vector<T>& vec, ::MonoClass* klass)
        : mRawClass       { klass }
        , mMonoManagerRef { mm }
    {
        mArray = mono_array_new(mm.CurrentDomain(), klass, static_cast<uintptr_t>(vec.size()));
        if (!vec.empty())
        {
            // Single bulk memcpy into the managed array buffer.
            // Valid only for value types whose C++ layout matches C# layout exactly.
            memcpy(GetArrayAddr(mArray, sizeof(T), 0), vec.data(), sizeof(T) * vec.size());
        }
    }

    template <typename T>
    T OhnoArray::Get(uint32_t index) const
    {
        T value{};
        memcpy(&value, GetArrayAddr(mArray, sizeof(T), index), sizeof(T));
        return value;
    }

    template <typename T>
    void OhnoArray::Set(uint32_t index, const T& value)
    {
        memcpy(GetArrayAddr(mArray, sizeof(T), index), &value, sizeof(T));
    }

    // String specializations defined in OhnoArray.cpp.
    template<> void OhnoArray::Set<std::string>(uint32_t index, const std::string& value);
    template<> std::string OhnoArray::Get<std::string>(uint32_t index) const;
}
