#pragma once

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
	class MonoManager;
	class OhnoAssembly;
	class OhnoClass;
	class OhnoMethod;
	class OhnoClassField;

	enum class MonoMemberAccess
	{
		Private,
		Internal,
		Protected,
		ProtectedInternal,
		Public
	};

}