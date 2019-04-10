/************************************************************************************		
__________     ___.   .__  .__                      	
\______   \__ _\_ |__ |  | |__| ____ _____    ____  
 |     ___/  |  \ __ \|  | |  |/ ___\\__  \  /    \ 	Entity specific fucntions
 |    |   |  |  / \_\ \  |_|  \  \___ / __ \|   |  \
 |____|   |____/|___  /____/__|\___  >____  /___|  /
                    \/             \/     \/     \/ 
		    
*************************************************************************************/

extern inline struct entity_char *GET_CHAR(struct entity *ent)
{
	return((struct entity_char *)ent->sub);
}

extern inline struct entity_furn *GET_FURN(struct entity *ent)
{
	return((struct entity_furn *)ent->sub);
}

extern inline struct entity_struc *GET_STRUC(struct entity *ent)
{
	return((struct entity_struc *)ent->sub);
}

extern inline struct entity *GET_CHARBASE(struct world_mode *world,
					  struct entity_char *chara)
{
	return(&world->entities[chara->entIndex]);
}

extern inline struct entity *GET_FURNBASE(struct world_mode *world,
					  struct entity_furn *furn)
{
	return(&world->entities[furn->entIndex]);
}

extern inline struct entity *GET_STRUCBASE(struct world_mode *world,
					    struct entity_struc *struc)
{
	return(&world->entities[struc->entIndex]);
}

#define ENT_BASE(a, b) _Generic((b), \
struct entity_char *: GET_CHARBASE, \
struct entity_furn *: GET_FURNBASE, \
struct entity_struc *: GET_STRUCBASE) ((a), (b))


extern inline struct entity_char *GET_CHARFROMINDEX(struct world_mode *world,
					               int32_t i)
{
	return(&world->characters[i]);
}

extern inline struct entity_furn *GET_FURNFROMINDEX(struct world_mode *world,
					               int32_t i)
{
	return(&world->furniture[i]);
}

extern inline struct entity_struc *GET_STRUCFROMENTINDEX(struct world_mode *world,
					               int32_t i)
{
	return(GET_STRUC(&world->entities[i]));
}

extern inline struct entity_furn *GET_FURNFROMENTINDEX(struct world_mode *world,
					               int32_t i)
{
	return(GET_FURN(&world->entities[i]));
}

extern inline struct entity_char *GET_CHARFROMENTINDEX(struct world_mode *world,
					               int32_t i)
{
	return(GET_CHAR(&world->entities[i]));
}

extern inline struct entity *GET_FURNBASEFROMINDEX(struct world_mode *world,
						   int32_t i)
{
	return(&world->entities[world->furniture[i].entIndex]);
}

extern inline struct entity_furn *GET_TARGETFURN(struct world_mode *world,
						 struct entity_char *chara)
{
	return((struct entity_furn *)chara->target.ent->sub);
}

extern inline struct entity *GET_TARGETFURNBASE(struct world_mode *world,
						struct entity_char *chara)
{
	return(GET_FURNBASE(world, (GET_TARGETFURN(world, chara))));
}

extern inline struct entity_char *GET_CHARACROSSBAR(struct world_mode *world,
					            struct entity_char *chara,
						    int32_t slot)
{
	assert(GET_TARGETFURNBASE(world, chara)->alias == alias_bar);
	return(GET_CHAR(GET_TARGETFURN(world, chara)->users[0].ent));
}

extern inline struct point3 GET_CHARCOORDS(struct world_mode *world,
					   struct entity_char *chara)
{
	struct point3 result =
		Vec3ToPoint3(GET_CHARBASE(world, chara)->pos.xyz);
	result.z /= 4;
	return(result);	
}

/*
*	Characters are in control of setting unsetting target,  
*	furniture should never to it itself
*/
extern inline void SET_TARGET(struct world_mode *world,
			      struct entity_char *chara,
			      struct entity_furn *furn,			      
			      uint8_t slot)
{
	chara->target.targetId = furn->entIndex;
	chara->target.ent = GET_FURNBASE(world, furn);
	chara->target.slot = slot;
	furn->users[slot].targetId = chara->entIndex;
	furn->users[slot].ent = GET_CHARBASE(world, chara); 	
}

extern inline void UNSET_TARGET(struct world_mode *world,
				struct entity_char *chara)
{
	struct entity_target nullTarget = {};
	GET_TARGETFURN(world, chara)->users[chara->target.slot] = nullTarget;
	chara->target = nullTarget;	
}

extern inline void SET_FURNTARGET(struct world_mode *world,
				  struct entity_furn *furnA,
				  struct entity_furn *furnB,			      
				  uint8_t slot)
{
	furnA->users[slot].targetId = furnB->entIndex;
	furnA->users[slot].ent = GET_FURNBASE(world, furnB);	
	furnB->users[1].targetId = furnA->entIndex;
	furnB->users[1].ent = GET_FURNBASE(world, furnA); 
	furnB->users[1].slot = slot;	
}

extern inline struct entity_char *GET_SOCIALTARGET(struct entity_char *chara)
{
	return(GET_CHAR(chara->socialTarget.ent));
}	

extern inline void SET_SOCIALTARGET(struct world_mode *world,
					  struct entity_char *charaA,
					  struct entity_char *charaB)
{
	charaA->socialTarget.targetId = charaB->entIndex;
	charaA->socialTarget.ent = GET_CHARBASE(world, charaB);
	charaB->socialTarget.targetId = charaA->entIndex;
	charaB->socialTarget.ent = GET_CHARBASE(world, charaA);
}	

extern inline void UNSET_SOCIALTARGET(struct world_mode *world,
				      struct entity_char *chara)
{	
	struct entity_target nullTarget = {};
	struct action_time nullAction = {};	
	GET_SOCIALTARGET(chara)->interaction = interactiondir_northull;
	GET_SOCIALTARGET(chara)->socialTime = nullAction;
	GET_SOCIALTARGET(chara)->socialTarget = nullTarget;
	chara->socialTime = nullAction;	
	chara->socialTarget = nullTarget;	
	chara->interaction = interactiondir_northull;		
}				  

extern inline struct entity_char *GET_TARGETCHAR(struct entity_furn *table,
						 int32_t slot)
{
	struct entity_furn *stool = GET_FURN(table->users[slot].ent);	
	if(stool->users[0].ent)	return(GET_CHAR(stool->users[0].ent));
	else return(NULL);
}

extern inline struct entity_furn *GET_TARGETTABLE(struct entity_char *chara)
{
	struct entity_furn *test = GET_FURN(chara->target.ent);	
	assert(test);
	return(GET_FURN(test->users[1].ent));
}

extern inline int8_t CHARISMA(struct entity_char *chara)
{
	return(chara->stats.attribs.charisma);
}

static struct cube3 SetBounds(struct entity *ent)
{
	union vec2 rOffset = {};
	
	if(ent->alias == alias_char) {
		rOffset.x -= 0.5;
		rOffset.y -= 0.5;
	} else {
		switch(ent->rotation) {
		case 0: 
		case 1: {
			break;
		} case 2: 
		case 3: {
			rOffset.x -= ent->dim.x + 1;
			rOffset.y -= ent->dim.y + 1;
			break;
		} default: {			
			assert(0);	
		}
		}
	}	
	
	union vec3 tMin = {
		.x = ent->pos.x + rOffset.x,
		.y = ent->pos.y + rOffset.y,
		.z = ent->pos.z + ent->dim.z,
	};
	union vec3 tMax = {
		.x = ent->pos.x + ent->dim.x,
		.y = ent->pos.y + ent->dim.y,
		.z = ent->pos.z + ent->dim.z,
	};
	union vec3 nMin = {
		.x = ent->pos.x + rOffset.x,
		.y = ent->pos.y + rOffset.y,
		.z = ent->pos.z,
	};
	union vec3 nMax = {
		.x = ent->pos.x + ent->dim.x,
		.y = ent->pos.y + ent->dim.y,
		.z = ent->pos.z + ent->dim.z,
	};
	union vec3 eMin = {
		.x = ent->pos.x + rOffset.x,
		.y = ent->pos.y + rOffset.y,
		.z = ent->pos.z,
	};
	union vec3 eMax = {
		.x = ent->pos.x + ent->dim.x,
		.y = ent->pos.y + ent->dim.y,
		.z = ent->pos.z + ent->dim.z,
	};
	union vec3 sMin = {
		.x = ent->pos.x + rOffset.x,
		.y = ent->pos.y + rOffset.y,
		.z = ent->pos.z,
	};
	union vec3 sMax = {
		.x = ent->pos.x + ent->dim.x,
		.y = ent->pos.y + ent->dim.y,
		.z = ent->pos.z + ent->dim.z,
	};
	union vec3 wMin = {
		.x = ent->pos.x + rOffset.x,
		.y = ent->pos.y + rOffset.y,
		.z = ent->pos.z,
	};
	union vec3 wMax = {
		.x = ent->pos.x + ent->dim.x,
		.y = ent->pos.y + ent->dim.y,
		.z = ent->pos.z + ent->dim.z,
	};
	union vec3 bMin = {
		.x = ent->pos.x + rOffset.x,
		.y = ent->pos.y + rOffset.y,
		.z = ent->pos.z,
	};
	union vec3 bMax = {
		.x = ent->pos.x + ent->dim.x,
		.y = ent->pos.y + ent->dim.y,
		.z = ent->pos.z,
	};
	struct cube3 bound = {};
	bound.t.min = tMin;
	bound.t.max = tMax;
	bound.n.min = nMin;
	bound.n.max = nMax;
	bound.e.min = eMin;
	bound.e.max = eMax;
	bound.s.min = sMin;
	bound.s.max = sMax;
	bound.w.min = wMin;
	bound.w.max = wMax;
	bound.b.min = bMin;
	bound.b.max = bMax;
	
	return(bound);
}

static inline void RemoveFromMid(struct entity *current)
{
	current->prev->next = current->next;
	current->next->prev = current->prev;
	current->prev = NULL;
	current->next = NULL;
}

static inline void RemoveFromEnd(struct world_mode *world,
				 struct entity *current)
{
	world->lastOnscreen = current->prev;
	current->prev->next = NULL;
	current->prev = NULL;
	current->next = NULL;
}

extern inline void entity_RemoveFromList(struct world_mode *world,
					 struct entity *current)
{
	if(current->next != NULL && current->prev != NULL) {
		RemoveFromMid(current);
	} else if(current->next == NULL && current->prev != NULL) {
		RemoveFromEnd(world, current);
	} else INVALID_PATH;
}
/*
*		TODO! track last list object avoid loop and try and make it not buggy as shit
*
*/
extern inline void entity_AddToList(struct world_mode *world,
				struct entity *new)
{
	if(!world->lastOnscreen) {
		struct entity *left = &world->entities[0];
	
		left->next = new;
		new->prev = left;
		new->next = NULL;
		world->lastOnscreen = new;
	} else {
		world->lastOnscreen->next = new;
		new->prev = world->lastOnscreen;
		new->next = NULL;
		world->lastOnscreen = new;
	}
	
}
/*
		

*/
extern inline uint32_t entity_AddChar(struct world_mode *world, 
				      uint32_t oldIndex)
{
	assert(world->charCount < ARRAY_COUNT(world->characters));
	
	uint32_t index = world->charCount++;
	world->characters[index].entIndex = oldIndex;
	
	return(index);
}

extern inline uint32_t entity_AddFurn(struct world_mode *world, 
				      uint32_t oldIndex)
{
	assert(world->furnCount < ARRAY_COUNT(world->furniture));
	
	uint32_t index = world->furnCount++;
	world->furniture[index].entIndex = oldIndex;
			
	return(index);
}

extern inline uint32_t entity_AddStruc(struct world_mode *world, 
				       uint32_t oldIndex)
{
	assert(world->furnCount < ARRAY_COUNT(world->structures));
	
	uint32_t index = world->strucCount++;
	world->structures[index].entIndex = oldIndex;
			
	return(index);
}

extern uint32_t entity_Add(struct world_mode *world, 
			   enum entity_type type)
{
	assert(world->entityCount < ARRAY_COUNT(world->entities));
	
	uint32_t index = world->entityCount++;
	
	switch(type) {
	case type_char: {
		uint32_t subIndex = entity_AddChar(world, index);
		world->entities[index].sub = &world->characters[subIndex];				
		break;
	} case type_struc: {
		uint32_t subIndex = entity_AddStruc(world, index);
		world->entities[index].sub = &world->structures[subIndex];				
		break;
	} case type_furn: {
		uint32_t subIndex = entity_AddFurn(world, index);
		world->entities[index].sub = &world->furniture[subIndex];
		break;
	} case typedir_northull: {
		break;
	} default: INVALID_PATH;	
	}				
	world->entities[index].type = type; 
	return(index);
}

extern inline struct entity *entity_Get(struct world_mode *world,
					uint32_t index)
{
	struct entity *ent = 0;
	
	if((index > 0) && (index < ARRAY_COUNT(world->entities))) {
		ent = &world->entities[index];
	}
	return(ent);
}

extern union vec3 DEF_CHAROFFSETELEV(float z)
{
	return(RealToVec3(-0.5f, 0.25f, z));
}

extern union vec3 DEF_CHAROFFSET(void)
{
	return(RealToVec3(-0.5f, 0.25f, 0.0f));
}

static inline void entity_InitRotOffset(union vec3 *offset,
					int32_t rot)
{
	switch(rot) {
	case 0: {
		break;
	} case 1: {
		*offset = AddVec3(_1OFFSET, *offset);
		break;
	} case 2: {
		*offset = AddVec3(_2OFFSET, *offset);
		break;
	} case 3: {
		*offset = AddVec3(_3OFFSET, *offset);
		break;
	} default: INVALID_PATH;	
	}
}

extern inline void entity_SetFacing(struct entity *ent,
				    int32_t rotation)
{
	ent->rotation = rotation;
	entity_InitRotOffset(&ent->offset, rotation);
}

static inline union vec3 entity_InitDim(struct entity *ent,
					struct entity_store *base)
{
	union vec3 result = base->dim;
	
	if(ent->type != type_char) {
		if(ent->rotation == 0 || ent->rotation == 2) {
			result.x = base->dim.x;	 
			result.y = base->dim.y;	 
		} else if(ent->rotation == 1 || ent->rotation == 3) {
			result.x = base->dim.y;	 
			result.y = base->dim.x;	
		} else INVALID_PATH;	
	}
	
	return(result);
}
		
static inline struct entity_store *entity_GetEntityFromStore(enum entity_alias alias)
{
	struct entity_store *result = NULL;
	
	for(int32_t i = 0; i < ARRAY_COUNT(entStore); ++i) {
		if(entStore[i].alias == alias) {
			result = &entStore[i];
			break;
		}
	}
	return(result);
}

static inline struct entity *entity_GetBaseData(struct world_mode *world, 
						uint32_t index,
						enum entity_alias alias,					
						uint32_t x, 
						uint32_t y,
						uint32_t z,
						uint32_t rot)
{
	struct entity *ent = entity_Get(world, index);
	struct entity_store *base = entity_GetEntityFromStore(alias);	
	assert(ent && base);
	
	ent->index = index;
	ent->alias = base->alias;
	ent->pos.x = (float)x;
	ent->pos.y = (float)y;				
	ent->pos.z = (float)z;
	ent->pos.w = 1.0f;						
	ent->rotation = rot;	
	ent->state = entstate_offscreen;			
	ent->next = NULL;
	ent->prev = NULL;
	ent->dim = entity_InitDim(ent, base);
	
	return(ent);
}

extern void entity_InitStruc(struct world_mode *world, 
			     uint32_t index,
			     enum entity_alias alias,					
			     uint32_t x, 
			     uint32_t y,
			     uint32_t z,
			     uint32_t rot,
			     union vec3 offset,
			     uint8_t bmp,
			     uint8_t mesh)
{
	struct entity *ent = entity_GetBaseData(world, index, 
		alias, x, y, z, rot);
	ent->offset = offset;	
	struct entity_struc *struc = GET_STRUC(ent);													
	assert(struc);
	struc->bmpType = bmp;
	struc->meshType = mesh;
	//entity_InitRotOffset(&ent->offset, ent->rotation);
	switch(alias) {	
	case alias_floor: {
		ent->offset.z -= 0.1f;
			
		break;
	} default: break;	
	}
}

extern void entity_InitFurn(struct world_mode *world, 
			    uint32_t index,
			    enum entity_alias alias,					
			    uint32_t x, 
			    uint32_t y,
			    uint32_t z,
			    uint32_t rot,
			    union vec3 offset)
{
	struct entity *ent = entity_GetBaseData(world, index, 
		alias, x, y, z, rot);
	ent->offset = offset;	
	struct entity_furn *furn = GET_FURN(ent);													
	assert(furn);
	
	entity_InitRotOffset(&ent->offset, ent->rotation);	
	switch(alias){
	case alias_bar: {
		assert(world->alerts.barCount < ARRAY_COUNT(world->alerts.bars));
		furn->maxUsers = 2;
		world->alerts.bars[world->alerts.barCount++] = ent;
		break;
	} case alias_table2x1: {
		furn->maxUsers = 4;								
		break;
	} case alias_stool: {
		furn->maxUsers = 2;
		break;
	} case alias_tiolet: {
		furn->maxUsers = 1;
		break;
	} default: break;
	}									
	
	ent->bound = SetBounds(ent);	
}

extern void entity_InitChar(struct world_mode *world, 
			    uint32_t index)
{
	struct entity *ent = entity_GetBaseData(world, index, 
		alias_char, 0, 0, 0, 0);
	struct entity_char *chara = GET_CHAR(ent);	
	assert(chara);
	
	chara->intent.type = intent_null;	
	ent->pos.x += 0.5f;	
	ent->pos.y += 0.5f;	
	ent->offset = DEF_CHAROFFSET();				
	ent->rotation = 5;	
	ent->state = entstate_dormant;	

	ent->bound = SetBounds(ent);	
}

extern struct entity_struc *entity_FindWallByPos(struct world_mode *world,
				                 struct point3 pos,
						 int32_t rot)
{
	struct entity_struc *result = NULL;
	printf("looking for %d %d rot: %d\n", pos.x, pos.y, rot);
	for(int32_t i = 0; i < world->strucCount; ++i) {	
		struct entity_struc *test = &world->structures[i];
		struct entity *base = ENT_BASE(world, test);
		if(base->alias != alias_wall) {continue;}
		
		struct point3 testPos = test->coords;
		printf("found %d %d rot: %d\n", testPos.x, testPos.y, base->rotation);
		if(COMP_POINT(testPos, pos) && base->rotation == rot) {
			result = test;
			break;	
		}
	}	
	return(result);
}

extern struct entity_struc *entity_FindPillarByPos(struct world_mode *world,
				                   struct point3 pos)
{
	struct entity_struc *result = NULL;
	
	for(int32_t i = 0; i < world->strucCount; ++i) {	
		struct entity_struc *test = &world->structures[i];
		struct entity *base = ENT_BASE(world, test);
		if(base->alias != alias_pillar) {continue;}
		
		struct point3 testPos = test->coords;
		
		if(COMP_POINT(testPos, pos)) {
			result = test;
			break;	
		}
	}	
	return(result);
}

/*
*		Moves a given enitity along a given path.
*		Runs once per frame for each moving entity.
*/
extern void entity_Move(struct entity_char *chara,
			struct world_mode *world)
{
	if(chara->pathLength <= 0) {
		chara->activePath = 0;	
		return;
	}
	
	union vec2 progress = {};			
	const float card = 0.05f;
	const float diag = sqrt((card * card) + (card * card));
	const float cardTot = 1.0f;
	const float diagTot = sqrt((cardTot * cardTot) + (cardTot * cardTot));	
		
	struct node_list *node = chara->currentNode;	
	
	switch(node->node.dir) {
	case dir_north: {
		progress.y = card;
		node->pathProgressC += card; 
		break;
	} case dir_northeast: {
		progress.x = card;
		progress.y = card;
		node->pathProgressD += diag; 
		break;
	} case dir_east: {
		progress.x = card;
		node->pathProgressC += card; 
		break;
	} case dir_southeast: {
		progress.x = card;
		progress.y = -card;
		node->pathProgressD += diag; 
		break;
	} case dir_south: {
		progress.y = -card;
		node->pathProgressC += card; 
		break;
	} case dir_southwest: {
		progress.x = -card;
		progress.y = -card;
		node->pathProgressD += diag; 
		break;
	} case dir_west: {
		progress.x = -card;
		node->pathProgressC += card; 
		break;
	} case dir_northwest: {
		progress.x = -card;
		progress.y = card;
		node->pathProgressD += diag; 
		break;
	} case dir_null: {					
		break;
	} default: {				
		INVALID_PATH;
		break;
	} 	
	}
	struct entity *ent = GET_CHARBASE(world, chara);
	if(node->node.dir) {ent->rotation = node->node.dir - 1;} 
	else {printf("%d %f %f\n", chara->pathLength,
	node->pathProgressC, node->pathProgressD);}
	ent->pos.x += progress.x;
	ent->pos.y += progress.y;
	ent->bound = SetBounds(ent);
	if(tile_GetStepped(&world->map, Vec3ToPoint3(ent->pos.xyz))) {
		
		struct step_data step = tile_GetStepData(
			&world->map, Vec3ToPoint3(ent->pos.xyz));
			
		if(step.rotation == 0) {
			if(node->node.dir == dir_north) {
				ent->offset.z += card;					
			} else if(node->node.dir == dir_south) {
				ent->offset.z = step.minElev - card;
			} else {
				//INVALID_PATH;
			}
		} else if(step.rotation == 1) {
			if(node->node.dir == dir_west) {
				ent->offset.z += card;					
			} else if(node->node.dir == dir_east) {
				ent->offset.z -= card;
			} else {
				//INVALID_PATH;
			}
		} else if(step.rotation == 2) {
			if(node->node.dir == dir_south) {
				ent->offset.z += card;					
			} else if(node->node.dir == dir_north) {
				ent->offset.z -= card;
			} else {
				//INVALID_PATH;
			}
		} else if(step.rotation == 3) {
			if(node->node.dir == dir_east) {
				ent->offset.z += card;					
			} else if(node->node.dir == dir_west) {
				ent->offset.z -= card;
			} else {
				//INVALID_PATH;
			}
		} else {
			INVALID_PATH;
		}	
			
	} else {
		ent->offset.z = 
			tile_GetElevationOffset(&world->map, Vec3ToPoint3(ent->pos.xyz));	
						   
	}
	
	if(node->pathProgressC >= cardTot || 	
	   node->pathProgressD >= diagTot) {		
		struct node_list *nodePtr = chara->currentNode;	
		chara->currentNode = --nodePtr;							
		--chara->pathLength;
		
		chara->path->pathProgressC = 0.0f;
		chara->path->pathProgressD = 0.0f;	

		ent->pos.x = nodePtr->node.p.x + 0.5f;	
		ent->pos.y = nodePtr->node.p.y + 0.5f;
		ent->pos.z = nodePtr->node.p.z * 4;	
		
		if(chara->pathLength == 0) {
			chara->activePath = 0;	
		}
	} 
}

