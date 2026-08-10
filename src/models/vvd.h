#include <vector>
#ifndef VVD_H
#define VVD_H

#include "mdl.h"

#pragma pack(push, 1)

struct vertexFileHeader_t
{
	int	id;				// MODEL_VERTEX_FILE_ID
	int	version;			// MODEL_VERTEX_FILE_VERSION
	int	checksum;			// same as studiohdr_t, ensures sync      ( Note: maybe long instead of int in versions other than 4. )
	// not needed and could become a problem if left in
	/*int	numLODs;			// num of valid lods
	int	numLODVertexes[MAX_NUM_LODS];	// num verts for desired root lod
	int	numFixups;			// num of vertexFileFixup_t
	int	fixupTableStart;		// offset from base to fixup table
	int	vertexDataStart;		// offset from base to vertex block
	int	tangentDataStart;		// offset from base to tangent block*/
};

static_assert(sizeof(vertexFileHeader_t) == 12);

#pragma pack(pop)

class VVD {
public:
	std::vector<std::byte> raw_original_data;
	int checksum;
};

VVD ReadVVD(std::vector<std::byte> vvd);
void WriteVVD(VVD vvd, std::ofstream& outputFile);

#endif