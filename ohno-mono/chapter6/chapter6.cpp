#include <iostream>
#include <filesystem>

#include <MonoCore/MonoManager.h>
#include <MonoCore/OhnoString.h>

// Strings don't pass through the C++/C# boundary automatically. Mono has its own
// MonoString* type for managed strings. To move strings across the boundary you need:
//   - mono_string_to_utf8       : MonoString* -> char* (C# to C++)
//   - mono_string_new_utf16     : std::wstring -> MonoString* (C++ to C#)
//
// OhnoString wraps both directions:
//   - OhnoString::MonoToString  : MonoString* -> std::string
//   - OhnoString::StringToMono  : std::string -> MonoString*

// --- Internal call function signatures ---
// When C# declares a method as 'extern string', C++ must return MonoString*.
// When C# declares a parameter as 'string', C++ receives MonoString*.
// Never use std::string or const char* directly in internal call signatures.

// C++ to C#: C# calls this and receives the result as a managed string.
::MonoString* Internal_GetName()
{
    ohno::MonoManager& mm = ohno::MonoManager::Get();
    // StringToMono converts std::string -> wstring -> MonoString* via mono_string_new_utf16.
    // The result is a heap-allocated managed string owned by the Mono GC.
    return ohno::OhnoString::StringToMono(mm, "ohno-entity");
}

// C# to C++: C# passes a string; C++ receives it as MonoString*.
void Internal_ReceiveString(::MonoString* msg)
{
    // MonoToString calls mono_string_to_utf8, copies the result into std::string,
    // then frees the Mono-allocated buffer with mono_free.
    std::string str = ohno::OhnoString::MonoToString(msg);
    std::cout << "C++: Received string = \"" << str << "\"\n";
}

// Round-trip - step 1: C# calls this to get the original name.
::MonoString* Internal_GetOriginalName()
{
    ohno::MonoManager& mm = ohno::MonoManager::Get();
    return ohno::OhnoString::StringToMono(mm, "entity_001");
}

// Round-trip - step 2: C# sends back the modified name.
void Internal_ReceiveModifiedName(::MonoString* name)
{
    std::string str = ohno::OhnoString::MonoToString(name);
    std::cout << "C++: Received modified name = \"" << str << "\"\n";
}

const std::string& GetCsProj()
{
    static const std::string csProj{ (std::filesystem::current_path().parent_path() / "sample-csharp" / "sample.csproj").string() };
    return csProj;
}

int main()
{
    std::cout << "=== Chapter 6: String Marshalling ===\n\n";

    ohno::MonoManager& mm = ohno::MonoManager::Get();
    mm.Init();
    mm.CompileAssembly(GetCsProj());
    mm.LoadAssembly();

    const ohno::OhnoClass* bridgeClass = mm.GetClass("StringBridge");

    // Register all internal calls before any C# code runs.
    // The names must match the extern method names in the C# class exactly.
    bridgeClass->AddInternalCall("Internal_GetName", Internal_GetName);
    bridgeClass->AddInternalCall("Internal_ReceiveString", Internal_ReceiveString);
    bridgeClass->AddInternalCall("Internal_GetOriginalName", Internal_GetOriginalName);
    bridgeClass->AddInternalCall("Internal_ReceiveModifiedName", Internal_ReceiveModifiedName);

    ::MonoObject* bridgeObj = bridgeClass->CreateInstance();

    // --- C++ to C#: internal call returns string ---
    // C++ registers the function; C# calls it and receives a MonoString* which Mono
    // automatically wraps as a managed System.String. The C# code sees a normal string.
    std::cout << "--- C++ to C#: internal call returns string ---\n";
    std::cout << "C++: Registering GetName(), will return \"ohno-entity\"\n";
    bridgeClass->InvokeMethod("PrintName", bridgeObj);
    std::cout << "\n";

    // --- C# to C++: internal call receives string ---
    // C# constructs a string and passes it to an internal call. Mono marshals the managed
    // System.String as MonoString* on the C++ side. MonoToString converts it to std::string.
    std::cout << "--- C# to C++: internal call receives string ---\n";
    bridgeClass->InvokeMethod("SendString", bridgeObj);
    std::cout << "\n";

    // --- Round-trip ---
    // C# fetches a string from C++ via internal call, appends a suffix, and passes it back.
    // This tests both directions working together in a single flow.
    // In a real engine this pattern appears often: C# reads a resource name, modifies it
    // (e.g. appends a variant tag), and passes the result back to C++ for a lookup.
    std::cout << "--- Round-trip ---\n";
    std::cout << "C++: Original name = \"entity_001\"\n";
    bridgeClass->InvokeMethod("RoundTrip", bridgeObj);
}
