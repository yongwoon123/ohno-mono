#include <iostream>
#include <filesystem>

#include <MonoCore/MonoManager.h>

// C++ struct must match the C# struct field layout exactly for Mono to pass it by value.
// Vector3 here has float x, y, z in the same order as the C# struct in SampleClass.cs.
// If the order or types differ, you get garbage values at the boundary.
struct Vector3
{
    float x{ 1.0f }, y{ 2.0f }, z{ 3.0f };
};

const std::string& GetCsProj()
{
    static const std::string csProj{ (std::filesystem::current_path().parent_path() / "sample-csharp" / "sample.csproj").string() };
    return csProj;
}

// This plain C function is what gets registered as an internal call.
// It must not be a class method - Mono calls it like a raw function pointer.
// The name here doesn't need to match the C# declaration, but the registered
// name passed to AddInternalCall() must match exactly.
void Callback()
{
    std::cout << "C++: InternalCall reached from C#\n";
}

int main()
{
    std::cout << "=== Chapter 1: The Basics ===\n\n";

    ohno::MonoManager& mm = ohno::MonoManager::Get();
    mm.Init();
    std::cout << "[Init] Mono runtime initialized\n\n";

    mm.CompileAssembly(GetCsProj());
    mm.LoadAssembly();
    std::cout << "[Load] Assembly loaded: Assembly-CSharp\n\n";

    const ohno::OhnoClass* entityClass = mm.GetClass("Entity");

    // --- Default Constructor ---
    std::cout << "--- Default Constructor ---\n";

    // ::MonoObject* is the raw Mono C API type for a managed object instance.
    // The :: prefix means global namespace - it distinguishes the Mono C API type from
    // any MonoObject wrapper class we might define ourselves in the ohno:: namespace.
    ::MonoObject* objPtr = entityClass->CreateInstance();
    std::cout << "\n";

    // --- Parameterized Constructor ---
    // ctorArgs is a void* array where each element points to the argument value.
    // The types and count must exactly match a constructor signature in the C# class.
    // Mono will select the matching overload - if none matches, it will crash or silently fail.
    //
    // Note: objPtr from the default constructor above is reassigned here.
    // The previous instance is no longer referenced and will be collected by the GC.
    // Use a GC handle (mono_gchandle_new) to keep an instance alive across reassignments.
    std::cout << "--- Parameterized Constructor ---\n";
    int id = 42;
    void* ctorArgs[] = { &id };
    objPtr = entityClass->CreateInstance(ctorArgs, 1);
    std::cout << "\n";

    // --- Field Get ---
    // mono_field_get_value copies the field's raw bytes into the output buffer.
    // For value types (int, float, struct), pass a pointer to a local of the matching type.
    // For reference types, you would receive a ::MonoObject*.
    std::cout << "--- Field Get ---\n";
    const ohno::OhnoClassField* healthField = entityClass->GetField("Health");
    int value = 0;
    healthField->Get(objPtr, &value);
    std::cout << "C++: field 'Health' initial value = " << value << "\n\n";

    // --- Field Set ---
    // mono_field_set_value copies the bytes pointed to by inputValue into the field.
    // Same pointer-to-value convention as Get.
    std::cout << "--- Field Set ---\n";
    int newVal = 50;
    healthField->Set(objPtr, &newVal);
    healthField->Get(objPtr, &value);
    std::cout << "C++: field 'Health' after set = " << value << "\n\n";

    // --- Method (no params) ---
    std::cout << "--- Method (no params) ---\n";
    entityClass->InvokeMethod("SampleVoidMethod", objPtr);
    std::cout << "\n";

    // --- Method (struct param) ---
    // Structs are value types. Pass a pointer to the struct in the args array.
    // Mono reads sizeof(Vector3) bytes from that address, so the C++ layout must match C#.
    std::cout << "--- Method (struct param) ---\n";
    Vector3 v{ 1.0f, 2.0f, 3.0f };
    void* moveArgs[] = { &v };
    entityClass->InvokeMethod("Move", objPtr, moveArgs, 1);
    std::cout << "\n";

    // --- Internal Call ---
    // AddInternalCall must be called BEFORE any C# method that uses it is invoked.
    // Mono resolves internal call bindings at the point of invocation - if the function
    // is not registered yet, the call will fail. In larger engines, all internal calls
    // are typically registered together at startup before any C# code runs.
    std::cout << "--- Internal Call ---\n";
    entityClass->AddInternalCall("Internal_Callback", Callback);
    entityClass->InvokeMethod("TriggerCallback", objPtr);
}
