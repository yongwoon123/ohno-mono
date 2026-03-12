#include <iostream>
#include <filesystem>
#include <string>

#include <MonoCore/MonoManager.h>
#include <MonoCore/OhnoArray.h>
#include <MonoCore/OhnoString.h>

// C++ struct must match the C# Vector3 struct field layout (float x, y, z) exactly.
struct Vector3
{
    float x, y, z;
};

const std::string& GetCsProj()
{
    static const std::string csProj{ (std::filesystem::current_path().parent_path() / "sample-csharp" / "sample.csproj").string() };
    return csProj;
}

int main()
{
    std::cout << "=== Chapter 8: Receiving Arrays ===\n\n";

    ohno::MonoManager& mm = ohno::MonoManager::Get();
    mm.Init();
    mm.CompileAssembly(GetCsProj());
    mm.LoadAssembly();

    const ohno::OhnoClass* senderClass = mm.GetClass("ArraySender");
    ::MonoObject* senderObj = senderClass->CreateInstance();

    // -----------------------------------------------------------------------
    // Receive int array from return value
    // -----------------------------------------------------------------------
    // InvokeMethod returns ::MonoObject*. Arrays are objects in the CLR, so
    // casting to ::MonoArray* is safe when you know the C# return type is an array.
    // mono_array_length and mono_array_get then give you element count and values.
    std::cout << "--- Receive int array from return value ---\n";
    {
        ::MonoObject* result = senderClass->InvokeMethod("GetIntArray", senderObj);
        ::MonoArray* arr = reinterpret_cast<::MonoArray*>(result);
        uint32_t len = (uint32_t)mono_array_length(arr);
        std::cout << "C++: Received int[" << len << "]: ";
        for (uint32_t i = 0; i < len; ++i)
        {
            std::cout << mono_array_get(arr, int, i);
            if (i + 1 < len) std::cout << ", ";
        }
        std::cout << "\n\n";
    }

    // -----------------------------------------------------------------------
    // Receive string array from return value
    // -----------------------------------------------------------------------
    // String elements are ::MonoString* (reference types). mono_array_get with
    // type MonoString* retrieves each element. OhnoString::MonoToString converts
    // to std::string via mono_string_to_utf8 and frees the Mono-allocated buffer.
    std::cout << "--- Receive string array from return value ---\n";
    {
        ::MonoObject* result = senderClass->InvokeMethod("GetStringArray", senderObj);
        ::MonoArray* arr = reinterpret_cast<::MonoArray*>(result);
        uint32_t len = (uint32_t)mono_array_length(arr);
        std::cout << "C++: Received string[" << len << "]: ";
        for (uint32_t i = 0; i < len; ++i)
        {
            ::MonoString* monoStr = mono_array_get(arr, ::MonoString*, i);
            std::string str = ohno::OhnoString::MonoToString(monoStr);
            std::cout << "\"" << str << "\"";
            if (i + 1 < len) std::cout << ", ";
        }
        std::cout << "\n\n";
    }

    // -----------------------------------------------------------------------
    // Receive Vector3 array from return value
    // -----------------------------------------------------------------------
    // Value-type structs are stored inline in the managed array (no boxing).
    // mono_array_get with your matching C++ struct type copies the bytes out directly.
    // The C++ struct layout must match the C# struct field order and sizes exactly.
    std::cout << "--- Receive Vector3 array from return value ---\n";
    {
        ::MonoObject* result = senderClass->InvokeMethod("GetVectorArray", senderObj);
        ::MonoArray* arr = reinterpret_cast<::MonoArray*>(result);
        uint32_t len = (uint32_t)mono_array_length(arr);
        std::cout << "C++: Received Vector3[" << len << "]:\n";
        for (uint32_t i = 0; i < len; ++i)
        {
            Vector3 v = mono_array_get(arr, Vector3, i);
            std::cout << "C++:   [" << i << "] x=" << v.x << " y=" << v.y << " z=" << v.z << "\n";
        }
        std::cout << "\n";
    }

    // -----------------------------------------------------------------------
    // Receive int array from class field
    // -----------------------------------------------------------------------
    // OhnoClassField::GetMonoObject() reads the field as a raw ::MonoObject*.
    // GetIsArray() checks whether that object is a managed array.
    // Together they let you inspect and read array-typed fields without knowing
    // the element type at compile time.
    std::cout << "--- Receive int array from class field ---\n";
    {
        const ohno::OhnoClass* hasArrayClass = mm.GetClass("HasArrayField");
        ::MonoObject* hasArrayObj = hasArrayClass->CreateInstance();

        const ohno::OhnoClassField* field = hasArrayClass->GetField("Numbers");
        ::MonoObject* fieldObj = field->GetMonoObject(hasArrayObj);

        if (field->GetIsArray())
        {
            ::MonoArray* arr = reinterpret_cast<::MonoArray*>(fieldObj);
            uint32_t len = (uint32_t)mono_array_length(arr);
            std::cout << "C++: Field 'Numbers' is an array, length = " << len << "\n";
            std::cout << "C++: Values: ";
            for (uint32_t i = 0; i < len; ++i)
            {
                std::cout << mono_array_get(arr, int, i);
                if (i + 1 < len) std::cout << ", ";
            }
            std::cout << "\n\n";
        }
        else
        {
            std::cout << "C++: Field 'Numbers' is not an array\n\n";
        }
    }

    // -----------------------------------------------------------------------
    // Type dispatch on received array
    // -----------------------------------------------------------------------
    // In a real engine, a generic inspection tool (asset serializer, debugger,
    // remote inspector) may receive arrays whose element type is not known at
    // compile time. mono_class_get_element_class() returns the element class,
    // which you can compare against known primitive classes to branch on type.
    std::cout << "--- Type dispatch on received array ---\n";
    {
        ::MonoObject* result = senderClass->InvokeMethod("GetIntArray", senderObj);
        ::MonoArray* arr = reinterpret_cast<::MonoArray*>(result);

        ::MonoClass* elemClass = mono_class_get_element_class(mono_object_get_class(result));

        if (elemClass == mono_get_int32_class())
        {
            uint32_t len = (uint32_t)mono_array_length(arr);
            std::cout << "C++: Detected int[] - reading " << len << " element(s): ";
            for (uint32_t i = 0; i < len; ++i)
            {
                std::cout << mono_array_get(arr, int, i);
                if (i + 1 < len) std::cout << ", ";
            }
            std::cout << "\n";
        }
        else if (elemClass == mono_get_string_class())
        {
            std::cout << "C++: Detected string[] - would read with MonoToString\n";
        }
        else
        {
            std::cout << "C++: Unknown element type\n";
        }
    }
}
