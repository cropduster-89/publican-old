/************************************************************************************		
__________     ___.   .__  .__                      	
\______   \__ _\_ |__ |  | |__| ____ _____    ____  
 |     ___/  |  \ __ \|  | |  |/ ___\\__  \  /    \ 	UI processing and rendering 
 |    |   |  |  / \_\ \  |_|  \  \___ / __ \|   |  \
 |____|   |____/|___  /____/__|\___  >____  /___|  /
                    \/             \/     \/     \/ 
		    @no longer placeholder!
		    @try and make good!
		    @TODO: remove all text arrays & add strings to asset pipeline
		    @TODO: add semi permanent texture storage for text (how!?)	
*************************************************************************************/

extern void ui_TextOut(char *string,
		       int32_t len,
		       struct render_group *renderGroup,
		       struct temp_state *tState,
		       struct point2 startPos,	
		       float scale)
{		
	float lineWeight = 20.0f;	
	struct object_transform trans = render_FlatTrans();
	trans.offset.x = startPos.x;
	trans.offset.y = startPos.y;	
	char *current = string;		
	float prevX = 0;
	float prevSizeX = 0;
	for(uint32_t i = 0; i < len; ++i, ++current) {
		if(*current == ' ') {
			trans.offset.x += 15;
			continue;
		} else if(*current == '\n') {
			trans.offset.y -= lineWeight;
		} else if(*current == '\0') {
			break;
		}		
		struct bmp_id id = asset_FindChar(tState->assets, asset_font, _CHAR(*current));	
		struct loaded_bmp *bmp = assets_GetBMP(tState->assets, id);							   
		if(!bmp) {
			assets_LoadBMP(renderGroup->assets, id, true, final_bmp);
			return;
		} else {
			union vec2 size = {
				.x = bmp->w,
				.y = bmp->h,
			};
			union vec2 align = {
				.x = bmp->alignX,
				.y = bmp->alignY,
			};
			
			align = MultVec2(align, 1.0f);				
			align = MultVec2(align, scale);		
			size = MultVec2(size, 1.0f);		
			size = MultVec2(size, scale);				
								
			trans.offset.x +=  align.x + prevX + prevSizeX;				
			trans.offset.y -=  size.y + align.y;
			
			prevX = align.x;
			prevSizeX = size.x;
			render_PushBMP(renderGroup, trans, id, size.y, uiOffset, 1.0f);			
			
		}				
	}		
}

#define ui_StateClear(elements) \
	_ui_ChangeState(elements, 0, 0, true)	
#define ui_ChangeState(elements, start, end) \
	_ui_ChangeState(elements, start, end, false)	
static void _ui_ChangeState(struct ui_element elements[],
			    int32_t start,
			    int32_t end,			   
			    bool fullClear)
{	
	for(int32_t i = 0; i < UI_COUNT; ++i) {		
		if(BITCHECK(elements[i].state, UISTATE_ALWAYSON)) {
			continue;
		} else if(fullClear) {
			BITCLEAR(elements[i].state, UISTATE_VISIBLE);			
		} else if(i >= start && i < end) { 
			BITSET(elements[i].state, UISTATE_VISIBLE);
		} else {
			BITCLEAR(elements[i].state, UISTATE_VISIBLE);
		}		
	}		
}

/*		
*	Init all ui elements up front.
*	Looks like shit, but tbh probably the best way	
*	without baking it into assets/external files (good idea?)	
*/
static void ui_InitWorldUI(struct world_mode *world,
			   struct render_group *group)
{
	float x = (float)group->commands->w;
	float y = (float)group->commands->h;
	
	struct ui_element *bottomPanel = &world->ui.elements[UIALIAS_BOTTOMPANEL];
	bottomPanel->type = UITYPE_PANEL;	
	bottomPanel->state = UISTATE_ALWAYSON|UISTATE_VISIBLE;	
	bottomPanel->dim.x = x;
	bottomPanel->dim.y = 70;
	bottomPanel->pos = RealToVec2(-x / 2, -y / 2);
	bottomPanel->panel.colour = RealToVec4(0.8f, 0.7f, 0.5f, 0.75f);
	BITSET(bottomPanel->state, UISTATE_ALWAYSON);
	BITSET(bottomPanel->state, UISTATE_VISIBLE);
	
	struct ui_element *floorUp = &world->ui.elements[UIALIAS_FLOORUP];
	floorUp->type = UITYPE_TEXTBUTTON;	
	floorUp->parent = bottomPanel;
	floorUp->dim.x = 80;
	floorUp->dim.y = 50;
	floorUp->pos = RealToVec2(floorUp->parent->pos.x + 10, floorUp->parent->pos.y + 10);
	floorUp->textButton.text = ASSET_UPTXT;
	floorUp->textButton.colour = RealToVec4(0.9f, 0.8f, 0.5f, 0.75f);
	floorUp->textButton.textOffset = RealToVec2(10, 10);
	floorUp->callback = &FloorUp;	
	BITSET(floorUp->state, UISTATE_ALWAYSON);
	BITSET(floorUp->state, UISTATE_VISIBLE);
	
	struct ui_element *floorDown = &world->ui.elements[UIALIAS_FLOORDOWN];
	floorDown->type = UITYPE_TEXTBUTTON;
	floorDown->state = UISTATE_ALWAYSON|UISTATE_VISIBLE;	
	floorDown->parent = bottomPanel;
	floorDown->dim.x = 80;
	floorDown->dim.y = 50;
	floorDown->pos = RealToVec2(floorUp->parent->pos.x + 100, floorUp->parent->pos.y + 10);
	floorDown->textButton.text = ASSET_DOWNTXT;
	floorDown->textButton.colour = RealToVec4(0.9f, 0.8f, 0.5f, 0.75f);
	floorDown->textButton.textOffset = RealToVec2(10, 10);	
	floorDown->callback = &FloorDown;	
	BITSET(floorDown->state, UISTATE_ALWAYSON);
	BITSET(floorDown->state, UISTATE_VISIBLE);
	
	struct ui_element *build = &world->ui.elements[UIALIAS_BUILD];
	build->type = UITYPE_TEXTBUTTON;
	build->state = UISTATE_ALWAYSON|UISTATE_VISIBLE;	
	build->parent = bottomPanel;
	build->dim.x = 80;
	build->dim.y = 50;
	build->pos = RealToVec2(x / 2 - 90, build->parent->pos.y + 10);
	build->textButton.text = ASSET_BUILDTXT;
	build->textButton.colour = RealToVec4(0.9f, 0.8f, 0.5f, 0.75f);
	build->textButton.textOffset = RealToVec2(10, 10);		
	BITSET(build->state, UISTATE_VISIBLE);
	
	struct ui_element *charPanel = &world->ui.elements[UIALIAS_CHARPANEL];
	charPanel->type = UITYPE_PANEL;
	charPanel->state = 0;
	charPanel->dim.x = 360;
	charPanel->dim.y = y - 70;
	charPanel->pos = RealToVec2(x / 2 - 360, -y / 2 + 70);
	charPanel->panel.colour = RealToVec4(0.9f, 0.5f, 0.3f, 0.75f);	
	
	struct ui_element *charName = &world->ui.elements[UIALIAS_CHARNAME];
	charName->type = UITYPE_DYNAMIC_LABEL;
	charName->state = UISTATE_VISIBLE|UISTATE_ALWAYSON;
	charName->dim.x = 256;
	charName->dim.y = 128;
	charName->parent = charPanel;
	charName->pos = RealToVec2(charPanel->pos.x + 20, 
		y / 2 - 50);	
		
	struct ui_element *thirstLabel = &world->ui.elements[UIALIAS_CHARTHIRSTLBL];
	thirstLabel->type = UITYPE_STATIC_LABEL;	
	thirstLabel->label.text = ASSET_THIRSTTXT;
	thirstLabel->parent = charPanel;
	thirstLabel->dim.x = 128;
	thirstLabel->dim.y = 64;
	thirstLabel->pos = RealToVec2(charPanel->pos.x + 20, y / 2 - 100);
	
	struct ui_element *thirstBar = &world->ui.elements[UIALIAS_CHARTHIRSTBAR];
	thirstBar->alias = UIALIAS_CHARTHIRSTBAR;	
	thirstBar->type = UITYPE_BAR;	
	thirstBar->parent = charPanel;
	thirstBar->pos = RealToVec2(charPanel->pos.x + 105, y / 2 - 100);
	thirstBar->dim.x = (x / 2) - thirstBar->pos.x - 20;
	thirstBar->dim.y = 16;	
	thirstBar->bar.colour = RealToVec4(0.3f, 0.8f, 0.3f, 1.0f);
	
	struct ui_element *bladderLabel = &world->ui.elements[UIALIAS_CHARBLADDERLBL];
	bladderLabel->type = UITYPE_STATIC_LABEL;	
	bladderLabel->label.text = ASSET_BLADDERTXT;
	bladderLabel->parent = charPanel;
	bladderLabel->dim.x = 128;
	bladderLabel->dim.y = 64;
	bladderLabel->pos = RealToVec2(charPanel->pos.x + 20, y / 2 - 130);
	
	struct ui_element *bladderBar = &world->ui.elements[UIALIAS_CHARBLADDERBAR];
	bladderBar->alias = UIALIAS_CHARBLADDERBAR;	
	bladderBar->type = UITYPE_BAR;	
	bladderBar->parent = charPanel;
	bladderBar->pos = RealToVec2(charPanel->pos.x + 105, y / 2 - 130);
	bladderBar->dim.x = (x / 2) - bladderBar->pos.x - 20;
	bladderBar->dim.y = 16;	
	bladderBar->bar.colour = RealToVec4(0.3f, 0.8f, 0.3f, 1.0f);
	
	struct ui_element *drunkLabel = &world->ui.elements[UIALIAS_CHARDRUNKLBL];
	drunkLabel->type = UITYPE_STATIC_LABEL;	
	drunkLabel->label.text = ASSET_DRUNKTXT;
	drunkLabel->parent = charPanel;
	drunkLabel->dim.x = 128;
	drunkLabel->dim.y = 64;
	drunkLabel->pos = RealToVec2(charPanel->pos.x + 20, y / 2 - 160);
	
	struct ui_element *drunkBar = &world->ui.elements[UIALIAS_CHARDRUNKBAR];
	drunkBar->alias = UIALIAS_CHARDRUNKBAR;	
	drunkBar->type = UITYPE_BAR;	
	drunkBar->parent = charPanel;
	drunkBar->pos = RealToVec2(charPanel->pos.x + 105, y / 2 - 160);
	drunkBar->dim.x = (x / 2) - drunkBar->pos.x - 20;
	drunkBar->dim.y = 16;	
	drunkBar->bar.colour = RealToVec4(0.3f, 0.8f, 0.3f, 1.0f);	
		
	world->ui.init = true;
}

static inline bool IsWithinUiElement(struct ui_element *element,
				     union vec2 cursorPos)
{
	struct rect2 dim = {
		{
			.x = element->pos.x, 
			.y = element->pos.y
		}, {
			.x = element->pos.x + element->dim.x, 
			.y = element->pos.y + element->dim.y
		},
	};
	return(cursorPos.x >= dim.min.x && cursorPos.y >= dim.min.y && 
	       cursorPos.x <= dim.max.x && cursorPos.y <= dim.max.y &&
	       element->state & UISTATE_VISIBLE);
}

static void DrawPanel(struct render_group *group,
		      struct ui_element *element)
{
	render_PushRect(group, 
		RealToVec3(element->pos.x, element->pos.y, 4000.0f), 
		RealToVec2(element->dim.x, element->dim.y),
		element->panel.colour);
}

static void DrawStaticLabel(struct render_group *group,
			    struct ui_element *element,
			    struct temp_state *tState,
			    struct string_id idString)
{
	struct loaded_string *string = assets_GetString(group->assets, idString);
	if(string) {
		ui_TextOut(string->data, string->size,
			group, tState, Vec2ToPoint2(ADDVEC(element->pos,
			element->textButton.textOffset)), 0.4f);
	} else {
		assets_LoadString(group->assets, idString, false);
	}	
}

static void DrawDynamicLabel(struct world_mode *world,
			     struct render_group *group,
			     struct ui_element *element,
			     struct temp_state *tState)
{	
	if(!world->selectedEnt.ent) {return;}
	struct entity_char *chara = GET_CHAR(world->selectedEnt.ent);	
	char nameBuffer[65];
	sprintf(nameBuffer, "%s %s", chara->stats.firstName, chara->stats.lastName);
	
	ui_TextOut(nameBuffer, strlen(nameBuffer),
		group, tState, Vec2ToPoint2(ADDVEC(element->pos,
		element->textButton.textOffset)), 0.4f);	
}

static void DrawTextButton(struct render_group *group,
			   struct ui_element *element,
			   struct temp_state *tState,
			   struct string_id idString)
{
	render_PushRect(group, 
		RealToVec3(element->pos.x, element->pos.y, 5000.0f), 
		RealToVec2(element->dim.x, element->dim.y),
		element->textButton.colour);
	DrawStaticLabel(group, element, tState, idString);
}

extern void ui_OpenCharPanel(struct world_mode *world)
{
	ui_StateClear(world->ui.elements);
	BITSET(world->ui.elements[UIALIAS_CHARPANEL].state, UISTATE_VISIBLE);
}

static inline bool ElementIsVisible(struct ui_element *element)
{
	return(BITCHECK(element->state, UISTATE_VISIBLE) || (element->parent &&
	       BITCHECK(element->parent->state, UISTATE_VISIBLE)));
}

static float GetNeedsBarVal(struct world_mode *world,
			    struct ui_element *element)
{
	float result = 0;
	struct entity_char *chara = GET_CHAR(world->selectedEnt.ent);
	
	switch(element->alias) {
	case UIALIAS_CHARTHIRSTBAR: 
	{	
		result = chara->stats.thirst;
		break;
	}
	case UIALIAS_CHARBLADDERBAR: 
	{	
		result = chara->stats.bladder;
		break;
	}
	case UIALIAS_CHARDRUNKBAR: 
	{	
		result = chara->stats.drunk;
		break;
	} default: INVALID_PATH;
	}
	
	return(result);
}

static bool IsNeedsBar(struct ui_element *element)
{
	return(element->alias >= UIALIAS_CHARTHIRSTBAR && 
	       element->alias <= UIALIAS_CHARDRUNKBAR);
}

extern bool ui_WorldMode(struct world_mode *world,
			 struct pub_input *input,
			 struct render_group *group,
			 struct temp_state *tState)
{
	bool result = false;
	
	if(!world->ui.init) {ui_InitWorldUI(world, group);}	
	union vec2 mouseP = {
		.x = input->mouseX - (group->commands->w / 2),
		.y = input->mouseY - (group->commands->h / 2),
	};	
	
	for(int32_t i = 0; i < UI_COUNT; ++i) 
	{
		struct ui_element *element = &world->ui.elements[i];
		if(!ElementIsVisible(element)) {continue;} 
		
		switch(element->type) {
		case UITYPE_PANEL: {
			DrawPanel(group, element);
			break;
		} case UITYPE_TEXTBUTTON: {
			DrawTextButton(group, element, tState,
				asset_FindString(tState->assets, 
				element->textButton.text));
			break;
		} case UITYPE_STATIC_LABEL: {
			DrawStaticLabel(group, element, tState,
				asset_FindString(tState->assets, 
				element->textButton.text));
			break;
		} case UITYPE_DYNAMIC_LABEL: {
			DrawDynamicLabel(world, group, element, tState);
			break;
		} case UITYPE_BAR: {			
			if(IsNeedsBar(element)) {
				float ammountFull = GetNeedsBarVal(world, element);
				union vec2 drawLength = {
					.x = element->dim.x * ammountFull,
					.y = element->dim.y
				};			
				render_PushRect(group, Vec2ToVec3(element->pos, 5001.0f), 
					drawLength, element->bar.colour);					
			}				
			break;
		}		
		}
		if(IsWithinUiElement(element, mouseP)) 
		{
			result = true;
			if(element->type == UITYPE_TEXTBUTTON && 
			   input->buttons.lClick.endedDown) {
				assert(element->callback);
				element->callback(world, element->alias);
				input->buttons.lClick.endedDown = false;
			}
		}		
	}
	
	struct object_transform mouseTrans = {};	
	mouseTrans.offset.xy = mouseP;
	mouseTrans.offset.y -= 32;
	render_PushBMP(group, mouseTrans, 
			asset_FindBMP(tState->assets, asset_cursor, 0),
			32.0f, EMPTY_V3, 1);
			
	return(result);
} 
