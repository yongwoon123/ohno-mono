#include <iostream>

#include "MonoCore/MonoClassField.h"

namespace ohno
{
	MonoClassField::MonoClassField(::MonoClassField* monoClassField)
		: mField{ monoClassField }
		, mFieldName{ mono_field_get_name(mField) }
		, mType{ mono_field_get_type(mField) }
		, mClass{ mono_class_from_mono_type(mType) }
		, mClassName{ mono_class_get_name(mClass) }
		, mFlags{ mono_field_get_flags(mField) }
	{
		std::cout << "|------" << *this << std::endl;
	}

	void MonoClassField::Get(::MonoObject* objInstance, void* outputValue) const
	{
		mono_field_get_value(objInstance, mField, outputValue);
	}

	void MonoClassField::Set(::MonoObject* objInstance, void* inputValue) const
	{
		mono_field_set_value(objInstance, mField, inputValue);
	}

	const char* MonoClassField::GetFieldName() const
	{
		return mFieldName;
	}

	const char* MonoClassField::GetClassName() const
	{
		return mClassName;
	}

	const char* MonoClassField::GetAccess() const
	{
		switch (mFlags & MONO_FIELD_ATTR_FIELD_ACCESS_MASK)
		{
			case MONO_FIELD_ATTR_PRIVATE:
				return "private";

			case MONO_FIELD_ATTR_FAM_AND_ASSEM:
				return "protected internal";

			case MONO_FIELD_ATTR_ASSEMBLY:
				return "internal";

			case MONO_FIELD_ATTR_FAMILY:
				return "protected";

			case MONO_FIELD_ATTR_PUBLIC:
				return "public";

			default:
				return "unknown";
		}
	}

	std::ostream& operator<<(std::ostream& cout, const MonoClassField& rhs)
	{
		cout << rhs.GetAccess() << " " << rhs.mClassName << " " << rhs.mFieldName;
		return cout;
	}
}
