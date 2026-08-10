#include "vvd.h"
#include <iostream>
#include <fstream>

VVD ReadVVD(std::vector<std::byte> vvd) {
    auto data = vvd.data();
    VVD out_vvd = {};
    out_vvd.raw_original_data = vvd;

    std::cout << "READING VVD" << std::endl;

    vertexFileHeader_t* vvd_header = (vertexFileHeader_t*)data;

    out_vvd.checksum = vvd_header->checksum;

    return out_vvd;
}

void WriteVVD(VVD vvd, std::ofstream& outputFile) {
    std::cout << "WRITING VVD" << std::endl;

    if (!outputFile.is_open()) {
        std::cerr << "Error opening the file!" << std::endl;
        return;
    }
    
    auto data = vvd.raw_original_data.data();
    vertexFileHeader_t* vvd_header = (vertexFileHeader_t*)data;

    vvd_header->checksum = vvd.checksum;
    
    // Baseline file structure
    outputFile.write(reinterpret_cast<const char*>(data), vvd.raw_original_data.size());

    // Currently useless
    // Write accurate header at the beginning
    outputFile.seekp(0);
    outputFile.write(reinterpret_cast<const char*>(vvd_header), sizeof(vvd_header));
    
    outputFile.close();
}