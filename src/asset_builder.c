#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>
#include <windows.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define STB_ONLY_TGA
#include "stb_image.h"
#include "stb_truetype.h"
#include "vec.h"

#define _270D 4.712389
#define _180D 3.141593
#define _90D 1.570796
#define _60D 1.047198
#define _45D 0.7853982
#define _30D 0.5235988

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

#define PUB_FILE(a, b, c, d) (((uint32_t)(a) << 0) |\
							  ((uint32_t)(b) << 8) |\
							  ((uint32_t)(c) << 16) |\
							  ((uint32_t)(d) << 24))

#define _MIN(a,b) (((a)<(b))?(a):(b))
#define _MAX(a,b) (((a)>(b))?(a):(b))

struct chara_attributes {
	uint8_t strength;
	uint8_t charisma;
	uint8_t intelligence;
	uint8_t dexterity;
};


enum body_type {
	body_thin,
	body_med,
	body_fat	
};

enum dress_type {
	dressdir_northude,
	
	dress_1,	
	dress_2,	
	dress_3,	
};

enum hair_type {
	hair_1,	
	hair_2,	
	hair_3,	
	hair_4,	
	hair_5,	
	hair_6,	
	hair_7,	
	hair_8,	
	hair_9,	
};

enum face_type {
	face_1,	
	face_2,	
	face_3,	
};

enum race_type {
	racedir_northull,
	race_human,
	race_elf,	
};

enum trait_type {
	traitdir_northull,
	trait_mediator,
	trait_troublemaker,	
};

enum job_type {
	jobdir_northull,
	job_owner,
	job_staff,
	job_blacksmith,
	job_builder,
	job_shopkeep,
	job_labourer,
	job_housewife,
	job_unemployed,
	job_militia,
	job_police,
	job_vagrant,
};

struct memory_arena {
		struct platform_memory_block *currentBlock;
		uintptr_t minBlockSize;
		
		uint64_t allocationFlags;		
		int32_t tempCount;
};						  
							  
#include "asset_builder.h"

static uint32_t totalGlyphs;

struct file_read_output {
		uint32_t contentsSize;
		void *contents;
};

extern struct file_read_output win32_ReadFile(char *filename)
{
		struct file_read_output result = {};
		HANDLE handle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 
						0, OPEN_EXISTING, 0, 0);
		if(handle == INVALID_HANDLE_VALUE) {
				printf("Invalid handle value\n");
		}		
		LARGE_INTEGER size;
		if(GetFileSizeEx(handle, &size)) {
				uint32_t size32 = size.QuadPart;
				result.contents = VirtualAlloc(0, size32, 
									MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
				if(!result.contents) {
						printf("No contents\n");
				}
				DWORD bytesRead;
				if(ReadFile(handle, result.contents, size32, &bytesRead, 0) &&
				(size32 == bytesRead)) {
						result.contentsSize = size32;
				} else {
						VirtualFree(result.contents, 0, MEM_RELEASE);
						result.contents = 0;
				}
		}
		return(result);
}

static void GetPLYHeader(uint8_t *contents,
			 struct loaded_mesh *mesh,
			 size_t *headerSize)
{
	bool formatFlag = false;
	bool vertexFlag = false;
	bool faceFlag = false;
	char *at = contents;
	char *start = contents;
	
	for(; *at; ) {
		while(IsWhitespace(*at)) {++at;}
		char *end = at;
		while(*end && !IsWhitespace(*end)) {++end;}
		
		uintptr_t count = end - at;
								
		if(StringsAreEqual(count, at, "format")) {
			formatFlag = true;								
		}  else if(formatFlag) {
			assert(StringsAreEqual(count, at, "binary_little_endian"));
			formatFlag = false;
		}
		if(StringsAreEqual(count, at, "vertex")) {
			vertexFlag = true;								
		} else if(vertexFlag) {
			char bufferV[4];						
			for(int32_t i = 0; i <= count; ++i) {
				if(i == count) {
					bufferV[i] = '\0';
				} else {
					bufferV[i] = *(at + i);
				}	
			}
			char *endPtr;
			mesh->vertexCount = strtoumax(bufferV, &endPtr, 10);
			printf("Vertices : %d\n", mesh->vertexCount);			
			vertexFlag = false;
		}
		if(StringsAreEqual(count, at, "face")) {
				faceFlag = true;								
		} else if(faceFlag) {
			char bufferF[4];
			for(int32_t i = 0; i <= count; ++i) {
				if(i == count) {
					bufferF[i] = '\0';
				} else {
					bufferF[i] = *(at + i);
				}								
			}
			
			char *endPtr;
			mesh->faceCount = strtoumax(bufferF, &endPtr, 10);
			printf("Faces : %d\n", mesh->faceCount);			
			faceFlag = false;
		}
		if(StringsAreEqual(count, at, "end_header")) {
			at = ++end;									
			*headerSize = at - start;		
			printf("Header read\n");
			break;
		}
		at = end;
	}		
}

#define _32BIT sizeof(int32_t)

static void LoadPLY(struct loaded_mesh *mesh,
		    char *fileName)
{
	printf("\n%s\n", fileName);
	
	FILE *file = fopen(fileName, "rb");
	if(file) {
		fseek(file, 0L, SEEK_END);
		int32_t size = ftell(file);
		fseek(file, 0L, SEEK_SET);
		uint8_t *contents = malloc(size);
		
		if(!fread(contents, size, 1, file)) {						
			printf("fread failed : %d\n", feof(file));						
		}
		size_t headerSize;
		GetPLYHeader(contents, mesh, &headerSize);
		
		size_t vertexDataSize = sizeof(float) * mesh->vertexCount * VERTEX_STRIDE; 
		size_t faceDataSize = (sizeof(uint32_t) * FACE_STRIDE * mesh->faceCount);
		mesh->data = malloc(vertexDataSize + faceDataSize);
		memcpy(mesh->data, contents + headerSize, vertexDataSize);
				
		uint8_t *src = contents + headerSize + vertexDataSize + 1;		
		uint32_t *dest = (uint32_t *)(mesh->data + vertexDataSize);
		
		for(int32_t j = 0; j < mesh->faceCount; ++j) {
			uint32_t *facePtr = (int32_t *)src;
			for(int32_t i = 0; i < FACE_STRIDE; ++i) {
				*(dest + j * FACE_STRIDE + i) = *(facePtr + i);				
			}	
			src += 38;			
		}					
		free(contents);			
		fclose(file);				
				
	} else {
		printf("Read .ply file \"%s\" failed\n", fileName);
	}
}

static void GetKerningTable(struct pfile_header *header,
						    char *fileName)
{
	struct loaded_bmp result = {};
	
	struct file_read_output TTF_file = win32_ReadFile(fileName);
	if(TTF_file.contentsSize != 0) {
			stbtt_fontinfo font;
			stbtt_InitFont(&font, (uint8_t *)TTF_file.contents, 
					stbtt_GetFontOffsetForIndex((uint8_t *)TTF_file.contents, 0));	

			for(uint32_t i = '!'; i < '~'; ++i) {
					for(uint32_t j = '!'; j < '~'; ++j) {
							header->kerningTable[i - 33][j - 33] =
							stbtt_GetCodepointKernAdvance(&font, i, j);
							//printf("%d\n", header->kerningTable[i - 33][j - 33]);
					}
			}			
			
			VirtualFree(TTF_file.contents, 0, MEM_RELEASE);			
	} else {
			printf("Couldn't load font file for kern\n");
	}		
}

static struct loaded_bmp LoadGlyphBMP(struct pfile_header *header,
									  char *fileName, 
									  uint32_t codepoint)
{
	struct loaded_bmp result = {};
	
	struct file_read_output TTF_file = win32_ReadFile(fileName);
	if(TTF_file.contentsSize != 0) {
		stbtt_fontinfo font;
		stbtt_InitFont(&font, (uint8_t *)TTF_file.contents, 
			stbtt_GetFontOffsetForIndex((uint8_t *)TTF_file.contents, 0));								
		uint8_t *monoBmp = stbtt_GetCodepointBitmap(&font, 0, 
			stbtt_ScaleForPixelHeight(&font, 64.0f), codepoint, 
			&result.w, &result.h, &result.alignX, &result.alignY);					
		printf("%d, %d\n", result.w, result.h);
		result.pitch = result.w * 4;
		result.data = malloc(result.h * result.pitch);
		uint8_t *src = monoBmp;
		uint8_t *destRow = (uint8_t *)result.data + (result.h - 1) * result.pitch;
		for(int32_t y = 0; y < result.h; ++y) {
			uint32_t *dest = (uint32_t *)destRow;
			for(int32_t x = 0; x < result.w; ++x) {
				uint8_t alpha = *src++;
				*dest++ = ((alpha << 24) |
					   (alpha << 16) |
					   (alpha <<  8) |
					   (alpha <<  0));
			}
			destRow -= result.pitch;
		}					
		int32_t adv, left;
		stbtt_GetCodepointHMetrics(&font, codepoint, &adv, &left);				
		stbtt_FreeBitmap(monoBmp, 0);				
		VirtualFree(TTF_file.contents, 0, MEM_RELEASE);			
			
	} else {
		printf("Couldn't load font file\n");
	}		
	return(result);
}
							  
static void StartType(struct asset_plan *assets,
		      enum asset_type_id type)
{
	assert(assets->currentType == 0);
	
	assets->currentType = assets->types + type;
	assets->currentType->id = type;
	assets->currentType->firstAsset = assets->assetCount;
	assets->currentType->nextType = assets->currentType->firstAsset;
}

static void EndType(struct asset_plan *assets)
{
	assert(assets->currentType);
	assets->assetCount = assets->currentType->nextType;
	assets->currentType = 0;
	assets->index = 0;
}

static struct bmp_id AddBMP(struct asset_plan *assets,
							char *fileName)
{
	assert(assets->currentType);
	assert(assets->currentType->nextType < ARRAY_COUNT(assets->assets));
	
	struct bmp_id result = {assets->currentType->nextType++};
	struct asset_source *src = assets->sources + result.val;
	struct pfile_asset *asset = assets->assets + result.val;
		
	src->format = format_bmp;
	src->bmp.fileName = fileName;		
	
	return(result);
}

static struct string_id AddString(struct asset_plan *assets,
			          char *text)
{
	assert(assets->currentType);
	assert(assets->currentType->nextType < ARRAY_COUNT(assets->assets));
	
	struct string_id result = {assets->currentType->nextType++};
	struct asset_source *src = assets->sources + result.val;
	struct pfile_asset *asset = assets->assets + result.val;
	
	src->format = format_string;
	src->string.text = text;		
	
	return(result);
}

static struct mesh_id AddMesh(struct asset_plan *assets,
			      char *fileName)
{
	assert(assets->currentType);
	assert(assets->currentType->nextType < ARRAY_COUNT(assets->assets));
	
	struct mesh_id result = {assets->currentType->nextType++};
	struct asset_source *src = assets->sources + result.val;
	struct pfile_asset *asset = assets->assets + result.val;
		
	src->format = format_mesh;
	src->mesh.fileName = fileName;	
	
	return(result);
}

static struct bmp_id AddChar(struct asset_plan *assets,
							 char *fileName,
							 uint32_t codepoint)
{
	assert(assets->currentType);
	assert(assets->currentType->nextType < ARRAY_COUNT(assets->assets));
	
	struct bmp_id result = {assets->currentType->nextType++};
	struct asset_source *src = assets->sources + result.val;
	struct pfile_asset *asset = assets->assets + result.val;
		
	src->format = format_font;
	src->glyph.fileName = fileName;		
	src->glyph.codepoint = codepoint;		
	
	return(result);
}

static void InitAssetPlan(struct asset_plan *assets)
{
	assets->assetCount = 1;
	assets->currentType = 0;
	assets->index = 0;
	
	assets->typeCount = asset_count;
	memset(assets->types, 0, sizeof(assets->types));
}

int main(void)
{
	struct asset_plan assets_;
	struct asset_plan *assets = &assets_;

	InitAssetPlan(assets);
	
	StartType(assets, ASSET_UPTXT);
	AddString(assets, "Up\0");
	EndType(assets);
	
	StartType(assets, ASSET_DOWNTXT);
	AddString(assets, "Down\0");
	EndType(assets);
	
	StartType(assets, ASSET_BUILDTXT);
	AddString(assets, "Build\0");
	EndType(assets);
	
	StartType(assets, ASSET_THIRSTTXT);
	AddString(assets, "Thirst\0");
	EndType(assets);
	
	StartType(assets, ASSET_DRUNKTXT);
	AddString(assets, "Drunk\0");
	EndType(assets);
	
	StartType(assets, ASSET_BLADDERTXT);
	AddString(assets, "Bladder\0");
	EndType(assets);
	
	StartType(assets, asset_floor);
	AddBMP(assets, "data/struct/floor_mud.tga");
	AddBMP(assets, "data/struct/floor_wood.tga");		
	AddMesh(assets, "data/struct/floor.ply");
	EndType(assets);
	
	StartType(assets, asset_floor);
	AddBMP(assets, "data/struct/floor_mud.tga");
	AddBMP(assets, "data/struct/floor_wood.tga");		
	AddMesh(assets, "data/struct/floor.ply");
	EndType(assets);

	StartType(assets, asset_support);
	AddBMP(assets, "data/struct/support.tga");		
	AddMesh(assets, "data/struct/support.ply");
	EndType(assets);	

	StartType(assets, asset_steps);
	AddBMP(assets, "data/struct/steps.tga");
	AddMesh(assets, "data/struct/steps.ply");	
	EndType(assets);		
	
	StartType(assets, asset_wall);	
	AddBMP(assets, "data/struct/wall_4_inner.tga");
	AddMesh(assets, "data/struct/wall_inner_3.ply");		
	AddMesh(assets, "data/struct/wall_inner_4.ply");	
	AddMesh(assets, "data/struct/wall_inner_small.ply");		
	EndType(assets);
	
	StartType(assets, asset_door);
	AddBMP(assets, "data/struct/door_inner_4.tga");
	AddMesh(assets, "data/struct/door_inner_4.ply");
	EndType(assets);
	
	StartType(assets, asset_pillar);
	AddBMP(assets, "data/struct/pillar_4_uv.tga");
	AddMesh(assets, "data/struct/pillar_4.ply");
	EndType(assets);
	
	StartType(assets, asset_stool);
	AddBMP(assets, "data/furn/stool.tga");
	AddMesh(assets, "data/furn/stool.ply");
	EndType(assets);
	
	StartType(assets, asset_bar);
	AddBMP(assets, "data/furn/bar.tga");
	AddMesh(assets, "data/furn/bar.ply");	
	EndType(assets);
	
	StartType(assets, asset_table);		
	AddBMP(assets, "data/furn/table.tga");
	AddMesh(assets, "data/furn/table_2x1.ply");	
	EndType(assets);
	
	StartType(assets, asset_tiolet);
	AddBMP(assets, "data/furn/tiolet.tga");
	AddMesh(assets, "data/furn/tiolet.ply");
	EndType(assets);
		
	StartType(assets, asset_man);	
	AddBMP(assets, "data/male.tga");
	EndType(assets);
	
	StartType(assets, asset_fem);	
	AddBMP(assets, "data/female.tga");
	EndType(assets);
				
	StartType(assets, asset_buttons);
	AddBMP(assets, "data/down_button.tga");
	AddBMP(assets, "data/up_button.tga");
	AddBMP(assets, "data/collision_button.tga");
	AddBMP(assets, "data/up_button.tga");	
	AddBMP(assets, "data/blank_button.tga");	
	AddBMP(assets, "data/down_button.tga");	
	EndType(assets);
	
	StartType(assets, asset_panels);
	AddBMP(assets, "data/ui_panel.tga");	
	AddBMP(assets, "data/ui_panel.tga");
	AddBMP(assets, "data/rel_panel.tga");
	EndType(assets);
	
	StartType(assets, asset_font);
	
	for(uint32_t i = '!'; i < '~'; ++i) {
		AddChar(assets, "C:/Windows/Fonts/tahoma.ttf", i);
		totalGlyphs = i - 33;
	}	
	EndType(assets);
	
	StartType(assets, asset_bubbles);
	AddBMP(assets, "data/misc/bubble_speech.tga");	
	EndType(assets);
	
	StartType(assets, asset_cursor);
	AddBMP(assets, "data/cursor.tga");	
	EndType(assets);
	
	FILE *out = fopen("test.pubb", "wb");
	
	if(out) {				
		stbi_set_flip_vertically_on_load(1);
		
		struct pfile_header header = {};
		header.name = ASSET_FILEEXT;			
		header.typeCount = asset_count;
		header.assetCount = assets->assetCount;				
		
		uint32_t assetTypeArraySize = header.typeCount * sizeof(struct pfile_type);
		uint32_t assetArraySize = header.assetCount * sizeof(struct pfile_asset);				
		
		header.types = sizeof(struct pfile_header);
		header.assets = header.types + assetTypeArraySize;
		
		GetKerningTable(&header, "c:/Windows/Fonts/tahoma.ttf");
		
		fwrite(&header, sizeof(header), 1, out);					
		fwrite(&assets->types, assetTypeArraySize, 1, out);	
		fseek(out, assetArraySize, SEEK_CUR);
		for(uint32_t i = 1; i < header.assetCount; ++i) 
		{
			struct asset_source *source = assets->sources + i;
			struct pfile_asset *dest = assets->assets + i;
			
			dest->offset = ftell(out);			
			
			if(source->format == format_bmp) 
			{
				struct loaded_bmp bmp;
				int c;							
				bmp.data = stbi_load(source->bmp.fileName, &bmp.w, &bmp.h, &c, 0);
				assert(bmp.data);
				bmp.pitch = bmp.w * 4;
				dest->bmp.x = bmp.w;
				dest->bmp.y = bmp.h;
				fwrite(bmp.data, bmp.w * bmp.h * 4, 1, out);
				stbi_image_free(bmp.data);								
			} 
			else if(source->format == format_mesh) 
			{
				struct loaded_mesh mesh;
				LoadPLY(&mesh, source->mesh.fileName);	
				assert(mesh.data);	
				dest->mesh.faceCount = mesh.faceCount;
				dest->mesh.vertexCount = mesh.vertexCount;				
				
				if(fwrite(mesh.data, (_32BIT * mesh.faceCount * FACE_STRIDE + 
				_32BIT * mesh.vertexCount * VERTEX_STRIDE), 1, out) != 1) {
					assert(0);
				}
				free(mesh.data);			
			} 
			else if(source->format == format_font) 
			{
				struct loaded_bmp bmp;
				bmp = LoadGlyphBMP(&header, source->glyph.fileName, 
								   source->glyph.codepoint);
				dest->bmp.x = bmp.w;
				dest->bmp.y = bmp.h;
				dest->bmp.alignX = bmp.alignX;
				dest->bmp.alignY = bmp.alignY;
				
				fwrite(bmp.data, bmp.w * bmp.h * 4, 1, out);
				free(bmp.data);
			} 
			else if(source->format == format_string) 
			{
				struct loaded_string string;
				dest->string.size = strlen(source->string.text);
				string.data = malloc(dest->string.size);
				strcpy(string.data, source->string.text);
				fwrite(string.data, dest->string.size, 1, out);
				free(string.data);
			}
		}
		fseek(out, (uint32_t)header.assets, SEEK_SET);
		fwrite(assets->assets, assetArraySize, 1, out);
		
		fclose(out);
	} else {
		printf("Fail\n");
	}
}