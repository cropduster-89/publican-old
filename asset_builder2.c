#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <gl/gl.h>
#include <math.h>

#include "vec.h"

#include <windows.h>

#define ONE_PAST_MAX_FONT_CODEPOINT (0x10FFFF + 1)
#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

#define MAX_FONT_WIDTH 1024
#define MAX_FONT_HEIGHT 1024

#include "publican_platform.h"
#include "publican_assets.h"
#include "asset_builder.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_TGA
#include "stb_image.h"

static VOID *globalFontBits;
static HDC globalFontDeviceContext;

static inline float Square(float a)
{
		return(a * a);
}

static inline union vec4 SRGB255ToLinear1(union vec4 C)
{
    union vec4 Result;

    float Inv255 = 1.0f / 255.0f;
    
    Result.r = Square(Inv255*C.r);
    Result.g = Square(Inv255*C.g);
    Result.b = Square(Inv255*C.b);
    Result.a = Inv255*C.a;

    return(Result);
}

static inline union vec4 Linear1ToSRGB255(union vec4 C)
{
    union vec4 Result;

    float One255 = 255.0f;

    Result.r = One255*sqrt(C.r);
    Result.g = One255*sqrt(C.g);
    Result.b = One255*sqrt(C.b);
    Result.a = One255*C.a;

    return(Result);
}

static void InitializeFontDC(void)
{
    globalFontDeviceContext = CreateCompatibleDC(GetDC(0));

    BITMAPINFO Info = {};
    Info.bmiHeader.biSize = sizeof(Info.bmiHeader);
    Info.bmiHeader.biWidth = MAX_FONT_WIDTH;
    Info.bmiHeader.biHeight = MAX_FONT_HEIGHT;
    Info.bmiHeader.biPlanes = 1;
    Info.bmiHeader.biBitCount = 32;
    Info.bmiHeader.biCompression = BI_RGB;
    Info.bmiHeader.biSizeImage = 0;
    Info.bmiHeader.biXPelsPerMeter = 0;
    Info.bmiHeader.biYPelsPerMeter = 0;
    Info.bmiHeader.biClrUsed = 0;
    Info.bmiHeader.biClrImportant = 0;
    HBITMAP Bitmap = CreateDIBSection(globalFontDeviceContext, &Info, DIB_RGB_COLORS, &globalFontBits, 0, 0);
    SelectObject(globalFontDeviceContext, Bitmap);
    SetBkColor(globalFontDeviceContext, RGB(0, 0, 0));
}

static struct loaded_bmp LoadGlyph(struct loaded_font *font, 
								   uint32_t codepoint,
								   struct pfile_asset *asset)
{
		struct loaded_bmp result = {};
		uint32_t glyphIndex = font->glyphIndexFromCodePoint[codepoint];
		SelectObject(globalFontDeviceContext, font->win32Handle);
		printf("memset 1 try\n");
		memset(globalFontBits, 0x00, MAX_FONT_WIDTH * MAX_FONT_HEIGHT *sizeof(uint32_t));
		printf("memset 1 success\n");
		wchar_t cheesePoint = (wchar_t)codepoint;
		SIZE size;
		GetTextExtentPoint32W(globalFontDeviceContext, &cheesePoint, 1, &size);
		int32_t prestepX = 128;
		int32_t boundW = size.cx + 2 * prestepX;
		int32_t boundH = size.cy;
		if(boundW > MAX_FONT_WIDTH) boundW = MAX_FONT_WIDTH;
		if(boundH > MAX_FONT_HEIGHT) boundH = MAX_FONT_HEIGHT;
		
		SetTextColor(globalFontDeviceContext, RGB(255, 255, 255));
		TextOutW(globalFontDeviceContext, prestepX, 0, &cheesePoint, 1);
		int32_t minX = 10000;
		int32_t minY = 10000;
		int32_t maxX = -10000;
		int32_t maxY = -10000;
		
		uint32_t *row = (uint32_t *)globalFontBits + (MAX_FONT_HEIGHT - 1) * MAX_FONT_WIDTH;
		for(int32_t y = 0; y < boundH; ++y) {
				uint32_t *pixel = row;
				for(int32_t x = 0; x < boundW; ++x) {
						if(*pixel != 0) {
								if(minX > x) {
										minX = x;
								} if(minY > y) {
										minY = y;
								} if(maxX < x) {
										maxX = x;
								} if(maxY < y) {
										maxY = y;
								}
						}
						++pixel;
				}
				row -= MAX_FONT_WIDTH;
		}
		float kerningCHange = 0;
		if(minX <= maxX) {
				int32_t width = (maxX - minX) + 1;
				int32_t height = (maxY - minY) + 1;
				
				result.w = width + 2;
				result.h = height + 2;
				result.pitch = result.w * 4;
				result.data = malloc(result.h * result.pitch);
				
				memset(result.data, 0, result.h * result.pitch);
				printf("memset 2 success\n");
				uint8_t *destRow = (uint8_t *)result.data + (result.h - 1 - 1) * result.pitch;
				uint32_t *srcRow = (uint32_t *)globalFontBits + (MAX_FONT_HEIGHT - 1 - minY) * MAX_FONT_WIDTH;
				for(int32_t y = minY; y <= maxY; ++y) {
						uint32_t *source = (uint32_t *)srcRow + minX;
						uint32_t *dest = (uint32_t *)destRow + 1;
						for(int32_t x = minX; x <= maxX; ++x) {
								uint32_t pixel = *source;
								float grey = (float)(pixel & 0xFF);
								union vec4 texel = {
										.r = 255.0f,
										.g = 255.0f,
										.b = 255.0f,
										.a = 255.0f
								};
								SRGB255ToLinear1(texel);
								texel.rgb = MultVec3(texel.rgb, texel.a);
								Linear1ToSRGB255(texel);
								
								*dest++ = (((uint32_t)(texel.a + 0.5f) << 24) |
										   ((uint32_t)(texel.r + 0.5f) << 16) |
										   ((uint32_t)(texel.g + 0.5f) << 8) |
										   ((uint32_t)(texel.b + 0.5f) << 0));
								++source;		   
						}
						destRow -= result.pitch;
						srcRow -= MAX_FONT_WIDTH;
				}
				kerningCHange = (float)(minX - prestepX);
		}
		INT thisWidth;
		GetCharWidth32W(globalFontDeviceContext, codepoint, codepoint, &thisWidth);
		float charAdvance = (float)thisWidth;
		for(uint32_t g = 0; g < font->maxGlyphCount; ++g) {
				font->horAdvance[glyphIndex * font->maxGlyphCount + g] += charAdvance = kerningCHange;
				if(g != 0) {
						font->horAdvance[g * font->maxGlyphCount + glyphIndex] += kerningCHange;
				}
		}
		return(result);
}

static struct loaded_font *LoadFont(char *filename,
									char *fontname,
									int32_t pixelHeight)
{
		struct loaded_font *font = malloc(sizeof(struct loaded_font));
		AddFontResourceExA(filename, FR_PRIVATE, 0);
		
		font->win32Handle = CreateFontA(pixelHeight, 0, 0, 0, FWdir_northORMAL, FALSE,
										FALSE, FALSE, DEFAULT_CHARSET, 
										OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
										ANTIALIASED_QUALITY, DEFAULT_PITCH|FF_DONTCARE,
										fontname);
		SelectObject(globalFontDeviceContext, font->win32Handle);
		GetTextMetrics(globalFontDeviceContext, &font->textMetric);
		font->minCodepoint = INT_MAX;
		font->maxCodepoint = 0;
		font->maxGlyphCount = 5000;
		font->glyphCount = 0;
		
		uint32_t glyphIndex = ONE_PAST_MAX_FONT_CODEPOINT * sizeof(uint32_t);
		font->glyphIndexFromCodePoint = malloc(glyphIndex);
		memset(font->glyphIndexFromCodePoint, 0, glyphIndex);
		
		font->glyphs = malloc(sizeof(struct pfile_glyph) * font->maxGlyphCount);
		size_t horAdvanceSize = sizeof(float) * font->maxGlyphCount * font->maxGlyphCount;
		font->horAdvance = malloc(horAdvanceSize);
		memset(font->horAdvance, 0, horAdvanceSize);

		font->onePastHighest = 0;
		
		font->glyphCount = 1;
		font->glyphs[0].codepoint = 0;
		font->glyphs[0].id.val = 0;
		
		return(font);
		
}

static void BeginType(struct pub_assets *assets, 
					  enum asset_type_id type)
{
		assets->assetType = assets->assetTypes + type;
		assets->assetType->id = type;
		assets->assetType->firstIndex = assets->assetCount;
		assets->assetType->lastPlus1Index = assets->assetType->firstIndex;
}

static void EndType(struct pub_assets *assets)
{
		assets->assetCount = assets->assetType->lastPlus1Index;
		assets->assetType = 0;
		assets->assetIndex = 0;
}

struct added_asset {
		uint32_t id;
		struct pfile_asset *pubb;
		struct pub_asset_source *source;
};

static struct added_asset AddAsset(struct pub_assets *assets)
{
		uint32_t index = assets->assetType->lastPlus1Index++;
		struct pub_asset_source *source = assets->assetSources + index;
		struct pfile_asset *pubb =assets->assets + index;
		pubb->firstIndex = assets->tagCount;
		pubb->lastPlus1Index = pubb->firstIndex;
		
		assets->assetIndex = index;
		
		struct added_asset result;
		result.id = index;
		result.pubb = pubb;
		result.source = source;
		return(result);
}

static struct bmp_id AddChar(struct pub_assets *assets,
							 struct loaded_font *font,
							 uint32_t codepoint)
{
		struct added_asset asset = AddAsset(assets);
		asset.source->type = format_glyph;
		asset.source->glyph.font = font;
		asset.source->glyph.codepoint = codepoint;

		struct bmp_id result = {asset.id};

		uint32_t glyphIndex = font->glyphCount++;
		struct pfile_glyph *glyph = font->glyphs + glyphIndex;
		glyph->codepoint = codepoint;
		glyph->id = result;
		font->glyphIndexFromCodePoint[codepoint] = glyphIndex;
		if(font->onePastHighest <= codepoint) {
				font->onePastHighest = codepoint + 1;
		}
		return(result);
}

static struct bmp_id AddBmp(struct pub_assets *assets,
							char *filename)
{
		struct added_asset asset = AddAsset(assets);	
		asset.source->type = format_bmp;
		asset.source->bmp.filename = filename;
		 
		struct bmp_id result = {
				asset.id
		};
		 
		return(result);
}

int main(void)
{
		printf("Begin\n");
		
		InitializeFontDC();
		
		struct pub_assets _assets;
		struct pub_assets *assets = &_assets;
		assets->tagCount = 1;
		assets->assetCount = 1;
		assets->assetType = 0;
		assets->assetIndex = 0;
		
		BeginType(assets, asset_wood_floor);
		AddBmp(assets, "data/wood_floor.tga");
		EndType(assets);
		
		BeginType(assets, asset_stool);
		AddBmp(assets, "data/stool.tga");
		EndType(assets);
		
		BeginType(assets, asset_person);
		AddBmp(assets, "data/personN.tga");
		AddBmp(assets, "data/personNE.tga");
		AddBmp(assets, "data/personE.tga");
		AddBmp(assets, "data/personSE.tga");
		AddBmp(assets, "data/personS.tga");
		AddBmp(assets, "data/personSW.tga");
		AddBmp(assets, "data/personW.tga");
		AddBmp(assets, "data/personNW.tga");
		EndType(assets);		
		
		BeginType(assets, asset_bar);
		AddBmp(assets, "data/bar_0.tga");
		AddBmp(assets, "data/bar_1.tga");
		AddBmp(assets, "data/bar_2.tga");
		AddBmp(assets, "data/bar_3.tga");
		EndType(assets);
		
		BeginType(assets, asset_table);
		AddBmp(assets, "data/table_0.tga");
		AddBmp(assets, "data/table_1.tga");
		AddBmp(assets, "data/table_2.tga");
		AddBmp(assets, "data/table_3.tga");
		EndType(assets);
		
		struct loaded_font *fonts[] = {
				LoadFont("c:/Windows/Fonts/arial.tff", "Arial", 128),
				LoadFont("c:/Windows/Fonts/LiberationMono-Regular.tff", "Liberation Mono", 20)
		};		
		BeginType(assets, asset_font);
		for(uint32_t i = 0; i < ARRAY_COUNT(fonts); ++i) {
				struct loaded_font *font = fonts[i];
				AddChar(assets, font, ' ');
				for(uint32_t j = '!'; j <= '~'; ++j) {
						AddChar(assets, font, j);
				}
		}
		EndType(assets);
		printf("End typing\n");		
		
		FILE *out = fopen("test.elwin", "wb");
		printf("Try\n");
		if(out) {
				printf("Success\n");
				
				stbi_set_flip_vertically_on_load(1);
				
				struct pfile_asset_header header = {};
				header.name = PUB_FILEdir_northAME;
				header.tagCount = assets->tagCount;
				header.assetTypeCount = asset_count;
				header.assetCount = assets->assetCount;
				
				uint32_t tagArraySize = header.tagCount * sizeof(struct pfile_tag);
				uint32_t assetTypeArraySize = header.assetTypeCount * 
											  sizeof(struct pfile_asset_type);
				uint32_t assetArraySize = header.assetCount * sizeof(struct pfile_asset);
				
				header.tags = sizeof(header);
				header.assetTypes = header.tags + tagArraySize;
				header.assets = header.assetTypes + assetTypeArraySize;

				fwrite(&header, sizeof(header), 1, out);	
				fwrite(&assets->tags, tagArraySize, 1, out);	
				fwrite(&assets->assetTypes, assetTypeArraySize, 1, out);	
				fseek(out, assetArraySize, SEEK_CUR);
				for(uint32_t i = 1; i < header.assetCount; ++i) {
						struct pub_asset_source *source = assets->assetSources + i;
						struct pfile_asset *dest = assets->assets + i;
						
						dest->dataOffset = ftell(out);
						
						printf("Format: %u\n", source->type);					
						
						
						if(source->type == format_bmp) {
								struct loaded_bmp bmp;
								int c;							
								bmp.data = stbi_load(source->bmp.filename, &bmp.w, &bmp.h, &c, 0);
								assert(bmp.data);
								bmp.pitch = bmp.w * 4;
								dest->bmp.dim[0] = bmp.w;
								dest->bmp.dim[1] = bmp.h;
								fwrite(bmp.data, bmp.w * bmp.h * 4, 1, out);
								stbi_image_free(bmp.data);								
						} else if(source->type == format_glyph){
								struct loaded_bmp bmp;
								
								bmp = LoadGlyph(source->glyph.font, source->glyph.codepoint, dest);
								dest->bmp.dim[0] = bmp.w;
								dest->bmp.dim[1] = bmp.h;
								fwrite(bmp.data, bmp.w * bmp.h * 4, 1, out);
						}
				}
				fseek(out, (uint32_t)header.assets, SEEK_SET);
				fwrite(assets->assets, assetArraySize, 1, out);
				
				fclose(out);
		} else {
				printf("Fail\n");
		}
}