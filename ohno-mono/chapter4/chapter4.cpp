#include <iostream>
#include <filesystem>
#include <vector>

#include <MonoCore/MonoManager.h>

const std::string& GetCsProj()
{
    static const std::string csProj { (std::filesystem::current_path().parent_path() / "sample-csharp" / "sample.csproj").string() };
    return csProj;
}

int main()
{
    std::cout << "=== Chapter 4: Script Component Pattern ===\n\n";

    ohno::MonoManager& mm = ohno::MonoManager::Get();
    mm.Init();
    mm.CompileAssembly(GetCsProj());
    mm.LoadAssembly();

    const ohno::OhnoClass* baseClass = mm.GetClass("SimpleBehaviour");

    // GetInheritedClass scans all loaded assemblies via Mono reflection and returns every
    // class that directly or indirectly inherits from the given base. The base class itself
    // is excluded from the results - only concrete subclasses are returned.
    // This is how a real engine discovers user scripts at startup without any manual registration.
    std::vector<const ohno::OhnoClass*> scripts = mm.GetInheritedClass(baseClass);

    // --- Discovery ---
    std::cout << "--- Discovery ---\n";
    std::cout << "C++: Scanning for SimpleBehaviour subclasses...\n";
    std::cout << "C++: Found " << scripts.size() << " script(s):\n";
    for (const ohno::OhnoClass* script : scripts)
    {
        std::cout << "C++:   - " << script->GetClassName() << "\n";
    }
    std::cout << "\n";

    // --- Instantiation ---
    // Each script gets one instance. The instances vector holds raw ::MonoObject* pointers.
    // In a real engine these would be protected with GC handles (mono_gchandle_new) so the GC
    // cannot collect or relocate them between frames.
    std::vector<::MonoObject*> instances;
    for (const ohno::OhnoClass* script : scripts)
    {
        instances.push_back(script->CreateInstance());
    }

    // --- Awake ---
    // InvokeVirtualMethod is used here instead of InvokeMethod because we only hold a pointer
    // to the base class (SimpleBehaviour). The instances are concrete subclasses, and their
    // overrides live in the vtable. InvokeVirtualMethod walks the vtable to find the correct
    // override - without it, Mono would call SimpleBehaviour::Awake (the empty base version)
    // on every instance regardless of its actual type.
    std::cout << "--- Awake ---\n";
    for (::MonoObject* inst : instances)
    {
        baseClass->InvokeVirtualMethod("Awake", inst);
    }
    std::cout << "\n";

    // --- Start ---
    std::cout << "--- Start ---\n";
    for (::MonoObject* inst : instances)
    {
        baseClass->InvokeVirtualMethod("Start", inst);
    }
    std::cout << "\n";

    // --- Update Loop (3 ticks) ---
    std::cout << "--- Update Loop (3 ticks) ---\n";
    for (int tick = 0; tick < 3; ++tick)
    {
        for (::MonoObject* inst : instances)
        {
            baseClass->InvokeVirtualMethod("Update", inst);
        }
    }
}
