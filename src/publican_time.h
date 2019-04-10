#pragma once 

struct action_time {
	uint64_t actionTime;	
	uint64_t currentActionTime;			
	uint64_t actionStart;	
	uint64_t lastTick;
	uint32_t tickCount;	
};

struct game_time {
	uint32_t baseTime;
	uint32_t currentTime;	
};