#pragma once

#include <string>
#include <memory>
#include <map>
#include <unordered_map>

#include "MonoCore/MonoPrereq.h"
#include "MonoCore/OhnoMethod.h"
#include "MonoCore/OhnoClassField.h"

namespace ohno
{
    // OhnoClass wraps a single C# class (::MonoClass*) and owns its fields and methods.
    // Obtain via MonoManager::GetClass() - do not construct directly.
    //
    // Fields are loaded eagerly at construction.
    // Methods are loaded via LoadAllMethods(), called from OhnoAssembly::LoadDependencies()
    // after all assemblies are present, so method signature types resolve correctly.
    class OhnoClass
    {
    public:
        OhnoClass(const MonoManager& mm, ::MonoClass* rawClass);
        ~OhnoClass();

        // Allocate and construct a new C# instance of this class.
        // Pass args=nullptr for the default constructor, or a void* array for a parameterised one.
        // Each element of args must point to the argument value (not the value itself).
        ::MonoObject* CreateInstance(void* args[] = nullptr, size_t num = 0) const;

        // Invoke a method by name and parameter count on the given instance.
        // Returns the return value as ::MonoObject*, or nullptr for void methods or if not found.
        // Searches the class's own methods only - does not walk the inheritance chain.
        ::MonoObject* InvokeMethod(const char* name,
                                   ::MonoObject* instance = nullptr,
                                   void** params = nullptr,
                                   size_t numParams = 0) const;

        // Invoke a virtual method by dispatching through the vtable of the given instance.
        // Use when holding a base class pointer but the instance may be a concrete subclass.
        // Only supports zero-parameter virtual methods.
        ::MonoObject* InvokeVirtualMethod(const char* name, ::MonoObject* instance) const;

        // Register a C++ function as a Mono internal call.
        // The registered name becomes "ClassName::name" and must match the C# extern declaration.
        // Must be called before any C# method that uses the internal call is invoked.
        // Must be re-registered after every domain reload.
        void AddInternalCall(const std::string& name, const void* method) const;

        // Look up a field by name. Returns nullptr if not found.
        // Returned pointer is owned by this OhnoClass - do not delete it.
        const OhnoClassField* GetField(const char* fieldName) const;

        // Look up a method by name and parameter count. Returns nullptr if not found.
        const OhnoMethod* GetMethod(const char* methodName, size_t numParam = 0) const;

        // Returns true if this class is a subclass of monoClass (including transitive inheritance).
        [[nodiscard]] bool isSubClassOf(::MonoClass* monoClass) const;

        // Returns the C# class name as a null-terminated string owned by Mono.
        [[nodiscard]] const char* GetClassName() const;

        // Returns the raw ::MonoClass* for use with Mono API calls (e.g. OhnoArray element type).
        [[nodiscard]] ::MonoClass* GetRawClass() const;

    private:
        friend OhnoAssembly;
        friend std::ostream& operator<<(std::ostream& cout, const OhnoClass& rhs);

        void LoadAllMethods();
        void LoadAllFields();
        void LoadAllInheritedFields();

    private:
        ::MonoClass* mClass { nullptr };
        const char*  mName  { nullptr };

        std::vector<std::unique_ptr<OhnoClassField>> mFields {};

        // std::map keeps subclass names in sorted order for deterministic debug output.
        std::map<std::string, std::vector<const OhnoClassField*>> mInheritedFields {};

        std::vector<std::unique_ptr<OhnoMethod>> mMethods {};

        const MonoManager& mMonoManagerRef;
    };
}
