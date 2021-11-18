#pragma once

#include <unordered_map>

#include "MonoCore/MonoPrereq.h"
#include "MonoCore/OhnoClass.h"

namespace ohno
{
	class OhnoAssembly
	{
	public://Public Functions

		//Constructor
		OhnoAssembly(const MonoManager& mm, const std::string& filePath);

		//Destructor
		~OhnoAssembly();

		void LoadDependencies();

		const OhnoClass* GetClass(const std::string_view& ohnoClassName);

		std::vector<const OhnoClass*> GetInheritedClass(const OhnoClass* myClass) const;

	private:
		friend std::ostream& operator<<(std::ostream& cout, const OhnoAssembly& rhs);

		//Helper to load all classes
		void LoadAllClass();

		//Mono pointers
		::MonoAssembly* mAssembly { nullptr };
		::MonoImage* mImage { nullptr };

		std::unordered_map<std::string_view, std::unique_ptr<OhnoClass>> mClasses {};

		const MonoManager& monoManagerRef;
	};
}