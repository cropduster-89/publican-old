/************************************************************************************		
__________     ___.   .__  .__                      	
\______   \__ _\_ |__ |  | |__| ____ _____    ____  
 |     ___/  |  \ __ \|  | |  |/ ___\\__  \  /    \ 	OpenGL Renderer
 |    |   |  |  / \_\ \  |_|  \  \___ / __ \|   |  \
 |____|   |____/|___  /____/__|\___  >____  /___|  /
                    \/             \/     \/     \/ 
		    @TODO move all non opengl stuff out to render.c
		    @TODO get better at OpenGL
*************************************************************************************/

static uint32_t globalFrameBufferCount = 0;
static GLuint globalFrameBufferHandles[256] = {0};
static GLuint globalFrameBufferTexture[256] = {0};

#define GL_FRAMEBUFFER                    0x8D40
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5

#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_SRGB_ALPHA                     0x8C42
#define GL_SRGB8_ALPHA8                   0x8C43

#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_VALIDATE_STATUS                0x8B83

#define GL_FRAMEBUFFER                    0x8D40
#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5

#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
#define GL_DEPTH_COMPONENT32F             0x8CAC
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_MULTISAMPLE                    0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_SAMPLE_ALPHA_TO_ONE            0x809F
#define GL_SAMPLE_COVERAGE                0x80A0
#define GL_SAMPLE_BUFFERS                 0x80A8
#define GL_SAMPLES                        0x80A9
#define GL_DRAW_INDIRECT_BUFFER           0x8F3F
#define GL_SAMPLE_COVERAGE_VALUE          0x80AA
#define GL_SAMPLE_COVERAGE_INVERT         0x80AB
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE_2D_ARRAY               0x8C1A
#define GL_TEXTURE_2D_MULTISAMPLE         0x9100
#define GL_MAX_SAMPLES                    0x8D57
#define GL_MAX_COLOR_TEXTURE_SAMPLES      0x910E
#define GL_MAX_DEPTH_TEXTURE_SAMPLES      0x910F

#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893

#define GL_STATIC_DRAW                    0x88E4

#define GL_UNSIGNED_INT_8_8_8_8           0x8035
#define GL_UNSIGNED_INT_8_8_8_8_REV       0x8367

enum render_program {
	render_program_mesh,
	render_program_colour,
	render_program_sprite,
};

struct gl_runtime {		
	struct render_state *rState;
	
	GLuint VAO[3];
	
	GLuint vertexBuffer;
	GLuint indexBuffer;	
	GLuint transformBuffer;	
	GLuint indirectBuffer;	
	GLuint genericBuffer;
	GLuint texOffsetBuffer;
	
	GLuint texArray;
	int32_t texArrayCount;
		
	GLuint normal;
	GLuint colour;
	GLuint sprite;
	struct mat4 proj;
	float mouse[3];
	
} glRuntime;

struct gl_info {
	int32_t properContext;
	
	char *vendor;
	char *renderer;
	char *version;
	char *slVersion;
	char *ext;
	
	bool GL_EXT_texture_sRGB;
	bool GL_EXT_framebuffer_sRGB;
	bool GL_ARB_framebuffer_sRGB;
	bool GL_ARB_framebuffer_object;
};

static bool gl_GetError(void)
{
	uint32_t error = glGetError();
	char *name = 0;
	
	if(error) {
		switch(error) {
		case 0x0500: {
			name = "GL_INVALID_ENUM";
			break;
		} case 0x0501: {
			name = "GL_INVALID_VALUE";
			break;
		} case 0x0502: {
			name = "GL_INVALID_OPERATION";
			break;
		} case 0x0503: {
			name = "GL_STACK_OVERFLOW";
			break;
		} case 0x0504: {
			name = "GL_STACK_UNDERFLOW";
			break;
		} case 0x0505: {
			name = "GL_OUT_OF_MEMORY";
			break;
		} case 0x0506: {
			name = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
		} case 0x0507: {
			name = "GL_CONTEXT_LOST";
			break;
		} case 0x8031: {
			name = "GL_TABLE_TOO_LARGE1";
			break;
		}
		}
		printf("ERROR: %s\n", name);
	}	
	return(error);
}

static struct gl_info gl_GetInfo(bool modernContext)
{
	struct gl_info result = {};
	result.properContext = modernContext;
	result.vendor = (char *)glGetString(GL_VENDOR);
	result.renderer = (char *)glGetString(GL_RENDERER);
	result.version = (char *)glGetString(GL_VERSION);
	if(result.properContext) {
		result.slVersion = 
			(char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
	} else {
		result.slVersion = "(none)";
	}
	result.ext = (char *)glGetString(GL_EXTENSIONS);
	for(char *at = result.ext; *at; ) {
		while(IsWhitespace(*at)) {++at;}
		char *end = at;
		while(*end && !IsWhitespace(*end)) {++end;}
		
		uintptr_t count = end - at;
		
		if(0) {
		} else if(StringsAreEqual(count, at, "GL_EXT_texture_sRGB")) {
			result.GL_EXT_texture_sRGB = true;			
		} else if(StringsAreEqual(count, at, "GL_EXT_framebuffer_sRGB")) {
			result.GL_EXT_framebuffer_sRGB = true;
		} else if(StringsAreEqual(count, at, "GL_ARB_framebuffer_sRGB")) {
			result.GL_ARB_framebuffer_sRGB = true;
		} else if(StringsAreEqual(count, at, "GL_ARB_framebuffer_object")) {
			result.GL_ARB_framebuffer_object = true;
		}
		at = end;				
	}
	char *majorAt = result.version;
	char *minorAt = 0;
	for(char *at = result.version; *at; ++at) {
		if(at[0] == '.') {
			minorAt = at + 1;
			break;
		}
	}
	int32_t major = 1;
	int32_t minor = 0;
	if(minorAt) {
		major = IntFromZ(majorAt);
		minor = IntFromZ(minorAt);
	}
	if((major > 2) || ((major == 2) && (minor >= 1))) {
		result.GL_EXT_texture_sRGB = true;
	}
	return(result);
}

static GLuint gl_CreateProgram(const char **vertexShader, const char **fragmentShader)
{
	GLuint vs_ID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs_ID, ARRAY_COUNT(vertexShader), (GLchar **)vertexShader, 0);
	glCompileShader(vs_ID);
	
	GLuint fs_ID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs_ID, ARRAY_COUNT(fragmentShader), (GLchar **)fragmentShader, 0);
	glCompileShader(fs_ID);
	
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vs_ID);
	glAttachShader(programID, fs_ID);
	glLinkProgram(programID);

	glValidateProgram(programID);
	GLint linked = 0;
	glGetProgramiv(programID, GL_LINK_STATUS, &linked);
	if(!linked) {
		GLsizei ignored;
		char vsErrors[4096];				
		char fsErrors[4096];				
		char pgErrors[4096];
		glGetShaderInfoLog(vs_ID, sizeof(vsErrors), &ignored, vsErrors);	
		glGetShaderInfoLog(fs_ID, sizeof(fsErrors), &ignored, fsErrors);	
		glGetProgramInfoLog(programID, sizeof(pgErrors), &ignored, pgErrors);
		INVALID_PATH;	
	}
	return(programID);
}

extern struct gl_info gl_Init(struct gl_info info, 
			      bool modernContext, 
			      bool SRGBSupported,
			      struct pub_memory *memory)
{
	OpenGLDefaultInternalTextureFormat = GL_RGBA8;
	if(SRGBSupported && info.GL_EXT_texture_sRGB && info.GL_EXT_framebuffer_sRGB) {
			OpenGLDefaultInternalTextureFormat = GL_SRGB8_ALPHA8;
			glEnable(GL_FRAMEBUFFER_SRGB);			
	} 
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	static const char *vsMesh[] = {
		"#version 450 core							\n"
		"layout (location = 0) in vec3 inPos;					\n"
		"layout (location = 1) in vec3 inNormal;				\n"
		"layout (location = 2) in vec2 inUV;					\n"
		"layout (location = 3) in int inArrayID;				\n"
		"layout (location = 4) in mat4 model;					\n"
		"									\n"
		"out vec2 uv;								\n"
		"out vec3 normal;							\n"
		"out vec3 pos;								\n"
		"flat out int arrayID;							\n"
		"									\n"
		"layout (location = 1) uniform mat4 proj; 				\n"
		"uniform vec3 canonPos;							\n"
		"									\n"
		"void main(void) {							\n"
		"	gl_Position = proj * model * vec4(inPos, 1.0f);			\n"
		"	pos = (model * gl_Position).xyz;				\n"
		"	arrayID = inArrayID;						\n"
		"	uv = inUV;							\n"
		"	normal = (model * vec4(inNormal, 0.0f)).xyz;			\n"
		"}									\n"
	};
	
	static const char *fsMesh[] = {				
		"#version 450 core							\n"
		"out vec4 fragColour;							\n"
		"									\n"
		"in vec2 uv;								\n"
		"in vec3 normal;							\n"
		"in vec3 pos;								\n"
		"flat in int arrayID;							\n"
		"									\n"
		"layout (location = 0) uniform sampler2DArray texture1;			\n"
		"uniform vec3 mouseP;							\n"
		"									\n"
		"void main(void) {							\n"
		"	float ambientLight = 1.2;					\n"
		"									\n"
		"	vec3 lightP = vec3(mouseP.x, mouseP.y, mouseP.z + 2.5);		\n"
		"	float stren = 1.2;						\n"
		"									\n"
		"	vec3 norm = normalize(normal);					\n"
		"	vec3 lightDir = normalize(lightP - pos);			\n"
		"	vec3 lightCol = vec3(1, 0.95, 0.8);				\n"
		"									\n"
		"	float diff = max(dot(norm, lightDir), 0.0);			\n"
		"	vec3 diffuse = (diff * lightCol) * stren;			\n"
		"									\n"
		"	fragColour = texture(texture1, vec3(uv, arrayID));		\n"
		"	vec3 result = (ambientLight + diffuse); 			\n"
		"									\n"
		"									\n"
		"									\n"
		"	fragColour.rgb = fragColour.rgb * result;			\n"
		"}									\n"
	};
	
	static const char *vs_Sprite[] = {
		"#version 450 core							\n"
		"layout (location = 0) in vec3 inPos;					\n"				
		"layout (location = 1) in vec2 inTexCoord;				\n"
		"									\n"
		"out vec2 texCoord;							\n"
		"									\n"                                                                
		"out vec3 pos;								\n"
		"									\n"
		"uniform mat4 projection;						\n"
		"									\n"
		"void main(void) {							\n"
		"	vec3 newPos = inPos;						\n"
		"									\n"
		"	gl_Position = projection * vec4(newPos, 1.0f);			\n"
		"	pos = newPos;							\n"
		"	texCoord = vec2(inTexCoord.x, inTexCoord.y);			\n"		
		"}									\n"
	};
	
	static const char *fs_Sprite[] = {				
		"#version 450 core							\n"
		"out vec4 fragColour;							\n"
		"									\n"
		"in vec2 texCoord;							\n"				
		"in vec3 pos;								\n"
		"									\n"
		"uniform float zBias;							\n"
		"uniform sampler2D texture1;						\n"				
		"									\n"			
		"void main(void) {							\n"				
		"									\n"
		"	float ambientLight = 1.1;					\n"	
		"	vec3 newPos = pos;						\n"
		"	gl_FragDepth = gl_FragCoord.z - zBias;				\n"
		"	fragColour = texture(texture1, texCoord);			\n"	
		"									\n"
		"									\n"
		"									\n"
		"	fragColour.rgb = fragColour.rgb * ambientLight;			\n"			
		"}									\n"
	};
	
	static const char *vs_Colour[] = {
		"#version 450 core							\n"
		"layout (location = 0) in vec3 inPos;					\n"				
		"layout (location = 1) in vec4 inColour;				\n"					
		"									\n"
		"flat out vec4 entColour;						\n"
		"//noperspective out vec4 entColour;					\n"
		"//smooth out vec4 entColour;						\n"
		"uniform mat4 projection;						\n"
		"									\n"
		"									\n"
		"void main(void) {							\n"
		"	entColour = inColour;						\n"
		"	gl_Position = projection * vec4(inPos, 1.0f);			\n"			
		"}									\n"
	};
	
	static const char *fs_Colour[] = {				
		"#version 450 core							\n"
		"out vec4 fragColour;							\n"
		"									\n"
		"flat in vec4 entColour;						\n"
		"//noperspective in vec4 entColour;					\n"
		"//smooth in vec4 entColour;						\n"
		"									\n"							
		"void main(void) {							\n"
		"	fragColour = entColour;						\n"
		"}									\n"
	};
	
	glRuntime.normal = gl_CreateProgram(vsMesh, fsMesh);
	glRuntime.colour = gl_CreateProgram(vs_Colour, fs_Colour);
	glRuntime.sprite = gl_CreateProgram(vs_Sprite, fs_Sprite);	
	
	glGenVertexArrays(3, &glRuntime.VAO[0]);		
	glBindVertexArray(glRuntime.VAO[render_program_mesh]);	
	
	glGenBuffers(1, &glRuntime.indexBuffer);
	glGenBuffers(1, &glRuntime.vertexBuffer);		
	glGenBuffers(1, &glRuntime.transformBuffer);
	glGenBuffers(1, &glRuntime.indirectBuffer);
	glGenBuffers(1, &glRuntime.genericBuffer);
	glGenBuffers(1, &glRuntime.texOffsetBuffer);
	assert(!gl_GetError());		
	
	glBindBuffer(GL_ARRAY_BUFFER, glRuntime.genericBuffer);	
	glBindVertexArray(glRuntime.VAO[render_program_colour]);
	assert(!gl_GetError());
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
		7 * sizeof(float), (void *)0);
		assert(!gl_GetError());
	glEnableVertexAttribArray(0);	
	assert(!gl_GetError());
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 
		7 * sizeof(float), (void *)(3 * sizeof(float)));
	assert(!gl_GetError());
	glEnableVertexAttribArray(1);	
	assert(!gl_GetError());
	glBindVertexArray(glRuntime.VAO[render_program_sprite]);	
	assert(!gl_GetError());
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
		5 * sizeof(float), (void *)0);
	assert(!gl_GetError());	
	glEnableVertexAttribArray(0);	
	assert(!gl_GetError());
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 
		5 * sizeof(float), (void *)(3 * sizeof(float)));
	assert(!gl_GetError());	
	glEnableVertexAttribArray(1);	
	assert(!gl_GetError());
		
	glGenTextures(1, &glRuntime.texArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, glRuntime.texArray);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 256, 256, 256);
	assert(!gl_GetError());
	
	printf("%s\n", glGetString(GL_VERSION));
	 	
	return(info);	
}

static inline float SafeRatioN(float num, float div, float n)
{
	float result = n;
	if(div != 0.0f) {
		result = num / div;
	}
	return(result);
}

static inline float SafeRatio1(float num, float div)
{
	float result = SafeRatioN(num, div, 1.0f);
	return(result);
}

static inline float SafeRatio0(float num, float div)
{
	float result = SafeRatioN(num, div, 0.0f);
	return(result);
}

static void gl_BindFramebuffer(uint32_t index, struct rect_int drawRegion)
{
	uint32_t wW = GetWidthI(drawRegion);
	uint32_t wH = GetHeightI(drawRegion);
	
	glBindFramebuffer(GL_FRAMEBUFFER, globalFrameBufferHandles[index]);		
	glViewport(0, 0, wW, wH);	
}

static void *gl_AddToTexArray(void *data)
{
	assert(!gl_GetError());		
	glBindTexture(GL_TEXTURE_2D_ARRAY, glRuntime.texArray);
	
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, glRuntime.texArrayCount++, 256, 256, 1,
		GL_RGBA, GL_UNSIGNED_BYTE, data);
	
	assert(!gl_GetError());	
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);			 
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
	
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP);	
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP);
	assert(!gl_GetError());
	void *result = PointerFromU32(void, glRuntime.texArray);
	return(result);	
}

static void *gl_AllocateTexture(uint32_t w, uint32_t h, void *data)
{
	GLuint handle;
	
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexImage2D(GL_TEXTURE_2D, 0, OpenGLDefaultInternalTextureFormat, w, h, 0, GL_RGBA, 
				 GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);			 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);				 
	
	glBindTexture(GL_TEXTURE_2D, 0);

	assert(sizeof(handle) <= sizeof(void *));
	
	void *result = PointerFromU32(void, handle);
	
	return(result);
}

static void gl_MultiRenderMesh(
	struct render_commands *commands)
{
	int32_t meshes = commands->meshCount;
	
	glBindVertexArray(glRuntime.VAO[render_program_mesh]);	
		
	glBindBuffer(GL_ARRAY_BUFFER, glRuntime.vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, (size_t)(commands->vertexData - commands->vertexBase),
		commands->vertexBase, GL_STATIC_DRAW);
		
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
		8 * sizeof(float), (void *)0);		
	glEnableVertexAttribArray(0);				
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 
		8 * sizeof(float), (void *)(3 * sizeof(float)));	   
	glEnableVertexAttribArray(1);		
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 
		8 * sizeof(float), (void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(2); 	

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glRuntime.indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (size_t)(commands->elementData - commands->elementBase),
		commands->elementBase, GL_STATIC_DRAW);
	
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, glRuntime.indirectBuffer);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(struct gl_mesh_render_command) *
		meshes, commands->meshCommands, GL_DYNAMIC_DRAW);	
		
	glBindBuffer(GL_ARRAY_BUFFER, glRuntime.texOffsetBuffer);
	glBufferData(GL_ARRAY_BUFFER, meshes * _32BIT,
		commands->texArrayOffsets, GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);	
	glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(uint32_t), (void * )0);
	glVertexAttribDivisor(3, 1);
	
	glBindBuffer(GL_ARRAY_BUFFER, glRuntime.transformBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(struct mat4) * meshes, 
		commands->transforms, GL_STATIC_DRAW);
	glEnableVertexAttribArray(4 + 0);
	glEnableVertexAttribArray(4 + 1);
	glEnableVertexAttribArray(4 + 2);
	glEnableVertexAttribArray(4 + 3);
	glVertexAttribPointer(4 + 0, 4, GL_FLOAT, GL_FALSE, sizeof(struct mat4), (void *)0); 
	glVertexAttribPointer(4 + 1, 4, GL_FLOAT, GL_FALSE, sizeof(struct mat4), (void *)(sizeof(float) * 4)); 
	glVertexAttribPointer(4 + 2, 4, GL_FLOAT, GL_FALSE, sizeof(struct mat4), (void *)(sizeof(float) * 8)); 
	glVertexAttribPointer(4 + 3, 4, GL_FLOAT, GL_FALSE, sizeof(struct mat4), (void *)(sizeof(float) * 12)); 
	glVertexAttribDivisor(4 + 0, 1);
	glVertexAttribDivisor(4 + 1, 1);
	glVertexAttribDivisor(4 + 2, 1);
	glVertexAttribDivisor(4 + 3, 1);
	
	glUseProgram(glRuntime.normal);	 
	glUniformMatrix4fv(1, 1, GL_TRUE, &glRuntime.proj.e[0][0]);
	glUniform3fv(glGetUniformLocation(glRuntime.normal, "mouseP"), 
					1, glRuntime.mouse);

	glBindTexture(GL_TEXTURE_2D_ARRAY, (GLuint)U32FromPointer(glRuntime.texArray));
	
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, meshes, 0);
}

extern void gl_Output(struct render_commands *commands,			      
		      struct rect_int drawRegion,
		      uint32_t wW, uint32_t wH)
{		
	TIMED_START(render);

	glDepthMask(GL_TRUE);		
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glAlphaFunc(GL_GREATER, 0.0f);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	glEnable(GL_SAMPLE_ALPHA_TO_ONE);
	glEnable(GL_MULTISAMPLE);
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glEnable(GL_TEXTURE_2D);		
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);		
	
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);		
					
	bool useRenderTargets = (glBindFramebuffer != 0);
	assert(useRenderTargets);
	uint32_t maxRenderTargets = commands->maxRenderTargetIndex;	
	
	if(maxRenderTargets >= globalFrameBufferCount) {
		uint32_t newFrameBufferCount = commands->maxRenderTargetIndex + 1;
		assert(newFrameBufferCount < ARRAY_COUNT(globalFrameBufferHandles));
		uint32_t newCount = newFrameBufferCount - globalFrameBufferCount;
		glGenFramebuffers(newCount, globalFrameBufferHandles + globalFrameBufferCount);
		for(uint32_t i = globalFrameBufferCount; i < newFrameBufferCount; ++i) {			
			
			assert(!gl_GetError());
			
			GLint maxSampleCount;
			glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &maxSampleCount);
			if(maxSampleCount > 16) {
				maxSampleCount = 16;
			}					
#if 1						
			GLuint slot = GL_TEXTURE_2D_MULTISAMPLE;
#else					
			GLuint slot = GL_TEXTURE_2D;
#endif	
			GLuint texHandle[2];
			glGenTextures(2, texHandle);
			glBindTexture(slot, texHandle[0]);					
			assert(!gl_GetError());
			
			if(slot == GL_TEXTURE_2D_MULTISAMPLE) {
				glTexImage2DMultisample(slot, maxSampleCount, GL_RGBA8, 
							GetWidthI(drawRegion), GetHeightI(drawRegion),
							GL_FALSE);
			} else {
				glTexImage2D(slot, 0, GL_RGBA8, 
					GetWidthI(drawRegion), GetHeightI(drawRegion),
					0,  GL_RGBA, GL_UNSIGNED_BYTE, 0);

			}						
			assert(!gl_GetError());
			
			glBindTexture(slot, texHandle[1]);
			glTexImage2DMultisample(slot, maxSampleCount, GL_DEPTH_COMPONENT16, 
						GetWidthI(drawRegion), GetHeightI(drawRegion),
						GL_FALSE);
			
			assert(!gl_GetError());

			glBindTexture(slot, 0);

			globalFrameBufferTexture[i] = texHandle[0];
			glBindFramebuffer(GL_FRAMEBUFFER, globalFrameBufferHandles[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
								   slot, texHandle[0], 0);	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
								   slot, texHandle[1], 0);				
			
			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			assert(status == GL_FRAMEBUFFER_COMPLETE);
			
		}
		globalFrameBufferCount = newFrameBufferCount;
	}
	for(uint32_t i = 0; i <= maxRenderTargets; ++i) {
		
		if(useRenderTargets) {
			gl_BindFramebuffer(i, drawRegion);
		}
		
		if(i == 0) {
			glScissor(0, 0, wW, wH);
		} else {
			glScissor(0, 0, GetWidthI(drawRegion), GetHeightI(drawRegion));
		}
		glClearDepth(1.0f);
		glClearColor(0.2f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	}	
	
	float clipScaleX = SafeRatio0((float)GetWidthI(drawRegion), (float)commands->w);
	float clipScaleY = SafeRatio0((float)GetHeightI(drawRegion), (float)commands->h);
		
	uint32_t currentRenderTargetIndex = 0xFFFFFFFF;
	for(uint8_t *headerAt = commands->pushBase;
		headerAt < commands->pushData; 
		headerAt += sizeof(struct render_entry_header)) {				
				
		struct render_entry_header *header = (struct render_entry_header *)headerAt;	
		struct render_setup *setup = &header->setup;
				
		if(useRenderTargets) {				
			assert(!gl_GetError());
			
			if(currentRenderTargetIndex != setup->renderTargetIndex) {
				
				if(useRenderTargets) {	
					currentRenderTargetIndex = setup->renderTargetIndex;
					assert(currentRenderTargetIndex <= maxRenderTargets);	
					gl_BindFramebuffer(currentRenderTargetIndex, drawRegion);
				}
			}
			struct rect_int clipRect = setup->clipRect;
			clipRect.minX = Float2Int(clipScaleX * clipRect.minX);
			clipRect.maxX = Float2Int(clipScaleX * clipRect.maxX);
			clipRect.minY = Float2Int(clipScaleY * clipRect.minY);
			clipRect.maxY = Float2Int(clipScaleY * clipRect.maxY);
			
			if(!useRenderTargets||(setup->renderTargetIndex == 0)) {
				clipRect = OffsetI(clipRect, drawRegion.minX, drawRegion.minY);
			}
			glScissor(clipRect.minX, clipRect.minY, 
					  clipRect.maxX - clipRect.minX,
					  clipRect.maxY - clipRect.minY);
				
			assert(!gl_GetError());		
			void *data = (uint8_t *)header + sizeof(*header);
			switch(header->type) {
			case entry_mouse: {
				headerAt += sizeof(struct render_entry_mouse);	
				struct render_entry_mouse *entry = (struct render_entry_mouse *)data;
				assert(entry);
				
				glRuntime.mouse[0] = entry->pos.x;
				glRuntime.mouse[1] = entry->pos.y;
				glRuntime.mouse[2] = entry->pos.z;											
				
				break;
				
			} case entry_rect: {		
				headerAt += sizeof(struct render_entry_rect);					
				
				struct render_entry_rect *entry = (struct render_entry_rect *)data;
				assert(entry);
				
				union vec3 minXMinY = entry->pos;
				union vec3 minXMaxY = RealToVec3(
					entry->pos.x, entry->pos.y + entry->dim.y, entry->pos.z);								
				union vec3 maxXMinY = RealToVec3(
					entry->pos.x + entry->dim.x, entry->pos.y, entry->pos.z);
				union vec3 maxXMaxY = RealToVec3(
					entry->pos.x + entry->dim.x, 
					entry->pos.y + entry->dim.y, entry->pos.z);
				
				union vec4 col = entry->colour;								
				float vertices[] = {
					minXMinY.x, minXMinY.y, minXMinY.z, col.r, col.g, col.b, col.a, 
					maxXMinY.x, maxXMinY.y, maxXMinY.z, col.r, col.g, col.b, col.a,
					maxXMaxY.x, maxXMaxY.y, maxXMaxY.z, col.r, col.g, col.b, col.a,
					                                    
					minXMinY.x, minXMinY.y, minXMinY.z, col.r, col.g, col.b, col.a,
					maxXMaxY.x, maxXMaxY.y, maxXMaxY.z, col.r, col.g, col.b, col.a,
					minXMaxY.x, minXMaxY.y, minXMaxY.z, col.r, col.g, col.b, col.a,
				};
				glBindBuffer(GL_ARRAY_BUFFER, glRuntime.genericBuffer);		
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
				glUseProgram(glRuntime.colour);				
								
		        	glBindVertexArray(glRuntime.VAO[render_program_colour]);	     
										
				glUniformMatrix4fv(glGetUniformLocation(glRuntime.colour, "projection"), 
					1, GL_TRUE, &setup->proj.e[0][0]);
												
				glDrawArrays(GL_TRIANGLES, 0, 6);								
				break;
				
			} case entry_cube: {
				headerAt += sizeof(struct render_entry_cube);
				struct render_entry_cube *cube = (struct render_entry_cube *)data;
				
				float nX = cube->pos.x - cube->radius;
				float pX = cube->pos.x + cube->radius;
				float nY = cube->pos.y - cube->radius;
				float pY = cube->pos.y + cube->radius;
				float nZ = cube->pos.z - cube->height;
				float pZ = cube->pos.z;				
				
				union vec3 p0 = {.x = nX, .y = nY, .z = pZ};				
				union vec3 p1 = {.x = pX, .y = nY, .z = pZ};				
				union vec3 p2 = {.x = pX, .y = pY, .z = pZ};				
				union vec3 p3 = {.x = nX, .y = pY, .z = pZ};				
				union vec3 p4 = {.x = nX, .y = nY, .z = nZ};				
				union vec3 p5 = {.x = pX, .y = nY, .z = nZ};				
				union vec3 p6 = {.x = pX, .y = pY, .z = nZ};				
				union vec3 p7 = {.x = nX, .y = pY, .z = nZ};				
				
				const union vec4 entColour = {
					.r = (float)(cube->colour & 0xFF) / 255.0f,
					.g = (float)((cube->colour >> 8) & 0xFF) / 255.0f,
					.b = (float)((cube->colour >> 16) & 0xFF) / 255.0f,
					.a = (float)((cube->colour >> 24) & 0xFF) / 255.0f,										
				};
				float vertices[] = {
					//
					p0.x, p0.y, p0.z, entColour.r, entColour.g, entColour.b, entColour.a, 
					p1.x, p1.y, p1.z, entColour.r, entColour.g, entColour.b, entColour.a,
					p2.x, p2.y, p2.z, entColour.r, entColour.g, entColour.b, entColour.a,
					                        
					p0.x, p0.y, p0.z, entColour.r, entColour.g, entColour.b, entColour.a,
					p2.x, p2.y, p2.z, entColour.r, entColour.g, entColour.b, entColour.a,
					p3.x, p3.y, p3.z, entColour.r, entColour.g, entColour.b, entColour.a,
					//
					p7.x, p7.y, p7.z, entColour.r, entColour.g, entColour.b, entColour.a, 
					p6.x, p6.y, p6.z, entColour.r, entColour.g, entColour.b, entColour.a,
					p5.x, p5.y, p5.z, entColour.r, entColour.g, entColour.b, entColour.a,
					                        
					p7.x, p7.y, p7.z, entColour.r, entColour.g, entColour.b, entColour.a,
					p5.x, p5.y, p5.z, entColour.r, entColour.g, entColour.b, entColour.a,
					p4.x, p4.y, p4.z, entColour.r, entColour.g, entColour.b, entColour.a,
					//
					p4.x, p4.y, p4.z, entColour.r, entColour.g, entColour.b, entColour.a, 
					p5.x, p5.y, p5.z, entColour.r, entColour.g, entColour.b, entColour.a,
					p1.x, p1.y, p1.z, entColour.r, entColour.g, entColour.b, entColour.a,
					                        
					p4.x, p4.y, p4.z, entColour.r, entColour.g, entColour.b, entColour.a,
					p5.x, p5.y, p5.z, entColour.r, entColour.g, entColour.b, entColour.a,
					p0.x, p0.y, p0.z, entColour.r, entColour.g, entColour.b, entColour.a,
					//
					p2.x, p2.y, p2.z, entColour.r, entColour.g, entColour.b, entColour.a, 
					p6.x, p6.y, p6.z, entColour.r, entColour.g, entColour.b, entColour.a,
					p7.x, p7.y, p7.z, entColour.r, entColour.g, entColour.b, entColour.a,
					                        
					p2.x, p2.y, p2.z, entColour.r, entColour.g, entColour.b, entColour.a,
					p7.x, p7.y, p7.z, entColour.r, entColour.g, entColour.b, entColour.a,
					p3.x, p3.y, p3.z, entColour.r, entColour.g, entColour.b, entColour.a,
					//
					p1.x, p1.y, p1.z, entColour.r, entColour.g, entColour.b, entColour.a, 
					p5.x, p5.y, p5.z, entColour.r, entColour.g, entColour.b, entColour.a,
					p6.x, p6.y, p6.z, entColour.r, entColour.g, entColour.b, entColour.a,
					                        
					p1.x, p1.y, p1.z, entColour.r, entColour.g, entColour.b, entColour.a,
					p6.x, p6.y, p6.z, entColour.r, entColour.g, entColour.b, entColour.a,
					p2.x, p2.y, p2.z, entColour.r, entColour.g, entColour.b, entColour.a,
					//
					p7.x, p7.y, p7.z, entColour.r, entColour.g, entColour.b, entColour.a, 
					p4.x, p4.y, p4.z, entColour.r, entColour.g, entColour.b, entColour.a,
					p0.x, p0.y, p0.z, entColour.r, entColour.g, entColour.b, entColour.a,
					                        
					p7.x, p7.y, p7.z, entColour.r, entColour.g, entColour.b, entColour.a,
					p0.x, p0.y, p0.z, entColour.r, entColour.g, entColour.b, entColour.a,
					p3.x, p3.y, p3.z, entColour.r, entColour.g, entColour.b, entColour.a,
				};
				
				glBindBuffer(GL_ARRAY_BUFFER, glRuntime.genericBuffer);		
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);																
				glUseProgram(glRuntime.colour);	
				glBindVertexArray(glRuntime.VAO[render_program_colour]);		
																
				glUniformMatrix4fv(glGetUniformLocation(glRuntime.colour, "projection"), 
					1, GL_TRUE, &setup->proj.e[0][0]);
				
				
				glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
				break;
				
			} case entry_bmp: {
				TIMED_START(render_bmp);
				
				headerAt += sizeof(struct render_entry_bmp);		
				struct render_entry_bmp *entry = (struct render_entry_bmp *)data;								
				assert(entry->bmp);
				
				union vec3 x = entry->xAxis;
				union vec3 y = entry->yAxis;								
				union vec3 minP = entry->pos; 
				
				glBindTexture(GL_TEXTURE_2D, (GLuint)U32FromPointer(entry->bmp->handle));
				
				union vec2 minUV; 
				union vec2 maxUV; 
				
				if(!entry->usesAtlas) {
					float oneTexelU = 1.0f / (float)entry->bmp->w;
					float oneTexelV = 1.0f / (float)entry->bmp->h;
					minUV = RealToVec2(oneTexelU, oneTexelV);
					maxUV = RealToVec2(1.0f - oneTexelU, 1.0f - oneTexelV);
				} else {
					minUV = entry->atlasOffset.min;
					maxUV = entry->atlasOffset.max;
				}				
				
				union vec3 minXMinY = minP;
				union vec3 minXMaxY = AddVec3(minP, y);
				union vec3 maxXMinY = AddVec3(minP, x);
				union vec3 maxXMaxY = AddVec3(AddVec3(minP, x), y);				
				
				float vertices[] = {
					minXMinY.x, minXMinY.y, minXMinY.z, minUV.x, minUV.y,
					maxXMinY.x, maxXMinY.y, maxXMinY.z, maxUV.x, minUV.y,
					maxXMaxY.x, maxXMaxY.y, maxXMaxY.z, maxUV.x, maxUV.y,
					                                    
					minXMinY.x, minXMinY.y, minXMinY.z, minUV.x, minUV.y,
					maxXMaxY.x, maxXMaxY.y, maxXMaxY.z, maxUV.x, maxUV.y,
					minXMaxY.x, minXMaxY.y, minXMaxY.z, minUV.x, maxUV.y,
				};
				glBindBuffer(GL_ARRAY_BUFFER, glRuntime.genericBuffer);				
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
						
				
				glUseProgram(glRuntime.sprite);		
				glBindBuffer(GL_ARRAY_BUFFER, glRuntime.vertexBuffer);	
				glBindVertexArray(glRuntime.VAO[render_program_sprite]);                
												
				glUniformMatrix4fv(glGetUniformLocation(glRuntime.sprite, "projection"), 
					1, GL_TRUE, &setup->proj.e[0][0]);				
				
				glUniform1f(glGetUniformLocation(glRuntime.sprite, "zBias"), 
					entry->zBias);				
				
				glDrawArrays(GL_TRIANGLES, 0, 6);
				TIMED_END(render_bmp);
				break;				
			} case entry_mesh_batch: {
				TIMED_START(render_mesh);
				headerAt += sizeof(struct render_entry_mesh_batch);
				
				glRuntime.proj = setup->proj;
				gl_MultiRenderMesh(commands);
				TIMED_END(render_mesh);
				break;
			}
			default: break;
			}
		}
	}			
			
	glBindFramebuffer(GL_READ_FRAMEBUFFER, globalFrameBufferHandles[0]);		
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);		
	glViewport(drawRegion.minX, drawRegion.minY, wW, wH);
	glBlitFramebuffer(0, 0, GetWidthI(drawRegion), GetHeightI(drawRegion),
					  drawRegion.minX, drawRegion.minY, 
					  drawRegion.maxX, drawRegion.maxY,
					  GL_COLOR_BUFFER_BIT, GL_LINEAR);
	assert(!gl_GetError());				  
	
	TIMED_END(render);				  
}

extern void gl_ManageTextures(struct texture_op *first)
{
	for(struct texture_op *op = first; op; op = op->next) {
		if(op->isAllocate) {	
			if(op->type == final_bmp) {
				*op->alloc.resultHandle = gl_AllocateTexture(op->alloc.w, op->alloc.h, op->alloc.data);
			} else {
				*op->alloc.arrayOffset = glRuntime.texArrayCount;
				*op->alloc.resultHandle = gl_AddToTexArray(op->alloc.data);
			}
			
		} else {
			GLuint handle = U32FromPointer(op->deAlloc.handle);
			glDeleteTextures(1, &handle);
		}
	}
}

extern void gl_InitTextureLoadQueue(
	struct platform_texture_op_queue *textureOpQueue,
	uint32_t textureOpCount)
{
	for(uint32_t i = 0; i < (textureOpCount - 1); ++i) {
		struct texture_op *op = textureOpQueue->firstFree + i;
		op->next = textureOpQueue->firstFree + i + 1;
	}	
} 

extern void gl_ProcessTextureLoad(
	struct platform_texture_op_queue *textureOpQueue)
{
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
}