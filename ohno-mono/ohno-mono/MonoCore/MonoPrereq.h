#pragma once

// MonoPrereq.h - Single include point for all Mono C API headers.
// Every MonoCore and MonoUtil file includes this instead of individual Mono headers.
// Also holds the forward declarations and shared enums for the ohno namespace.

#include <mono/jit/jit.h>
#include <mono/metadata/class.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/tokentype.h>
#include <mono/metadata/attrdefs.h>
#include <mono/utils/mono-logger.h>

namespace ohno
{
    // Forward declarations - avoids circular includes between wrapper classes.
    class MonoManager;
    class OhnoAssembly;
    class OhnoClass;
    class OhnoMethod;
    class OhnoClassField;
    class OhnoString;
    class OhnoArray;

    // Mirrors the Mono field/method access flag values in a readable C++ enum.
    // Used by OhnoClassField to expose the C# access modifier of a field.
    enum class MonoMemberAccess
    {
        Private,
        Internal,
        Protected,
        ProtectedInternal,
        Public
    };
}
