#include <iostream>
#include <filesystem>

#include "MonoCore/MonoManager.h"

struct Vector3
{
	float x{ 5 }, y{ 6 }, z{ 7 };
};

const std::string& GetCsProj()
{
	static const std::string csProj{ (std::filesystem::current_path().parent_path() / "sample-csharp" / "sample.csproj").string() };
	return csProj;
}

int main()
{
	ohno::MonoManager mm{};
	mm.Init();

	mm.CompileAssembly(GetCsProj());
	mm.LoadAssembly();

	//Get Class
	const ohno::MonoClass* classPtr = mm.GetClass("A");
	::MonoObject* objPtr = nullptr;

	{
		//Create instance of class default constructor
		::MonoObject* objPtr = mm.CreateInstance("A");
	}

	{
		//Create instance of conversion constructor
		int val = 51234;
		void* args[] = { &val };
		objPtr = mm.CreateInstance("A", args, 1);
	}

	//Field Example
	{
		const ohno::MonoClassField* fieldPtr1 = classPtr->GetField("BaseValueIs20");
		const ohno::MonoClassField* fieldPtr2 = classPtr->GetField("BaseValueIs10F");

		int input = 5, output = 0;
		fieldPtr1->Get(objPtr, &output);
		std::cout << "Initial: " << output << std::endl;

		fieldPtr1->Set(objPtr, &input);
		fieldPtr1->Get(objPtr, &output);
		std::cout << "After Set: " << output << std::endl;

		float inputF = 5, outputF = 0;
		fieldPtr2->Get(objPtr, &outputF);
		std::cout << "Initial: " << outputF << std::endl;

		fieldPtr2->Set(objPtr, &inputF);
		fieldPtr2->Get(objPtr, &outputF);
		std::cout << "After Set: " << outputF << std::endl;
	}

	//Method Example
	{
		const ohno::MonoMethod* methodPtr1 = classPtr->GetMethod("SampleFunctionC");
		const ohno::MonoMethod* methodPtr2 = classPtr->GetMethod("SampleFunctionD", 1);

		classPtr->InvokeMethod("SampleFunctionC", objPtr);
		methodPtr1->Invoke(objPtr);

		Vector3 val = {};
		void* args[] = { &val };
		classPtr->InvokeMethod("SampleFunctionD", objPtr, args, 1);
		methodPtr2->Invoke(objPtr, args);
	}
}