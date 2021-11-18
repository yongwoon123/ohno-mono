#include <iostream>
#include <filesystem>

#include <MonoCore/MonoManager.h>

struct Vector3
{
	float x { 5 }, y { 6 }, z { 7 };
};

const std::string& GetCsProj()
{
	static const std::string csProj { (std::filesystem::current_path().parent_path() / "sample-csharp" / "sample.csproj").string() };
	return csProj;
}

void CPPFunc()
{
	std::cout << "Called the CPPFunc" << std::endl;
}

int main()
{
	ohno::MonoManager mm {};
	mm.Init();

	mm.CompileAssembly(GetCsProj());
	mm.LoadAssembly();

	//Get Class
	const ohno::OhnoClass* classPtr = mm.GetClass("A");
	::MonoObject* objPtr = nullptr;
	::MonoObject* inhObjPtr = nullptr;

	//Default Constructor
	{
		//Create instance of class default constructor
		objPtr = mm.CreateInstance("A");
		std::cout << std::endl;
	}

	//Custom Constructor
	{
		//Create instance of conversion constructor
		int val = 51234;
		void* args[] = { &val };
		objPtr = mm.CreateInstance("A", args, 1);
		std::cout << std::endl;
	}

	//Internal Call example
	{
		classPtr->AddInternalCall("Internal_CallCPPFunc", CPPFunc);
	}

	//Field Example
	{
		const ohno::OhnoClassField* fieldPtr1 = classPtr->GetField("BaseValueIs20");
		const ohno::OhnoClassField* fieldPtr2 = classPtr->GetField("BaseValueIs10F");

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
		std::cout << "After Set: " << outputF << std::endl << std::endl;
	}

	//Inheritance Example
	{
		inhObjPtr = mm.CreateInstance("B");
		const ohno::OhnoClassField* fieldPtr1 = classPtr->GetField("PrivateValueOf50");

		int input = 5, output = 0;
		fieldPtr1->Get(inhObjPtr, &output);
		std::cout << "Initial: " << output << std::endl;

		fieldPtr1->Set(inhObjPtr, &input);
		fieldPtr1->Get(inhObjPtr, &output);
		std::cout << "After Set: " << output << std::endl << std::endl;
	}

	//Method Example
	{
		const ohno::OhnoMethod* methodPtr1 = classPtr->GetMethod("SampleFunctionC");
		const ohno::OhnoMethod* methodPtr2 = classPtr->GetMethod("SampleFunctionD", 1);
		const ohno::OhnoMethod* methodPtr3 = classPtr->GetMethod("SampleFunctionE");

		Vector3 val = {};
		void* args[] = { &val };

		classPtr->InvokeMethod("SampleFunctionC", objPtr);
		methodPtr1->Invoke(objPtr);

		classPtr->InvokeMethod("SampleFunctionD", objPtr, args, 1);
		methodPtr2->Invoke(objPtr, args);

		classPtr->InvokeMethod("SampleFunctionE", objPtr);
		methodPtr3->Invoke(objPtr);
	}
}