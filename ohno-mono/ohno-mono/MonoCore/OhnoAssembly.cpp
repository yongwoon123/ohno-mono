#include <iostream>
#include <fstream>
#include <cassert>
#include <algorithm>

#include "MonoCore/OhnoAssembly.h"

namespace ohno
{
    // Reads a binary file into a buffer.
    // Loading the .dll into memory (rather than passing the path to mono_domain_assembly_open)
    // avoids keeping a file lock on disk, which would block hot reload overwrites.
    struct FileStreams
    {
        std::ifstream file;
        size_t        size { 0 };

        FileStreams(const std::string& filePath)
            : file { filePath, std::ios::binary }
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

    OhnoAssembly::OhnoAssembly(const MonoManager& mm, const std::string& filePath)
        : mMonoManagerRef { mm }
    {
        FileStreams file { filePath };

        std::vector<char> buffer(file.size);
        file.Read(buffer.data());

        MonoImageOpenStatus status = MonoImageOpenStatus::MONO_IMAGE_OK;

        // "copy data" = true: Mono copies the buffer internally, safe to free ours after.
        // "ref only" = false: the assembly is fully loaded and executable.
        mImage = mono_image_open_from_data_with_name(buffer.data(),
            static_cast<uint32_t>(file.size),
            true,
            &status,
            false,
            filePath.c_str());

        if (status != MONO_IMAGE_OK || mImage == nullptr)
        {
            std::cout << "Failed loading assembly " << filePath << "\n";
        }

        mAssembly = mono_assembly_load_from_full(mImage, filePath.c_str(), &status, false);
        if (status != MONO_IMAGE_OK || mAssembly == nullptr)
        {
            std::cout << "Failed loading assembly" << filePath << "\n";
        }

        LoadAllClass();
    }

    OhnoAssembly::~OhnoAssembly()
    {
        mClasses.clear();
        mono_image_close(mImage);
        mAssembly = nullptr;
    }

    void OhnoAssembly::LoadDependencies()
    {
        std::ranges::for_each(mClasses, [] (const auto& ptr) -> void
            {
                ptr.second->LoadAllMethods();
                ptr.second->LoadAllInheritedFields();
            });
    }

    void OhnoAssembly::LoadAllClass()
    {
        // MONO_TABLE_TYPEDEF lists every type defined in the assembly.
        // Index 0 is reserved for the "<Module>" pseudo-class - start from 1.
        const int numRows = mono_image_get_table_rows(mImage, MONO_TABLE_TYPEDEF);

        for (int i = 1; i < numRows; ++i)
        {
            // Tokens are 1-based in the metadata spec, so (i + 1) skips the <Module> entry.
            std::unique_ptr<OhnoClass> classPtr
                = std::make_unique<OhnoClass>(mMonoManagerRef, mono_class_get(mImage, (i + 1) | MONO_TOKEN_TYPE_DEF));

            mClasses[classPtr->GetClassName()] = std::move(classPtr);
        }
    }

    const OhnoClass* OhnoAssembly::GetClass(const std::string_view& ohnoClassName)
    {
        auto it = mClasses.find(ohnoClassName);
        return it == mClasses.end() ? nullptr : it->second.get();
    }

    std::vector<const OhnoClass*> OhnoAssembly::GetInheritedClass(const OhnoClass* myClass) const
    {
        std::vector<const OhnoClass*> inheritedClass {};

        const char* className = myClass->GetClassName();

        std::ranges::for_each(mClasses, [&myClass, &inheritedClass, &className] (const auto& klass) -> void
            {
                const OhnoClass* itClass = klass.second.get();

                if (itClass->isSubClassOf(myClass->GetRawClass())
                    && std::strcmp(className, itClass->GetClassName()) != 0)
                {
                    inheritedClass.emplace_back(itClass);
                }
            });

        return inheritedClass;
    }

    std::ostream& operator<<(std::ostream& cout, const OhnoAssembly& rhs)
    {
        cout << "|--Classes\n";

        std::ranges::for_each(rhs.mClasses, [&cout] (const auto& klass)->void
            {
                cout << "|----" << klass.first << "\n";
                cout << *(klass.second);
            });

        return cout;
    }
}
