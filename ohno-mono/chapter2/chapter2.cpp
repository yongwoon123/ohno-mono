#include <iostream>
#include <filesystem>

#include <MonoCore/MonoManager.h>

const std::string& GetCsProj()
{
    static const std::string csProj{ (std::filesystem::current_path().parent_path() / "sample-csharp" / "sample.csproj").string() };
    return csProj;
}

void Callback()
{
    std::cout << "C++: InternalCall reached from C#\n";
}

// Grouping internal call registration into a dedicated function is the recommended pattern.
// In a real engine, each system has its own RegisterInternalCalls() that is called both at
// startup and after every Reload. This keeps registration logic close to the C++ functions
// it registers, rather than scattered through main().
void AddCallbackForScriptA()
{
    ohno::MonoManager& mm = ohno::MonoManager::Get();
    const ohno::OhnoClass* scriptClass = mm.GetClass("ScriptA");
    scriptClass->AddInternalCall("Internal_Callback", Callback);
}

int main()
{
    std::cout << "=== Chapter 2: Hot Reload ===\n\n";

    ohno::MonoManager& mm = ohno::MonoManager::Get();
    mm.Init();
    mm.CompileAssembly(GetCsProj());
    mm.LoadAssembly();
    std::cout << "[Load] Assembly loaded\n\n";

    // Internal calls must be re-registered every time a new AppDomain is created (including Reload).
    // Mono does not carry registrations across domain boundaries.
    AddCallbackForScriptA();

    const ohno::OhnoClass* scriptClass = mm.GetClass("ScriptA");
    ::MonoObject* objPtr = scriptClass->CreateInstance();

    std::cout << "--- Before Reload ---\n";
    scriptClass->InvokeMethod("Hello", objPtr);
    std::cout << "\n";

    std::cout << "Modify sample-csharp and press Enter to reload...\n";
    std::cin.get();

    // Reload destroys and recreates the script AppDomain.
    // After this call, every ::MonoObject* and OhnoClass* obtained before reload is a dangling pointer.
    // Do not use objPtr or scriptClass again until you re-fetch them below.
    mm.Reload(GetCsProj());
    std::cout << "\n";

    // Re-register internal calls for the new domain.
    // Reload() handles unload/compile/load - re-registration is the caller's responsibility
    // because the registration function lives in chapter2.cpp, not in MonoManager.
    std::cout << "[Reload] Re-registering internal calls...\n";
    AddCallbackForScriptA();
    std::cout << "[Reload] Complete\n";

    // Re-fetch the class pointer from the new domain - the old one is gone.
    // We also create a brand-new instance, so any field values set before reload are lost.
    // This demo accepts that: we just want to show the new code runs.
    //
    // In a real engine, you would preserve field state across reload like this:
    //   1. Before Reload(): read all public fields from each live object and store them
    //      (e.g. serialize to a temporary JSON/binary buffer keyed by class name).
    //   2. After Reload(): recreate instances, then write the saved values back in.
    // That is not a Mono concern - Mono gives you mono_field_get_value / mono_field_set_value
    // to do the reads and writes. The serialization design belongs in your engine's scene system.
    scriptClass = mm.GetClass("ScriptA");
    objPtr = scriptClass->CreateInstance();

    std::cout << "--- After Reload ---\n";
    scriptClass->InvokeMethod("Hello", objPtr);
    std::cout << "\n";

    std::cout << "--- Internal Call still works after reload ---\n";
    scriptClass->InvokeMethod("TriggerCallback", objPtr);
}
