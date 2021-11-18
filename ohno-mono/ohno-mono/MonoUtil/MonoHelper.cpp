#include <iostream>

#include "Monoutil/MonoHelper.h"

namespace ohno
{
	std::string MonoHelper::MonoToString(MonoString* monoStr)
	{
		if (monoStr == nullptr)
		{
			return "";
		}

		return mono_string_to_utf8(monoStr);
	}

	//MonoString* MonoHelper::WStringToMono(const std::wstring& wstr)
	//{
	//	return mono_string_new_utf16(MonoManager::Get()->CurrentDomain(),
	//								 static_cast<const mono_unichar2*>(wstr.c_str()),
	//								 static_cast<int32_t>(wstr.length()));
	//}

	//MonoString* MonoHelper::StringToMono(const std::string& str)
	//{
	//	return WStringToMono(std::wstring { str.begin(), str.end() });
	//}

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

			std::string msg = "Managed exception: " + MonoToString(exceptionMsg) + "\n" + MonoToString(exceptionStackTrace);

			std::cout << msg << "\n";
		}
	}
}