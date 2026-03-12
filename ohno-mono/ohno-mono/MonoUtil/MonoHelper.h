#pragma once

#include <string>

#include "MonoCore/MonoPrereq.h"

namespace ohno
{
    // Utility functions for handling managed exceptions thrown by C# code.
    class MonoHelper
    {
    public:
        // If exception is non-null, prints its message and stack trace to stdout.
        // Does not abort or rethrow. Call after every mono_runtime_invoke that could throw.
        static void ThrowIfException(::MonoObject* exception);
    };
}
