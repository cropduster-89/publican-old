#pragma once

enum floor_bmp_type_id {
	floor_bmp_dirt,	
	floor_bmp_wood,
	
	floor_bmp_count
};

enum wall_bmp_type_id {
	wall_bmp_placeholder,
	
	wall_bmp_count,
};

enum wall_mesh_type_id {
	wall_3,
	wall_4,	
	wall_small,	
	
	wall_mesh_count
};

enum asset_type_id {	
	ASSET_UPTXT,
	ASSET_DOWNTXT,
	ASSET_BUILDTXT,
	
	ASSET_THIRSTTXT,
	ASSET_DRUNKTXT,
	ASSET_BLADDERTXT,
	
	asset_floor,
	asset_support,
	asset_steps,	
	asset_wall,
	asset_door,
	asset_pillar,
	
	asset_stool,	
	asset_bar,
	asset_table,
	asset_tiolet,	
	
	asset_man,
	asset_fem,
	
	asset_buttons,
	asset_panels,
	asset_font,
	asset_bubbles,
	asset_cursor,
	
	asset_character,
	
	asset_count,	
};

struct char_file_header {
	int32_t count;	
};

struct loaded_char {
	char firstName[32];
	char lastName[32];
	enum race_type race;
	enum job_type job;
	int32_t age;
	int8_t gender;
	
	enum body_type body;
	enum dress_type dress;
	enum face_type face;
	enum hair_type hair;
	
	enum trait_type traits[32];
	
	struct chara_attributes attribs;
};

struct loaded_bmp {
	void *data;
	int32_t w, h;
	int32_t pitch;
	int32_t stride;
	
	float wOverH;
	
	int32_t alignX;
	int32_t alignY;		
	
	int32_t adv;
	int32_t lead;
	
	int32_t arrayOffset;
	
	void *handle;		
};

#define VERTEX_STRIDE 8
#define FACE_STRIDE 3

struct loaded_string {
	void *data;
	uint32_t size;
};

struct loaded_mesh {
	void *data;
	uint32_t vertexCount;	
	uint32_t faceCount;		
};

struct loaded_sound {
	uint32_t sampleCount;
	uint32_t channelCount;
	int16_t *samples[2];
};

enum finalize_asset_op {
	finaldir_northone,
	final_font,
	final_bmp,
	final_skin,
	final_mesh,
	final_string,
};

struct load_asset_work {
	struct task_with_memory *task;
	struct asset *asset;
	
	struct platform_file_handle *handle;
	uint64_t offset;
	uint64_t size;
	void *dest;
	
	enum finalize_asset_op finalOp;
	uint32_t finalState;
	
	struct platform_texture_op_queue *textureOpQueue;
};

#include "asset_builder.h"

struct asset_file {
	struct platform_file_handle handle;	
	
	struct pfile_header header;
	struct pfile_type *assetTypeArray;		
	
	float bmpIDOffset;
};

struct asset_type {
	uint32_t firstIndex;
	uint32_t lastIndexPlus1;
};

enum asset_state {
	state_unloaded,
	state_queued,
	state_loaded,
};

struct asset {
	uint32_t state;
	struct asset_memory_header *header;
	
	struct pfile_asset pubb;
	uint32_t fileIndex;
};

struct asset_memory_size {
	uint32_t total;
	uint32_t data;
	uint32_t section;
};

enum asset_header_type {	
	header_bmp,
	ASSETHEADER_STRING,
	header_mesh,
	header_character,
};

struct asset_memory_header {
	struct asset_memory_header *prev;
	struct asset_memory_header *next;

	uint32_t assetType;	
	uint32_t assetIndex;	
	uint32_t totalSize;	
	
	union {
		struct loaded_bmp bmp;				
		struct loaded_mesh mesh;
		struct loaded_string string;
	};		
};

struct asset_memory_block {
	struct asset_memory_block *prev;
	struct asset_memory_block *next;
	uint64_t flags;
	size_t size;
};

struct game_assets {
	struct memory_arena nonRestoredMem;	
	
	struct platform_texture_op_queue *textureOpQueue;
	
	struct temp_state *tState;		
	
	struct asset_memory_block memSentinal;
	struct asset_memory_header loadedAssetSentinal;
	
	int8_t *kerningTable;
	
	uint32_t fileCount;
	struct asset_file *files;		
	
	uint32_t assetCount;
	struct asset *assets;		
	
	struct asset_type types[asset_count];			
};



struct texture_op_allocate {
	uint32_t w;
	uint32_t h;
	void *data;
	
	void **resultHandle;	
	int32_t *arrayOffset;
};

struct texture_op_deallocate {
	void *handle;
};

struct texture_op {
	struct texture_op *next;
	enum finalize_asset_op type;
	
	bool isAllocate;
	union {
		struct texture_op_allocate alloc;
		struct texture_op_deallocate deAlloc;
	};
};


