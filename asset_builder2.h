#pragma once

#define FILE_CODE(a, b, c, d) (((uint32_t)(a) << 0) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

#pragma pack(push, 1)
struct pfile_asset_header {
#define PUB_FILE_NAME FILE_CODE('p', 'u', 'b', 'b')	
		uint32_t name;
		
		uint32_t tagCount;
		uint32_t assetTypeCount;
		uint32_t assetCount;
		
		uint64_t tags;
		uint64_t assetTypes;
		uint64_t assets;
};

struct bmp_id {
		uint32_t val;
};

struct sound_id {
		uint32_t val;
};

struct pfile_tag {
		uint32_t id;
		float val;
};

struct pfile_asset_type {
		uint32_t id;
		uint32_t firstIndex;
		uint32_t lastPlus1Index;
};

struct pfile_bmp {
		uint32_t dim[2];
};

struct pfile_sound {
		uint32_t sampleCount;
		uint32_t channelCount;
		struct sound_id nextID;
};

struct pfile_glyph {
		uint32_t codepoint;
		struct bmp_id id;
};

struct pfile_font {
		uint32_t highestCodepointPlus1;
		uint32_t glyphCount;
		float ascenderHeight;
		float descenderHeight;
		float externalLeading;
};

struct pfile_asset {
		uint64_t dataOffset;
		uint32_t firstIndex;
		uint32_t lastPlus1Index;
		union {
				struct pfile_bmp bmp;
				struct pfile_sound sound;
				struct pfile_font font;
		};
};

#pragma pack(pop)

#define BIG_NUM 4096

enum pub_asset_format {
		format_sound,
		format_bmp,
		format_glyph,
};

struct pub_asset_bmp {
		char *filename;
};

struct pub_asset_glyph {
		struct loaded_font *font;
		uint32_t codepoint;
};

struct pub_asset_source {
		enum pub_asset_format type;		
		union {
				struct pub_asset_bmp bmp;
				struct pub_asset_glyph glyph;
		};		
};

struct pub_assets {
		uint32_t tagCount;
		struct pfile_tag tags[BIG_NUM];
		
		uint32_t assetTypeCount;
		struct pfile_asset_type assetTypes[asset_count];
		
		uint32_t assetCount;
		struct pub_asset_source assetSources[BIG_NUM];
		struct pfile_asset assets[BIG_NUM];
		
		struct pfile_asset_type *assetType;
		uint32_t assetIndex;
};