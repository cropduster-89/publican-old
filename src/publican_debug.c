extern void debug_OutputCycles(struct pub_memory *memory,
			       struct render_group *group,
			       struct temp_state *tState)
{
#if 0
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