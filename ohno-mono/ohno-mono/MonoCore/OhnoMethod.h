#pragma once

#include <vector>
#include <string>

#include "MonoCore/MonoPrereq.h"

namespace ohno
{
    // OhnoMethod wraps a single C# method (::MonoMethod*).
    // Stores the method name, parameter types, return type, and signature string.
    // Constructed by OhnoClass::LoadAllMethods() - do not create directly.
    class OhnoMethod
    {
    public:
        // Extract name, parameter types, return type, and signature from rawMethod.
        OhnoMethod(::MonoMethod* rawMethod);
        ~OhnoMethod();

        // Invoke the method on instance with the given parameters.
        // params is a void* array where each element points to the argument value.
        // For static methods, pass nullptr for instance.
        // Exceptions thrown by C# are caught and printed via MonoHelper::ThrowIfException.
        ::MonoObject* Invoke(::MonoObject* instance, void** params = nullptr) const;

        // Returns the method name as reported by Mono (e.g. "Update", ".ctor").
        const char* GetMethodName() const;

        // Returns the number of parameters this method declares.
        // Used by OhnoClass::InvokeMethod to select the correct overload.
        size_t GetNumParam() const;

    private:
        friend std::ostream& operator<<(std::ostream& cout, const OhnoMethod& rhs);

        ::MonoMethod* mMethod { nullptr };
        const char*   mName   { nullptr };

        // Parameter classes, one per declared parameter. Used for debug output.
        std::vector<::MonoClass*> mParams   {};
        size_t                    mNumParam { 0 };

        ::MonoClass* mRetType { nullptr }; // Return type class (for debug output).
        std::string  mSig     {};          // Human-readable signature string from Mono.
    };

}
