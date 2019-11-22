extern int32_t chara_GetAtrMod(int32_t val)
{
	int32_t result = 0;
	
	switch(val) {
	case 1: 
	case 2: {
		result = -4;
		break;
	}		
	case 3: 	
	case 4: {
		result = -3;
		break;
	}	
	case 5:	
	case 6: {
		result = -2;
		break;
	}	
	case 7:	
	case 8: {
		result = -1;
		break;
	}	
	case 9:	
	case 10: result = 0;
		break;	
	case 11:	
	case 12: {
		result = 1;
		break;
	}	
	case 13:	
	case 14: {
		result = 2;
		break;
	}		
	case 15:	
	case 16: {
		result = 3;
		break;
	}		
	case 17:	
	case 18: {
		result = 4;
		break;
	}			
	case 19:	
	case 20: {
		result = 5;
		break;
	} default: INVALID_PATH;			
	}
	return(result);
}

static inline void social_NewRelation(struct entity_char *chara,
				      uint32_t index)
{
	struct social_relation newRelation = {};
	newRelation.index = index;
	chara->stats.relations[chara->stats.relationCount++] = newRelation;		
} 

static bool social_HasMet(struct entity_char *charaA,
		          struct entity_char *charaB) 
{
	bool result = false;	
	for(int32_t  i = 0; i < charaA->stats.relationCount; ++i) {
		int32_t entIndex = charaA->stats.relations[i].index;
		if(entIndex == charaB->entIndex) {
			result = true;
			break;
		}		
	}
	return(result);
}

static inline enum interaction_types social_PickNeutralInteraction(int32_t roll)
{
	int32_t selection = rand() % 2;
	enum interaction_types result = INTERACT_NULL;
	
	if(roll < 5) {
		if(!selection) {
			result = INTERACT_BADJOKE;
		} else {
			result = INTERACT_BADCHAT;
		}
	} else if(roll < 10) {		
		result = INTERACT_TALK;		
	} else {
		if(!selection) {
			result = INTERACT_GOODJOKE;
		} else {
			result = INTERACT_GOODCHAT;
		}
	}
	return(result);
}
	
static inline enum interaction_types social_GetInteraction(int32_t charisma)
{
	enum interaction_types result = INTERACT_NULL;
	
	int32_t roll = chara_GetAtrMod(charisma) + (rand() % 20); 	
	if(roll > 12) {
		result = social_PickNeutralInteraction(roll);
	}
	return(result);
}

extern void social_UpdateInteraction(struct world_mode *world,
				     struct entity_char *chara)
{
	if(time_TimePast(world, &chara->socialTime, CHAT_TIME / 2)) {
		chara->bubble = false;
		GET_SOCIALTARGET(chara)->bubble = true;
	} if(time_EndTimedEvent(world, &chara->socialTime)) {
		GET_SOCIALTARGET(chara)->bubble = false;
		chara->bubble = false;
		UNSET_SOCIALTARGET(world, chara);
	} 
}

extern void social_InitInteraction(struct world_mode *world,
				   struct entity_char *charaA,
				   struct entity_char *charaB)
{	
	charaA->interaction = social_GetInteraction(CHARISMA(charaA));
	if(!charaA->interaction) {return;}
	else if(!social_HasMet(charaA, charaB)) {
		social_NewRelation(charaA, charaB->entIndex);
		social_NewRelation(charaB, charaA->entIndex);
	}
	
	if(charaB->interaction && 
	   charaB->socialTarget.ent != GET_CHARBASE(world, charaA)) { 
		return;	
	}

    SET_SOCIALTARGET(world, charaA, charaB);
	time_StartTimedEvent(world, &charaA->socialTime, CHAT_TIME);
	charaA->bubble = true;
}

extern void social_InitInteractionTable(struct world_mode *world,
					struct entity_char *chara)
{	
	struct entity_furn *table = GET_TARGETTABLE(chara);
	struct entity_char *testChar = NULL;	
	for(int32_t i = 0; i < table->maxUsers; ++i) {			
		if(!table->users[i].ent) {			
			continue;
		} else if(GET_FURN(table->users[i].ent)->users[0].ent) {			
			testChar = GET_TARGETCHAR(table, i);	
		} else continue;	
		
		if(!testChar) {
			continue;
		} else if(testChar->intent.drink.stage != drink_drink) {
			continue;
		} else if(testChar != chara) {
			social_InitInteraction(world, chara, testChar);
			if(chara->interaction) {continue;}
		}		
	}
}