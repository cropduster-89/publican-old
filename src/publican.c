/**************************************************************************************************
		Publican, Platform Independant 
		
		
**************************************************************************************************/

#include "publican.h"

extern void pub_MainLoop(struct pub_memory *memory, 
			 struct pub_input *input,						
			 struct render_commands *commands)
{
	platform = memory->platformAPI;
#ifdef DEBUG		
	debugMemory = memory;
#endif			
	struct game_state *state = memory->state;
	if(!state) {
		state = memory->state = BootStrapPushSize(struct game_state, mainArena, DEF_BOOT, DEF_PUSH);	
		printf("State Allocated\n");
	}	
	struct temp_state *tState = (struct temp_state *)memory->tState;
	if(!tState) {
		tState = memory->tState = BootStrapPushSize(struct temp_state, tempArena, DEF_BOOT, DEF_PUSH);		
		for(uint32_t i = 0; i < ARRAY_COUNT(tState->tasks); ++i) {
			tState->highPriorityQueue = memory->highPriorityQueue;
			tState->lowPriorityQueue = memory->lowPriorityQueue;
			struct task_with_memory *task = tState->tasks + i;
			task->inUse = false;						
		}				
		tState->assets = assets_Allocate(MEGABYTES(256), tState, &memory->textureOpQueue);  	
		printf("TState Allocated\n");	
	}		 
		
	struct loaded_bmp drawBuffer = {};		
	drawBuffer.w = commands->w;
	drawBuffer.h = commands->h;
	
	struct render_group renderGroup_ = render_AllocGroup(tState->assets, commands, 
							     drawBuffer.w, drawBuffer.h);
	struct render_group *renderGroup = &renderGroup_;

	struct render_group textGroup_ = render_AllocGroup(tState->assets, commands,
							   drawBuffer.w, drawBuffer.h);
	struct render_group *textGroup = &textGroup_;	
		
	world_UpdateAndRender(state, tState, input, renderGroup, textGroup, drawBuffer);	
	
	debug_OutputCycles(memory, textGroup, tState);
						
	CheckArena(&state->modeArena);		
	CheckArena(&tState->tempArena);							
}