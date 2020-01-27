extern void debug_TextOut(char *string,
			  struct render_group *renderGroup,
			  struct temp_state *tState,
			  uint32_t lineStart,	
			  float scale)
{		
	union vec3 offset = {
		.x = 50.0f - (WIN_X / 2),
		.y = (WIN_Y / 2) - 50.0f,
		.z = 0.0f
	};
	float lineWeight = 20.0f;
	offset.y -= lineStart * lineWeight;	
	struct object_transform trans = render_FlatTrans();
	trans.offset = offset;
	uint32_t len = text_StringCount(string);
	//uint32_t len = 20;
	char *current = string;		
	float prevX = 0;
	float prevSizeX = 0;
	for(uint32_t i = 0; i < len; ++i, ++current) {
		if(*current == ' ') {
			trans.offset.x += 15;
			continue;
		} else if(*current == '\n') {
			offset.y -= lineWeight;
		} else if(*current == '\0') {
			break;
		}		
		struct bmp_id id = asset_FindChar(tState->assets, asset_font, _CHAR(*current));	
		struct loaded_bmp *bmp = assets_GetBMP(tState->assets, id);							   
		if(!bmp) {
			assets_LoadBMP(renderGroup->assets, id, true, final_bmp);
			return;
		} else {
			union vec2 size = {
				.x = bmp->w,
				.y = bmp->h,
			};
			union vec2 align = {
				.x = bmp->alignX,
				.y = bmp->alignY,
			};
			
			align = MultVec2(align, 1.0f);				
			align = MultVec2(align, scale);		
			size = MultVec2(size, 1.0f);		
			size = MultVec2(size, scale);				
								
			trans.offset.x +=  align.x + prevX + prevSizeX;				
			trans.offset.y -=  size.y + align.y;
			
			//trans = render_UprightTrans();		
			
			prevX = align.x;
			prevSizeX = size.x;
			render_PushBMP(renderGroup, trans, id, size.y, uiOffset, 0);
			trans.offset.y = offset.y;			
		}				
	}		
}

extern void debug_OutputCycles(struct pub_memory *memory,
			       struct render_group *group,
			       struct temp_state *tState)
{
#if 1
	for(int i = 0; i < ARRAY_COUNT(memory->counters); ++i) {
		struct cycle_counter *counter = memory->counters + i;
		
		if(counter->hitCount) {
			char buffer[256];
			sprintf(buffer, "%d: %I64ucy %uh %I64ucy/h", i,
				counter->cycleCount, counter->hitCount,
				counter->cycleCount / counter->hitCount);
					
			counter->cycleCount = 0;
			counter->hitCount = 0;
			
			debug_TextOut(buffer,group, tState, 10 + i,	0.3f);
		}
	}
#endif		
}