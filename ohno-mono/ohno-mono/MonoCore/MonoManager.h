#pragma once

#include <memory>
#include <filesystem>
#include <functional>
#include <unordered_map>

#include "MonoCore/MonoPrereq.h"
#include "MonoCore/OhnoAssembly.h"

namespace ohno
{
    // MonoManager is the top-level entry point for the Mono scripting system.
    // It owns the Mono runtime, manages AppDomain lifetime, compiles C# projects,
    // and provides access to classes across all loaded assemblies.
    //
    // Usage order: Init() -> CompileAssembly() -> LoadAssembly() -> GetClass() / CreateInstance()
    class MonoManager
    {
    public:

        MonoManager() = default;

        // Unloads all assemblies and shuts down the Mono JIT runtime.
        ~MonoManager();

        // Returns the single shared MonoManager instance.
        static MonoManager& Get();

        // Initialize the Mono runtime. Call once at program startup before anything else.
        // Sets the Mono lib/etc directories, parses the default config, and creates the root domain.
        void Init();

        // Invoke msbuild to compile a C# .csproj into a .dll under the MonoDll/ output directory.
        // Must be called before LoadAssembly() so the .dll exists on disk.
        void CompileAssembly(const std::string& csprojPath);

        // Scan MonoDll/ for all .dll files and load each as an OhnoAssembly.
        // Creates the script AppDomain on the first call; subsequent calls (after Reload) create a new one.
        void LoadAssembly();

        // Load a single assembly by path. Called internally by the directory-scanning overload above.
        void LoadAssembly(const std::filesystem::path& assemblyPath);

        // Unload the script AppDomain, recompile from source, and reload.
        // All ::MonoObject* and OhnoClass* pointers obtained before this call become dangling.
        // Re-register all internal calls and re-fetch class/instance pointers after calling this.
        void Reload(const std::string& csprojPath);

        // Look up a class by name and create an instance in one call.
        // Equivalent to GetClass(name)->CreateInstance(args, num).
        // Returns nullptr if the class is not found in any loaded assembly.
        ::MonoObject* CreateInstance(const char* monoClassName, void** args = nullptr, size_t num = 0);

        // Search all loaded assemblies for a class by name.
        // Returns a pointer owned by this MonoManager - do not delete it.
        // Returns nullptr if the class is not found.
        const OhnoClass* GetClass(const char* monoClassName);

        // Returns the current script AppDomain.
        [[nodiscard]] ::MonoDomain* ScriptDomain() const;

        // Alias for ScriptDomain().
        [[nodiscard]] ::MonoDomain* CurrentDomain() const;

        // Return all classes in the loaded assemblies that inherit from myClass.
        // The base class itself is excluded from results.
        std::vector<const OhnoClass*> GetInheritedClass(const OhnoClass* myClass) const;

    private:

        // Finalize and unload the script AppDomain, then clear all assembly wrappers.
        void UnloadAllAssembly();

    private:

        // All loaded assemblies, keyed by filename stem (e.g. "Assembly-CSharp").
        std::unordered_map<std::string, std::unique_ptr<OhnoAssembly>> mAssemblies {};

        // The root domain is created by mono_jit_init_version and lives for the process lifetime.
        // It cannot be unloaded - only cleaned up via mono_jit_cleanup in the destructor.
        ::MonoDomain* mRootDomain {};

        // The script domain hosts the game assembly. Created on the first LoadAssembly()
        // call and destroyed by UnloadAllAssembly(). Reload() creates a new one each cycle.
        ::MonoDomain* mScriptDomain {};
    };
}
