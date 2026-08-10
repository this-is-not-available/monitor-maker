#include "mdl.h"
#include <iostream>
#include <fstream>
#include <list>

// Ok, so..
// Trying to modify the materials in-place requires recalculating offsets in every single struct the model uses
// Reading and rewriting the entire MDL is tremendous effort...
// So we will just append our data to the end of the file and change only 3 offsets
// Leads to minimally larger file sizes (4*skin_count+64*skin_count+size of all the material strings+4+size of model name with path but without .mdl), but
// this will save me hours of coding really boring untested unsafe code and is not that big of a deal
// If it annoys you, fix it yourself

// TODO: this file has loads of potential for refactoring

MDL ParseMDL(std::vector<std::byte> mdl) {
    auto data = mdl.data();
    std::cout << "READING MDL" << std::endl;

    MDL model = {};
    model.raw_original_data = mdl;

    studiohdr_t* mdl_header = (studiohdr_t*)data;

    model.model_name = mdl_header->name;
    std::cout << model.model_name << std::endl;

    model.checksum = mdl_header->checksum;

    // Get $cdmaterials entries
    std::cout << mdl_header->texturedir_count << std::endl;
    for (int textureDirIndex = 0; textureDirIndex < mdl_header->texturedir_count; textureDirIndex++) {
        int offset = *(int*)(data + mdl_header->texturedir_offset + textureDirIndex * sizeof(int));
        char* materialDirName = (char*)data + offset;
        model.cdmaterials.push_back(materialDirName);
        std::cout << materialDirName << std::endl;
    };

    // Get material names
    for (int textureIndex = 0; textureIndex < mdl_header->texture_count; textureIndex++) {
        auto texture = (mstudiotexture_t*)(data + mdl_header->texture_offset + textureIndex * sizeof(mstudiotexture_t));
        std::string material_name = (char*)texture + texture->name_offset;
        model.material_names.push_back(material_name);
        std::cout << material_name << std::endl;
    };

    short* skinTableStart = (short*)(data + mdl_header->skinReferenceOffset);
    int skinTableWidth = mdl_header->skinReferenceCount;

    std::vector<std::vector<short>> skinTable = {};

    for (int row = 0; row < mdl_header->skinReferenceFamilyCount; row++) {
        std::vector<short> skinTableRow = {};
        for (int column = 0; column < skinTableWidth; column++) {
            short skin = (skinTableStart[row * skinTableWidth + column]);
            skinTableRow.push_back(skin);
        }
        skinTable.push_back(skinTableRow);
    }

    for (auto row : skinTable) {
        for (auto skin : row) {
            std::cout << skin << " ";
        }
        std::cout << std::endl;
    }

    model.skin_table = skinTable;

    return model;

    //from vdc
    // width = skinReferenceCount
    // 0000   0002
    // 0001   0002   length = skinReferenceFamilyCount
    // 0002   0002

    // size = skinReferenceCount * skinReferenceFamilyCount
}

void WriteMDL(MDL model, std::ofstream& outputFile) {
    std::cout << "WRITING MDL" << std::endl;

    if (!outputFile.is_open()) {
        std::cerr << "Error opening the file!" << std::endl;
        return;
    }

    auto data = model.raw_original_data.data();
    studiohdr_t* mdl_header = (studiohdr_t*)data;

    // Baseline file structure
    outputFile.write(reinterpret_cast<const char*>(data), model.raw_original_data.size());

    // Write skin family table and update header
    mdl_header->skinReferenceCount = (int)model.skin_table[0].size();
    mdl_header->skinReferenceFamilyCount = (int)model.skin_table.size();

    mdl_header->skinReferenceOffset = (int)outputFile.tellp();
    for (std::vector<short> row : model.skin_table) {
        for (short column : row) {
            outputFile.write(reinterpret_cast<const char*>(&column), sizeof(column));
        }
    }



    // Write empty offsets to cdmaterials
    mdl_header->texturedir_count = (int)model.cdmaterials.size();
    mdl_header->texturedir_offset = (int)outputFile.tellp();
    for (std::string materialdir : model.cdmaterials) {
        std::array<char, sizeof(int)> zeros = {0};
        outputFile.write(zeros.data(), sizeof(zeros));
    }

    // Write material string and store absolute position in a list
    std::vector<std::streampos> textureDirAbsoluteOffsets = {};

    outputFile.seekp(0, std::ios::end);
    for (std::string cdmaterial : model.cdmaterials) {
        textureDirAbsoluteOffsets.push_back(outputFile.tellp());
        outputFile << cdmaterial.c_str() << '\0';
        // TODO: necessary? idk I just added it
        outputFile.seekp(0, std::ios::end);
    }

    // Write actual offsets
    outputFile.seekp(mdl_header->texturedir_offset);
    for (std::streampos absOffset : textureDirAbsoluteOffsets) {
        int offset = (int)absOffset;
        outputFile.write(reinterpret_cast<char*>(&offset), sizeof(offset));
    }
    outputFile.seekp(0, std::ios::end);



    // Write empty material headers
    mdl_header->texture_count = (int)model.material_names.size();
    mdl_header->texture_offset = (int)outputFile.tellp();
    for (std::string material : model.material_names) {
        std::array<char, sizeof(mstudiotexture_t)> zeros = {0};
        outputFile.write(zeros.data(), sizeof(zeros));
    }

    // Generate material strings and store in list
    std::vector<std::streampos> stringAbsoluteOffsets = {};

    for (std::string material : model.material_names) {
        stringAbsoluteOffsets.push_back(outputFile.tellp());
        outputFile << material.c_str() << '\0';
        // TODO: necessary? idk I just added it
        outputFile.seekp(0, std::ios::end);
    }

    // Write material headers with actual contents
    outputFile.seekp(mdl_header->texture_offset);
    for (std::streampos absOffset : stringAbsoluteOffsets) {
        std::streampos relativeOffset = absOffset - outputFile.tellp();

        mstudiotexture_t materialHeader = {};
        materialHeader.name_offset = (int)relativeOffset;
        // TODO: does anything else need to be written?

        outputFile.write(reinterpret_cast<char*>(&materialHeader), sizeof(materialHeader));
    }
    
    // Calculate size of file
    outputFile.seekp(0, std::ios::end);
    mdl_header->dataLength = (int)outputFile.tellp();

    mdl_header->checksum = model.checksum;

    for (char i = 0; i < 63; i++) {
        char c = '\0';
        if (i < model.model_name.length()) {
            c = model.model_name[i];
        }
        mdl_header->name[i] = c;
    }

    // Write accurate header at the beginning
    outputFile.seekp(0, std::ios::beg);
    outputFile.write(reinterpret_cast<const char*>(mdl_header), sizeof(*mdl_header));
    outputFile.flush();

    outputFile.close();
}