#include <iostream>
#include <filesystem>

#include "MonoCore/MonoManager.h"


std::string GetCsProj()
{
	return std::string{ (std::filesystem::current_path().parent_path() / "sample-csharp" / "sample.csproj").string() };
}

int main()
{
	ohno::MonoManager mm{};
	mm.Init();
	
	mm.CompileAssembly(GetCsProj());
	mm.LoadAssembly();
}