static void DEBUG_AddChar(struct world_mode *world)
{
	for(int32_t i = 1; i <= world->charCount; ++i) {
		struct entity *ent = GET_CHARBASE(world, &world->characters[i]);
		if(ent->state == entstate_dormant) {
			ent->state = entstate_offscreen;
			break;
		}
	}
}

static void edit_SaveWorld(struct world_mode *world)
{
	FILE *handle1 = fopen("world", "wb");;
	if(!handle1) {printf("world save fialed\n");}	
	fwrite(world, sizeof(struct world_mode), 1, handle1);
	
	FILE *handle2 = fopen("tile", "wb");
	if(!handle2) {printf("tile save fialed\n");}
	fwrite(world->map.tiles, sizeof(struct tile_data) * world->map.sizeX * 
		world->map.sizeY * world->map.sizeZ, 1, handle2);		
}

static void edit_WallDraw(struct world_mode 	*world, 						  
			  struct render_group 	*group,
			  struct temp_state 	*tState)
{
	struct edit_state *edit = &world->edit;
	
	for(int32_t i = 0; i < edit->maxLine; ++i) {
		if(i > ARRAY_COUNT(edit->wallPlan)) break;
		
		int8_t wall1Rot = 0;
		int8_t wall2Rot = 0;
		union vec3 alteredOffset = {};
		switch(edit->dirLock) {
		case dirlock_north:	{	
			edit->wallPlan[i].x = edit->anchorX;
			edit->wallPlan[i].y = edit->anchorY + i;	
			wall1Rot = 0;	
			wall2Rot = 2;	
			alteredOffset.y += 1;
			break;
		} case dirlock_south: {							
			edit->wallPlan[i].x = edit->anchorX;
			edit->wallPlan[i].y = edit->anchorY - i;
			wall1Rot = 2;	
			wall2Rot = 0;	
			alteredOffset.y -= 1;
			break;
		} case dirlock_east: {	
			edit->wallPlan[i].x = edit->anchorX + i + 1;				
			edit->wallPlan[i].y = edit->anchorY;	
			wall1Rot = 1;	
			wall2Rot = 3;	
			alteredOffset.x -= 1;
			break;
		} case dirlock_west: {						
			edit->wallPlan[i].x = edit->anchorX - i - 1;				
			edit->wallPlan[i].y = edit->anchorY;
			wall1Rot = 3;	
			wall2Rot = 1;	
			alteredOffset.x += 1;	
			break;
		} default: INVALID_PATH;
		}	
		edit->wallPlan[i].z = edit->anchorZ;		
		union vec3 offset = {
			.x = edit->wallPlan[i].x,
			.y = edit->wallPlan[i].y,
			.z = edit->wallPlan[i].z,
		};		
		struct object_transform trans =	render_FlatTrans();										
		trans.offset = SubVec3(offset, world->cam.pos);	
		enum wall_mesh_type_id wallType = wall_4;
		if(edit->elevLock) {			
			trans.offset.z += 1.0f;
			wallType = wall_3;			
		}			
		render_PushMesh(group, asset_FindMesh(tState->assets, asset_wall, 
			wall_bmp_count + wallType), 
			asset_FindBMP(tState->assets, asset_wall, 0),
			trans.offset, EMPTY_V3, wall1Rot);	
		trans.offset = ADDVEC(trans.offset, alteredOffset);	
		render_PushMesh(group, asset_FindMesh(tState->assets, asset_wall, 
			wall_bmp_count + wallType), 
			asset_FindBMP(tState->assets, asset_wall, 0),
			trans.offset, EMPTY_V3, wall2Rot);		
	}
}

static inline struct point3 edit_WallPosCorrect(struct world_mode *world,
						struct point3 pos)
{
	struct edit_state *edit = &world->edit;	
	
	switch(edit->dirLock) {
	case dirlock_north:	{							
		break;
	} case dirlock_south: {	
		pos.y -= 1;											
		break;
	} case dirlock_east: {	
		pos.x -= 1;
		pos.y -= 1;						
		break;
	} case dirlock_west: {	
		pos.y -= 1;						
		break;
	} default: INVALID_PATH;
	}		
	
	return(pos);
}

static inline void edit_MakePillar(struct world_mode *world, 
				   struct point3 pos)
{
	if(entity_FindPillarByPos(world, pos)) {return;}
		
	uint32_t index = entity_Add(world, type_struc);
	entity_InitStruc(world, index, alias_pillar, pos.x, pos.y, pos.z, 
	0, EMPTY_V3, 0, 0);	
	struct entity_struc *placedPillar = GET_STRUCFROMENTINDEX(world, index);
	placedPillar->coords = pos;
}

static void edit_WallCorrect(struct world_mode *world)
{
	struct edit_state *edit = &world->edit;	
	
	struct point3 xOffsetPlus = {1, 0, 0};
	struct point3 xOffsetMinus = {-1, 0, 0};
	struct point3 yOffsetPlus = {0, 1, 0};
	struct point3 yOffsetMinus = {0, -1, 0};
	
	for(int32_t i = 0;  i < edit->maxLine; ++i) {
		
		struct point3 pos = edit->wallPlan[i];		
		
		int32_t n = tile_GetWalled(&world->map, pos);
		int32_t s = tile_GetWalled(&world->map, GET_S_COORD(pos, 0));
		int32_t w = tile_GetWalled(&world->map, GET_W_COORD(pos, 0));
		int32_t e = tile_GetWalled(&world->map, pos);		
		
		bool midFlag = false;
		bool startFlag = i == 0 ? true : false;
		bool endFlag = i == edit->maxLine - 1 ? true : false;		
		struct point3 startOffset = {};
		struct point3 endOffset = {};
		struct point3 midOffset = {};
		
		switch(edit->dirLock) {
		case dirlock_south: {	
			endOffset = yOffsetMinus;
			midOffset = yOffsetPlus;
						
			if(w == 2 || w == 3 || e == 2 || e == 3) {				
				midFlag = true;
			} 
			break;	
		} case dirlock_north: {			
			midOffset = yOffsetPlus;
			endOffset = yOffsetPlus;
			
			if(w == 2 || w == 3 || e == 2 || e == 3) {				
				midFlag = true;
			} 
			break;
		} case dirlock_east: {			
			startOffset = xOffsetMinus;
			
			if(n == 1 || n == 3 || s == 1 || s == 3) {				
				midFlag = true;
			} 
			break;
		} case dirlock_west: {			
			startOffset = xOffsetPlus;
			
			if(n == 1 || n == 3 || s == 1 || s == 3) {				
				midFlag = true;
			}
			break;
		}	
		}
		if(startFlag) {edit_MakePillar(world, AddPoint3(pos, startOffset));}
		if(endFlag) {edit_MakePillar(world, AddPoint3(pos, endOffset));}
		if(midFlag) {edit_MakePillar(world, AddPoint3(pos, midOffset));}
	}	
}

/*
*		Create entities for placed walls. 
	
*
*/
extern void edit_WallPlace(struct world_mode *world)
{
	struct edit_state *edit = &world->edit;	
	
	for(int32_t i = 0; i < edit->maxLine; ++i) {	
		
		struct point3 pos = edit->wallPlan[i];
		
		enum wall_mesh_type_id meshType = wall_4;
		union vec3 offset = EMPTY_V3;
		if(edit->elevLock) {
			offset.z += 1.0f;
			meshType = wall_3;
		} 
		
		int32_t rotation = (edit->dirLock % 2 != 0) ? 0 : 1;
		
		struct point3 coordPos = edit_WallPosCorrect(world, pos);
		if(entity_FindWallByPos(world, coordPos, rotation)) {printf("found wall\n"); continue;}	
				
		uint32_t wallVal = tile_GetWalled(&world->map, coordPos);	
				
		switch(edit->dirLock) {
		case dirlock_north: {			
			if(wallVal == 0) {tile_SetWalled(&world->map, coordPos, 1);}												
			if(wallVal == 2) {tile_SetWalled(&world->map, coordPos, 3);}
			break;
		} case dirlock_south: {	
			offset.y = -1;
			if(wallVal == 0) {tile_SetWalled(&world->map, coordPos, 1);}												
			if(wallVal == 2) {tile_SetWalled(&world->map, coordPos, 3);}												
			break;
		} case dirlock_east: {
			if(wallVal == 0) {tile_SetWalled(&world->map, coordPos, 2);}		
			break;
		} case dirlock_west: {			
			offset.x = 1;
			if(wallVal == 0) {tile_SetWalled(&world->map, coordPos, 2);}				
			break;
		} default: INVALID_PATH;
		}							
		if(wallVal == 3) {tile_SetWalled(&world->map, coordPos, 3);}	
			
		uint32_t index = entity_Add(world, type_struc);
		entity_InitStruc(world, index, alias_wall, pos.x, pos.y, pos.z, 
			rotation, offset, 0, meshType);
				
		struct entity_struc *placedWalled = GET_STRUCFROMENTINDEX(world, index);
		placedWalled->coords = coordPos;
		printf("placed at coords: %d %d\n", coordPos.x, coordPos.y);
		
	}
	edit_WallCorrect(world);	
}

/*
*		Display position of the walls not yet built. 
*/

static void edit_WallPlan(struct world_mode *world)
{
	struct edit_state *edit = &world->edit;
	union vec3 pos = world->mouseCoords[0];
	if(edit->elevLock) {
		pos = world->mouseCoords[2];
	} 	
	
	if(!edit->dirLock) {
		if(!edit->wallsInit) {
			edit->basePos = pos;
			edit->wallsInit = true;
		}		
		if(edit->basePos.x > pos.x + 0.5f) {
			edit->dirLock = dirlock_west;
		} else if(edit->basePos.x < pos.x - 0.5f) {
			edit->dirLock = dirlock_east;
		} else if(edit->basePos.y > pos.y + 0.5f) {
			edit->dirLock = dirlock_south;
		} else if(edit->basePos.y < pos.y - 0.5f) {
			edit->dirLock = dirlock_north;
		} else {	
			edit->dirLock = dirlock_null;
			return;
		}						
	}		
	if(edit->dirLock) {						
		switch(edit->dirLock) {
		case dirlock_north:	{							
		} case dirlock_south: {							
			edit->maxLine = abs(edit->anchorY - pos.y);		
			break;
		} case dirlock_east: {							
		} case dirlock_west: {						
			edit->maxLine = abs(edit->anchorX - pos.x);						
			break;
		} default: 
			INVALID_PATH;
		}	
		if(edit->maxLine > 10) {edit->maxLine = 10;}		
		if(!edit->newAnchor) {					
			edit->newAnchor = true;
		}				
	}			
}

static void edit_WallPrompt(struct world_mode 	*world,
			    struct render_group *group,
			    bool 		init)
{
	struct edit_state *edit = &world->edit;	
	float wallOffset = 4.0f;
	uint32_t colour = 0xFFF2FAF4;
	if(world->elevFlag && !edit->wallsInit) {	
		wallOffset = 5.0f;
		colour = 0xFF0AEF02;
	}	
	if(!edit->wallsInit) {
		edit->anchorX = roundf(world->mouseCoords[world->elevFlag].x);
		edit->anchorY = roundf(world->mouseCoords[world->elevFlag].y);		
		edit->anchorZ = floorf(world->mouseCoords[0].z);		
		edit->elevLock = world->elevFlag;
	}		
	
	union vec3 basePos = SubVec3(RealToVec3(edit->anchorX - 0.025f, 
						edit->anchorY - 0.025f,
						wallOffset + world->currentFloor * 4), 
             					world->cam.pos);	
	render_PushCube(group, basePos, 0.05f, 4.0f, colour);	
}

extern void edit_PlacementSelect(struct world_mode 	*state, 
				 struct pub_input 	*input)
{
	struct edit_state *edit = &state->edit;
	struct edit_placed_entity *current = &edit->current;
	
	int32_t alias = 0;	
	int32_t asset = 0;	
	if(input->buttons._C.endedDown) {
		alias = alias_stool;
		asset = asset_stool;
		input->buttons._C.endedDown = 0;
		
	} else if(input->buttons._T.endedDown) {
		alias = alias_table2x1;
		asset = asset_table;
		input->buttons._T.endedDown = 0;
		
	} else if(input->buttons._B.endedDown) {
		alias = alias_bar;
		asset = asset_bar;
		input->buttons._B.endedDown = 0;
		
	} else if(input->buttons._D.endedDown) {
		alias = alias_door;
		asset = asset_door;
		input->buttons._D.endedDown = 0;
		
	} else if(input->buttons._P.endedDown) {
		alias = alias_tiolet;
		asset = asset_tiolet;		
		input->buttons._P.endedDown = 0;
		
	} else if(input->buttons.DIR_SOUTH.endedDown) {
		alias = alias_support;
		asset = asset_support;	
		input->buttons.DIR_SOUTH.endedDown = 0;
		
	} else if(input->buttons.DIR_WEST.endedDown) {
		alias = alias_wall;
		asset = asset_wall;			
		input->buttons.DIR_WEST.endedDown = 0;
		
	} else if(input->buttons._U.endedDown) {
		alias = alias_bigsteps;
		asset = asset_steps;		
		input->buttons._U.endedDown = 0;
		
	} else if(input->buttons._A.endedDown) {
		alias = alias_steps;
		asset = asset_steps;	
		input->buttons._A.endedDown = 0;
		
	} else if(input->buttons.DIR_NORTH.endedDown) {
		current->ent = NULL;
		input->buttons.DIR_NORTH.endedDown = 0;
		return;
	} 		
	for(int32_t i = 0; i < ARRAY_COUNT(entStore); ++i) {
		if(entStore[i].alias == alias) {
			current->ent = &entStore[i]; 	
			current->id = asset;
			
			break;
		} 
		if(i == ARRAY_COUNT(entStore)) {
			INVALID_PATH;
		}								
	}
}

static void edit_PlaceFloor(struct world_mode *world)
{
	struct edit_state *edit = &world->edit;
	struct edit_placed_entity *current = &edit->current;
	union vec3 coords = world->mouseCoords[0];
	struct point3 basePos = {
		floorf(coords.x),
		floorf(coords.y),
		floorf(coords.z) / 4,
	};	
	if(basePos.x < 0 || basePos.x >= world->map.sizeX ||
	   basePos.y < 0 || basePos.y >= world->map.sizeY) {
		printf("out of bounds\n");
		return;
	}
	if(tile_GetElevation(&world->map, basePos) == 2) {
		printf("already exists\n\n");
		return;
	}	
	uint32_t index = entity_Add(world, type_struc);
	entity_InitStruc(world, index, current->ent->alias, 
		basePos.x, basePos.y, basePos.z, current->rot, 
		RealToVec3(0, 0, 0.0f), 0, 0);	
	index = entity_Add(world, type_struc);		
	entity_InitStruc(world, index, alias_floor, 
		basePos.x, basePos.y, basePos.z, current->rot, 
		RealToVec3(0, 0, 1.0f), floor_bmp_wood, 0);
	tile_SetElevation(&world->map, basePos, 2);		
}

static inline void SetStepElev(struct step_data testStep,
			       struct step_data *newStep)
{
	if(testStep.minElev == 0.0f) {
		newStep->minElev += 1.0f;	
		newStep->maxElev += 2.0f;	
	} else if(testStep.minElev == 1.0f) {
		newStep->minElev += 2.0f;	
		newStep->maxElev += 3.0f;	
	} else if(testStep.minElev == 2.0f) {
		newStep->minElev += 3.0f;	
		newStep->maxElev += 4.0f;	
	}	
}

static void edit_PlaceBigSteps(struct world_mode *world)
{
	struct edit_state *edit = &world->edit;
	struct edit_placed_entity *current = &edit->current;
	union vec3 coords = world->mouseCoords[0];
	struct point3 basePos = {
		floorf(coords.x),
		floorf(coords.y),
		floorf(coords.z),
	};	
	if(basePos.x < 0 || basePos.x >= world->map.sizeX ||
	   basePos.y < 0 || basePos.y >= world->map.sizeY) {
		printf("out of bounds\n");
		return;
	}	
	struct step_data newStep = {
		0.0f, 0.0f, current->rot
	};	
	
	for(int32_t i = 0; i < 4; ++i) {
		
		union vec3 newP = {};
		if(current->rot == 0) {
			newP.x = basePos.x;
			newP.y = basePos.y + i;
			newP.z = basePos.z + i;
		} else if(current->rot == 1) {
			newP.x = basePos.x - i;
			newP.y = basePos.y;
			newP.z = basePos.z + i;
		} else if(current->rot == 2) {
			newP.x = basePos.x;
			newP.y = basePos.y - i;
			newP.z = basePos.z + i;
		} else if(current->rot == 3) {
			newP.x = basePos.x + i;
			newP.y = basePos.y;
			newP.z = basePos.z + i;
		}
		
		newStep.minElev = i;
		newStep.maxElev = i + 1;
		
		struct point3 coordP = {
			newP.x,
			newP.y,
			basePos.z / 4,
		};		
		tile_SetStepData(&world->map, coordP, newStep);			
		tile_SetStepped(&world->map, coordP, true);	
		
		uint32_t index = entity_Add(world, type_struc);
		entity_InitStruc(world, index, alias_steps, 
			newP.x, newP.y, newP.z, 
			current->rot, RealToVec3(0.0, 0.0, 0.0), 0, 0);
	}
}

static void edit_PlaceSteps(struct world_mode *world)
{
	struct edit_state *edit = &world->edit;
	struct edit_placed_entity *current = &edit->current;
	union vec3 coords = world->mouseCoords[0];
	struct point3 basePos = {
		floorf(coords.x),
		floorf(coords.y),
		floorf(coords.z),
	};	
	if(basePos.x < 0 || basePos.x >= world->map.sizeX ||
	   basePos.y < 0 || basePos.y >= world->map.sizeY) {
		printf("out of bounds\n");
		return;
	}	
	bool validStep = false;
	struct step_data newStep = {
		0.0f, 1.0f, current->rot
	};
	switch(current->rot) {
	case 0: {		
		if(tile_GetElevation(&world->map, GET_N_COORD(basePos, 0)) == 2) {			
			validStep = true;
		} else if(tile_GetStepped(&world->map, GET_S_COORD(basePos, 0))) {
			struct step_data testStep = 
				tile_GetStepData(&world->map, GET_S_COORD(basePos, 0));
			SetStepElev(testStep, &newStep);	
			validStep = true;
		}
		break;
	} case 1: {
		if(tile_GetElevation(&world->map, GET_W_COORD(basePos, 0)) == 2) {			
			validStep = true;
		} else if(tile_GetStepped(&world->map, GET_E_COORD(basePos, 0))) {
			struct step_data testStep = 
				tile_GetStepData(&world->map, GET_E_COORD(basePos, 0));
			SetStepElev(testStep, &newStep);	
			validStep = true;
		}
		break;
	} case 2: {
		if(tile_GetElevation(&world->map, GET_S_COORD(basePos, 0)) == 2) {			
			validStep = true;
		} else if(tile_GetStepped(&world->map, GET_N_COORD(basePos, 0))) {
			struct step_data testStep = 
				tile_GetStepData(&world->map, GET_N_COORD(basePos, 0));
			SetStepElev(testStep, &newStep);	
			validStep = true;
		}
		break;
	} case 3: {
		if(tile_GetElevation(&world->map, GET_E_COORD(basePos, 0)) == 2) {			
			validStep = true;
		} else if(tile_GetStepped(&world->map, GET_W_COORD(basePos, 0))) {
			struct step_data testStep = 
				tile_GetStepData(&world->map, GET_W_COORD(basePos, 0));
			SetStepElev(testStep, &newStep);	
			validStep = true;
		}
		break;
	} default: INVALID_PATH;
	}
	if(validStep) {		
	
		tile_SetStepData(&world->map, basePos, newStep);
		tile_SetStepped(&world->map, basePos, true);	
		
		uint32_t index = entity_Add(world, type_struc);
		
		entity_InitStruc(world, index, current->ent->alias, 
			basePos.x, basePos.y, 
			basePos.z / 4 + newStep.minElev, 
			current->rot, RealToVec3(0, 0, 0), 0, 0);			    
	} else {
		printf("invalid placement\n");
	}
	
}

static int32_t edit_GetTableSlot(struct point3 chairP,
				 struct point3 tableP,
				 int8_t rotation)
{
	int32_t result = 0x0FFFFFFF;
	
	switch(rotation) {
	case 0: {
		if(chairP.x == tableP.x + 1 && chairP.y == tableP.y - 1) {
			result = 0;
		} else if(chairP.x == tableP.x && chairP.y == tableP.y - 1) {
			result = 1;
		} else if(chairP.x == tableP.x && chairP.y == tableP.y + 1) {
			result = 2;
		} else if(chairP.x == tableP.x + 1 && chairP.y == tableP.y + 1) {
			result = 3;
		}
		break;
	} case 1: {
		if(chairP.x == tableP.x + 1 && chairP.y == tableP.y + 1) {
			result = 0;
		} else if(chairP.x == tableP.x + 1 && chairP.y == tableP.y) {
			result = 1;
		} else if(chairP.x == tableP.x - 1 && chairP.y == tableP.y) {
			result = 2;
		} else if(chairP.x == tableP.x - 1 && chairP.y == tableP.y + 1) {
			result = 3;
		}
		break;
	} case 2: {
		if(chairP.x == tableP.x - 1 && chairP.y == tableP.y + 1) {
			result = 0;
		} else if(chairP.x == tableP.x && chairP.y == tableP.y + 1) {
			result = 1;
		} else if(chairP.x == tableP.x - 1 && chairP.y == tableP.y - 1) {
			result = 2;
		} else if(chairP.x == tableP.x && chairP.y == tableP.y - 1) {
			result = 3;
		}
		break;
	} case 3: {
		if(chairP.x == tableP.x - 1 && chairP.y == tableP.y - 1) {
			result = 0;
		} else if(chairP.x == tableP.x - 1 && chairP.y == tableP.y) {
			result = 1;
		} else if(chairP.x == tableP.x + 1 && chairP.y == tableP.y) {
			result = 2;
		} else if(chairP.x == tableP.x + 1 && chairP.y == tableP.y - 1) {
			result = 3;
		}
		break;
	} default: INVALID_PATH;
	}
	assert(result >= 0 && result < 4);
	return(result);
}

static void edit_PlaceStool(struct world_mode *world)
{
	struct edit_state *edit = &world->edit;
	struct edit_placed_entity *current = &edit->current;
	struct point3 basePos = Vec3ToPoint3(world->mouseCoords[world->elevFlag]);
	
	if(basePos.x < 0 || basePos.x >= world->map.sizeX ||
	   basePos.y < 0 || basePos.y >= world->map.sizeY) {
		printf("out of bounds\n");
		return;
	}
	struct point3 coords = {
		basePos.x,
		basePos.y,
		basePos.z / 4,
	};	
	union vec3 offset = RealToVec3(0, 0, 0);
	offset.z = tile_GetElevation(&world->map, coords) ? 1.0f : 0;
			
	uint32_t index = entity_Add(world, type_furn);
	entity_InitStruc(world, index, current->ent->alias, 
		basePos.x, basePos.y, basePos.z, current->rot, 
		offset, 0, 0);	
	int32_t slot = edit_GetTableSlot(coords, 
		Vec3ToPoint3(edit->parentEntity->pos.xyz), edit->parentEntity->rotation);	
	SET_FURNTARGET(world, GET_FURN(edit->parentEntity), 
		 GET_FURNFROMENTINDEX(world, index), slot);	
}

static void edit_PlaceObject(struct world_mode *world)
{
	struct edit_state *edit = &world->edit;
	struct edit_placed_entity *current = &edit->current;
	union vec3 coords = world->mouseCoords[world->elevFlag];
	
	struct point3 basePos = {
		floorf(coords.x),
		floorf(coords.y),
		floorf(coords.z),
	};	
	if(basePos.x < 0 || basePos.x >= world->map.sizeX ||
	   basePos.y < 0 || basePos.y >= world->map.sizeY) {
		printf("out of bounds\n");
		return;
	}
	
	int32_t xMin = 0;
	int32_t yMin = 0;
	int32_t xMax = 0;
	int32_t yMax = 0;
	
	int32_t zMin = 0;
	
	switch(current->rot) {
	case 0: {
		xMin = basePos.x;
		yMin = basePos.y;
		xMax = xMin + roundf(current->ent->dim.x);
		yMax = yMin + roundf(current->ent->dim.y);				
		zMin = basePos.z;				
		break;
	} case 1: {
		xMin = basePos.x - (roundf(current->ent->dim.y) - 1);
		yMin = basePos.y;
		xMax = xMin + roundf(current->ent->dim.y);
		yMax = yMin + roundf(current->ent->dim.x);
		zMin = basePos.z;		
		break;
	} case 2: {
		xMin = basePos.x - (roundf(current->ent->dim.x) - 1);
		yMin = basePos.y - (roundf(current->ent->dim.y) - 1);
		xMax = xMin + roundf(current->ent->dim.x);
		yMax = yMin + roundf(current->ent->dim.y);
		zMin = basePos.z;		
		break;
	} case 3: {
		xMin = basePos.x;
		yMin = basePos.y - (roundf(current->ent->dim.x) - 1);				
		xMax = xMin + roundf(current->ent->dim.y);
		yMax = yMin + roundf(current->ent->dim.x);
		zMin = basePos.z;		
		break;
	} default: INVALID_PATH;	
	}
	
	if(xMin < 0 || yMin < 0 || xMax > world->map.sizeX || yMax > world->map.sizeY) {
		printf("out of bounds 2\n");
		return;
	}		
	for(int32_t y = yMin; y < yMax; ++y) {
	for(int32_t x = xMin; x < xMax; ++x) {
		struct point3 coord = {
			x, y, zMin,	
		};
		struct tile_data tile = tile_GetTile(&world->map, coord);
		if(tile.impassable || x < 0 || y < 0) {
			printf("not placable here\n");
			return;
		} 				
	}	
	}
	struct point3 coord = {
		basePos.x,
		basePos.y,
		basePos.z / 4,
	};	
	uint32_t index = entity_Add(world, type_furn);
	union vec3 offset = RealToVec3(0, 0, 0);
	offset.z = tile_GetElevation(&world->map, coord) ? 1.0f : 0;
	entity_InitFurn(world, index, current->ent->alias, 
		    basePos.x, basePos.y, basePos.z , current->rot, 
		    offset);
				
	if(current->ent->alias == alias_stool || current->ent->alias == alias_wall) return;
	
	printf("placed at %d %d %d\n", basePos.x, basePos.y, basePos.z);
	
	for(int32_t y = yMin; y < yMax; ++y) {
	for(int32_t x = xMin; x < xMax; ++x) {
		struct point3 coord = {
			x,
			y,
			basePos.z / 4,
		};
		tile_SetPass(&world->map, coord , 1);
		printf("impass at %d %d %d\n", x, y, basePos.z / 4);	
	}	
	}		
}

static void edit_PlaceItem(struct world_mode *world)
{
	struct edit_state *edit = &world->edit;
	struct edit_placed_entity *current = &edit->current;
			
	switch(current->ent->alias) {
	case alias_wall: {
		break;
	} case alias_support: {
		edit_PlaceFloor(world);
		break;
	} case alias_steps: {
		edit_PlaceSteps(world);
		break;
	} case alias_stool: {
		if(edit->parentEntity) {
			edit_PlaceStool(world);
		}		
		break;
	} case alias_bigsteps: {
		edit_PlaceBigSteps(world);
		break;
	}default: {
		edit_PlaceObject(world);
		break;
	}	
	}
}

static bool edit_ChairPlacement(struct point3 chairP,
				struct point3 tableP,
				int8_t rotation)
{
	bool result = false;
	
	if(rotation == 0 && 
	  (chairP.x == tableP.x || chairP.x == tableP.x + 1) &&
	  (chairP.y == tableP.y - 1 || chairP.y == tableP.y + 1)) {
		result = true;
	} else if(rotation == 1 && 
	  (chairP.x == tableP.x - 1 || chairP.x == tableP.x + 1) &&
	  (chairP.y == tableP.y || chairP.y == tableP.y + 1)) {
		result = true;
	} else if(rotation == 2 && 
	  (chairP.x == tableP.x || chairP.x == tableP.x - 1) &&
	  (chairP.y == tableP.y - 1 || chairP.y == tableP.y + 1)) {
		result = true;
	} else if(rotation == 3 && 
	  (chairP.x == tableP.x - 1 || chairP.x == tableP.x + 1) &&
	  (chairP.y == tableP.y || chairP.y == tableP.y - 1)) {
		result = true;
	}	
	return(result);
}

static void edit_DisplayPlacement(struct world_mode 	*world,
				  struct temp_state 	*tState,
				  struct pub_input 	*input,
			 	  struct render_group 	*group,
				  struct render_group 	*textGroup)
{
	struct edit_state *edit = &world->edit;
	struct edit_placed_entity *current = &edit->current;	
	
	if(input->buttons._R.endedDown) {		
		++current->rot;					
		if(current->rot >= 4 ) current->rot = 0;		
		switch(current->rot) {
		case 0: {
			current->rotOffset = _0OFFSET;
			break;
		} case 1: {
			current->rotOffset = _1OFFSET;
			break;
		} case 2: {
			current->rotOffset = _2OFFSET;
			break;
		} case 3: {
			current->rotOffset = _3OFFSET;
			break;
		}	
		}	
		input->buttons._R.endedDown = 0;
	} 	
	union vec3 coords = world->mouseCoords[world->elevFlag];	
	if(current->ent->alias == alias_support) {coords = world->mouseCoords[0];} 
	
	union vec3 offset = {
		.x = floorf(coords.x) - world->cam.pos.x,
		.y = floorf(coords.y) - world->cam.pos.y,
		.z = current->ent->alias == alias_support ? coords.z : 
			coords.z + world->elevFlag * 0.5f,
	};	
	
	current->trans.offset = offset;				
	current->trans.offset = AddVec3(current->trans.offset, current->rotOffset);			
	
	if(edit->current.ent->alias == alias_wall)  {	
	
		if(input->buttons.lClick.isDown) {	
			edit_WallPrompt(world, group, true);		
			edit_WallPlan(world);
			if(edit->wallsInit && edit->newAnchor) {
				edit_WallDraw(world, group, tState);
			}							
		} else {						
			edit_WallPrompt(world, group, false);
		}
		
	} else if(edit->current.ent->alias == alias_stool) {		
		for(int32_t i = 0; i < world->furnCount; ++i) {
			
			struct entity *ent = GET_FURNBASEFROMINDEX(world, i);
			if(ent->alias == alias_table2x1) {
				if(edit_ChairPlacement(Vec3ToPoint3(coords), 
					Vec3ToPoint3(ent->pos.xyz), ent->rotation)) {
					render_PushMesh(group, asset_FindMesh(tState->assets, current->id, 1), 
						asset_FindBMP(tState->assets, current->id, 0), 
						current->trans.offset, EMPTY_V3, current->rot);	
					edit->parentEntity = ent;	
				} else {
					edit->parentEntity = NULL;
				}				
			}
		}
	} else if(edit->current.ent->alias == alias_bigsteps) {
		
		for(int32_t i = 0; i < 4; ++i) {				
			union vec3 newP = {};
			if(current->rot == 0) {
				newP.x = offset.x;
				newP.y = offset.y + i;
				newP.z = offset.z + i;
			} else if(current->rot == 1) {
				newP.x = offset.x - i + 1;
				newP.y = offset.y;
				newP.z = offset.z + i;
			} else if(current->rot == 2) {
				newP.x = offset.x + 1;
				newP.y = offset.y - i + 1;
				newP.z = offset.z + i;
			} else if(current->rot == 3) {
				newP.x = offset.x + i;
				newP.y = offset.y + 1;
				newP.z = offset.z + i;
			}	
			
			render_PushMesh(group, asset_FindMesh(tState->assets, current->id, 1), 
						asset_FindBMP(tState->assets, current->id, 0), 
						newP, EMPTY_V3, current->rot);	
		}
	} else {		
		render_PushMesh(group, asset_FindMesh(tState->assets, current->id, 1), 
						asset_FindBMP(tState->assets, current->id, 0), 
						current->trans.offset, EMPTY_V3, current->rot);		
	}
}

extern void edit_Editor(struct world_mode 	*world,
			struct temp_state 	*tState,
			struct pub_input 	*input,
			struct render_group 	*group,
			struct render_group 	*textGroup)
{		
#ifdef DEBUG	
	if(input->buttons.DIR_NORTH.endedDown) {
		DEBUG_AddChar(world);				
		input->buttons.DIR_NORTH.endedDown = 0;
	} if(input->buttons.f1.endedDown) {
		edit_SaveWorld(world);
		input->buttons.f1.endedDown = 0;
	}
		
#endif	
	edit_PlacementSelect(world, input);
			
	if(world->edit.current.ent != NULL) {
		edit_DisplayPlacement(world, tState, input, group, textGroup);	
	}	
}