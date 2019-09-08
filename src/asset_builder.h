#pragma once

#pragma pack(push, 1)
struct pfile_header {
		uint32_t name;
#define PUBdir_northAME PUB_FILE('p', 'u', 'b', 'b')		
		uint32_t typeCount;		
		uint32_t assetCount;
		
		uint64_t types;
		uint64_t assets;	
		
		int8_t kerningTable[128][128];
};

struct pfile_bmp {
		uint32_t x, y;
		float alignX, alignY;		
};

struct pfile_mesh {
	uint32_t faceCount;
	uint32_t vertexCount;
};

struct pfile_type {
	uint32_t id;
	uint32_t firstAsset;
	uint32_t nextType;
};

struct pfile_asset{
	uint64_t offset;		
	union {
		struct pfile_bmp bmp;
		struct pfile_mesh mesh;
	};
};
#pragma pack(pop)

enum asset_format_id {
	format_bmp,
	format_sound,
	format_mesh,
	format_font,
};

#include "publican_platform.h"

#include "publican_assets.h"
#include "publican_utils.h"


struct asset_source_bmp {
	char *fileName;
};

struct asset_source_sound {
	char *fileName;
	uint32_t firstSample;
};

struct asset_source_glyph {
	char *fileName;
	uint32_t codepoint;
};

struct asset_source_mesh {
	char *fileName;		
	uint8_t rot;
};

struct asset_source {
		enum asset_format_id format;
		union {
				struct asset_source_bmp bmp;
				struct asset_source_sound sound;
				struct asset_source_glyph glyph;
				struct asset_source_mesh mesh;
		};	
};

struct asset_plan {
		uint32_t typeCount;
		struct pfile_type types[asset_count];
		struct pfile_type *currentType;
		
		uint32_t assetCount;
		struct asset_source sources[4096];
		struct pfile_asset assets[4096];
		uint32_t index;
};

struct new_asset {
		uint32_t id;
		struct pfile_asset *asset;
		struct asset_source *source;
};

struct bmp_id {
		uint32_t val;
};

struct mesh_id {
		uint32_t val;
};