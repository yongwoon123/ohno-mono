#include "MonoCore/MonoPrereq.h"

namespace ohno
{
	class OhnoClassField
	{
	public:
		OhnoClassField(::MonoClassField* monoClassField);

		void Get(::MonoObject* objInstance, void* outputValue) const;
		void Set(::MonoObject* objInstance, void* inputValue) const;

		const char* GetFieldName() const;
		const char* GetClassName() const;
		const char* GetAccess() const;

		friend std::ostream& operator<<(std::ostream& cout, const OhnoClassField& rhs);

	private:
		::MonoClassField* mField{};
		const char*       mFieldName{};

		::MonoType*       mType{};
		::MonoClass*      mClass{};
		const char*       mClassName{};

		uint32_t mFlags{};
	};
}