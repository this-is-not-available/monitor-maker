#include <vector>
#ifndef VTX_H
#define VTX_H

#include "mdl.h"

#pragma pack(push, 1)

// this structure is in <mod folder>/src/public/optimize.h
struct FileHeader_t
{
	// file version as defined by OPTIMIZED_MODEL_FILE_VERSION (currently 7)
	int version;

	// hardware params that affect how the model is to be optimized.
	int vertCacheSize;
	unsigned short maxBonesPerStrip;
	unsigned short maxBonesPerTri;
	int maxBonesPerVert;

	// must match checkSum in the .mdl
	int checkSum;

	int numLODs; // Also specified in ModelHeader_t's and should match

	// Offset to materialReplacementList Array. one of these for each LOD, 8 in total
	int materialReplacementListOffset;

    //Defines the size and location of the body part array
	int numBodyParts;
	int bodyPartOffset;
};

static_assert(sizeof(FileHeader_t) == 36);

struct MaterialReplacementListHeader_t
{
	int numReplacements;
	int replacementOffset;
};

static_assert(sizeof(MaterialReplacementListHeader_t) == 8);

#pragma pack(pop)

class VTX {
public:
	std::vector<std::byte> raw_original_data;
	int checksum;
};

VTX ReadVTX(std::vector<std::byte> vtx);
void WriteVTX(VTX vtx, std::ofstream& outputFile);

#endif