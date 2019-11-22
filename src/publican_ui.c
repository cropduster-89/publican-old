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
		       struct render_group *renderGroup,
		       struct temp_state *tState,
		       struct point2 startPos,	
		       float scale)
{		
	float lineWeight = 20.0f;	
	struct object_transform trans = render_FlatTrans();
	trans.offset.x = startPos.x;
	trans.offset.y = startPos.y;
	uint32_t len = text_StringCount(string);	
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
			
			//trans = render_UprightTrans();		
			
			prevX = align.x;
			prevSizeX = size.x;
			render_PushBMP(renderGroup, trans, id, size.y, uiOffset, 1.0f);			
			
		}				
	}		
}

#define ui_StateClear(elements) \
	_ui_ChangeState(elements, 0, 0, ARRAY_COUNT(elements), true)	
#define ui_ChangeState(elements, start, end) \
	_ui_ChangeState(elements, start, end, ARRAY_COUNT(elements), false)	
static void _ui_ChangeState(struct ui_element elements[],
			    int32_t start,
			    int32_t end,		
			    int32_t count,
			    bool fullClear)
{	
	for(int32_t i = 0; i < count; ++i) {		
		if(BITCHECK(elements[i].state, UISTATE_ALWAYSON)) {continue;}		
		else if(fullClear) {BITCLEAR(elements[i].state, UISTATE_VISIBLE);}			
		else if(i >= start && i < end) {BITSET(elements[i].state, UISTATE_VISIBLE);}	
		else {BITCLEAR(elements[i].state, UISTATE_VISIBLE);}		
	}		
}

/*		
*	Init all ui elements up front.
*	Looks like shit, but tbh probably the best way.		
*		
*/
static void ui_InitWorldUI(struct world_mode *world,
			   struct render_group *group)
{
	float x = (float)group->commands->w;
	float y = (float)group->commands->h;
	
	struct ui_element *bottomPanel = &world->ui.elements[UIALIAS_BOTTOMPANEL];
	bottomPanel->type = UIALIAS_BOTTOMPANEL;
	bottomPanel->state = UISTATE_ALWAYSON|UISTATE_VISIBLE;	
	bottomPanel->dim.x = x;
	bottomPanel->dim.y = 70;
	bottomPanel->pos = RealToVec2(-x / 2, -y / 2);
	bottomPanel->panel.colour = RealToVec4(0.8f, 0.7f, 0.5f, 0.75f);
	
	struct ui_element *floorUp = &world->ui.elements[UIALIAS_FLOORUP];
	floorUp->type = UITYPE_TEXTBUTTON;
	floorUp->state = UISTATE_ALWAYSON|UISTATE_VISIBLE;	
	floorUp->parent = bottomPanel;
	floorUp->dim.x = 80;
	floorUp->dim.y = 50;
	floorUp->pos = RealToVec2(floorUp->parent->pos.x + 10, floorUp->parent->pos.y + 10);
	floorUp->textButton.colour = RealToVec4(0.9f, 0.8f, 0.5f, 0.75f);
	floorUp->textButton.textOffset = RealToVec2(10, 10);
	floorUp->callback = FloorUp;
	strcpy_s(floorUp->textButton.text, DEF_LABEL_SIZE, "Up");
	
	struct ui_element *floorDown = &world->ui.elements[UIALIAS_FLOORDOWN];
	floorDown->type = UITYPE_TEXTBUTTON;
	floorDown->state = UISTATE_ALWAYSON|UISTATE_VISIBLE;	
	floorDown->parent = bottomPanel;
	floorDown->dim.x = 80;
	floorDown->dim.y = 50;
	floorDown->pos = RealToVec2(floorUp->parent->pos.x + 100, floorUp->parent->pos.y + 10);
	floorDown->textButton.colour = RealToVec4(0.9f, 0.8f, 0.5f, 0.75f);
	floorDown->textButton.textOffset = RealToVec2(10, 10);	
	floorDown->callback = FloorDown;
	strcpy_s(floorDown->textButton.text, DEF_LABEL_SIZE, "Down");
	
	struct ui_element *build = &world->ui.elements[UIALIAS_BUILD];
	build->type = UITYPE_TEXTBUTTON;
	build->state = UISTATE_ALWAYSON|UISTATE_VISIBLE;	
	build->parent = bottomPanel;
	build->dim.x = 80;
	build->dim.y = 50;
	build->pos = RealToVec2(x / 2 - 90, build->parent->pos.y + 10);
	build->textButton.colour = RealToVec4(0.9f, 0.8f, 0.5f, 0.75f);
	build->textButton.textOffset = RealToVec2(10, 10);
	strcpy_s(build->textButton.text, DEF_LABEL_SIZE, "Build");	
	
	struct ui_element *charPanel = &world->ui.elements[UIALIAS_CHARPANEL];
	charPanel->type = UITYPE_PANEL;
	charPanel->state = UISTATE_VISIBLE|UISTATE_ALWAYSON;
	charPanel->dim.x = 512;
	charPanel->dim.y = y - 70;
	charPanel->pos = RealToVec2(x / 2 - 512 , -y / 2 + 70);
	charPanel->panel.colour = RealToVec4(0.9f, 0.5f, 0.3f, 0.75f);	
	
		
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

static void DrawTextButton(struct render_group *group,
			   struct ui_element *element,
			   struct temp_state *tState)
{
	render_PushRect(group, 
		RealToVec3(element->pos.x, element->pos.y, 5000.0f), 
		RealToVec2(element->dim.x, element->dim.y),
		element->textButton.colour);
	ui_TextOut(element->textButton.text, 
		group, tState, Vec2ToPoint2(ADDVEC(element->pos,
		element->textButton.textOffset)), 0.4f);
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
	
	for(int32_t i = 0; i < UI_COUNT; ++i) {
		struct ui_element *element = &world->ui.elements[i];
		switch(element->type) {
		case UITYPE_PANEL: {
			DrawPanel(group, element);
			break;
		} case UITYPE_TEXTBUTTON: {
			DrawTextButton(group, element, tState);
			break;
		}	
		}
		if(IsWithinUiElement(element, mouseP)) {
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
