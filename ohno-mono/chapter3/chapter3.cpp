#include <iostream>
#include <filesystem>

#include <MonoCore/MonoManager.h>

const std::string& GetCsProj()
{
    static const std::string csProj{ (std::filesystem::current_path().parent_path() / "sample-csharp" / "sample.csproj").string() };
    return csProj;
}

int main()
{
    std::cout << "=== Chapter 3: GC Handles ===\n\n";

    ohno::MonoManager& mm = ohno::MonoManager::Get();
    mm.Init();
    mm.CompileAssembly(GetCsProj());
    mm.LoadAssembly();

    const ohno::OhnoClass* gcTestClass = mm.GetClass("GCTestObject");

    // --- Without GC Handle (danger demo) ---
    // The C# GC can move or collect objects at any time. If C++ holds a raw ::MonoObject*
    // and the GC runs, that pointer may now point at garbage memory or a completely different object.
    // This demo forces a collection to illustrate the risk. It may not visibly crash
    // (the GC is non-deterministic) but the pointer is no longer trustworthy.
    std::cout << "--- Without GC Handle (danger demo) ---\n";
    ::MonoObject* rawPtr = gcTestClass->CreateInstance();
    std::cout << "C++: Created instance, raw pointer stored\n";
    std::cout << "C++: Forcing GC collect...\n";
    mono_gc_collect(mono_gc_max_generation());
    // rawPtr is now potentially stale - using it here is undefined behaviour.
    std::cout << "C++: Attempting to use raw pointer after GC collect - result is undefined\n";
    std::cout << "\n";

    // --- With GC Handle (safe) ---
    // mono_gchandle_new() registers the object with Mono so the GC knows C++ is holding a reference.
    // The second argument is the "pinned" flag:
    //   false = rooted:  GC won't collect the object, but may still move it in memory.
    //   true  = pinned:  GC won't collect or move it. Use this only when passing the address
    //                    to unmanaged memory layouts. Heavier - avoid unless you need it.
    // For normal engine use (holding a script instance across frames), false is correct.
    //
    // Important: the handle does NOT freeze the pointer value. The GC may move the object,
    // so the original objPtr address can go stale even with a handle held.
    // Always call mono_gchandle_get_target() immediately before use to get the current address.
    std::cout << "--- With GC Handle (safe) ---\n";
    ::MonoObject* objPtr = gcTestClass->CreateInstance();
    uint32_t handle = mono_gchandle_new(objPtr, false);
    std::cout << "C++: Created instance, handle acquired\n";
    std::cout << "C++: Forcing GC collect (x3)...\n";
    mono_gc_collect(mono_gc_max_generation());
    mono_gc_collect(mono_gc_max_generation());
    mono_gc_collect(mono_gc_max_generation());
    std::cout << "C++: Resolving handle after GC collect...\n";
    // Resolve the handle to get the current (possibly relocated) address before every use.
    ::MonoObject* livePtr = mono_gchandle_get_target(handle);
    std::cout << "C++: Object still alive, invoking method:\n";
    gcTestClass->InvokeMethod("Ping", livePtr);
    std::cout << "\n";

    // --- Freeing Handle ---
    // Once you're done with the object, free the handle so the GC can collect it normally.
    // After mono_gchandle_free(), treat the handle value as invalid - do not use it again.
    // In a real engine, GC handles are the right tool for holding script instances alive
    // across frames. The pattern of acquire -> resolve -> free applies wherever C++ needs
    // to store a reference to a managed object beyond a single function call.
    std::cout << "--- Freeing Handle ---\n";
    mono_gchandle_free(handle);
    std::cout << "C++: Handle freed, object is now collectable\n";
    std::cout << "C++: Forcing GC collect...\n";
    mono_gc_collect(mono_gc_max_generation());
    std::cout << "C++: Done - object released safely\n";
}
