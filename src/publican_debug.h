#pragma once 

enum cycle_counters {
		debug_world,
		debug_list,
		debug_render,
		debug_render_bmp,
		debug_render_mesh,
		debug_render_loop,
		debug_ai,
		debug_count,
};

struct cycle_counter {
		uint64_t cycleCount;
		uint32_t hitCount;
};



