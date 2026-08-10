#include "phy.h"
#include <iostream>
#include <fstream>

PHY ReadPHY(std::vector<std::byte> phy) {
    auto data = phy.data();
    PHY out_phy = {};
    out_phy.raw_original_data = phy;

    std::cout << "READING PHY" << std::endl;

    phyheader_t* phy_header = (phyheader_t*)data;

    out_phy.checksum = phy_header->checkSum;

    return out_phy;
}

void WritePHY(PHY phy, std::ofstream& outputFile) {
    std::cout << "WRITING PHY" << std::endl;

    if (!outputFile.is_open()) {
        std::cerr << "Error opening the file!" << std::endl;
        return;
    }
    
    auto data = phy.raw_original_data.data();
    phyheader_t* phy_header = (phyheader_t*)data;

    phy_header->checkSum = phy.checksum;
    
    // Baseline file structure
    outputFile.write(reinterpret_cast<const char*>(data), phy.raw_original_data.size());

    // Currently useless
    // Write accurate header at the beginning
    outputFile.seekp(0);
    outputFile.write(reinterpret_cast<const char*>(phy_header), sizeof(phy_header));
    
    outputFile.close();
}