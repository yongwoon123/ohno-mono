# ohno-mono

A step-by-step tutorial for embedding [Mono](https://www.mono-project.com/) into a custom C++ engine so that C# can be used as a scripting language.

Mono is the open-source runtime that powers Unity's scripting layer. Embedding it means your C++ engine can compile, load, and call C# code at runtime - and C# code can call back into C++. This tutorial walks through how that works from first principles, one concept per chapter.

The wrapper classes (`MonoManager`, `OhnoAssembly`, `OhnoClass`, etc.) follow RAII principles: they own their Mono resources, manage lifetimes, and expose a clean C++ interface so you rarely touch the raw Mono C API directly.

---

## Who this is for

* C++ developers building or studying custom game engines who want to add a C# scripting layer
* Students who have seen RAII and basic templates in C++ and want to see how a real embedding API works
* Anyone curious about how Unity-style scripting actually works under the hood

You should be comfortable with C++ (classes, references, pointers, basic templates) and have a passing familiarity with C#. You do not need prior Mono or game engine experience.

---

## How to follow along

Each chapter is a self-contained program. The recommended approach:

1. Build and run the chapter to see what it does. The `expected-output/` folder has a `.txt` file per chapter showing exactly what the output should look like.
2. Read `chapterN.cpp` - it is the primary teaching file and is commented throughout
3. Look at `sample-csharp/SampleClass.cs` to see the C# side
4. Move to the next chapter
 
Start with Chapter 1 and work forward. Each chapter's comments are self-contained — you don't need to have read earlier chapters to understand them. That said, the concepts do build progressively, so reading in order is still recommended.

---

## Chapters

| # | Topic |
| --- | --- |
| 1 | Initialize the Mono runtime, load a C# assembly, read and write fields from C++, invoke methods, and register your first internal call (C# calling a C++ function) |
| 2 | Hot reload - tear down the script domain and reload a recompiled assembly at runtime without restarting the process, and re-register internal calls for the new domain |
| 3 | GC handles - tell the Mono garbage collector to keep a C# object alive so C++ can safely hold a reference to it across frames |
| 4 | Script component pattern - use virtual dispatch (`mono_object_get_virtual_method`) so C++ can call overridden C# methods without knowing the concrete type, the same way Unity calls `Update()` |
| 5 | C# properties backed by internal calls - C# getters and setters that call into C++ to read or write engine state |
| 6 | String marshalling - move strings across the C++/C# boundary in both directions using `MonoString*` and the `OhnoString` utilities |
| 7 | `OhnoArray` - allocate a managed array in C++, fill it with values, and pass it to C# |
| 8 | `OhnoArray` - receive a managed array from C# return values or directly from C# class fields using `OhnoClassField::GetMonoObject` and `GetIsArray`, and dispatch on the element type at runtime |

---

## Prerequisites

* Visual Studio 2022 - the project files use the `v145` platform toolset. If you are on VS2019 or earlier, you can retarget the projects (right-click solution -> Retarget) to your installed toolset.
* No separate Mono installation required - the runtime is bundled under `ohno-mono/External/mono/`

---

## Building

1. Open `ohno-mono/ohno-mono.sln` in Visual Studio 2022
2. Build the shared library project before any chapter project - the chapters depend on it. Figuring out project dependencies in Solution Explorer is a useful exercise.
3. Set any chapter project as the startup project and run it

The chapter executable compiles the C# sample automatically at startup using the bundled MSBuild. You do not need to install anything extra.

### MSBuild note

The stock Mono distribution is missing some Roslyn components. The bundled copy in `External/mono/` already includes them, so no extra setup is needed. The compiler path is set in `MonoManager::GetFullCompilerPath()` in `MonoCore/MonoManager.cpp` if you ever need to point it elsewhere.

`sample.csproj` also sets `<UseSharedCompilation>false</UseSharedCompilation>`. Without this, MSBuild tries to reuse a running `VBCSCompiler` daemon between builds, which can cause the hot reload compilation in Chapter 2 to stall. Disabling it forces a fresh compiler process each time.

---

## Project structure

The outer `ohno-mono/` folder is the git root. Inside it, `ohno-mono/ohno-mono/` is the Visual Studio solution root - that extra level of nesting is just how VS organizes things.

```
ohno-mono/                      <- git root
  ohno-mono/                    <- Visual Studio solution root
    External/mono/              # Bundled Mono runtime (headers, libs, DLLs)
    chapterN/
      chapterN.cpp              # The teaching file for chapter N - start here
      MonoDll/                  # Compiled C# DLL output (written at runtime)
    ohno-mono/                  # Shared C++ wrapper library
      MonoCore/                 # MonoManager, OhnoAssembly, OhnoClass,
      |                         #   OhnoMethod, OhnoClassField, OhnoArray,
      |                         #   OhnoString (string conversion utilities)
      MonoUtil/                 # MonoHelper (exception logging)
    sample-csharp/
      SampleClass.cs            # C# source used by all chapters
      sample.csproj             # C# project file
```

---

## Design notes

**RAII ownership** - `MonoManager::Init()` starts the Mono JIT and `MonoManager::~MonoManager()` calls `mono_jit_cleanup`. The wrapper classes do not do manual memory management beyond what Mono requires (GC handles, `mono_free` for `mono_string_to_utf8` output). Objects allocated by Mono are owned by the Mono GC.

**Singleton** - `MonoManager::Get()` returns the single instance. `OhnoArray` and `OhnoClassField`, take an explicit `const MonoManager&` reference rather than calling the singleton internally - this makes their domain dependency visible at the call site and avoids hidden coupling. For a larger engine, a service locator or dependency injection would be a better fit - but for a tutorial, the singleton keeps the focus on Mono rather than engine architecture.

**`::` prefix convention** - raw Mono C API handles (`::MonoObject*`, `::MonoClass*`, etc.) always carry the global-namespace qualifier. This makes it immediately obvious in any file whether you are looking at a raw Mono handle or an ohno wrapper type.

**AppDomain per reload** - hot reload (Chapter 2) creates a fresh `AppDomain`, loads the recompiled assembly into it, and tears down the old one. Every `::MonoObject*` and `OhnoClass*` obtained before a reload is a dangling pointer after it returns. Re-fetch everything from the new domain.

**Internal calls** - C++ functions registered with `mono_add_internal_call` must be registered before any C# code calls them, and must be re-registered after every hot reload (the new domain has no knowledge of the previous registrations).

---

## What this does not cover

The tutorial stops where Mono API concepts end and engine-specific design decisions begin. Things deliberately left out:

* A full script component system - Chapter 4 covers virtual dispatch but not entity management or per-frame dispatch
* Array resize and copy (`mono_array_new` + element copy for live scene resize)
* Preserving field state across hot reload (serialize before unload, restore after load)
* Exhaustive `MonoTypeEnum` dispatch
* Exception propagation beyond logging (re-throwing into C++ or surfacing to an editor UI)

Two natural things to build from here:

* **MonoObject wrapper** - a thin RAII type that owns a GC handle, resolves it via `mono_gchandle_get_target` on demand, and frees it in the destructor — optionally invoking a C# `OnDestroy` before it does. Once you have this, you can differentiate between generic object identifiers and specialised component classes, like the ones in Chapter 4.
* **Field inspector** - iterate a class's fields via Mono reflection, map `MonoTypeEnum` values to an enum of your own, read each value with `mono_field_get_value`, and pass the result to whatever UI layer you are using.

---

## License

The ohno-mono wrapper code is MIT licensed. See [LICENSE](LICENSE).

The bundled Mono runtime (`External/mono/`) is licensed separately under the MIT X11 and LGPL licenses.
See the [Mono project](https://www.mono-project.com/docs/about-mono/licensing/) for details.
