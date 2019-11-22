#ifndef PUB_WIN32_H
#define PUB_WIN32_H
/*************************************************************************************************
		Win32 Platform layer Header
*************************************************************************************************/

struct win32_file_handle {
	HANDLE win32Handle;
};

struct win32_file_group {
	HANDLE findHandle;
	WIN32_FIND_DATAW findData;
};

struct win32_memory_block {
	struct platform_memory_block block;
	struct win32_memory_block *next;	
	struct win32_memory_block *prev;
	uint64_t loopingFlags;	
};

struct win32_state {
	struct ticket_mutex memMutex;		
	struct win32_memory_block memSentinal;
};

enum win32_mem_flags {
	win32mem_AllocDuringLoop = 0x1,
	win32mem_FreeDuringLoop = 0x2,
};

struct platform_work_queue_entry {
	platform_work_queue_callback *callback;
	void *data;
};

struct platform_work_queue {
	uint32_t volatile completionGoal;
	uint32_t volatile completionCount;
	
	uint32_t volatile nextToRead;;
	uint32_t volatile nextToWrite;
	HANDLE semaphoreHandle;
	
	struct platform_work_queue_entry entries[256];		
};

struct win32_thread_startup {
	struct platform_work_queue *queue;		
};
/*************************************************************************************************
		Win32 WGL Guff
*************************************************************************************************/

#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_SRGB8_ALPHA8                   0x8C43

#define GL_SHADING_LANGUAGE_VERSION       0x8B8C



// NOTE(casey): Windows-specific


int win32_OpenGLAttribs[] =
{
    WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
    WGL_CONTEXT_MINOR_VERSION_ARB, 5,
    WGL_CONTEXT_FLAGS_ARB, 0,
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
    0,
};

#endif
