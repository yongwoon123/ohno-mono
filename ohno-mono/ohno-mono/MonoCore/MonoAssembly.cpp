#include <iostream>
#include <fstream>

#include "MonoCore/MonoAssembly.h"

namespace ohno
{
	struct FileStreams
	{
		std::ifstream file;
		size_t        size{ 0 };

		FileStreams(const std::string& filePath)
			: file{ filePath, std::ios::binary }
		{
			file.seekg(0, std::ios_base::end);
			size = file.tellg();
			file.seekg(0, std::ios_base::beg);
		}

		void Read(char* buffer)
		{
			file.read(buffer, size);
		}
	};

	MonoAssembly::MonoAssembly(const std::string& filePath)
	{
		FileStreams file{ filePath };

		std::vector<char> buffer(file.size);
		file.Read(buffer.data());

		MonoImageOpenStatus status = MonoImageOpenStatus::MONO_IMAGE_OK;

		// Load assembly from memory because mono_domain_assembly_open keeps a lock on the file
		mImage = mono_image_open_from_data_with_name(buffer.data(),
													 static_cast<uint32_t>(file.size),
													 true /* copy data */,
													 &status,
													 false /* ref only */,
													 filePath.c_str());

		if (status != MONO_IMAGE_OK || mImage == nullptr)
		{
			std::cout << "Failed loading assembly " << filePath << std::endl;
		}

		mAssembly = mono_assembly_load_from_full(mImage, filePath.c_str(), &status, false);
		if (status != MONO_IMAGE_OK || mAssembly == nullptr)
		{
			std::cout << "Failed loading assembly" << filePath << std::endl;
		}

		std::cout << "Assembled " << filePath << std::endl;

		LoadAllClass();
	}

	MonoAssembly::~MonoAssembly()
	{
		//for (auto& p : mClasses)
		//{
		//	p.second->Unload();
		//}

		//mClasses.clear();

		mono_image_close(mImage);
		mIsLoaded = false;
		mAssembly = nullptr;
	}

	bool MonoAssembly::GetLoaded()
	{
		return mIsLoaded;
	}

	void MonoAssembly::LoadAllClass()
	{
		std::cout << "Load All Class" << std::endl;

		const int numRows = mono_image_get_table_rows(mImage, MONO_TABLE_TYPEDEF);

		for (int i = 1; i < numRows; ++i)
		{
			auto* mClass = mono_class_get(mImage, (i + 1) | MONO_TOKEN_TYPE_DEF);

			if (mClass == nullptr)
			{
				std::cout << "Some class issue" << std::endl;
				continue;
			}

			std::cout << mono_class_get_name(mClass) << std::endl;

			//std::unique_ptr<MonoClass> monoClass = std::make_unique<MonoClass>();

			//str name = monoClass->LoadClass(mImage, (i + 1) | MONO_TOKEN_TYPE_DEF);

			//if (monoClass->mIsLoaded == false)
			//{
			//	continue;
			//}

			//mClasses[name] = std::move(monoClass);
		}

		mIsLoaded = true;
	}
}