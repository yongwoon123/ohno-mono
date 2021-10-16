#include <iostream>
#include <filesystem>

#include "MonoCore/MonoManager.h"

const std::string& GetCsProj()
{
	static const std::string csProj { (std::filesystem::current_path().parent_path() / "sample-csharp" / "sample.csproj").string() };
	return csProj;
}

int main()
{
	ohno::MonoManager mm{};
	mm.Init();
	
	mm.CompileAssembly(GetCsProj());
	mm.LoadAssembly();
}