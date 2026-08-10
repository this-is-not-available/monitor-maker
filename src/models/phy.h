#ifndef PHY_H
#define PHY_H

#include <vector>
#include "mdl.h"

#pragma pack(push, 1)

typedef struct phyheader_s
{
	int		size;          // Size of this header section (generally 16)
	int		id;            // Often zero, unknown purpose.
	int		solidCount;    // Number of solids in file
	int	    checkSum;	   // checksum of source .mdl file (32 bits)
} phyheader_t;

static_assert(sizeof(phyheader_s) == 16);

#pragma pack(pop)

class PHY {
public:
	std::vector<std::byte> raw_original_data;
	int checksum;
};

PHY ReadPHY(std::vector<std::byte> phy);
void WritePHY(PHY phy, std::ofstream& outputFile);

#endif