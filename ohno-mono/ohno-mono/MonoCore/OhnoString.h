#pragma once

#include <string>

#include "MonoCore/MonoPrereq.h"

namespace ohno
{
    // String conversion utilities for crossing the C++/C# boundary.
    class OhnoString
    {
    public:
        // Convert a std::string to a Mono managed string (::MonoString*).
        // The result is owned by the Mono GC - do not free it manually.
        static ::MonoString* StringToMono(const MonoManager& mm, const std::string& str);

        // Convert a Mono managed string to std::string.
        // Handles the mono_string_to_utf8 / mono_free lifetime correctly internally.
        static std::string MonoToString(::MonoString* monoStr);
    };
}
