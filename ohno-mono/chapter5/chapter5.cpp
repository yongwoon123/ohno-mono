#include <iostream>
#include <iomanip>
#include <filesystem>
#include <cstdint>

#include <MonoCore/MonoManager.h>

// C# Vector3 struct memory layout: float x, y, z (12 bytes).
// Defined here to match exactly so Mono can pass it by value across the boundary.
struct Vector3
{
    float x, y, z;
};

// Entity is just a uint32 ID. C# holds it; C++ uses it to look up engine data.
// This is the core of the property pattern - C# knows nothing about C++ memory,
// it only holds the ID. C++ does the lookup on the other side of each call.
using Entity = uint32_t;

// Fake transform data. In a real engine this lives in a transform component system,
// keyed by entity ID. A single global struct is enough to demonstrate the pattern.
struct TransformData
{
    float x{ 0.f }, y{ 0.f }, z{ 0.f };
    bool isActive{ true };
};

static TransformData gTransform{};

// --- Internal call functions ---
// These are plain C-style functions - not methods, no 'this'.
// Their signatures must exactly match the [MethodImpl(InternalCall)] declarations in C#.
// Entity (uint32) is always the first parameter - C# passes its stored goId back to C++
// so that C++ can look up the correct object.

Vector3 Internal_GetPosition(Entity id)
{
    return { gTransform.x, gTransform.y, gTransform.z };
}

void Internal_SetPosition(Entity id, Vector3 pos)
{
    gTransform.x = pos.x;
    gTransform.y = pos.y;
    gTransform.z = pos.z;
}

bool Internal_GetIsActive(Entity id)
{
    return gTransform.isActive;
}

void Internal_SetIsActive(Entity id, bool val)
{
    gTransform.isActive = val;
}

const std::string& GetCsProj()
{
    static const std::string csProj{ (std::filesystem::current_path().parent_path() / "sample-csharp" / "sample.csproj").string() };
    return csProj;
}

int main()
{
    std::cout << "=== Chapter 5: C# Properties Backed by Internal Calls ===\n\n";

    ohno::MonoManager& mm = ohno::MonoManager::Get();
    mm.Init();
    mm.CompileAssembly(GetCsProj());
    mm.LoadAssembly();

    const ohno::OhnoClass* transformClass = mm.GetClass("MyTransform");

    // --- Registration ---
    // All internal calls for a system are registered together before any C# code runs.
    // In a real engine each system has its own registration function called from a
    // central RegisterAll() at startup, and again after every domain reload.
    std::cout << "--- Registration ---\n";
    transformClass->AddInternalCall("Internal_GetPosition", Internal_GetPosition);
    transformClass->AddInternalCall("Internal_SetPosition", Internal_SetPosition);
    transformClass->AddInternalCall("Internal_GetIsActive", Internal_GetIsActive);
    transformClass->AddInternalCall("Internal_SetIsActive", Internal_SetIsActive);
    std::cout << "C++: Internal calls registered for MyTransform\n\n";

    // Create a MyTransform instance bound to entity 0.
    // The entity ID is the only thing C# holds - all real data lives in C++.
    // C# stores this ID in goId and passes it back on every property access.
    Entity entityId = 0;
    void* ctorArgs[] = { &entityId };
    ::MonoObject* transformObj = transformClass->CreateInstance(ctorArgs, 1);

    // --- Read position (initial) ---
    // The getter calls Internal_GetPosition(goId), which reads from gTransform.
    // gTransform is zero-initialized, so the result is (0, 0, 0).
    std::cout << "--- Read position (initial) ---\n";
    transformClass->InvokeMethod("PrintPosition", transformObj);
    std::cout << "\n";

    // --- Write position ---
    // SetPositionTo logs the incoming value then assigns to the property.
    // The setter calls Internal_SetPosition(goId, value), which updates gTransform.
    // After the C# call returns, C++ can read back from gTransform to confirm the write.
    std::cout << "--- Write position ---\n";
    Vector3 newPos{ 1.f, 2.f, 3.f };
    void* setPosArgs[] = { &newPos };
    transformClass->InvokeMethod("SetPositionTo", transformObj, setPosArgs, 1);
    std::cout << std::fixed << std::setprecision(2)
        << "C++: Transform data confirmed: x=" << gTransform.x
        << ", y=" << gTransform.y
        << ", z=" << gTransform.z << "\n\n";

    // --- Read position (after set) ---
    std::cout << "--- Read position (after set) ---\n";
    transformClass->InvokeMethod("PrintPosition", transformObj);
    std::cout << "\n";

    // --- isActive property ---
    // Same pattern - bool property backed by getter/setter internal calls.
    std::cout << "--- isActive property ---\n";
    transformClass->InvokeMethod("PrintIsActive", transformObj);
    bool falseVal = false;
    void* setActiveArgs[] = { &falseVal };
    transformClass->InvokeMethod("SetIsActiveTo", transformObj, setActiveArgs, 1);
    std::cout << std::boolalpha << "C++: isActive confirmed: " << gTransform.isActive << "\n";
    transformClass->InvokeMethod("PrintIsActive", transformObj);
}
