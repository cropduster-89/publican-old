/************************************************************************************		
__________     ___.   .__  .__                      	
\______   \__ _\_ |__ |  | |__| ____ _____    ____  
 |     ___/  |  \ __ \|  | |  |/ ___\\__  \  /    \ 	Main code for gameplay.
 |    |   |  |  / \_\ \  |_|  \  \___ / __ \|   |  \
 |____|   |____/|___  /____/__|\___  >____  /___|  /
                    \/             \/     \/     \/ 
		    @Always adding stuff, never cleaning stuff	
*************************************************************************************/

static void CheckGlobalBars(struct world_mode *world)
{
	for(int32_t i = 0; i < world->alerts.barCount; ++i) {
		struct entity *ent = world->alerts.bars[i];
		struct entity_furn *furn = (struct entity_furn *)ent->sub;		
		
		if(!furn->users[1].ent) continue;
				
		struct entity_char *punter = (struct entity_char *)furn->users[1].ent->sub;	
		if(punter->intent.purchase.stage == purchase_wait) {													
			world->alerts.serveAlert = true;
			
			return;	
		} 							
	}
	world->alerts.serveAlert = false;
}

static inline bool IsWithinBounds(struct cube3 box,
				  union vec3 testRay)
{
		
	return((testRay.z > box.b.min.z) &&
	       (testRay.z < box.t.max.z) &&
	       (testRay.y < box.n.max.y) &&
	       (testRay.y > box.s.min.y) &&
	       (testRay.x > box.w.min.x) &&
	       (testRay.x < box.e.max.x));
}
/*
*	loops through onscreen entities, and performs
*	ray intersection test
*/
static void SelectionTest(struct world_mode *world,
			  union vec3 testRay)
{
	for(struct entity *ent = &world->entities[0]; ent; ent = ent->next) 
	{		
		if(ent->type == ENTTYPE_NULL || ent->type == ENTTYPE_OBJ) {continue;}			
		else if(IsWithinBounds(ent->bound, testRay)) 
		{			 
			if(ent == world->mouseOverEnt.ent ||
			   ent == world->selectedEnt.ent) {continue;}
			   
			world->mouseOverEnt.ent = ent;
			world->mouseOverEnt.targetId = ent->index;	
			
			return;
		}		
	}	
	world->mouseOverEnt.ent = NULL;
	world->mouseOverEnt.targetId = 0;
}

extern  void world_GetMouseCoords(struct pub_input *const input,
				  struct render_group *const group,
				  struct world_mode *const world)
{		
	union vec4 mousePosF = Unproject(group, 
		RealToVec2(input->mouseX, input->mouseY), 0);
	union vec4 mousePosN = Unproject(group, 
		RealToVec2(input->mouseX, input->mouseY), 2);			
	union vec4 mouseVector = SUBVEC(mousePosF, mousePosN);		
	union vec3 testVec = ADDVEC(mouseVector.xyz, 
		MULTVEC(BACKWARD_VEC, 10));	
	
	union vec3 baseVector = AddVec3(testVec, world->cam.pos);
	union vec3 forwardVector = AddVec3(testVec, world->cam.pos);	
	
	float floorCap = BACKWARD_VEC_HALF.z;
#define MAX_MOUSE_DISTANCE 180
	for(int32_t i = 0; i < MAX_MOUSE_DISTANCE; ++i)
	{
		forwardVector = ADDVEC(baseVector, MULTVEC(FORWARD_VEC_HALF, i));
		SelectionTest(world, forwardVector);	
		if(forwardVector.z < floorCap) 
		{	
			float remainder = forwardVector.z / floorCap;				
			world->mouseCoords[0] = ADDVEC(forwardVector, 
				MULTVEC(FORWARD_VEC_HALF, remainder));
			world->mouseRay[0] = SUBVEC(world->mouseCoords[0],
				world->cam.pos);	
			break;
		}
#ifdef DEBUG 
		else if(forwardVector.z > floorCap && i == MAX_MOUSE_DISTANCE - 1)
		{
			printf("Mouse ray not extended enough!\n");
			INVALID_PATH;
		}
#endif		
	}					
}

static inline uint32_t world_GetMouseElev(struct world_mode *world)
{
	uint32_t result = 0;
	struct point3 coord1 = {
		world->mouseCoords[1].x,	
		world->mouseCoords[1].y,	
		world->mouseCoords[1].z / 4	
	};
	uint32_t val = tile_GetElevation(&world->map, coord1);	
	if(val == 1) {	
		result = val;
	}
	struct point3 coord2 = {
		world->mouseCoords[2].x,	
		world->mouseCoords[2].y,	
		world->mouseCoords[2].z / 4	
	};	
	val = tile_GetElevation(&world->map, coord2);	
	if(val == 2) {		
		result = val;
	}	
	return(result);
}

static inline bool world_CheckFaces(struct world_mode *world,
			            struct entity *ent)
{
	bool result = false;
	if(
		world->mouseCoords[world->elevFlag].x >= ent->bounds.min.x &&
		world->mouseCoords[world->elevFlag].y >= ent->bounds.min.y &&
		world->mouseCoords[world->elevFlag].x <= ent->bounds.max.x &&
		world->mouseCoords[world->elevFlag].y <= ent->bounds.max.y 
		
	) {
		result = true;
	} 
	return(result);
}

static inline void UpdateCamera(struct world_cam *cam,
				struct pub_input *input,
				struct loaded_bmp buffer)
{	
	cam->lastPos = cam->pos;
	float camTransX = 0.0f;
	float camTransY = 0.0f;
	float camTransZ = 0.0f;
	if(input->mouseX <= 0) {
		camTransX = -0.1f;
		camTransY = 0.1f;					
	} if(input->mouseX >= buffer.w - 10) {
		camTransX = 0.1f;
		camTransY = -0.1f;				
	} if(input->mouseY <= 0 && (cam->pos.x > -5 && cam->pos.y > -5)) {
		camTransX = -0.1f;
		camTransY = -0.1f;				
	} if(input->mouseY >= buffer.h - 10) {
		camTransX = 0.1f;
		camTransY = 0.1f;				
	} if(input->mouseZ > 0) {
		camTransZ = 0.002f;
	} if(input->mouseZ < 0) {
		camTransZ = -0.002f;
	}
	cam->pos.x = cam->lastPos.x + camTransX;
	cam->pos.y = cam->lastPos.y + camTransY;		
	cam->dolly += camTransZ;		
}


/*
*		Processes mouse clicks during gameplay.
*		The order, and exclusivity of these is
*		important, or two actions may result from 
*		one click.
*/
static void world_Control(struct world_mode *world,
			  struct pub_input *input,
			  struct loaded_bmp drawBuffer)
{		
	
	UpdateCamera(&world->cam, input, drawBuffer);		
	
	if(input->buttons.lClick.endedDown) {
		if(world->edit.editorOpen && world->edit.current.ent != NULL) {
			if(world->edit.current.ent->alias == ENTALIAS_WALL) {
				world->edit.wallsInit = false;
				edit_WallPlace(world);
				world->edit.newAnchor = 0;
				world->edit.anchorX = 0;
				world->edit.anchorY = 0;
				world->edit.maxLine = 0;
				world->edit.endX = 0;
				world->edit.endY = 0;	
				world->edit.dirLock = dirlock_null;						
				world->edit.elevLock = 0;						
				union vec3 dummyPos = {};
				world->edit.basePos = dummyPos;
				input->buttons.lClick.endedDown = 0;
				printf("walls end\n");
				return;
			} else {
				edit_PlaceItem(world);
				input->buttons.lClick.endedDown = 0;
				return;
			}			
		} 	
	}	
}

static inline void pub_RenderLimits(struct world_mode *world)
{				
	float limitXHigh = world->cam.pos.x + 20; 
	float limitXLow = world->cam.pos.x - 20; 
	float limitYHigh = world->cam.pos.y + 14; 
	float limitYLow = world->cam.pos.y - 14; 
	
	for(int32_t i = 0; i < world->entityCount; ++i) {
		struct entity *ent = &world->entities[i];
		
		if(ent->pos.x <= limitXHigh && 
		   ent->pos.x >= limitXLow &&
		   ent->pos.y <= limitYHigh &&
		   ent->pos.y >= limitYLow &&
	 	   roundf(ent->pos.z) <= (world->currentFloor) * 4) {
			   
			if(ent->state == entstate_offscreen) {
				entity_AddToList(world, ent);  
				ent->state = entstate_onscreen;
				world->onscreenCount++;
			} 
		} else {
			if(ent->state == entstate_onscreen) {
				entity_RemoveFromList(world, ent);
				ent->state = entstate_offscreen;
				world->onscreenCount--;
			}
		}	
	}	
	
}

static void worlddir_northewCharFromFile(struct chara_stats *stats, 
				  struct loaded_char *chara)
{
	strcpy_s(stats->firstName, 32, chara->firstName);	
	strcpy_s(stats->lastName, 32, chara->lastName);		
	stats->race = chara->race;
	stats->dress = chara->dress;		
	stats->hair = chara->hair;	
	stats->face = chara->face;	
	stats->body = chara->body;	
	stats->job = chara->job;
	stats->age = chara->age;	
	stats->gender = chara->gender;
	stats->attribs = chara->attribs;	
}

static void world_StartWorld(struct game_state *state)
{
	state->worldMode = PUSH_STRUCT(&state->modeArena, struct world_mode, DEF_PUSH);
	struct world_mode *world = state->worldMode;	
#if 0
	FILE *handle1;
	fopen_s(&handle1, "world", "rb");
	if(!handle1) {printf("world read fialed\n");}	
	
	FILE *handle2;
	fopen_s(&handle2, "tile", "rb");
	if(!handle2) {printf("tile read fialed\n");}	
	
	if(!fread(world, sizeof(struct world_mode), 1, handle1)) {
		if(ferror(handle1)) {
			printf("ferror\n");		
		} else if(feof(handle1)) {
			printf("feof\n");	
		}
		printf("world transfer fialed %d\n", errno);		
	} 
	struct map_data *map = &state->worldMode->map;	
	map->tiles = PUSH_ARRAY(&state->modeArena, map->sizeX * map->sizeY * map->sizeZ,
						          struct tile_data, DEF_PUSH);
							  
	if(!fread(world->map.tiles, sizeof(struct tile_data) * world->map.sizeX * 
		world->map.sizeY * world->map.sizeZ, 1, handle2)) {
		printf("tile transfer fialed\n");			
	}		
	fclose(handle1);	
	fclose(handle2);		
	
	world->alerts.barCount = 0;
	for(int32_t i = 0; i < world->furnCount; ++i) {		
		struct entity_furn *furn = &world->furniture[i];
		struct entity *ent = &world->entities[furn->entIndex];
		ent->sub = furn;
		if(ent->alias == alias_bar) {
			world->alerts.bars[world->alerts.barCount++] = ent;			
		}
	}
#else		
	struct map_data *map = &state->worldMode->map;				
	map->sizeX = 40;
	map->sizeY = 40;
	map->sizeZ = 2;
	map->activeX = 2;
	map->activeY = 2;
	map->activeXLen = 15;
	map->activeYLen = 15;
	map->tiles = PUSH_ARRAY(&state->modeArena, map->sizeX * map->sizeY * map->sizeZ,
						          struct tile_data, DEF_PUSH);
	
	uint32_t index = entity_Add(world, ENTTYPE_NULL);
	for(int32_t z = 0; z < map->sizeZ; ++z) {	
	for(int32_t y = 0; y < map->sizeY; ++y) {
	for(int32_t x = 0; x < map->sizeX; ++x) {	
		struct point3 pos = {
			x, y, z	
		};
		int32_t floorType = floor_bmp_dirt;
		if(x >= map->activeX && x < map->activeXLen &&
		   y >= map->activeY && y < map->activeYLen) {			
			floorType = floor_bmp_wood;
			
			if(y < map->activeYLen / 2 && z == 1){
				tile_SetPass(&world->map, pos, true);
				continue;
			}
		} else if(z == 1) {
			tile_SetPass(&world->map, pos, true);
			continue;
		}
		index = entity_Add(world, ENTTYPE_OBJ);
		entity_InitStruc(world, index, ENTALIAS_FLOOR, x, y, z * 4, 0, 
			DEF_OFFSET, floorType, 0);														
	}							
	}				
	}
	index = entity_Add(world, ENTTYPE_CHAR);
	entity_InitChar(world, index);	
	struct chara_stats *stats = &world->characters[0].stats;
	strcpy_s(stats->firstName, 32, "Arston");	
	strcpy_s(stats->lastName, 32, "Blowfield");	 
	stats->race = race_human;
	stats->dress = dress_2;	
	stats->body = body_fat;	
	stats->hair = hair_7;	
	stats->face = face_3;	
	stats->job = job_owner;
	stats->isStaff = true;
	stats->gender = false;
	struct chara_attributes attribs = {
		15, 16, 10, 8
	};	
	stats->attribs = attribs;
	world->entities[world->characters[0].entIndex].state = entstate_offscreen;
	
	FILE *charHandle;
	fopen_s(&charHandle, "data/test_chars.chara", "rb");
	if(!charHandle) {printf("char file read fialed\n");}
	struct char_file_header header = {};
	if(!fread(&header, sizeof(struct char_file_header), 1, charHandle)) {
		if(ferror(charHandle)) {
			printf("ferror\n");		
		} else if(feof(charHandle)) {
			printf("feof\n");	
		}
		printf("world transfer fialed %d\n", errno);		
	} 
	for(int32_t i = 0; i < header.count; ++i) {
		struct loaded_char newChar = {};
		fread(&newChar, sizeof(struct loaded_char), 1, charHandle);
		index = entity_Add(world, ENTTYPE_CHAR);
		entity_InitChar(world, index);	
		world->entities[world->characters[i + 1].entIndex].state = entstate_dormant; 		
		worlddir_northewCharFromFile(&world->characters[i + 1].stats, &newChar);
	}
	fclose(charHandle);
	
	for(int32_t i = 0; i < world->charCount; ++i) {
		world->characters[i].stats.relations = PUSH_ARRAY(
			&state->modeArena, world->charCount, struct social_relation, DEF_PUSH);
	}	
#endif	
	union vec3 initCamPos = {
		.x = 4,
		.y = 4,
		.z = 0
	};				
	union vec3 initCamOffset = {};
	world->cam.pos = initCamPos;
	world->cam.offset = initCamOffset;
	world->cam.offset.z = 5.00;
	world->cam.pitch = DegreesToRads(60);
	world->cam.orbit = DegreesToRads(-45);
	world->cam.dolly = 0.06;
	world->currentFloor = 0;
	world->lastEnt = 0;
	
	world->timer.baseTime = time_GetTime();
	
	srand(world->timer.baseTime);
	
	printf("world init\n");									
}

/*
*	Set up camera transforms for both the world and the ui
*	for the current frame
*/
static void InitCamTransforms(struct render_group 	*group, 
			      struct render_group 	*textGroup, 
			      struct world_mode 	*world,
			      struct loaded_bmp 	drawBuffer)
{
	struct mat4 cam0 = MultMat4(RotZ(world->cam.orbit), RotX(world->cam.pitch));		
	union vec3 cam0T = Transform(cam0, AddVec3(world->cam.offset, 
					 RealToVec3(0, 0, world->cam.dolly)), 1.0f);			
	render_SetTransform(group, false, true, world->cam.dolly, 
		GetCol(cam0, 0),
		GetCol(cam0, 1),
		GetCol(cam0, 2),
		cam0T, -100.0f, 100.0f);						
	
	render_SetTransform(textGroup, false, true, 1.0f, 
		RealToVec3(2.0f / drawBuffer.w, 0, 0),
		RealToVec3(0, 2.0f / drawBuffer.w, 0),
		RealToVec3(0, 0, 1),							
		EMPTY_V3, -10000.0f, 10000.0f);	
}

extern void world_UpdateAndRender(struct game_state 	*state, 
				  struct temp_state 	*tState, 								  
				  struct pub_input 	*input, 
				  struct render_group 	*group, 
				  struct render_group 	*textGroup, 
				  struct loaded_bmp 	drawBuffer)
{
	TIMED_START(world);
	
	if(!state->worldMode){
		world_StartWorld(state);
	}		
	struct world_mode *world = state->worldMode;		
	
	InitCamTransforms(group, textGroup, world, drawBuffer);	
		
	world_GetMouseCoords(input, group, world);		
	
	world->elevFlag = world_GetMouseElev(world);	
	
	render_PushMouseCoords(group, world->mouseRay[0]);
	
#ifdef DEBUG
	char buffer[128];
	sprintf(buffer, "x: %.1f y: %.1f z: %.1f", 
	world->mouseCoords[0].x, world->mouseCoords[0].y, world->mouseCoords[0].z);
	debug_TextOut(buffer, textGroup, tState, 0, 0.3);
#endif	

	world->timer = time_Update(world->timer);

	CheckGlobalBars(world);
	
	pub_RenderLimits(world);	
	
	for(struct entity *ent = &world->entities[0]; ent; ent = ent->next) {
		if(ent->type == ENTTYPE_NULL) {continue;}
		
		
		if(ent->state != entstate_onscreen && ent->alias != alias_char) continue;									
		
		struct object_transform trans = {};
		if(ent->alias == alias_char) {
			trans = render_UprightTrans();		
		} else {
			trans = render_FlatTrans();
		}
								
		trans.offset = SubVec3(ent->pos.xyz, world->cam.pos);				
						
		switch(ent->alias) {
		case ENTALIAS_FLOOR: {
			render_PushMesh(group, 
			asset_FindMesh(tState->assets, asset_floor, floor_bmp_count), 
			asset_FindBMP(tState->assets, asset_floor, GET_STRUC(ent)->bmpType),
			trans.offset, ent->offset, ent->rotation);	
			
			if(world->collisionOverlay) {
				union vec4 col = {};	
				struct point3 coord = {
					(int32_t)ent->pos.x,
					(int32_t)ent->pos.y,
					(int32_t)ent->pos.z / 4,
				};	
				struct tile_data tile =
					tile_GetTile(&world->map, coord);
				if(tile.impassable) {
					col = RealToVec4(1.0f, 0.0f, 0.0f, 0.3f);
				} else {
					col = RealToVec4(0.0f, 1.0f, 0.0f, 0.3f);
				}
				if(tile.elevation == 2) {
					trans.offset.z += 1.0f;
				}					
				render_PushRect(group, trans.offset, RealToVec2(1.0f, 1.0f), col);								
			}
			/* if(world->collisionOverlay) {
				union vec4 col = {};	
				struct point3 coord = {
					(int32_t)ent->pos.x,
					(int32_t)ent->pos.y,
					(int32_t)ent->pos.z / 4,
				};					
				struct tile_data tile =	tile_GetTile(&world->map, coord);
				if(tile.stepped) {
					col = RealToVec4(1.0f, 0.0f, 0.0f, 0.3f);
				} else {
					col = RealToVec4(0.0f, 1.0f, 0.0f, 0.3f);
				}
				if(tile.elevation == 2) {
					trans.offset.z += 1.0f;
				}					
				render_PushRect(group, trans.offset, RealToVec2(1.0f, 1.0f), col);								
			} 	*/					
			break;				
		} case ENTALIAS_SUPPORT: {				
			render_PushMesh(group, asset_FindMesh(tState->assets, asset_support, 1), 
				asset_FindBMP(tState->assets, asset_support, 0),
				trans.offset, ent->offset, ent->rotation);			
			break;				
		} case ENTALIAS_STOOL: {		
					
			render_PushMesh(group, asset_FindMesh(tState->assets, asset_stool, 1), 
				asset_FindBMP(tState->assets, asset_stool, 0),
				trans.offset, ent->offset, ent->rotation);			
			break;				
		} case ENTALIAS_TABLE: {			
			render_PushMesh(group, asset_FindMesh(tState->assets, asset_table, 1), 
				asset_FindBMP(tState->assets, asset_table, 0),
				trans.offset, ent->offset, ent->rotation);					
			break;
		} case alias_bar: {		
			render_PushMesh(group, asset_FindMesh(tState->assets, asset_bar, 1), 
				asset_FindBMP(tState->assets, asset_bar, 0),
				trans.offset, ent->offset, ent->rotation);				
			break;
		} case alias_pillar: {		
			render_PushMesh(group, asset_FindMesh(tState->assets, asset_pillar, 1), 
				asset_FindBMP(tState->assets, asset_pillar, 0),
				trans.offset, ent->offset, ent->rotation);				
			break;
		} case ENTALIAS_WALL: {			
			uint32_t wallType = GET_STRUC(ent)->meshType;				
			
			render_PushMesh(group, 
				asset_FindMesh(tState->assets, asset_wall, 			
				wallType + wall_bmp_count), 
				asset_FindBMP(tState->assets, asset_wall, 0),
				trans.offset, ent->offset, ent->rotation);	
			
			uint32_t secondaryRot = 0;
			switch(ent->rotation) {
			case 0: {
				secondaryRot = 2;
				trans.offset.y += 1;
				break;
			} case 1: {
				secondaryRot = 3;
				trans.offset.x -= 1;
				break;
			} case 2: {
				secondaryRot = 0;
				trans.offset.y -= 1;
				break;
			} case 3: {
				secondaryRot = 1;
				trans.offset.x += 1;
				break;
			} default: INVALID_PATH;	
			}
			
			render_PushMesh(group, 
				asset_FindMesh(tState->assets, asset_wall, 			
				wallType + wall_bmp_count), 
				asset_FindBMP(tState->assets, asset_wall, 0),
				trans.offset, ent->offset, secondaryRot);	
			break;
		} case alias_char: {			
			struct entity_char *current = (struct entity_char *)ent->sub;	
			if(ent->state == entstate_dormant) break;	
			ai_Think(world, current);					
			
			if(ent->state != entstate_onscreen) break;
						
			float defaultSpriteZ = 0.005;
			
			int32_t gender; 
			if(current->stats.gender) {
				gender = asset_fem;
			} else {
				gender = asset_man;
			}
			
			render_PushSprite(group, trans, 
					asset_FindBMP(tState->assets, gender, 0), 
					3.0f, ent->offset, sprite_body, current->stats.body, 
					ent->rotation, defaultSpriteZ);
					
			render_PushSprite(group, trans, 
					asset_FindBMP(tState->assets, gender, 0), 
					3.0f, ent->offset, sprite_dress, 
					current->stats.body * 3 + current->stats.dress, 
					ent->rotation, defaultSpriteZ);

			
			union vec3 headOffset = {
				.x = ent->offset.x,
				.y = ent->offset.y,
				.z = ent->offset.z + 1.75f,
			};
			
			render_PushSprite(group, trans, 
					asset_FindBMP(tState->assets, gender, 0), 
					1.5f, headOffset, sprite_head, 
					0, ent->rotation, defaultSpriteZ);
					
			render_PushSprite(group, trans, 
					asset_FindBMP(tState->assets, gender, 0), 
					1.5f, headOffset, sprite_hair, 
					current->stats.hair, ent->rotation, defaultSpriteZ);	

			if(ent->rotation > 2) {
				render_PushSprite(group, trans, 
					asset_FindBMP(tState->assets, gender, 0), 
					1.5f, headOffset, sprite_face, 
					current->stats.face, ent->rotation, defaultSpriteZ);	
			}			
			
			if(current->bubble) {
				union vec3 bubbleOffset = {
					.x = ent->offset.x - 0.25,
					.y = ent->offset.y,
					.z = ent->offset.z + 2.6,
				};
				render_PushBMP(group, trans, 
					asset_FindBMP(tState->assets, asset_bubbles, 0), 
					0.5f, bubbleOffset, defaultSpriteZ);		
			}	
			trans.offset.z += 3;
			render_PushCube(group, trans.offset, 0.5f, 3.0f, 0xF00000FF);	
			break;				
		} case alias_tiolet: {
			render_PushMesh(group, asset_FindMesh(tState->assets, asset_tiolet, 1 + ent->rotation), 
					asset_FindBMP(tState->assets, asset_tiolet, 0),
					trans.offset, ent->offset, ent->rotation);
					
			break;	
		} case alias_steps: {
			render_PushMesh(group, asset_FindMesh(tState->assets, asset_steps, 1 + ent->rotation), 
					asset_FindBMP(tState->assets, asset_steps, 0),
					trans.offset, ent->offset, ent->rotation);
				
			break;	
		} case ENTALIAS_NULL:{
			break;
		}  default:{
			INVALID_PATH;
			break;
		}	
		}									  				
	}
	
	world_Control(world, input, drawBuffer);
	ui_WorldMode(world, input, textGroup, tState);
	
	//for(int32_t i = 0; i < 8; ++i) {
	//	struct object_transform trans = {};
	//		trans.offset = SubVec3(world->mouseCoords[i], world->cam.pos);	
	//		render_PushCube(group, trans.offset, 0.1f, 0.1f, 0xFF0000FF);	
	//}
	
	struct edit_state *edit = &world->edit;
	if(input->buttons.shift.endedDown && !edit->editorOpen) { 
		struct edit_placed_entity newCurrent = {};
		edit->editorOpen = true;
		edit->current= newCurrent;
		printf("editor open\n");
		input->buttons.shift.endedDown = 0;
	} else if(input->buttons.shift.endedDown && edit->editorOpen) { 
		edit->editorOpen = false;		
		printf("editor closed\n");
		input->buttons.shift.endedDown = 0;
	}
	
	if(edit->editorOpen) edit_Editor(world, tState, input, group, textGroup);		
	
	TIMED_END(world);	
}								  