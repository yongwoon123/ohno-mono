#pragma once

#include <string>

#include "MonoCore/MonoPrereq.h"

namespace ohno
{
	class MonoHelper
	{
	public:
		// static MonoString* WStringToMono(const std::wstring& wstring);
		// static MonoString* StringToMono(const std::string& str);

		static std::string MonoToString(MonoString* monoStr);

		static void ThrowIfException(::MonoObject* exception);
	};
}