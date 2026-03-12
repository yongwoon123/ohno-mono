#include <iostream>

#include "Monoutil/MonoHelper.h"
#include "MonoCore/OhnoString.h"

namespace ohno
{
    void MonoHelper::ThrowIfException(::MonoObject* exception)
    {
        if (exception != nullptr)
        {
            ::MonoClass* exceptionClass = mono_object_get_class(exception);
            ::MonoProperty* exceptionMsgProp = mono_class_get_property_from_name(exceptionClass, "Message");
            ::MonoMethod* exceptionMsgGetter = mono_property_get_get_method(exceptionMsgProp);
            ::MonoString* exceptionMsg = (::MonoString*)mono_runtime_invoke(exceptionMsgGetter,
                                                                            exception,
                                                                            nullptr,
                                                                            nullptr);

            ::MonoProperty* exceptionStackProp = mono_class_get_property_from_name(exceptionClass, "StackTrace");
            ::MonoMethod* exceptionStackGetter = mono_property_get_get_method(exceptionStackProp);
            ::MonoString* exceptionStackTrace = (::MonoString*)mono_runtime_invoke(exceptionStackGetter,
                                                                                   exception,
                                                                                   nullptr,
                                                                                   nullptr);

            std::string msg = "Managed exception: " + OhnoString::MonoToString(exceptionMsg) + "\n" + OhnoString::MonoToString(exceptionStackTrace);

            std::cout << msg << "\n";
        }
    }
}
