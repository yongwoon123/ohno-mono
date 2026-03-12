#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include <MonoCore/MonoManager.h>
#include <MonoCore/OhnoArray.h>

// OhnoArray wraps the Mono array API to provide a clean C++ interface for
// constructing managed arrays and passing them to C# via internal calls.
//
// Two construction paths:
//   - OhnoArray(vector, klass)     : value types - single bulk memcpy
//   - OhnoArray(klass, size)       : empty array, fill with Set<T> per element
//                                    required for strings (reference types)
//
// Arrays are reference types in the CLR. When passing to C# via void* args[],
// pass the raw ::MonoArray* directly - do NOT take its address (unlike value
// types such as int or float which need &val).

// C++ struct must match the C# struct field layout exactly for memcpy to be valid.
// Vector3 has three floats in the same order as the C# struct, so it is safe.
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
    std::cout << "=== Chapter 7: Sending Arrays ===\n\n";

    ohno::MonoManager& mm = ohno::MonoManager::Get();
    mm.Init();
    mm.CompileAssembly(GetCsProj());
    mm.LoadAssembly();

    const ohno::OhnoClass* receiverClass = mm.GetClass("ArrayReceiver");
    ::MonoObject* receiverObj = receiverClass->CreateInstance();

    // --- Int array ---
    // OhnoArray(vector, klass) takes the element class and does a single memcpy.
    // mono_get_int32_class() returns the Mono class for System.Int32 (C# int).
    // Value types have identical layout in C++ and C# so memcpy is safe here.
    std::cout << "--- Int array ---\n";
    std::cout << "C++: Sending int[] { 10, 20, 30, 40, 50 }\n";
    std::vector<int> ints = { 10, 20, 30, 40, 50 };
    ohno::OhnoArray intArr(mm, ints, mono_get_int32_class());
    ::MonoArray* rawInts = intArr.GetInternal();
    void* intArgs[] = { rawInts };
    receiverClass->InvokeMethod("ReceiveIntArray", receiverObj, intArgs, 1);
    std::cout << "\n";

    // --- String array ---
    // Strings are reference types: memcpy would store raw GC pointers without
    // firing the write barrier, breaking the garbage collector.
    // Instead, allocate by size and call Set<std::string> per element.
    // Set<std::string> calls OhnoString::StringToMono then mono_array_set,
    // which fires the write barrier correctly. (See OhnoArray.cpp specialization.)
    std::cout << "--- String array ---\n";
    std::cout << "C++: Sending string[] { \"alpha\", \"beta\", \"gamma\" }\n";
    std::vector<std::string> names = { "alpha", "beta", "gamma" };
    ohno::OhnoArray strArr(mm, mono_get_string_class(), (uint32_t)names.size());
    for (uint32_t i = 0; i < (uint32_t)names.size(); ++i)
    {
        strArr.Set<std::string>(i, names[i]);
    }
    ::MonoArray* rawStrs = strArr.GetInternal();
    void* strArgs[] = { rawStrs };
    receiverClass->InvokeMethod("ReceiveStringArray", receiverObj, strArgs, 1);
    std::cout << "\n";

    // --- Vector3 array ---
    // Structs use the same vector constructor as primitives. The C++ struct layout
    // must match the C# struct field order and size exactly.
    // OhnoClass::GetRawClass() returns the raw ::MonoClass* for any wrapper - use
    // it to get the element class for any user-defined struct type.
    //
    // In a real engine, you would look up the Transform, Rigidbody, or other
    // component classes the same way to pass batches of component data to scripts.
    std::cout << "--- Vector3 array ---\n";
    std::cout << "C++: Sending Vector3[] { (1,0,0), (0,1,0), (0,0,1) }\n";
    std::vector<Vector3> vecs = { {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f} };
    const ohno::OhnoClass* vecClass = mm.GetClass("Vector3");
    ohno::OhnoArray vecArr(mm, vecs, vecClass->GetRawClass());
    ::MonoArray* rawVecs = vecArr.GetInternal();
    void* vecArgs[] = { rawVecs };
    receiverClass->InvokeMethod("ReceiveVectorArray", receiverObj, vecArgs, 1);
}
