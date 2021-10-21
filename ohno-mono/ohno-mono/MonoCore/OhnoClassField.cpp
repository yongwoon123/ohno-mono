#include <iostream>

#include "MonoCore/OhnoClassField.h"

namespace ohno
{
	OhnoClassField::OhnoClassField(::MonoClassField* monoClassField)
		: mField{ monoClassField }
		, mFieldName{ mono_field_get_name(mField) }
		, mType{ mono_field_get_type(mField) }
		, mClass{ mono_class_from_mono_type(mType) }
		, mClassName{ mono_class_get_name(mClass) }
		, mFlags{ mono_field_get_flags(mField) }
	{
		std::cout << "|------" << *this << std::endl;
	}

	void OhnoClassField::Get(::MonoObject* objInstance, void* outputValue) const
	{
		mono_field_get_value(objInstance, mField, outputValue);
	}

	void OhnoClassField::Set(::MonoObject* objInstance, void* inputValue) const
	{
		mono_field_set_value(objInstance, mField, inputValue);
	}

	const char* OhnoClassField::GetFieldName() const
	{
		return mFieldName;
	}

	const char* OhnoClassField::GetClassName() const
	{
		return mClassName;
	}

	const char* OhnoClassField::GetAccess() const
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

	std::ostream& operator<<(std::ostream& cout, const OhnoClassField& rhs)
	{
		cout << rhs.GetAccess() << " " << rhs.mClassName << " " << rhs.mFieldName;
		return cout;
	}
}
