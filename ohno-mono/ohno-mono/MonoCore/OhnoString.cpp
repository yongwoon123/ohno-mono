#include "MonoCore/OhnoString.h"
#include "MonoCore/MonoManager.h"

namespace ohno
{
    MonoString* WStringToMono(const MonoManager& mm, const std::wstring& wstr)
    {
        return mono_string_new_utf16(mm.CurrentDomain(),
                                     static_cast<const mono_unichar2*>(wstr.c_str()),
                                     static_cast<int32_t>(wstr.length()));
    }

    ::MonoString* OhnoString::StringToMono(const MonoManager& mm, const std::string& str)
    {
        return WStringToMono(mm, std::wstring{ str.begin(), str.end() });
    }

    std::string OhnoString::MonoToString(::MonoString* monoStr)
    {
        if (monoStr == nullptr)
        {
            return "";
        }

        // mono_string_to_utf8 allocates a new char* buffer - it must be freed with mono_free.
        // We copy it into std::string first, then free the Mono-owned buffer.
        // Returning the char* directly would construct the std::string correctly 
        // (it copies the bytes), but the original buffer would never be freed.
        char* utf8 = mono_string_to_utf8(monoStr);
        std::string result(utf8);
        mono_free(utf8);
        return result;
    }
}
