extern inline struct push_buffer_result PushBuffer(struct render_group *group,
												   uint32_t dataSize)
{
	struct render_commands *commands = group->commands;
	
	struct push_buffer_result result = {};
	
	uint8_t *pushBufferEnd = commands->pushBase + commands->maxPushSize;
	if((commands->pushData + dataSize) <= pushBufferEnd) {				
			result.header = (struct render_entry_header *)commands->pushData;
			commands->pushData += dataSize;
	} else {
			INVALID_PATH;
	}
	return(result);
}

static inline void *render_PushEntry(struct render_group *group,
				     uint32_t size,
				     enum entry_type type)
{
	void *result = 0; 
	
	size += sizeof(struct render_entry_header);
	struct push_buffer_result push = PushBuffer(group, size);
	if(push.header) {
		struct render_entry_header *header = push.header;
		header->setup = group->lastSetup;
		header->type = (uint16_t)type;		
		result = (uint8_t *)header + sizeof(*header);			
	} else INVALID_PATH;
	
	return(result);
}

static union vec3 render_GetBasis(struct object_transform objT, 
								  union vec3 origin)
{
	union vec3 result = AddVec3(origin, objT.offset);
	
	return(result);	
}

static inline struct bmp_dim render_BMPDim(struct render_group *group,
					   struct object_transform objT,	
					   struct loaded_bmp *bmp,
					   float height,				
					   union vec3 offset)
{
	struct bmp_dim dim;
	dim.size = RealToVec2(height * bmp->wOverH, height);			
	dim.pos = offset;
	dim.basis = render_GetBasis(objT, dim.pos);		
	
	return(dim);
}

extern inline void render_PushLoadedBMP(struct render_group *group,
					struct object_transform objT,	
					struct loaded_bmp *bmp,
					float height,				
					union vec3 offset,
					float zBias)
{		
	struct bmp_dim dim = render_BMPDim(group, objT, bmp, height, offset);		
	union vec2 size = dim.size;		
	
	struct render_entry_bmp *entry = 
		render_PushEntry(group, sizeof(struct render_entry_bmp), entry_bmp);
	if(entry) {			
		entry->bmp = bmp;
		entry->pos = dim.basis;	
		entry->zBias = zBias;	
		union vec2 x = {
			.x = 1,
			.y = 0								
		};
		union vec2 y = {
			.x = 0,
			.y = 1								
		};	
		entry->usesAtlas = false;		
		entry->xAxis = MULTVEC(Vec2ToVec3(x, 0.0f), size.x);
		entry->yAxis = MULTVEC(Vec2ToVec3(y, 0.0f), size.y);	
		if(objT.upright) {			
			entry->xAxis = MULTVEC(ADDVEC(MULTVEC(group->camX, x.x), 
						MULTVEC(group->camY, x.y)), size.x); 											
			entry->yAxis = MULTVEC(ADDVEC(MULTVEC(group->camX, y.x), 
						MULTVEC(group->camY, y.y)), size.y); 					
		}	
	} else INVALID_PATH;				
}

#define ATLAS_X 1024.0f
#define ATLAS_Y 4736.0f

static inline struct rect2 render_GetAtlasPosBody(int32_t rotation,
						  int32_t subtype)
{
	struct rect2 result = {};
	
	float dimX = (1.0f / ATLAS_X) * 128.0f;
	float dimY = (1.0f / ATLAS_Y) * 256.0f;
	
	result.min.x = (dimX * rotation); 	
	result.min.y = dimY * subtype;
	result.max.x = result.min.x + dimX; 
	result.max.y = result.min.y + dimY;
		
	return(result);
}      

static inline struct rect2 render_GetAtlasPosDress(int32_t rotation,
						   int32_t subtype)
{
	struct rect2 result = {};
	
	float dimX = (1.0f / ATLAS_X) * 128.0f;
	float dimY = (1.0f / ATLAS_Y) * 256.0f;
	
	result.min.x = (dimX * rotation); 	
	result.min.y = (dimY * 3) + (dimY * subtype);
	result.max.x = result.min.x + dimX; 
	result.max.y = result.min.y + dimY;
		
	return(result);
}  

static inline struct rect2 render_GetAtlasPosHead(int32_t rotation)
{
	struct rect2 result = {};
	
	float dimX = (1.0f / ATLAS_X) * 128.0f;
	float dimY = (1.0f / ATLAS_Y) * 128.0f;
	
	result.min.x = (dimX * rotation); 	
	result.min.y = dimY * 24;
	result.max.x = result.min.x + dimX; 
	result.max.y = result.min.y + dimY;
		
	return(result);
}  

static inline struct rect2 render_GetAtlasPosHair(int32_t rotation,
						  int32_t subtype)
{
	struct rect2 result = {};
	
	float dimX = (1.0f / ATLAS_X) * 128.0f;
	float dimY = (1.0f / ATLAS_Y) * 128.0f;
	
	result.min.x = (dimX * rotation); 	
	result.min.y = (dimY * 25) + (dimY * subtype);
	result.max.x = result.min.x + dimX; 
	result.max.y = result.min.y + dimY;
		
	return(result);
}  

static inline struct rect2 render_GetAtlasPosFace(int32_t rotation,
						  int32_t subtype)
{
	struct rect2 result = {};
	
	float dimX = (1.0f / ATLAS_X) * 128.0f;
	float dimY = (1.0f / ATLAS_Y) * 128.0f;
	
	result.min.x = (dimX * rotation); 	
	result.min.y = (dimY * 33) + (dimY * subtype);
	result.max.x = result.min.x + dimX; 
	result.max.y = result.min.y + dimY;
		
	return(result);
}  

static inline struct rect2 render_GetAtlasPos(enum sprite_elements sprite,
					      int32_t subtype,
					      int32_t rotation)
{
	struct rect2 result = {};
	
	switch(sprite) {
	case sprite_body: {
		result = render_GetAtlasPosBody(rotation, subtype);
		break;
	} case sprite_dress: {
		result = render_GetAtlasPosDress(rotation, subtype);
		break;
	} case sprite_head: {
		result = render_GetAtlasPosHead(rotation);
		break;
	} case sprite_hair: {
		result = render_GetAtlasPosHair(rotation, subtype);
		break;
	} case sprite_face: {
		result = render_GetAtlasPosFace(rotation, subtype);
		break;
	} 		
	}
	return(result);
}

static inline struct bmp_dim render_SpriteDim(struct render_group *group,
					   struct object_transform objT,	
					   struct loaded_bmp *bmp,
					   float height,				
					   union vec3 offset)
{
	struct bmp_dim dim;
	dim.size = RealToVec2(1.5f, height);	
	dim.pos = offset;
	dim.basis = render_GetBasis(objT, dim.pos);		
	
	return(dim);
}

extern inline void render_PushLoadedSprite(struct render_group *group,
					   struct object_transform objT,	
					   struct loaded_bmp *bmp,
					   float height,				
					   union vec3 offset,
					   enum sprite_elements sprite,
					   int32_t subtype,
					   int32_t rotation,
					   float zBias)
{		
	struct bmp_dim dim = render_SpriteDim(group, objT, bmp, height, offset);		
	union vec2 size = dim.size;		
	
	struct render_entry_bmp *entry = 
		render_PushEntry(group, sizeof(struct render_entry_bmp), entry_bmp);
	if(entry) {			
		entry->bmp = bmp;
		entry->pos = dim.basis;	
		entry->zBias = zBias;	
		entry->usesAtlas = true;
		union vec2 x = {
			.x = 1,
			.y = 0								
		};
		union vec2 y = {
			.x = 0,
			.y = 1								
		};	
		entry->xAxis = MULTVEC(Vec2ToVec3(x, 0.0f), size.x);
		entry->yAxis = MULTVEC(Vec2ToVec3(y, 0.0f), size.y);	
		if(objT.upright) {			
			entry->xAxis = MULTVEC(ADDVEC(MULTVEC(group->camX, x.x), 
						MULTVEC(group->camY, x.y)), size.x); 											
			entry->yAxis = MULTVEC(ADDVEC(MULTVEC(group->camX, y.x), 
						MULTVEC(group->camY, y.y)), size.y); 					
		}	
		entry->atlasOffset = render_GetAtlasPos(sprite, subtype, rotation);
	} else INVALID_PATH;				
}

extern inline void render_PushBMP(struct render_group *group,
				  struct object_transform trans,	
				  struct bmp_id id,
				  float height,				
				  union vec3 offset,
				  float zbias)
{		
	struct loaded_bmp *bmp = assets_GetBMP(group->assets, id);
	
	if(bmp) {
		render_PushLoadedBMP(group, trans, bmp, height, offset, zbias);
	} else {			
		assets_LoadBMP(group->assets, id, true, final_bmp);				
	}		
}

extern inline void render_PushSprite(struct render_group *group,
				     struct object_transform trans,	
				     struct bmp_id id,
				     float height,				
				     union vec3 offset,				     
				     enum sprite_elements sprite,
				     int32_t subtype,
				     int32_t rotation,
				     float zbias)
{		
	struct loaded_bmp *bmp = assets_GetBMP(group->assets, id);
	
	if(bmp) {
		render_PushLoadedSprite(group, trans, bmp, height, 
			offset, sprite, subtype, rotation,zbias);
	} else {			
		assets_LoadBMP(group->assets, id, true , final_bmp);				
	}		
}

extern inline void render_PushRect(struct render_group *group,
				   union vec3 pos,
				   union vec2 dim,
				   union vec4 colour)
{
	struct render_entry_rect *entry = render_PushEntry(
		group, sizeof(struct render_entry_rect), entry_rect);
		
	assert(entry);
	if(entry) {
		entry->pos = pos;
		entry->pos.z += 0.01f;				
		entry->colour = colour;
		entry->dim = dim;				
	}	
}
/*
*	TODO: uint32_t colour is old! convert to vec4
*
*/
extern inline void render_PushCube(struct render_group *group,				   
				   union vec3 pos,
				   float radius,
				   float height,
				   uint32_t colour)
{	
	struct render_entry_cube *entry = render_PushEntry(
		group, sizeof(struct render_entry_cube), entry_cube);
		
	assert(entry);										
	if(entry) {		
		entry->pos = pos;
		entry->pos.x += radius;						
		entry->pos.y += radius;						
		entry->height = height;
		entry->radius = radius;
		entry->colour = colour;
	}	
}

extern inline void render_PushMouseCoords(struct render_group *group,
					  union vec3 pos)
{
	struct render_entry_mouse *entry = render_PushEntry(group, 
						sizeof(struct render_entry_mouse),
						entry_mouse);
	assert(entry);
	entry->pos = pos;	
}

static struct mat4 inline render_ApplyRotation(int32_t rotation)
{
	struct mat4 result = Identity();
	
	switch(rotation) {
	case 0: {
		break;
	} case 1: {
		result = RotZ(DegreesToRads(90));
		break;
	} case 2: {
		result = RotZ(DegreesToRads(180));
		break;
	} case 3: {
		result = RotZ(DegreesToRads(270));
		break;
	} default: INVALID_PATH;	
	}
	return(result);
}

extern inline void render_PushMesh(struct render_group *group,
				   struct mesh_id idMesh,
				   struct bmp_id idBMP,
				   union vec3 pos,	
				   union vec3 offset,
				   int32_t rotation)
{
	struct loaded_mesh *mesh = assets_GetMesh(group->assets, idMesh);
	struct loaded_bmp *bmp = assets_GetBMP(group->assets, idBMP);
	if(mesh && bmp) {
		struct render_entry_mesh *entry = render_PushEntry(group, 
			sizeof(struct render_entry_mesh),
			entry_mesh);
		assert(entry);										
		if(entry) {			
			entry->mesh = mesh;
			entry->bmp = bmp;			
			entry->pos = ADDVEC(pos, offset);
			entry->transform = Translate(render_ApplyRotation(rotation), entry->pos);					
		}										
	} if(!mesh) {
		assets_LoadMesh(group->assets, idMesh, false);				
	} if(!bmp) {
		assets_LoadBMP(group->assets, idBMP, false, final_skin);		
	}
}

static inline void render_PushSetup(struct render_group *group,
				    struct render_setup *newSetup)
{
	if(group->commands->maxRenderTargetIndex < newSetup->renderTargetIndex) {
		group->commands->maxRenderTargetIndex = newSetup->renderTargetIndex;
	}
	group->lastSetup = *newSetup;
}

extern void render_SetTransform(struct render_group *group,				
				bool debug,
				bool ortho,
				float focalLength,
				union vec3 camX,
				union vec3 camY,
				union vec3 camZ,
				union vec3 camP,
				float nearClip,
				float farClip)
{
	float b = SafeRatio1((float)group->commands->w, (float)group->commands->h);
	struct mat4_inv proj;
	
	struct render_setup newSetup = group->lastSetup;
	
	if(!ortho) {
		proj = PerspProj(b, focalLength, nearClip, farClip);
	} else {
		proj = OrthoProj(b, focalLength, nearClip, farClip);
	}
			
	if(!debug) {
		group->camX = camX;
		group->camY = camY;
		group->camZ = camZ;
		group->camP = camP;
	} 
	struct mat4_inv camC = CamTrans(camX, camY, camZ, camP);	
	proj.forward = 	MultMat4(proj.forward, camC.forward);
	proj.inverse =  MultMat4(camC.inverse, proj.inverse);
	group->projForward = proj.forward;		
	group->projInverse = proj.inverse;
	newSetup.proj = group->projForward;
	
	render_PushSetup(group, &newSetup);	
}

/*
*		Creates and initializes a render group struct. 
*		
*/
extern struct render_group render_AllocGroup(struct game_assets *assets,	
					     struct render_commands *commands,												 								 
					     uint32_t resX, uint32_t resY)
{
	struct render_group result = {};
	
	result.commands = commands;
	
	result.screenDim = IntToVec2(resX, resY);		
	result.assets = assets;
	
	
	struct render_setup newSetup = {};
	newSetup.clipRect = RectMinDimI(IntToVec2(0, 0), IntToVec2(resX, resY));
	newSetup.proj = Identity();
	render_PushSetup(&result, &newSetup);
	
	return(result);
}

extern struct rect_int render_AspectFit(uint32_t rW,
					uint32_t rH,
					uint32_t wW,
					uint32_t wH)
{
	struct rect_int result = {};
	if((rW > 0) && (rH > 0) && (wW > 0) && (wH > 0)) {
		float optimalW = (float)wH * ((float)rW/(float)rH);
		float optimalH = (float)wW * ((float)rH/(float)rW);
		
		if(optimalW > (float)wW) {
			result.minX = 0;
			result.maxX = wW;
			
			float empty = (float)wH - optimalH;
			int32_t halfEmpty = Float2Int(0.5f * empty);
			int32_t useH = Float2Int(optimalH);
			
			result.minY = halfEmpty;
			result.maxY = result.minY + useH;
		} else {
			result.minY = 0;
			result.maxY = wH;
			
			float empty = (float)wW - optimalW;
			int32_t halfEmpty = Float2Int(0.5f * empty);
			int32_t useW = Float2Int(optimalW);
			
			result.minX = halfEmpty;
			result.maxX = result.minX + useW;
		}
	}
	return(result);
}

extern inline union vec4 Unproject(struct render_group *group,								    
		        	   union vec2 pixelsXY,
		        	   float zFromTarget)
{
	union vec4 probeZ = {};
	probeZ = Vec3ToVec4(SUBVEC(group->camP, 
		   (MULTVEC(group->camZ, zFromTarget))), 
			   0);	
	
	probeZ = Transform4(group->projForward, probeZ);
	
	union vec2 screenCentre = MULTVEC(group->screenDim, 0.5f);
	union vec2 clipSpaceXY = SUBVEC(pixelsXY, screenCentre);
			
	clipSpaceXY.x *= 2.0f / group->screenDim.x;	
	clipSpaceXY.y *= 2.0f / group->screenDim.y;	
	clipSpaceXY.x *= probeZ.w;
	clipSpaceXY.y *= probeZ.w;
	
	union vec4 clip = RealToVec4(clipSpaceXY.x, clipSpaceXY.y, probeZ.z, probeZ.w);
	
	union vec4 result = Transform4(group->projInverse, clip);
	
	return(result);
}