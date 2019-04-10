/************************************************************************************		
__________     ___.   .__  .__                      	
\______   \__ _\_ |__ |  | |__| ____ _____    ____  
 |     ___/  |  \ __ \|  | |  |/ ___\\__  \  /    \ 	Ai actions, and needs/work  
 |    |   |  |  / \_\ \  |_|  \  \___ / __ \|   |  \	processing
 |____|   |____/|___  /____/__|\___  >____  /___|  /
                    \/             \/     \/     \/ 
		    @Standardize the way needs are increment? 
		    @Action/animation times are current processed here too,
		     maybe pull it out to it's own thing?
0*************************************************************************************/

static void ai_ConstrainNeeds(struct entity_char *chara)
{
	if(chara->stats.drunk < 0) {chara->stats.drunk = 0;}
	else if(chara->stats.drunk > 1) {chara->stats.drunk = 1;}
	
	if(chara->stats.thirst < 0) {chara->stats.thirst = 0;}
	else if(chara->stats.thirst > 1) {chara->stats.thirst = 1;}
	
	if(chara->stats.bladder < 0) {chara->stats.bladder = 0;}
	else if(chara->stats.bladder > 1) {chara->stats.bladder = 1;}
}

static uint64_t ai_GetActionTime(struct char_intent intent)
{
	uint64_t result = 0;
	
	switch(intent.type) {
	case intent_drink: {
		switch(intent.drink.stage) {
		case drink_drink: {
			result = DRINK_TIME;
			break;
		}	
		}		
		break;
	} case intent_purchase: {
		switch(intent.purchase.stage) {
		case purchase_buy: {
			result = SERVE_TIME;
			break;
		}		
		}
		break;
	} case intent_serve: {
		switch(intent.serve.stage) {
		case serve_sell: {
			result = SERVE_TIME;
			break;
		}		
		}
		break;
	} case intent_tiolet: {
		switch(intent.tiolet.stage) {
		case tiolet_defecate: {
			result = TIOLET_TIME;
			break;
		}		
		}
		break;
	}	
	}
	return(result);
}

static inline void ai_StartTimedEvent(struct world_mode *world,
				      struct entity_char *chara)
{
	chara->actionTime.actionStart = world->timer.currentTime;
	chara->actionTime.actionTime = ai_GetActionTime(chara->intent);	
	chara->actionTime.lastTick = chara->actionTime.actionStart;
}

static inline bool ai_EndTimedEvent(struct world_mode *world,
				    struct entity_char *chara)
{
	chara->actionTime.currentActionTime = 
		world->timer.currentTime - chara->actionTime.actionStart;
	return(chara->actionTime.currentActionTime >= chara->actionTime.actionTime);
}

static inline bool ai_TickIncrement(struct world_mode *world,
				    struct entity_char *chara)
{
	bool result = false;
	
	if(world->timer.currentTime - chara->actionTime.lastTick > 100000L) {
		result = true;
		chara->actionTime.lastTick = world->timer.currentTime;
	}
	return(result);	
}

static bool GetEmptyFurn(struct world_mode 	*world,
			 struct entity_char 	*chara,			
			 enum entity_alias 	alias)
{
	bool result = false;
	struct entity_furn *nearestFurn = NULL;
	float nearDist = 999999.0f;
	for(int32_t i = 0; i < world->furnCount; ++i) {		
	
		struct entity_furn *furn = GET_FURNFROMINDEX(world, i);
		
		if(GET_FURNBASEFROMINDEX(world, i)->alias == alias && 
		   !furn->users[0].ent) {
			   
			union vec3 charaP = GET_CHARBASE(world, chara)->pos.xyz;
			union vec3 furnP = GET_FURNBASEFROMINDEX(world, i)->pos.xyz;			
			if(!nearestFurn) {
				nearestFurn = furn;
				nearDist = abs(charaP.x - furnP.x) +
					abs(charaP.y - furnP.y);
			} else {
				float newDist = abs(charaP.x - furnP.x) +
					abs(charaP.y - furnP.y);
				if(newDist < nearDist) {
					nearDist = newDist;
					nearestFurn = furn;
				}
			}							
		}
	}
	if(nearestFurn) {
		SET_TARGET(world, chara, nearestFurn, 0);			
		result = true;	
	}	
	return(result);
}

static struct point3 ai_GetTioletPos(struct world_mode *world,
				      struct entity_char *user,
				      struct entity_furn *tiolet)
{
	struct point3 result = {};	
	struct entity *tioletBase = GET_FURNBASE(world, tiolet);	
	
	switch(tioletBase->rotation) {
	case 0: {
		result.x = tioletBase->pos.x; 
		result.y = tioletBase->pos.y - 1; 
		break;
	} case 1: {
		result.x = tioletBase->pos.x + 1; 
		result.y = tioletBase->pos.y; 
		break;
	} case 2: {
		result.x = tioletBase->pos.x; 
		result.y = tioletBase->pos.y + 1; 
		break;
	} case 3: {
		result.x = tioletBase->pos.x - 1; 
		result.y = tioletBase->pos.y; 
		break;
	} default: INVALID_PATH;	
	}
	return(result);
}				      

/*
*	BAR INFO :)

	staff slot is 0, punter slot is 1 
*
*/

static struct point3 ai_GetBarSlotPos(struct world_mode *world,
				      struct entity_char *barUser,
				      struct entity_furn *bar)
{
	struct point3 result = {};	
	struct entity *barBase = GET_FURNBASE(world, bar);	
	
	switch(barBase->rotation) {
	case 0: {						
		switch(barUser->target.slot) {
		case 0: {								
			result.x = barBase->pos.x;
			result.y = barBase->pos.y + 1;
			result.z = barBase->pos.z;
			break;
		} case 1: {
			result.x = barBase->pos.x;
			result.y = barBase->pos.y - 1;
			result.z = barBase->pos.z;
			break;
		} default: {
			printf("ERROR: Slot %d is not valid.\n", barUser->target.slot);
			INVALID_PATH;
		}		
		}
		break;
	} case 1: {
		switch(barUser->target.slot) {
		case 0: {								
			result.x = barBase->pos.x - 1;
			result.y = barBase->pos.y;
			result.z = barBase->pos.z;
			break;
		} case 1: {
			result.x = barBase->pos.x + 1;
			result.y = barBase->pos.y;
			result.z = barBase->pos.z;
			break;
		} default: {
			printf("ERROR: Slot %d is not valid.\n", barUser->target.slot);
			INVALID_PATH;
		}
		}						
		break;
	} case 2: {
		switch(barUser->target.slot) {
		case 0: {								
			result.x = barBase->pos.x;
			result.y = barBase->pos.y - 1;
			result.z = barBase->pos.z;
			break;
		} case 1: {
			result.x = barBase->pos.x;
			result.y = barBase->pos.y + 1;
			result.z = barBase->pos.z;
			break;
		} default: {
			printf("ERROR: Slot %d is not valid.\n", barUser->target.slot);
			INVALID_PATH;
		}	
		}		
		break;
	} case 3: {
		switch(barUser->target.slot) {
		case 0: {								
			result.x = barBase->pos.x + 1;
			result.y = barBase->pos.y;
			result.z = barBase->pos.z;
			break;
		} case 1: {
			result.x = barBase->pos.x - 1;
			result.y = barBase->pos.y;
			result.z = barBase->pos.z;
			break;
		} default: {
			printf("ERROR: Slot %d is not valid.\n", barUser->target.slot);
			INVALID_PATH;
		}	
		}		
		break;
	}
	}
	
	return(result);
}

static bool ai_CheckBar(struct world_mode 	*world,
			struct entity_char 	*chara)
{	
	bool result = false;
	
	for(int32_t i = 0; i < world->alerts.barCount; ++i) {
		struct entity *ent = world->alerts.bars[i];
		struct entity_furn *furn = GET_FURN(ent);
		
		if(!furn->users[1].ent) continue;		
		
		if(GET_CHAR(furn->users[1].ent)->intent.purchase.stage == purchase_wait) {				
			SET_TARGET(world, chara, furn, 0);			
			result = true;
			return(result);	
		} 							
	}
	return(result);
}

static bool ai_CheckInv(const enum item_type	type, 
			struct item 		*inv)
{		
	for(int32_t i = 0; i < 4; ++i) {
		if(inv[i].type == type) {			
			return(true);
		}
	}
	return(false);
}

static inline void ai_DestroyItem(struct item *invSlot, 
				  struct chara_stats *stats)
{
	assert(stats->itemCount > 0);
	struct item dummy = {};
	*invSlot = dummy;
	--stats->itemCount;
} 

static void ai_EmptyInvOfItem(struct chara_stats *stats,
			      enum item_type type)
{
	for(int32_t i = 0; i < ARRAY_COUNT(stats->inv); ++i) {
		if(stats->inv[i].type == type) {
			ai_DestroyItem(&stats->inv[i], stats);
		}
	}
	assert(stats->itemCount >= 0);
	assert(stats->itemCount <= ARRAY_COUNT(stats->inv));
}

static bool ai_AddItem(struct chara_stats *stats, enum item_type type) 
{
	for(int32_t i = 0; i < ARRAY_COUNT(stats->inv); ++i) {
		if(itemStore[i].type == type) {
			stats->inv[stats->itemCount++] = itemStore[i];			
			return(true);
		}
	} 
	return(false);
}

static bool PunterSearchBar(struct world_mode *world,
		            struct entity_char *chara)
{
	bool result = false;
	
	//TODO cut out this loop, make all bars (how many total?) instantly accessable 
	for(int32_t i = 0; i < world->furnCount; ++i) {
		
		struct entity *ent = GET_FURNBASEFROMINDEX(world, i);				
		if(ent->alias == alias_bar && !world->furniture[i].users[1].ent) {			
			SET_TARGET(world, chara, GET_FURN(ent), 1);							
			result = true;		
			break;	
		} 							
	}	
	return(result);
}

static inline bool IsBeingServed(struct entity_char *chara)
{
	bool result = false;
	
	struct entity_furn *bar = GET_FURN(chara->target.ent);
	if(bar->users[0].ent) {
		
		struct entity_char *barman = GET_CHAR(bar->users[0].ent);
		assert(barman->stats.isStaff);
			
		if(barman->target.ent == chara->target.ent  &&
		   barman->intent.type == intent_serve && 
		   barman->intent.serve.stage == serve_sell) {
			result = true;
		}
	}
	return(result);
}

static inline int32_t SetTableFacing(struct world_mode *world,
				     struct entity_char *user)
{
	int32_t result = 0; 
	struct entity_furn *table = GET_TARGETTABLE(user);
	
	switch(GET_FURNBASE(world, table)->rotation) {
	case 1: {
		if(GET_FURN(user->target.ent)->users[1].slot == 0 ||
		GET_FURN(user->target.ent)->users[1].slot == 1) {
			result = 6;
		} else if (GET_FURN(user->target.ent)->users[1].slot == 2 ||
		GET_FURN(user->target.ent)->users[1].slot == 3) {
			result = 2;
		}
		break;
	} case 0: {
		if(GET_FURN(user->target.ent)->users[1].slot == 0 ||
		GET_FURN(user->target.ent)->users[1].slot == 1) {
			result = 0;
		} else if (GET_FURN(user->target.ent)->users[1].slot == 2 ||
		GET_FURN(user->target.ent)->users[1].slot == 3) {
			result = 4;
		}
		break;
	} case 3: {
		if(GET_FURN(user->target.ent)->users[1].slot == 0 ||
		GET_FURN(user->target.ent)->users[1].slot == 1) {
			result = 2;
		} else if (GET_FURN(user->target.ent)->users[1].slot == 2 ||
		GET_FURN(user->target.ent)->users[1].slot == 3) {
			result = 6;
		}
		break;
	} case 2: {
		if(GET_FURN(user->target.ent)->users[1].slot == 0 ||
		GET_FURN(user->target.ent)->users[1].slot == 1) {
			result = 4;
		} else if (GET_FURN(user->target.ent)->users[1].slot == 2 ||
		GET_FURN(user->target.ent)->users[1].slot == 3) {
			result = 0;
		}
		break;
	}	
	}
	return(result);
}

static inline int32_t SetBarFacing(struct world_mode *world,
				   struct entity_char *user)
{
	int32_t result = 0;
	
	
	struct entity *barBase = GET_FURNBASE(world, GET_FURN(user->target.ent));
	struct entity *userBase = GET_CHARBASE(world, user);
		
	if((int32_t)barBase->pos.x < (int32_t)userBase->pos.x) {
		result = 6;
	} else if((int32_t)barBase->pos.x > (int32_t)userBase->pos.x) {
		result = 2;
	} else if((int32_t)barBase->pos.y < (int32_t)userBase->pos.y) {
		result = 4;
	} else if((int32_t)barBase->pos.y > (int32_t)userBase->pos.y) {
		result = 0;
	}	
	return(result);
}

static void Serve(struct world_mode *world,
		  struct entity_char *barman)
{
	assert(barman->intent.type == intent_serve);
	
	struct char_intent *intent = &barman->intent;
	
	switch(intent->serve.stage) {
	case serve_init: {
		intent->serve.stage = serve_wait;
		break;
	} case serve_walk: {		
		entity_Move(barman, world);
		if(!barman->activePath) {						
			intent->serve.stage = serve_sell;
			GET_CHARBASE(world, barman)->rotation = SetBarFacing(world, barman);
			ai_StartTimedEvent(world, barman);	
		}
		break;
	} case serve_sell: {		
		struct entity_furn *furn = GET_FURN(barman->target.ent);
		if(ai_EndTimedEvent(world, barman)) {
			if(!furn->users[1].ent) {
				barman->intent.type = intent_null;	
				UNSET_TARGET(world, barman);		
			}
		}		
		break;
	} case serve_wait: {		
		if(ai_CheckBar(world, barman)) {
						
			struct point3 currentPos = Vec3ToPoint3(
				GET_CHARBASE(world, barman)->pos.xyz);
			struct point3 targetPos = ai_GetBarSlotPos(
				world, barman, GET_TARGETFURN(world, barman));
			
			if(CompPoint3(targetPos, currentPos)) {					
				intent->serve.stage = serve_sell;
			} else {				
				barman->currentNode = path_GetPath(currentPos, targetPos,
					barman, world, barman->path);	
				if(!barman->currentNode) {
					barman->intent.type = intent_null;
					UNSET_TARGET(world, barman);
#ifdef DEBUG
					printf("%s %s cannot find path to bar\n", 
						barman->stats.firstName,
						barman->stats.lastName);
#endif								
					break;
				} else {
					intent->serve.stage = serve_walk;				
					barman->activePath = 1;
				}				
			}					
		}
		break;
	}	
	}
}

static void Tiolet(struct world_mode *world,
		   struct entity_char *user)
{
	assert(user->intent.type == intent_tiolet);
	struct char_intent *intent = &user->intent;
	
	switch(intent->tiolet.stage) {
	case tiolet_init: {
		if(GetEmptyFurn(world, user, alias_tiolet)) {
			struct point3 currentPos = Vec3ToPoint3(
				GET_CHARBASE(world, user)->pos.xyz);
			struct point3 targetPos = ai_GetTioletPos(
				world, user, GET_TARGETFURN(world, user));
			
			user->currentNode = path_GetPath(currentPos, targetPos,						
				user, world, user->path);
			if(!user->currentNode) {
				user->intent.type = intent_null;	
				UNSET_TARGET(world, user);
#ifdef DEBUG
				printf("%s %s cannot find path to tiolet\n", 
				   user->stats.firstName,
				   user->stats.lastName);
#endif								
				break;
			} else {
				intent->drink.stage = tiolet_walk;					
				user->activePath = 1;
			}				
		}
		break;
	} case tiolet_walk: {
		entity_Move(user, world);
		if(!user->activePath) {						
			intent->purchase.stage = tiolet_defecate;
			ai_StartTimedEvent(world, user);	
		}		
		break;
	} case tiolet_defecate: {
		if(ai_TickIncrement(world, user)) {
			user->stats.bladder -= 0.05f;			
		}			
		if(ai_EndTimedEvent(world, user)) {			
			user->intent.type = intent_null;
			UNSET_TARGET(world, user);			
		}		
		break;
	}
	}
}

static void Drink(struct world_mode *world,
		  struct entity_char *punter)
{
	assert(punter->intent.type == intent_drink);		
	struct char_intent *intent = &punter->intent;
	
	switch(intent->drink.stage) {
	case drink_init: {
		if(GetEmptyFurn(world, punter, alias_stool)) {			
			
			struct point3 currentPos = Vec3ToPoint3(
				GET_CHARBASE(world, punter)->pos.xyz);
			struct point3 targetPos = Vec3ToPoint3(
				GET_TARGETFURNBASE(world, punter)->pos.xyz);
				
			punter->currentNode = path_GetPath(
				currentPos, targetPos, punter, world, punter->path);
				
			if(!punter->currentNode) {
				punter->intent.type = intent_null;
				UNSET_TARGET(world, punter);
#ifdef DEBUG
				printf("%s %s cannot find path to stool\n", 
				   punter->stats.firstName,
				   punter->stats.lastName);
#endif								
				break;
			} else {
				intent->drink.stage = drink_walk;				
				punter->activePath = 1;
			}						
		} else {
						//no seat found
		}				
		break;
	} case drink_walk: {
		entity_Move(punter, world);
		if(!punter->activePath) {	
			GET_CHARBASE(world, punter)->rotation = SetTableFacing(world, punter);
			intent->purchase.stage = drink_drink;			
			time_StartTimedEvent(world, &punter->actionTime,
				ai_GetActionTime(punter->intent));
		}
		break;
	} case drink_drink: {		
		if(time_TickIncrement(world, &punter->actionTime)) {
			punter->stats.bladder += 0.0025f;
			punter->stats.drunk += 0.005f;
			punter->stats.thirst -= 0.01f;
			if(!punter->interaction && punter->actionTime.tickCount % 6 == 0) {
				social_InitInteractionTable(world, punter);
			}
		}			
		if(time_EndTimedEvent(world, &punter->actionTime)) {
			ai_EmptyInvOfItem(&punter->stats, item_beer);
			punter->intent.type = intent_null;
			UNSET_TARGET(world, punter);
		}		
		break;
	} default: INVALID_PATH;	 
	}
}

static void Nothing(struct world_mode *world,
		    struct entity_char *chara)
{
	assert(chara->intent.type == intent_nothing);	
	struct char_intent *intent = &chara->intent;
	
	switch(intent->nothing.stage) {
	case nothing_init: {
		
		if(rand() % 4 < 1) {
			intent->nothing.stage = nothing_wander;
			int32_t x = GET_CHARCOORDS(world, chara).x + ((rand() % 6) - 3);
			int32_t y = GET_CHARCOORDS(world, chara).y + ((rand() % 6) - 3);
			if(x < 0 || x >= world->map.sizeX || 
			   y < 0 || y >= world->map.sizeY) {				
				break;
			} 
			struct point3 currentPos = Vec3ToPoint3(GET_CHARBASE(world, chara)->pos.xyz);
			struct point3 targetPos = {x, y, GET_CHARBASE(world, chara)->pos.z};					
			
			if(tile_GetTile(&world->map, targetPos).impassable ||
			   tile_GetTile(&world->map, targetPos).stepped) {				
				break;
			}
			chara->currentNode = path_GetPath(currentPos, targetPos,						
				chara, world, chara->path);
		}
		else {
			intent->nothing.stage = nothing_loiter;
			time_StartTimedEvent(world, &chara->actionTime, LOITER_TIME);
		}
		
		break;		
	} case nothing_wander: {		
		if(chara->activePath) {
			entity_Move(chara, world);	
				
		} else {
			chara->intent = SET_NULLINTENT();			
		}		
		break;
	} case nothing_loiter: {
		
		if(time_EndTimedEvent(world, &chara->actionTime)) {
			chara->intent = SET_NULLINTENT();
		}
		break;
	} 	
	default: INVALID_PATH;	
	}
	
}

static void Purchase(struct world_mode *world,
		     struct entity_char *punter)
{
	assert(punter->intent.type == intent_purchase);
	
	struct char_intent *intent = &punter->intent;
	
	switch(intent->purchase.stage) {
	case purchase_init: {
		if(PunterSearchBar(world, punter)) {		
		
			struct point3 currentPos = Vec3ToPoint3(
				GET_CHARBASE(world, punter)->pos.xyz);
			struct point3 targetPos = ai_GetBarSlotPos(
				world, punter, GET_TARGETFURN(world, punter));
			
			punter->currentNode = path_GetPath( 
				currentPos, targetPos, punter, world, punter->path);
				
			if(!punter->currentNode) {
				punter->intent.type = intent_null;
				UNSET_TARGET(world, punter);
#ifdef DEBUG
				//printf("%s %s cannot find path to bar\n", 
					//punter->stats.firstName,	
					//punter->stats.lastName);
#endif				
			} else {				
				intent->purchase.stage = purchase_walk;					
				punter->activePath = 1;				
			}						
		} else {
			punter->intent.type = intent_nothing;
		}				
		break;
	} case purchase_walk: {
		entity_Move(punter, world);		
		if(!punter->activePath) {						
			intent->purchase.stage = purchase_wait;
			GET_CHARBASE(world, punter)->rotation = SetBarFacing(world, punter);
		}		
		break;
	} case purchase_wait: {		
		if(IsBeingServed(punter)) {
			intent->purchase.stage = purchase_buy;			
			ai_StartTimedEvent(world, punter);			
		}
		break;
	} case purchase_buy: {
		if(!punter->intent.purchase.interaction) {
			social_InitInteraction(world, punter, 
				GET_CHARACROSSBAR(world, punter, BAR_STAFF));	
			punter->intent.purchase.interaction = true;				
		}		
		if(ai_EndTimedEvent(world, punter)) {			
			ai_AddItem(&punter->stats, item_beer);
			UNSET_TARGET(world, punter);			
			punter->intent = SET_NULLINTENT();			
		}		
		break;
	} default: INVALID_PATH;	 
	}
}

static enum entity_intent StaffIntent(struct world_mode *world,
			    	      struct entity_char *current)
{
	enum entity_intent result = intent_null;
	
	if(world->alerts.serveAlert) {
		result = intent_serve;
		current->intent.type = intent_serve;
		current->intent.serve.stage = serve_init;
	} else {
		result = intent_nothing;
		current->intent.type = intent_nothing;
		current->intent.purchase.stage = nothing_init;	
	}	
	return(result);
}

static enum entity_intent PunterIntent(struct world_mode *world,
				       struct entity_char *current)
{
	enum entity_intent result = intent_null;
	
	if(current->stats.bladder > 0.75f) {
		result = intent_tiolet; 
		current->intent.type = result;
		current->intent.purchase.stage = tiolet_init;
		return(result);
	} else if(current->stats.thirst < 0.5f) {
		if(ai_CheckInv(item_beer, current->stats.inv)) {
			result = intent_drink;
			current->intent.type = result;
			current->intent.purchase.stage = drink_init;
		} else if(!ai_CheckInv(item_beer, current->stats.inv)){
			result = intent_purchase;
			current->intent.type = intent_purchase;
			current->intent.purchase.stage = purchase_init;				
		} 
	} else {
		result = intent_nothing;
		current->intent.type = intent_nothing;
		current->intent.purchase.stage = nothing_init;		
	}		
	return(result);
}

static inline enum entity_intent ai_GuageIntent(struct world_mode *world,
						struct entity_char *current)
{
	enum entity_intent result = intent_null;
	
	if(current->stats.isStaff) result = StaffIntent(world, current);
 	else result = PunterIntent(world, current);
	
	return(result);
}

static void ai_Punter(struct world_mode *world,
		      struct entity_char *current)
{
	if(current->intent.type == intent_null) {
		current->intent.type = ai_GuageIntent(world, current);				
	}
			
	switch(current->intent.type) {
	case intent_purchase: {	
		Purchase(world, current);
			break;	
	} case intent_drink: {
		Drink(world, current);
			break;
	} case intent_tiolet: {	
		Tiolet(world, current);
			break;	
	} case intent_nothing: {
		Nothing(world, current);
		break;
	} case intent_null: 
	default: break;
	}	
}

static void ai_Staff(struct world_mode *world,
		     struct entity_char *current)
{
	if(current->intent.type == intent_null) {
		
		ai_GuageIntent(world, current);
	}
	
	switch(current->intent.type) {
	case intent_serve: {
		Serve(world, current);
		break;
	} case intent_tiolet: {	
		Tiolet(world, current);
			break;	
	} case intent_nothing: {
		Nothing(world, current);
		break;
	} case intent_null: 
	 default: break;
	}	
}

extern void ai_Think(struct world_mode *world,
		     struct entity_char *current)
{		
	ai_ConstrainNeeds(current);
	if(current->interaction) {social_UpdateInteraction(world, current);}	
	if(!current->stats.isStaff) {
		ai_Punter(world, current);
	} else {
		ai_Staff(world, current);
	}
	
}