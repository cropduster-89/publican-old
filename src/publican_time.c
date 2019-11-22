static inline uint64_t time_GetTime(void)
{
	//struct timeval tv;
	//gettimeofday(&tv, NULL);
	//return((uint64_t) tv.tv_sec * (uint64_t)1000000UL + (uint64_t)tv.tv_usec);
	return(1);
}

static inline struct game_time time_Update(struct game_time timer)
{
	assert(timer.baseTime);
	timer.currentTime = (time_GetTime() - timer.baseTime);
	return(timer);
}

extern bool time_TimePast(struct world_mode *world,
			  struct action_time *timer,
			  uint64_t testTime)
{
	return(world->timer.currentTime > timer->actionStart + testTime);
}

extern void time_StartTimedEvent(struct world_mode *world,
				 struct action_time *timer,
				 uint64_t timeTaken)
{
	timer->actionStart = world->timer.currentTime;
	timer->actionTime = timeTaken;
	timer->lastTick = timer->actionStart;
	timer->tickCount = 0;
}

extern bool time_EndTimedEvent(struct world_mode *world,
			       struct action_time *timer)
{
	return(world->timer.currentTime > timer->actionStart + timer->actionTime);
}   

static inline bool time_TickIncrement(struct world_mode *world,
				    struct action_time *timer)
{
	bool result = false;
	
	if(world->timer.currentTime - timer->lastTick > 500000L) {
		result = true;
		timer->lastTick = world->timer.currentTime;
		timer->tickCount++;
	}
	return(result);	
}