#pragma once
#include <string>

#include "MonoCore/MonoPrereq.h"

namespace ohno
{
	class MonoHelper
	{
	public:
		static std::string MonoToString(MonoString* monoStr);
		static void ThrowIfException(::MonoObject* exception);
	};
}