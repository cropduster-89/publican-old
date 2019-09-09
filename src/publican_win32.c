/************************************************************************************		
__________     ___.   .__  .__                      	Win32 Platform layer
\______   \__ _\_ |__ |  | |__| ____ _____    ____  
 |     ___/  |  \ __ \|  | |  |/ ___\\__  \  /    \ 
 |    |   |  |  / \_\ \  |_|  \  \___ / __ \|   |  \
 |____|   |____/|___  /____/__|\___  >____  /___|  /
                    \/             \/     \/     \/ 
*************************************************************************************/

#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wcomment"
#pragma GCC diagnostic ignored "-Wsizeof-pointer-div"

#define WINVER 0x0600
#define _WIN32_WINNT 0x0600

#include <windows.h>
#include <gl/gl.h>

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <errno.h>
#include <uchar.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <sys/time.h>

#define WIN_X 1400
#define WIN_Y 900

typedef char GLchar;
typedef int64_t GLsizeiptr;

struct platform_api platform;

static HDC globalDC;
static HGLRC globalHGLRC;
static struct win32_state globalWin32State;

#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_PIXEL_TYPE_ARB                      0x2013

#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_FULL_ACCELERATION_ARB               0x2027

#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB        0x20A9

#define WGL_RED_BITS_ARB                        0x2015
#define WGL_GREEN_BITS_ARB                      0x2017
#define WGL_BLUE_BITS_ARB                       0x2019
#define WGL_ALPHA_BITS_ARB                      0x201B
#define WGL_DEPTH_BITS_ARB                      0x2022

#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC hDC, HGLRC hShareContext,
    const int *attribList);

typedef BOOL WINAPI wgl_get_pixel_format_attrib_iv_arb(HDC hdc,
    int iPixelFormat,
    int iLayerPlane,
    UINT nAttributes,
    const int *piAttributes,
    int *piValues);

typedef BOOL WINAPI wgl_get_pixel_format_attrib_fv_arb(HDC hdc,
    int iPixelFormat,
    int iLayerPlane,
    UINT nAttributes,
    const int *piAttributes,
    FLOAT *pfValues);

typedef BOOL WINAPI wgl_choose_pixel_format_arb(HDC hdc,
    const int *piAttribIList,
    const FLOAT *pfAttribFList,
    UINT nMaxFormats,
    int *piFormats,
    UINT *nNumFormats);

typedef void WINAPI gl_bind_framebuffer(GLenum target, GLuint framebuffer);
typedef void WINAPI gl_gen_framebuffers(GLsizei n, GLuint *framebuffers);
typedef void WINAPI gl_framebuffer_texture_2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef GLenum WINAPI gl_check_framebuffer_status(GLenum target);

typedef void WINAPI gl_active_texture( GLenum texture);
typedef void WINAPI gl_tex_image_2d_multisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void WINAPI gl_gen_buffers(GLsizei n, GLuint *buffers);
typedef void WINAPI gl_bind_buffer(GLenum target, GLuint buffers);
typedef void WINAPI gl_buffer_data(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
typedef void WINAPI gl_buffer_sub_data(GLenum target, intptr_t offset, GLsizeiptr size, const GLvoid *data);
typedef void WINAPI gl_bind_framebuffer(GLenum target, GLuint framebuffer);
typedef void WINAPI gl_gen_vertex_arrays(GLsizei n, GLuint *arrays);
typedef void WINAPI gl_generate_mipmap(GLenum target);
typedef void WINAPI gl_bind_vertex_array(GLuint array);
typedef void WINAPI gl_vertex_attrib_divisor(GLuint index, GLuint divisor);
typedef void WINAPI gl_vertex_attrib_pointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void WINAPI gl_vertex_attrib_i_pointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void WINAPI gl_enable_vertex_attrib_array(GLuint index);
typedef void WINAPI gl_gen_framebuffers(GLsizei n, GLuint *framebuffers);
typedef void WINAPI gl_framebuffer_texture_2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef GLenum WINAPI gl_check_framebuffer_status(GLenum target);
typedef void WINAPI gl_blit_framebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void WINAPI gl_attach_shader(GLuint program, GLuint shader);
typedef void WINAPI gl_compile_shader(GLuint shader);
typedef GLuint WINAPI gl_create_program(void);
typedef GLuint WINAPI gl_create_shader(GLenum type);
typedef void WINAPI gl_link_program(GLuint program);
typedef void WINAPI gl_shader_source(GLuint shader, GLsizei count, GLchar **string, GLint *length);
typedef void WINAPI gl_use_program(GLuint program);
typedef void WINAPI gl_get_program_info_log(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void WINAPI gl_get_shader_info_log(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void WINAPI gl_validate_program(GLuint program);
typedef void WINAPI gl_get_program_iv(GLuint program, GLenum pname, GLint *params);
typedef GLint WINAPI gl_get_uniform_location (GLuint program, const GLchar *name);
typedef void WINAPI gl_tex_storage_3d (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
typedef void WINAPI gl_tex_sub_image_3d(GLenum target,  GLint level,  GLint xoffset,  GLint yoffset,  GLint zoffset,  GLsizei width,  GLsizei height,  GLsizei depth,  GLenum format,  GLenum type,  const GLvoid * data);
typedef void WINAPI gl_uniform_4fv(GLint location, GLsizei count, const GLfloat *value);
typedef void WINAPI gl_uniform_3fv(GLint location, GLsizei count, const GLfloat *value);
typedef void WINAPI gl_uniform_matrix_4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void WINAPI gl_uniform_1i(GLint location, GLint v0);
typedef void WINAPI gl_uniform_1f(GLint location, GLfloat v0);
typedef void WINAPI gl_multi_draw_elements_indirect(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride);

static gl_active_texture *glActiveTexture;
static gl_tex_image_2d_multisample *glTexImage2DMultisample;
static gl_bind_framebuffer *glBindFramebuffer;
static gl_gen_buffers *glGenBuffers;
static gl_bind_buffer *glBindBuffer;
static gl_buffer_data *glBufferData;
static gl_buffer_sub_data *glBufferSubData;
static gl_bind_vertex_array *glBindVertexArray;
static gl_enable_vertex_attrib_array *glEnableVertexAttribArray;
static gl_vertex_attrib_divisor *glVertexAttribDivisor;
static gl_vertex_attrib_pointer *glVertexAttribPointer;
static gl_vertex_attrib_i_pointer *glVertexAttribIPointer;
static gl_bind_framebuffer *glBindFramebuffer;
static gl_generate_mipmap *glGenerateMipmap;
static gl_gen_vertex_arrays *glGenVertexArrays;
static gl_gen_framebuffers *glGenFramebuffers;
static gl_framebuffer_texture_2D *glFramebufferTexture2D;
static gl_check_framebuffer_status *glCheckFramebufferStatus;
static gl_blit_framebuffer *glBlitFramebuffer;
static gl_attach_shader *glAttachShader;
static gl_compile_shader *glCompileShader;
static gl_create_program *glCreateProgram;
static gl_create_shader *glCreateShader;
static gl_link_program *glLinkProgram;
static gl_shader_source *glShaderSource;
static gl_use_program *glUseProgram;
static gl_get_program_info_log *glGetProgramInfoLog;
static gl_get_shader_info_log *glGetShaderInfoLog;
static gl_validate_program *glValidateProgram;
static gl_get_program_iv *glGetProgramiv;
static gl_get_uniform_location *glGetUniformLocation;
static gl_tex_storage_3d *glTexStorage3D;
static gl_tex_sub_image_3d *glTexSubImage3D;
static gl_uniform_4fv *glUniform4fv;
static gl_uniform_3fv *glUniform3fv;
static gl_uniform_matrix_4fv *glUniformMatrix4fv;
static gl_uniform_1i *glUniform1i;
static gl_uniform_1f *glUniform1f;
static gl_multi_draw_elements_indirect *glMultiDrawElementsIndirect;

static gl_bind_framebuffer *glBindFramebuffer;
static gl_gen_framebuffers *glGenFramebuffers;
static gl_framebuffer_texture_2D *glFramebufferTexture2D;
static gl_check_framebuffer_status *glCheckFramebufferStatus;

typedef BOOL WINAPI wgl_swap_interval_ext(int interval);
typedef const char * WINAPI wgl_get_extensions_string_ext(void);

static wgl_create_context_attribs_arb *wglCreateContextAttribsARB;
static wgl_choose_pixel_format_arb *wglChoosePixelFormatARB;
static wgl_swap_interval_ext *wglSwapIntervalEXT;
static wgl_get_extensions_string_ext *wglGetExtensionsStringEXT;
static bool OpenGLSupportsSRGBFramebuffer = false;
static GLuint OpenGLDefaultInternalTextureFormat;
static GLuint OpenGLReservedBlitTexture;

static int32_t boolRunning;
static int64_t globalPerfCount;

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
		//escape;
	}		
	LARGE_INTEGER size;
	if(GetFileSizeEx(handle, &size)) {
		uint32_t size32 = size.QuadPart;
		result.contents = VirtualAlloc(0, size32, 
							MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		if(!result.contents) {
				//escape;
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
#include "publican.c"

/********************************************************************************
		Platform API Specific Gubbins
********************************************************************************/

extern PLATFORM_GET_ALL_FILES_OF_TYPE_START(win32_GetFilesTypeStart)
{
	struct platform_file_group result = {};
	
	struct win32_file_group *win32FileGroup = (struct win32_file_group *)
		VirtualAlloc(0, sizeof(struct win32_file_group), 
		         	 MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	result.platform = win32FileGroup;

	wchar_t *wildCard = L"*.*";
	switch(type) {
	case filetype_asset: {
		wildCard = L"*.pubb";
		break;
	}
	}
	result.fileCount = 0;
	
	WIN32_FIND_DATAW findData;
	HANDLE findHandle = FindFirstFileW(wildCard, &findData);
	while(findHandle != INVALID_HANDLE_VALUE) {
		++result.fileCount;
		if(!FindNextFileW(findHandle, &findData)) {
			break;
		}
	}
	FindClose(findHandle);
	win32FileGroup->findHandle = FindFirstFileW(wildCard, &win32FileGroup->findData);
	return(result);
}

extern PLATFORM_GET_ALL_FILES_OF_TYPE_END(win32_GetFilesTypeEnd)
{
	struct win32_file_group *win32FileGroup =
			(struct win32_file_group *)fileGroup->platform;
	if(win32FileGroup) {
		FindClose(win32FileGroup->findHandle);
		VirtualFree(win32FileGroup, 0, MEM_RELEASE);
	}		
}

extern PLATFORM_OPEN_FILE(win32_OpenNextFile)
{
	struct win32_file_group *win32FileGroup = 
			(struct win32_file_group *)fileGroup->platform;
	struct platform_file_handle result = {};

	if(win32FileGroup->findHandle != INVALID_HANDLE_VALUE) {
		struct win32_file_handle *win32Handle = (struct win32_file_handle *)
			VirtualAlloc(0, sizeof(struct win32_file_handle),
				 MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		result.platform = win32Handle;
		
		if(win32Handle) {
			wchar_t *fileName = win32FileGroup->findData.cFileName;
			win32Handle->win32Handle = CreateFileW(fileName, GENERIC_READ, 
				FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
			result.noErrors = (win32Handle->win32Handle != INVALID_HANDLE_VALUE);									   
		}
		if(!FindNextFileW(win32FileGroup->findHandle, &win32FileGroup->findData)) {
			FindClose(win32FileGroup->findHandle);
			win32FileGroup->findHandle = INVALID_HANDLE_VALUE;
		}
	}	
	return(result);
}

extern PLATFORM_FILE_ERROR(win32_FileError)
{
	handle->noErrors = false;
	printf("%s\n", message);
}

extern PLATFORM_READ_DATA_FROM_FILE(win32_ReadDataFromFile)
{
	if(PlatformNoFileErrors(source)) {
		struct win32_file_handle *handle = (struct win32_file_handle *)source->platform;
		OVERLAPPED overlapped = {};
		overlapped.Offset = (uint32_t)((offset >> 0) & 0xFFFFFFFF);
		overlapped.OffsetHigh = (uint32_t)((offset >> 32) & 0xFFFFFFFF);
		
		uint32_t fileSize32 = SafeTruncateUInt64(size);
		DWORD bytesRead;
		if(ReadFile(handle->win32Handle, dest, fileSize32, &bytesRead, &overlapped) &&
		   (fileSize32 == bytesRead)) {
				//SUCCESS!!!!!!
		} else {
				win32_FileError(source, "win32_ReadDataFromFile Failed\n");
		}
	}
}

extern void win32_FreeBlock(struct win32_memory_block *block)
{
	BeginTicketMutex(&globalWin32State.memMutex);
	block->prev->next = block->next;
	block->next->prev = block->prev;
	EndTicketMutex(&globalWin32State.memMutex);
	
	BOOL result = VirtualFree(block, 0, MEM_RELEASE);		
	assert(result);
}

PLATFORM_DEALLOCATE_MEMORY(win32_DeAlloc)
{
	if(block) {		
		struct win32_memory_block *win32Block = ((struct win32_memory_block *)block);
		win32_FreeBlock(win32Block);
		
	}		
}

PLATFORM_ALLOCATE_MEMORY(win32_Alloc)
{
	assert(sizeof(struct win32_memory_block) == 64);
	
	uintptr_t pageSize = 4096;
	uintptr_t totalSize = size + sizeof(struct win32_memory_block);
	uintptr_t baseOffset = sizeof(struct win32_memory_block);
	uintptr_t protectOffset = 0;
	if(flags & Platform_UF) {
		totalSize = size + 2 * pageSize;
		baseOffset = 2 * pageSize;
		protectOffset = pageSize;
	} else if(flags & Platform_OF) {
		uintptr_t sizeRoundUp = AlignPow2(size, pageSize);
		totalSize = sizeRoundUp + 2 * pageSize;
		baseOffset = pageSize + sizeRoundUp - size;
		protectOffset = pageSize + sizeRoundUp;
	}
	struct win32_memory_block *block = (struct win32_memory_block *)
			VirtualAlloc(0, totalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		
	assert(block);
	block->block.base = (uint8_t *)block + baseOffset; 
	assert(block->block.used == 0);
	assert(block->block.arenaPrev == 0);
	if(flags & (Platform_UF|Platform_OF)) {
		DWORD oldProtect = 0;
		BOOL boolProtect = VirtualProtect((uint8_t *)block + protectOffset,
				pageSize, PAGE_NOACCESS, &oldProtect);
		assert(boolProtect);	
		
	}
	struct win32_memory_block *sentinal = &globalWin32State.memSentinal;
	block->next = sentinal; 
	block->block.size = size;
	block->block.flags = flags;
	
	BeginTicketMutex(&globalWin32State.memMutex);
	block->prev = sentinal->prev;
	block->prev->next = block;
	block->next->prev = block;
	EndTicketMutex(&globalWin32State.memMutex);
	
	struct platform_memory_block *patBlock = &block->block;
	
	return(patBlock);		
}

static inline LARGE_INTEGER win32_GetWallClock(void)
{
	LARGE_INTEGER result;
	QueryPerformanceCounter(&result);
	return(result);
} 

static inline float win32_GetSecondsElapsed(LARGE_INTEGER start,
					LARGE_INTEGER end) 
{
	float result = ((float)(end.QuadPart - start.QuadPart) /
			(float)globalPerfCount);
	return(result);				
}

static void win32_SetPixelFormat(HDC dc)
{
	int32_t suggestedFormatIndex = 0;
	GLuint extendedPick = 0;
	if(wglChoosePixelFormatARB)	{
		int32_t intAttribList[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,		
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,		
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
			0,
		};			
		assert(OpenGLSupportsSRGBFramebuffer);
		if(!OpenGLSupportsSRGBFramebuffer) {
			intAttribList[10] = 0;
			printf("SRGB Framebuffer Unsupported\n");
		}
		wglChoosePixelFormatARB(dc, intAttribList, 0, 1, 
			&suggestedFormatIndex, &extendedPick);
	}
	if(!extendedPick) {
		PIXELFORMATDESCRIPTOR desiredFormat = {};
		desiredFormat.nSize = sizeof(desiredFormat);
		desiredFormat.nVersion = 1;
		desiredFormat.iPixelType = PFD_TYPE_RGBA;
		desiredFormat.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
		desiredFormat.cColorBits = 32;
		desiredFormat.cAlphaBits = 8;
		desiredFormat.iLayerType = PFD_MAIN_PLANE;		
		suggestedFormatIndex = ChoosePixelFormat(dc, &desiredFormat);
	}
	PIXELFORMATDESCRIPTOR suggestedFormat;
	DescribePixelFormat(dc, suggestedFormatIndex, sizeof(suggestedFormat), 
						&suggestedFormat);
	SetPixelFormat(dc, suggestedFormatIndex, &suggestedFormat);
}

static void win32_LoadWGLExtensions(void)
{
	WNDCLASSA dummy = {};
	dummy.lpfnWndProc = DefWindowProcA;
	dummy.hInstance = GetModuleHandle(0);
	dummy.lpszClassName = "dummyWGL";	
	if(RegisterClassA(&dummy)) {
		HWND window = CreateWindowExA(0, dummy.lpszClassName,
			"Publican",	0, CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,	
			dummy.hInstance, 0);
		
		HDC dc = GetDC(window);
		win32_SetPixelFormat(dc);
		HGLRC openGLRC = wglCreateContext(dc);
		if(wglMakeCurrent(dc, openGLRC)) {
			wglChoosePixelFormatARB = (wgl_choose_pixel_format_arb *)wglGetProcAddress("wglChoosePixelFormatARB");						
			wglCreateContextAttribsARB = (wgl_create_context_attribs_arb *)wglGetProcAddress("wglCreateContextAttribsARB");						
			wglSwapIntervalEXT = (wgl_swap_interval_ext *)wglGetProcAddress("wglSwapIntervalEXT");					
			wglGetExtensionsStringEXT = (wgl_get_extensions_string_ext *)wglGetProcAddress("wglGetExtensionsStringEXT");		
			if(wglGetExtensionsStringEXT) {
				char *extensions = (char *)wglGetExtensionsStringEXT();
				char *at = extensions;
				while(*at) {
						while(IsWhitespace(*at)) {++at;}
					char *end = at;
					while(*end && !IsWhitespace(*end)) {++end;}
					uintptr_t count = end - at;
					
					if(0) {
					} else if(StringsAreEqual(count, at, "WGL_EXT_framebuffer_sRGB")) {
							OpenGLSupportsSRGBFramebuffer = true;
							
					} else if(StringsAreEqual(count, at, "WGL_ARB_framebuffer_sRGB")) {
							OpenGLSupportsSRGBFramebuffer = true;
					}
					at = end;
				}
			}	
			wglMakeCurrent(0, 0);
		} else {
				printf("wglMakeCurrent failed\n");
		}	
		wglDeleteContext(openGLRC);
		ReleaseDC(window, dc);
		DestroyWindow(window);
	}
}

static HGLRC win32_InitOpenGL(HDC dc,
			      struct pub_memory *memory)
{
	win32_LoadWGLExtensions();
	win32_SetPixelFormat(dc);
	bool modernContext = true;
	HGLRC openGLRC = 0;
	if(wglCreateContextAttribsARB) {				
		openGLRC = wglCreateContextAttribsARB(dc, 0, win32_OpenGLAttribs);
	} if(!openGLRC) {
		modernContext = false;
		openGLRC = wglCreateContext(dc);
	} if(wglMakeCurrent(dc, openGLRC)) {
		struct gl_info glInfo = gl_GetInfo(openGLRC);
		if(glInfo.GL_ARB_framebuffer_object) {
			glBindFramebuffer = (gl_bind_framebuffer *)wglGetProcAddress("glBindFramebuffer");
			if(!glBindFramebuffer) {printf("%d\n", (int32_t)GetLastError());}		
			glGenFramebuffers = (gl_gen_framebuffers *)wglGetProcAddress("glGenFramebuffers");
			glFramebufferTexture2D = 
				(gl_framebuffer_texture_2D *)wglGetProcAddress("glFramebufferTexture2D");
			glCheckFramebufferStatus = 
				(gl_check_framebuffer_status *)wglGetProcAddress("glCheckFramebufferStatus");
		}
		glActiveTexture = (gl_active_texture *)wglGetProcAddress("glActiveTexture");
		glTexImage2DMultisample = (gl_tex_image_2d_multisample *)wglGetProcAddress("glTexImage2DMultisample");
		glBlitFramebuffer = (gl_blit_framebuffer *)wglGetProcAddress("glBlitFramebuffer");
		glGenBuffers = (gl_gen_buffers *)wglGetProcAddress("glGenBuffers");
		glBindBuffer = (gl_bind_buffer *)wglGetProcAddress("glBindBuffer");
		glBufferData = (gl_buffer_data *)wglGetProcAddress("glBufferData");
		glBufferSubData = (gl_buffer_sub_data *)wglGetProcAddress("glBufferSubData");
		glGenVertexArrays = (gl_gen_vertex_arrays *)wglGetProcAddress("glGenVertexArrays");
		glBindVertexArray = (gl_bind_vertex_array *)wglGetProcAddress("glBindVertexArray");
		glVertexAttribDivisor = (gl_vertex_attrib_divisor *)wglGetProcAddress("glVertexAttribDivisor");
		glVertexAttribPointer = (gl_vertex_attrib_pointer *)wglGetProcAddress("glVertexAttribPointer");
		glVertexAttribIPointer = (gl_vertex_attrib_i_pointer *)wglGetProcAddress("glVertexAttribIPointer");
		glEnableVertexAttribArray  = (gl_enable_vertex_attrib_array *)wglGetProcAddress("glEnableVertexAttribArray");
		glAttachShader = (gl_attach_shader *)wglGetProcAddress("glAttachShader");
		glCompileShader = (gl_compile_shader *)wglGetProcAddress("glCompileShader");
		glCreateProgram = (gl_create_program *)wglGetProcAddress("glCreateProgram");
		glCreateShader = (gl_create_shader *)wglGetProcAddress("glCreateShader");
		glLinkProgram = (gl_link_program *)wglGetProcAddress("glLinkProgram");
		glShaderSource = (gl_shader_source *)wglGetProcAddress("glShaderSource");
		glUseProgram = (gl_use_program *)wglGetProcAddress("glUseProgram");
		glGenerateMipmap = (gl_generate_mipmap *)wglGetProcAddress("glGenerateMipmap");
		glGetProgramInfoLog = (gl_get_program_info_log *)wglGetProcAddress("glGetProgramInfoLog");
		glGetShaderInfoLog = (gl_get_shader_info_log *)wglGetProcAddress("glGetShaderInfoLog");
		glValidateProgram = (gl_validate_program *)wglGetProcAddress("glValidateProgram");
		glGetProgramiv = (gl_get_program_iv *)wglGetProcAddress("glGetProgramiv");
		glGetUniformLocation = (gl_get_uniform_location *)wglGetProcAddress("glGetUniformLocation");		
		glTexStorage3D = (gl_tex_storage_3d *)wglGetProcAddress("glTexStorage3D");
		glTexSubImage3D = (gl_tex_sub_image_3d *)wglGetProcAddress("glTexSubImage3D");
		glUniform4fv = (gl_uniform_4fv *)wglGetProcAddress("glUniform4fv");
		glUniform3fv = (gl_uniform_3fv *)wglGetProcAddress("glUniform3fv");
		glUniformMatrix4fv = (gl_uniform_matrix_4fv *)wglGetProcAddress("glUniformMatrix4fv");
		glUniform1i = (gl_uniform_1i *)wglGetProcAddress("glUniform1i");
		glUniform1f = (gl_uniform_1f *)wglGetProcAddress("glUniform1f");
		glMultiDrawElementsIndirect = (gl_multi_draw_elements_indirect *)wglGetProcAddress("glMultiDrawElementsIndirect");
		if(wglSwapIntervalEXT) {
			wglSwapIntervalEXT(1);
		}
		glGenTextures(1, &OpenGLReservedBlitTexture);
		glInfo = gl_Init(glInfo, modernContext, OpenGLSupportsSRGBFramebuffer, memory);
	} else {
		printf("%d\n", (int32_t)GetLastError());	
	}
	return(openGLRC);
}

/*
*		Get windows' coords for the current window, and return them in a POINT.
*/
static POINT win32_GetWindowDim(HWND window)
{
	POINT result;
	RECT rect;
	GetClientRect(window, &rect);
	result.x = rect.right - rect.left;
	result.y = rect.bottom - rect.top;
	
	return(result);
}

/*
*		Blt DIB to screen.
*/
static void win32_OutputBuffer(struct render_commands *commands, 
			   HDC dc,
			   struct rect_int drawRegion,
			   uint32_t w,
			   uint32_t h,
			   struct memory_arena *tempArena)
{	
	gl_Output(commands, drawRegion, w, h);
	
	SwapBuffers(dc);		
}
/*
*		"Legit" Proc, for anything not being processed directly 
*		by win32_ProcessMessage.
*/
LRESULT CALLBACK WindowProc(HWND window, 
			UINT message, 
							WPARAM wParam, 
							LPARAM lParam)
{
	LRESULT result = 0;
	
	switch(message) {
	case WM_DESTROY:
	case WM_CLOSE: {
		boolRunning = 0;
		printf("EXIT2\n");
		break;
	} case WM_WINDOWPOSCHANGING: {
		WINDOWPOS *newPos = (WINDOWPOS *)lParam;
		
		RECT winRect;
		RECT clientRect;
		GetWindowRect(window, &winRect);
		GetWindowRect(window, &clientRect);
		
		int32_t clientWidth = (clientRect.right - clientRect.left);
		int32_t clientHeight = (clientRect.bottom - clientRect.top);
		int32_t widthAdd = ((winRect.right - winRect.left) - clientWidth);
		int32_t heightAdd = ((winRect.bottom - winRect.top) - clientHeight);
		
		int32_t renderWidth = clientWidth;
		int32_t renderHeight =clientHeight;
		
		//int32_t sugX = newPos->cx;
		//int32_t sugY = newPos->cy;
		
		int32_t newCx = (renderWidth * (newPos->cy - heightAdd)) / renderHeight;
		int32_t newCy = (renderHeight * (newPos->cx - widthAdd)) / renderWidth;
		
		if(fabsf((float)(newPos->cx - newCx)) < 
		   fabsf((float)(newPos->cy - newCy))) {
				newPos->cx = newCx + widthAdd;
		} else {
				newPos->cy = newCy + heightAdd;
		}
		result = DefWindowProc(window, message, wParam, lParam);
		break;
	}	/*case WM_PAINT: {
		
			PAINTSTRUCT ps;
			HDC dc = BeginPaint(window, &ps);
			
			POINT dim = win32_GetWindowDim(window);
			win32_OutputBuffer(&globalBuffer, dc, dim.x, dim.y);
			EndPaint(window, &ps);
			break;
	} */default:{
			result = DefWindowProc(window, message, wParam, lParam);
	}
	}
	return(result);
}
/*
*		Handles all Windows messages that need to be directly
*		dealt with. Calls the "official" proc by default;
*/
static void win32_ProcessMessage(struct pub_input *input)
{
	MSG message;	
	for(;;) {
		BOOL gotMessage = FALSE;
		gotMessage = PeekMessage(&message, 0, 0, WM_PAINT - 1, PM_REMOVE);
		if(!gotMessage) {
			gotMessage = PeekMessage(&message, 0, WM_PAINT + 1, WM_MOUSEMOVE - 1, PM_REMOVE);
			if(!gotMessage) {
				gotMessage = PeekMessage(&message, 0, WM_MOUSEMOVE + 1, 0xFFFFFFFF, PM_REMOVE);
			}
		}
		if(!gotMessage) {
			break;
		}		
		switch(message.message) {
		case WM_QUIT: {	
			boolRunning = 0;
			printf("EXIT1\n");
			break;
		}
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN: {
			break;
		} case WM_KEYUP: {	
			uint32_t vkCode = (uint32_t)message.wParam;
			
			bool wasDown = ((message.lParam & (1 << 30)) != 0);
			bool isDown = ((message.lParam & (1 << 31)) == 0);
			if(wasDown != isDown) {
				switch(vkCode) {
				case 0x41: {
					input->buttons._A.endedDown = 1;
					break;
				} case 0x42: {
					input->buttons._B.endedDown = 1;
					break;
				} case 0x43: {
					input->buttons._C.endedDown = 1;
					break;
				} case 0x44: {
					input->buttons._D.endedDown = 1;
					break;
				} case 0x46: {
					input->buttons._F.endedDown = 1;
					break;
				} case 0x4E: {
					input->buttons.DIR_NORTH.endedDown = 1;
					break;
				} case 0x4F: {
					input->buttons._O.endedDown = 1;
					break;
				} case 0x50: {
					input->buttons._P.endedDown = 1;
					break;
				} case 0x52: {
					input->buttons._R.endedDown = 1;
					break;
				} case 0x53: {
					input->buttons.DIR_SOUTH.endedDown = 1;
					break;
				} case 0x54: {
					input->buttons._T.endedDown = 1;
					break;
				} case 0x55: {
					input->buttons._U.endedDown = 1;
					break;
				} case 0x57: {
					input->buttons.DIR_WEST.endedDown = 1;
					break;
				} case VK_SHIFT: {
					input->buttons.shift.endedDown = 1;
					break;
				} case VK_MENU: {
					input->buttons.alt.endedDown = 1;										
					break;
				} case VK_CONTROL: {										
					input->buttons.ctrl.endedDown = 1;			
					break;
				} case VK_ESCAPE: {
					input->buttons.esc.endedDown = 1;
					break;
				} case VK_UP: {								
					break;
				} case VK_F1: {	
					input->buttons.f1.endedDown = 1;
					break;
				}default: {
					break;
				}
				}									
			}
			break;
		} case WM_LBUTTONDOWN: {		
				input->buttons.lClick.isDown = true;
				break;						
		} case WM_LBUTTONUP: {		
				input->buttons.lClick.endedDown = true;
				input->buttons.lClick.isDown = false;						
				break;						
		} case WM_RBUTTONUP: {
				input->buttons.rClick.endedDown = 1;
				break;
		} case WM_MOUSEWHEEL: {						
				input->mouseZ += GET_WHEEL_DELTA_WPARAM(message.wParam) / 40.0f;
				break;
		} default: {
				TranslateMessage(&message);
				DispatchMessage(&message);
				break;
		}
		}
	}			
}

struct win32_thread {
	int32_t logicalIndex;
	struct platform_work_queue *queue;
};

static void win32_AddEntry(struct platform_work_queue *queue,
			   platform_work_queue_callback *callback,
			   void *data)
{
	uint32_t newNextEntryToWrite = (queue->nextToWrite + 1) % ARRAY_COUNT(queue->entries);
	assert(newNextEntryToWrite != queue->nextToRead);
	struct platform_work_queue_entry *entry = queue->entries + queue->nextToWrite;
	entry->callback = callback;
	entry->data = data;
	++queue->completionGoal;
	asm ("sfence");
	queue->nextToWrite = newNextEntryToWrite;
	ReleaseSemaphore(queue->semaphoreHandle, 1, 0);
}

static bool win32_DoNextQueueEntry(struct platform_work_queue *queue)								  
{
	bool needSleep = false;
	
	uint32_t originalNextToRead = queue->nextToRead;
	uint32_t newNextToRead = (originalNextToRead + 1) % ARRAY_COUNT(queue->entries);
	if(originalNextToRead != queue->nextToWrite) {
		uint32_t index = InterlockedCompareExchange((LONG volatile *)&queue->nextToRead,
														newNextToRead, originalNextToRead);
		if(index == originalNextToRead) {
			struct platform_work_queue_entry entry = queue->entries[index];
			entry.callback(queue, entry.data);
				InterlockedIncrement((LONG volatile *)&queue->completionCount);
		}											
	} else {
		needSleep = true;
	}
	return(needSleep);
}

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	struct win32_thread_startup *thread = 
			(struct win32_thread_startup *)lpParam;
	struct platform_work_queue *queue = thread->queue;

	uint32_t testThreadID = GetThreadID();
	assert(testThreadID == GetCurrentThreadId());		
	for(;;) {
		if(win32_DoNextQueueEntry(queue)) {
			WaitForSingleObjectEx(queue->semaphoreHandle, INFINITE, FALSE);
		}
	}
}

static void win32_CompleteAllWork(struct platform_work_queue *queue)
{
	while(queue->completionGoal != queue->completionCount) {
		win32_DoNextQueueEntry(queue);
	}
	queue->completionGoal = 0;
	queue->completionCount = 0;
}

static void win32_MakeQueue(struct platform_work_queue *queue,
			    uint32_t threadCount,
			    struct win32_thread_startup *startups)
{
	queue->completionGoal = 0;
	queue->completionCount = 0;
	queue->nextToWrite = 0;
	queue->nextToRead = 0;
	
	uint32_t initialCount = 0;
	queue->semaphoreHandle = CreateSemaphore(0, initialCount, threadCount, 0);
	
	for(uint32_t i = 0; i < threadCount; ++i) {				
		struct win32_thread_startup *startup = startups + i;
		startup->queue = queue;
		
		DWORD threadID;
		HANDLE threadHandle = CreateThread(0, 0, ThreadProc,
						   startups, 0, &threadID);
		CloseHandle(threadHandle);								   
	}												 
}

static void UpdateCursor(HWND window,
			 POINT dim,
			 struct pub_input *input)
{
	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(window, &mousePos);
	input->mouseX = (float)mousePos.x;
	input->mouseY = (float)((dim.y - 1) - mousePos.y);	
}

/*
*		Entry point for Windows. 
*/
int CALLBACK WinMain(HINSTANCE hInstance, 
		     HINSTANCE hPrevInstance, 
		     LPSTR lpCmdLine, 
		     int nCmdShow)
{
	struct win32_state *winState = &globalWin32State;
	winState->memSentinal.prev = &winState->memSentinal;
	winState->memSentinal.next = &winState->memSentinal;		
	
	LARGE_INTEGER perfCountFreq;
	QueryPerformanceFrequency(&perfCountFreq);
	globalPerfCount = perfCountFreq.QuadPart;
	
	UINT desiredMS = 1;
	bool granSleep = (timeBeginPeriod(desiredMS) == TIMERR_NOERROR);	
	
	WNDCLASSEX winClass = {};		
	winClass.cbSize = sizeof(WNDCLASSEX);
	winClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	winClass.lpfnWndProc = WindowProc;
	winClass.hInstance = hInstance;
	winClass.hCursor = LoadCursor(0, IDC_ARROW);
	winClass.lpszClassName = "Pub";
	
	if(!RegisterClassEx(&winClass)) {
#ifdef DEBUG		
		printf("RegisterClassEx failed.\n");
#endif			
		return(1);
	}	
	HWND window = CreateWindowEx(0, "Pub", "Publican",	
		WS_VISIBLE|WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
		CW_USEDEFAULT,	WIN_X, WIN_Y,	
		NULL, NULL, hInstance, NULL);
	if(!window) {
			MessageBox(NULL, TEXT("Window creation failed.\n"), NULL, MB_ICONERROR);
			return(1);
	}		
		
	struct win32_thread_startup highPriorityStart[6] = {};
	struct platform_work_queue highPriorityQueue = {};
	win32_MakeQueue(&highPriorityQueue, ARRAY_COUNT(highPriorityStart), highPriorityStart);
	
	struct win32_thread_startup lowPriorityStart[2] = {};
	struct platform_work_queue lowPriorityQueue = {};
	win32_MakeQueue(&lowPriorityQueue, ARRAY_COUNT(lowPriorityStart), lowPriorityStart);
	
	printf("Threads loaded\n");
	
	float fps = 60;
	float spf = 1.0f / fps;
	
	//LPVOID baseAddress = 0;
	struct pub_memory memory = {};
	memory.highPriorityQueue = &highPriorityQueue;
	memory.lowPriorityQueue = &lowPriorityQueue;
	//		Init platform api function pointers.
	memory.platformAPI.Alloc = win32_Alloc;
	memory.platformAPI.DeAlloc = win32_DeAlloc;		
	memory.platformAPI.AddEntry = win32_AddEntry;		
	memory.platformAPI.CompleteAllWork = win32_CompleteAllWork;		
	memory.platformAPI.GetFilesTypeStart = win32_GetFilesTypeStart;		
	memory.platformAPI.GetFilesTypeEnd = win32_GetFilesTypeEnd;		
	memory.platformAPI.OpenNextFile = win32_OpenNextFile;		
	memory.platformAPI.ReadDataFromFile = win32_ReadDataFromFile;		
	memory.platformAPI.FileError = win32_FileError;		
	
	
	struct pub_input _input = {};
	struct pub_input *input = &_input;
	
	LARGE_INTEGER lastCounter = win32_GetWallClock();			
	
	uint64_t pushBufferSize = MEGABYTES(64);	
	struct platform_memory_block *pushBufferBlock = win32_Alloc(pushBufferSize, Platformdir_northotRestored);
	uint8_t *pushBase = pushBufferBlock->base;
	
	uint32_t textureOpCount = 1024;
	struct platform_texture_op_queue *textureOpQueue = &memory.textureOpQueue;
	textureOpQueue->firstFree = (struct texture_op *)VirtualAlloc(0, sizeof(struct texture_op) * textureOpCount,
																  MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	for(uint32_t opIndex = 0; opIndex < (textureOpCount - 1); ++opIndex) {
			struct texture_op *op = textureOpQueue->firstFree + opIndex;
			op->next = textureOpQueue->firstFree + opIndex + 1;
	}															  
	
	platform = memory.platformAPI;
	globalDC = GetDC(window);
	globalHGLRC = win32_InitOpenGL(globalDC, &memory);
	
	ShowCursor(FALSE);
	
	boolRunning = 1;
	while(boolRunning) {				
		
		POINT dim = win32_GetWindowDim(window);
		struct render_commands commands = {};
		commands.w = dim.x;
		commands.h = dim.y;
		commands.maxRenderTargetIndex = 1;
		commands.entryCount = 0;
		commands.maxPushSize = pushBufferSize;
		commands.pushBase = pushBase;
		commands.pushData = pushBase;
		
		struct rect_int drawRegion = IntToRect(0, 0, dim.x, dim.y);
		render_AspectFit(commands.w, commands.h, dim.x, dim.y);									
		
		UpdateCursor(window, dim, input);
		
		win32_ProcessMessage(input);		
		
		pub_MainLoop(&memory, input, &commands);
		
		LARGE_INTEGER workCounter = win32_GetWallClock();
		float workSecondsElapsed = win32_GetSecondsElapsed(lastCounter, workCounter);				
		float secondsElapsedForFrame = workSecondsElapsed;
		
		if(secondsElapsedForFrame < spf) {
			DWORD sleepMS = (DWORD)(1000.0f * 
			(spf - secondsElapsedForFrame));
			if(granSleep) {
				if(sleepMS > 0.0f) Sleep(sleepMS);		
			}						
			while(secondsElapsedForFrame < spf) {
				secondsElapsedForFrame = 
					win32_GetSecondsElapsed(lastCounter, win32_GetWallClock());									
			}
		}
		LARGE_INTEGER endCounter = win32_GetWallClock();
		lastCounter = endCounter;
		//flipCounter = win32_GetWallClock();				
		
		BeginTicketMutex(&textureOpQueue->mutex);
		struct texture_op *firstTextureOp = textureOpQueue->first;
		struct texture_op *lastTextureOp = textureOpQueue->last;
		textureOpQueue->last = textureOpQueue->first = 0;
		EndTicketMutex(&textureOpQueue->mutex);
		
		if(firstTextureOp) {
			assert(lastTextureOp);
			gl_ManageTextures(firstTextureOp);
			BeginTicketMutex(&textureOpQueue->mutex);
			lastTextureOp->next = textureOpQueue->firstFree;
			textureOpQueue->firstFree = firstTextureOp;
			EndTicketMutex(&textureOpQueue->mutex);
		}				
		HDC dc = GetDC(window);
		struct memory_arena frameArena = {};
		win32_OutputBuffer(&commands, dc, drawRegion, 
				 dim.x, dim.y, &frameArena);
		ReleaseDC(window, dc);
		
		
		input->mouseZ = 0;
		
		
	}
	return(0);
}
