#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "MonoCore/MonoPrereq.h"

namespace ohno
{
    // OhnoArray wraps the Mono managed array API, providing typed construction,
    // element access, and runtime type dispatch from C++.
    class OhnoArray
    {
    public:
        // Allocate an empty array of [size] elements of type [klass].
        // Fill elements via Set<T> after construction.
        // Required for string arrays - strings cannot be bulk-copied via memcpy.
        OhnoArray(const MonoManager& mm, ::MonoClass* klass, uint32_t size);

        // Construct from a C++ vector via a single bulk memcpy.
        // Value types only (int, float, blittable structs).
        // Do NOT use for std::string - strings are reference types and memcpy
        // would store stale GC pointers, bypassing the write barrier.
        template <typename T>
        OhnoArray(const MonoManager& mm, const std::vector<T>& vec, ::MonoClass* klass);

        // Wrap an existing ::MonoArray* received from C#.
        // Derives the element class via mono_class_get_element_class.
        // Use GetRawClass() + mono_class_get_name() for runtime type dispatch.
        explicit OhnoArray(const MonoManager& mm, ::MonoArray* existing);

        // Read a value-type element at [index] via memcpy.
        // std::string specialization routes through OhnoString::MonoToString.
        template <typename T>
        T Get(uint32_t index) const;

        // Write a value-type element at [index] via memcpy.
        // std::string specialization converts via OhnoString::StringToMono
        // and stores with mono_array_set to fire the GC write barrier correctly.
        template <typename T>
        void Set(uint32_t index, const T& value);

        // Number of elements in the array.
        uint32_t Size() const;

        // Raw pointer for passing to C# via void* args[].
        // Pass the ::MonoArray* directly - do NOT take its address.
        // Arrays are reference types; unlike value types, they must not be double-indirected.
        ::MonoArray* GetInternal() const;

        // Element class for runtime type dispatch via mono_class_get_name().
        ::MonoClass* GetRawClass() const;

    private:
        static uint8_t* GetArrayAddr(::MonoArray* arr, int size, uintptr_t index);

    private:
        ::MonoArray* mArray    { nullptr };
        ::MonoClass* mRawClass { nullptr };

        const MonoManager& mMonoManagerRef;
    };
}

#include "MonoCore/OhnoArray.hpp"
