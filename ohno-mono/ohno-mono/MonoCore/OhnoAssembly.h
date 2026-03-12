#pragma once

#include <unordered_map>

#include "MonoCore/MonoPrereq.h"
#include "MonoCore/OhnoClass.h"

namespace ohno
{
    // OhnoAssembly wraps a single loaded Mono assembly (.dll) and owns the OhnoClass
    // objects for every C# class found inside it.
    class OhnoAssembly
    {
    public:

        // Load and parse the assembly at filePath. Reads the .dll into a memory buffer,
        // then hands it to Mono. All C# classes are discovered immediately at construction.
        OhnoAssembly(const MonoManager& mm, const std::string& filePath);

        // Releases the Mono image and clears all class wrappers.
        ~OhnoAssembly();

        // Second-pass initialization called by MonoManager after ALL assemblies are loaded.
        // Loads methods and inherited fields for every class. Must be deferred so
        // cross-assembly type references resolve correctly.
        void LoadDependencies();

        // Look up a class by name. Returns nullptr if not found.
        // Returned pointer is owned by this OhnoAssembly - do not delete it.
        const OhnoClass* GetClass(const std::string_view& ohnoClassName);

        // Return all classes in this assembly that are subclasses of myClass.
        // The base class itself is excluded from results.
        std::vector<const OhnoClass*> GetInheritedClass(const OhnoClass* myClass) const;

    private:
        friend std::ostream& operator<<(std::ostream& cout, const OhnoAssembly& rhs);

        // Scan the assembly's TypeDef metadata table and construct an OhnoClass for each entry.
        void LoadAllClass();

        ::MonoAssembly* mAssembly { nullptr };
        ::MonoImage*    mImage    { nullptr };

        // string_view keys point into Mono-owned memory.
        std::unordered_map<std::string_view, std::unique_ptr<OhnoClass>> mClasses {};

        const MonoManager& mMonoManagerRef;
    };
}
