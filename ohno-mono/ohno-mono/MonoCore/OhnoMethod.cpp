#include <iostream>

#include "MonoCore/OhnoMethod.h"
#include "MonoUtil/MonoHelper.h"

namespace ohno
{
	OhnoMethod::OhnoMethod(::MonoMethod* rawMethod)
		: mMethod { rawMethod }
		, mName   { mono_method_get_name(rawMethod) }
	{
		// Extract return type and parameter types from the method signature.
		// These are used for debug output (operator<<) and parameter count matching.
		MonoMethodSignature* sig = mono_method_signature(mMethod);

		MonoType* monoType = mono_signature_get_return_type(sig);
		mRetType = mono_class_from_mono_type(monoType);

		mNumParam = mono_signature_get_param_count(sig);
		if (mNumParam > 0)
		{
			void* typeIter = nullptr;
			for (size_t i = 0; i < mNumParam; i++)
			{
				monoType = mono_signature_get_params(sig, &typeIter);
				::MonoClass* rawClass = mono_class_from_mono_type(monoType);

				mParams.push_back(rawClass);
			}
		}

		// mono_signature_get_desc produces a human-readable string like "int,string".
		// false = do not include generic argument names.
		mSig = mono_signature_get_desc(sig, false);
	}

	OhnoMethod::~OhnoMethod()
	{
		mParams.clear();
	}

	::MonoObject* OhnoMethod::Invoke(::MonoObject* instance, void** params) const
	{
		::MonoObject* excep  = nullptr;
		::MonoObject* retVal = mono_runtime_invoke(mMethod, instance, params, &excep);

		// If C# threw an exception, print the message and stack trace.
		// ThrowIfException does NOT abort the program - it just logs and continues.
		MonoHelper::ThrowIfException(excep);
		return retVal;
	}

	const char* OhnoMethod::GetMethodName() const
	{
		return mName;
	}

	size_t OhnoMethod::GetNumParam() const
	{
		return mNumParam;
	}

	// Debug output: "ReturnType MethodName(param1,param2,...)"
	std::ostream& operator<<(std::ostream& cout, const OhnoMethod& rhs)
	{
		cout << "|----------" << mono_class_get_name(rhs.mRetType) << " " << rhs.mName << "(" << rhs.mSig << ")";
		return cout;
	}
}
