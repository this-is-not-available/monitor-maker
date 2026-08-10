#include "vtx.h"
#include <iostream>
#include <fstream>

VTX ReadVTX(std::vector<std::byte> vtx) {
    auto data = vtx.data();
    VTX out_vtx = {};
    out_vtx.raw_original_data = vtx;

    std::cout << "READING VTX" << std::endl;

    FileHeader_t* vtx_header = (FileHeader_t*)data;

    out_vtx.checksum = vtx_header->checkSum;

    MaterialReplacementListHeader_t* mrl_hdr = (MaterialReplacementListHeader_t*)(data + vtx_header->materialReplacementListOffset);

    return out_vtx;
}

void WriteVTX(VTX vtx, std::ofstream& outputFile) {
    std::cout << "WRITING VTX" << std::endl;

    if (!outputFile.is_open()) {
        std::cerr << "Error opening the file!" << std::endl;
        return;
    }
    
    auto data = vtx.raw_original_data.data();
    FileHeader_t* vtx_header = (FileHeader_t*)data;

    vtx_header->checkSum = vtx.checksum;
    
    // Baseline file structure
    outputFile.write(reinterpret_cast<const char*>(data), vtx.raw_original_data.size());

    // Currently useless
    // Write accurate header at the beginning
    outputFile.seekp(0);
    outputFile.write(reinterpret_cast<const char*>(vtx_header), sizeof(vtx_header));
    
    outputFile.close();
}