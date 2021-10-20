#pragma once

#include <vector>
#include <string>

#include "MonoCore/MonoPrereq.h"

namespace ohno
{
	class MonoMethod
	{
	public:
		MonoMethod(::MonoMethod* rawMethod);
		~MonoMethod();

		::MonoObject* Invoke(::MonoObject* instance, void** params = nullptr) const;

		const char* GetMethodName() const;
		size_t      GetNumParam() const;

		friend std::ostream& operator<<(std::ostream& cout, const MonoMethod& rhs);

	private:
		::MonoMethod* mMethod{ nullptr };
		const char* mName{ nullptr };

		std::vector<::MonoClass*> mParams{};
		size_t                    mNumParam{ 0 };

		::MonoClass* mRetType{ nullptr };
		std::string  mSig{};
	};

}