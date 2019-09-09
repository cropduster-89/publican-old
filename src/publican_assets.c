/************************************************************************************		
__________     ___.   .__  .__                      	
\______   \__ _\_ |__ |  | |__| ____ _____    ____  
 |     ___/  |  \ __ \|  | |  |/ ___\\__  \  /    \ 	Asset loading 
 |    |   |  |  / \_\ \  |_|  \  \___ / __ \|   |  \
 |____|   |____/|___  /____/__|\___  >____  /___|  /
                    \/             \/     \/     \/ 
*************************************************************************************/

#define _CHAR(a) ((uint32_t)a - 33)	

struct memory_arena nonRestoredMem;

extern inline void BeginAssetLock(struct game_assets *assets)
{
	for(;;) {
		if(AtomicCompareExchangeUInt32(&assets->tState->opLock, 1, 0) == 0) {
			break;
		}
	}
}

extern inline void EndAssetLock(struct game_assets *assets)
{
	asm ("sfence");
	assets->tState->opLock = 0;
}

extern inline void InsertAssetHeaderAtFront(struct game_assets *assets,
					    struct asset_memory_header *header)
{
	struct asset_memory_header *sentinal = &assets->loadedAssetSentinal;
	
	header->prev = sentinal;
	header->next = sentinal->next;
	
	header->prev->next = header;
	header->next->prev = header;
}

extern inline void RemoveAssetHeaderFromList(struct asset_memory_header *header)
{
	header->prev->next = header->next;
	header->next->prev = header->prev;
	
	header->next = header->prev = 0;
}

extern struct task_with_memory *BeginTaskWithMemory(struct temp_state *tState,
														   bool modeDepandant)
{
	struct task_with_memory *foundTask = 0;
	for(uint32_t i = 0; i < ARRAY_COUNT(tState->tasks); ++i) {
		struct task_with_memory *task = tState->tasks + i;
		if(!task->inUse) {
			foundTask = task;
			task->inUse = true;
			task->modeDepandant = modeDepandant;
			task->memFlush = BeginTempMem(&task->arena);
			break;
		}
	}
	return(foundTask);
}


extern inline void EndTaskWithMemory(struct task_with_memory *task)
{
	EndTempMem(task->memFlush);
	asm ("sfence");
	task->inUse = false;		
}

static void AddOp(struct platform_texture_op_queue *queue,
				  struct texture_op *source)
{
	BeginTicketMutex(&queue->mutex);
	
	assert(queue->firstFree);
	struct texture_op *dest = queue->firstFree;
	queue->firstFree = dest->next;
	
	*dest = *source;
	assert(dest->next == 0);
	
	if(queue->last) {
		queue->last = queue->last->next = dest;
	} else {
		queue->first = queue->last = dest;
	}
	EndTicketMutex(&queue->mutex);
}

static void LoadAssetWorkDirectly(struct load_asset_work *work)
{
	platform.ReadDataFromFile(work->handle, work->offset, work->size, work->dest);
	if(PlatformNoFileErrors(work->handle)) {
		switch(work->finalOp) {
		case finaldir_northone: {
			break;
		} case final_font: {
			//TODO this
			break;
		} case final_skin: 
		case final_bmp: {
			struct loaded_bmp *bmp = &work->asset->header->bmp;
			struct texture_op op = {};
			op.type = work->finalOp;
			op.isAllocate = true;
			op.alloc.w = bmp->w;
			op.alloc.h = bmp->h;
			op.alloc.data = bmp->data;
			op.alloc.resultHandle = &bmp->handle;
			op.alloc.arrayOffset = &bmp->arrayOffset;
			AddOp(work->textureOpQueue, &op);	
			break;	
		} case final_mesh: {
				break;
		}
		}
	}
	asm ("sfence");
	
	if(!PlatformNoFileErrors(work->handle)) {
		ZeroSize_(work->size, work->dest);
	}
	work->asset->state = work->finalState;
}

static PLATFORM_WORK_QUEUE_CALLBACK(LoadAssetWork)
{
	struct load_asset_work *work = (struct load_asset_work *)data;
	LoadAssetWorkDirectly(work);
	
	EndTaskWithMemory(work->task);
}

extern inline struct asset_file *GetFile(struct game_assets *assets,
					 uint32_t fileIndex)
{
	assert(fileIndex < assets->fileCount);
	struct asset_file *result = assets->files + fileIndex;
	return(result);
}

extern inline struct platform_file_handle *GetFileHandleFor(struct game_assets *assets,
															uint32_t fileIndex)
{
		struct platform_file_handle * result = &GetFile(assets, fileIndex)->handle;
		return(result);
}
			
static struct asset_memory_block *InsertBlock(struct asset_memory_block *prev,
					       uint64_t size,
					       void *memory)
{
	assert(size > sizeof(struct asset_memory_block));
	struct asset_memory_block *block = (struct asset_memory_block *)memory;
	block->flags = 0;
	block->size = size - sizeof(struct asset_memory_block);
	block->prev = prev;
	block->next = prev->next;
	block->prev->next = block;
	block->next->prev = block;
	return(block);
}

static struct asset_memory_block *FindBlockForSize(struct game_assets *assets,
					           size_t size)
{
	struct asset_memory_block *result = 0;
	
	for(struct asset_memory_block * block = assets->memSentinal.next;
		block != &assets->memSentinal; block = block->next) {
		if(!(block->flags & 0x1)) {
			if(block->size >= size) {
				result = block;
				break;
			}
		}
	}
	return(result);
}

static bool MergeIfPossible(struct game_assets *assets,
			    struct asset_memory_block *first,
			    struct asset_memory_block *second)
{
	bool result = false;
	
	if((first != &assets->memSentinal) && (second != &assets->memSentinal)) {
		if(!(first->flags & 0x1) && !(second->flags & 0x1)) {
			uint8_t *expectedSecond = (uint8_t *)first + 
				sizeof(struct asset_memory_block) + first->size;
			if((uint8_t *)second == expectedSecond) {
				second->next->prev = second->prev;
				second->prev->next = second->next;
				
				first->size += sizeof(struct asset_memory_block) + second->size;
				
				result = true;
			}	
		}
	}
	return(result);
}

static struct asset_memory_header *AquireAssetMemory(struct game_assets *assets, 
						     uint32_t size, 
						     uint32_t newAssetIndex,
						     enum asset_header_type assetType)
{
	struct asset_memory_header *result = 0;
	
	BeginAssetLock(assets);
	struct asset_memory_block *block = FindBlockForSize(assets, size);
	for(;;) {
		if(block && (size <= block->size)) {
			block->flags |= 0x1;
			
			result = (struct asset_memory_header *)(block + 1);
			
			size_t remainingSize = block->size - size;
			size_t blockSplitThreshold = 4096;
			if(remainingSize > blockSplitThreshold) {
					block->size -= remainingSize;
					InsertBlock(block, remainingSize, (uint8_t *)result + size);								
			} else {
					//merge maybe
			}
			break;
		} else {
			for(struct asset_memory_header *header = assets->loadedAssetSentinal.prev;
				header != &assets->loadedAssetSentinal;
				header = header->prev) {
				//uint32_t assetIndex = header->assetIndex;
				struct asset *asset = assets->assets + header->assetIndex;
				if((asset->state >= state_loaded)) {
					assert(asset->state == state_loaded);
					RemoveAssetHeaderFromList(header);
					if(asset->header->assetType == header_bmp) {
							struct texture_op op = {};
							op.isAllocate = false;
							op.deAlloc.handle = asset->header->bmp.handle;
							AddOp(assets->textureOpQueue, &op);
					}
					block = (struct asset_memory_block *)asset->header - 1;
					block->flags &= ~0x1;
					if(MergeIfPossible(assets, block->prev, block)) {
							block = block->prev;
					}
					MergeIfPossible(assets, block, block->next);
					
					asset->state = state_unloaded;
					asset->header = 0;
					break;
				}		
			}
		}
	}
	if(result) {
			result->assetType = assetType;
			result->assetIndex = newAssetIndex;
			result->totalSize = size;
			InsertAssetHeaderAtFront(assets, result);
	}
	EndAssetLock(assets);
	
	return(result);
}

extern void assets_LoadMesh(struct game_assets *assets,
			    struct mesh_id id,
			    bool immediate)
{
	struct asset *asset = assets->assets + id.val; 
	if(id.val) {							
		if(AtomicCompareExchangeUInt32((uint32_t *)&asset->state, 
			state_queued, state_unloaded) == state_unloaded) {
			struct task_with_memory	*task = 0;
			if(!immediate) {
				task = BeginTaskWithMemory(assets->tState, false);
			}
			if(immediate||task) {
				struct pfile_mesh *info = &asset->pubb.mesh;				
				struct asset_memory_size size = {};
				
				size_t vertexSize = _32BIT * info->vertexCount * VERTEX_STRIDE; 
				size_t faceSize = _32BIT * info->faceCount * FACE_STRIDE;	
				size.data = (vertexSize + faceSize);
				size.total = size.data + sizeof(struct asset_memory_header);
				
				asset->header = AquireAssetMemory(assets, size.total, id.val, header_mesh);
				
				struct loaded_mesh *mesh = &asset->header->mesh;								
				mesh->faceCount = info->faceCount;
				mesh->vertexCount = info->vertexCount;				
				mesh->data = (asset->header + 1);
																
				struct load_asset_work work;
				work.task = task;
				work.asset = assets->assets + id.val;
				work.handle = GetFileHandleFor(assets, asset->fileIndex);
				work.offset = asset->pubb.offset;
				work.size = size.data;
				work.dest = mesh->data;
				work.finalOp = final_mesh;
				work.finalState = state_loaded;
				work.textureOpQueue = assets->textureOpQueue;
				if(task) {
					struct load_asset_work *taskWork = PUSH_STRUCT(&task->arena, 
																   struct load_asset_work,
																   NoClear());
					*taskWork = work;
					platform.AddEntry(assets->tState->lowPriorityQueue, LoadAssetWork, taskWork);		
				} else {
					LoadAssetWorkDirectly(&work);
					//printf("No threading for mesh\n");
				}
			} else {
					asset->state = state_unloaded;
			}
		} else if(immediate) {
				enum asset_state volatile *state = (enum asset_state volatile *)&asset->state;
				while(*state == state_queued) {}
		}					
	}	
	//printf("Mesh loaded\n");	
}

extern void assets_LoadBMP(struct game_assets *assets,
			   struct bmp_id id,
			   bool immediate,
			   enum finalize_asset_op type)
{
	struct asset *asset = assets->assets + id.val; 
	if(id.val) {							
		if(AtomicCompareExchangeUInt32((uint32_t *)&asset->state, 
			state_queued, state_unloaded) == state_unloaded) {
			struct task_with_memory	*task = 0;
			if(!immediate) {
					task = BeginTaskWithMemory(assets->tState, false);
			}
			if(immediate||task) {
				struct pfile_bmp *info = &asset->pubb.bmp;
				
				struct asset_memory_size size = {};
				uint32_t w = info->x;
				uint32_t h = info->y;
				size.section = 4 * w;
				size.data = h * size.section;
				size.total = size.data + sizeof(struct asset_memory_header);
				
				asset->header = AquireAssetMemory(assets, size.total, id.val, header_bmp);
				
				struct loaded_bmp *bmp = &asset->header->bmp;
				bmp->alignX = info->alignX;
				bmp->alignY = info->alignY;													
				bmp->w = info->x;
				bmp->h = info->y;
				bmp->wOverH = (float)bmp->w/(float)bmp->h;			
				bmp->pitch = size.section;
				bmp->handle = 0;
				bmp->data = (asset->header + 1);
				
				struct load_asset_work work;
				work.task = task;
				work.asset = assets->assets + id.val;
				work.handle = GetFileHandleFor(assets, asset->fileIndex);
				work.offset = asset->pubb.offset;
				work.size = size.data;
				work.dest = bmp->data;
				work.finalOp = type == final_skin ? final_skin : final_bmp;
				work.finalState = state_loaded;
				work.textureOpQueue = assets->textureOpQueue;
				if(task) {
					struct load_asset_work *taskWork = PUSH_STRUCT(&task->arena, 
					         					   struct load_asset_work,
					         					   NoClear());
					*taskWork = work;
					platform.AddEntry(assets->tState->lowPriorityQueue, LoadAssetWork, taskWork);		
				} else {
					LoadAssetWorkDirectly(&work);
					//printf("No threading for BMP\n");
				}
			} else {
				asset->state = state_unloaded;
			}
		} else if(immediate) {
			enum asset_state volatile *state = (enum asset_state volatile *)&asset->state;
			while(*state == state_queued) {}
		}					
	}			
}

static uint32_t Find_Asset(struct game_assets *assets, 
			   enum asset_type_id id, 
			   uint32_t offset)
{
	struct asset_type *type = assets->types + id;		
	return(type->firstIndex + offset);
}

extern inline struct bmp_id asset_FindBMP(struct game_assets *assets, 
					  enum asset_type_id id,
					  uint32_t bmpOffset)
{
	struct bmp_id result = {Find_Asset(assets, id, bmpOffset)};		
	return(result);
}

extern inline struct bmp_id asset_FindChar(struct game_assets *assets, 
					   enum asset_type_id id,
					   uint32_t glyph)
{
	struct bmp_id result = {Find_Asset(assets, id, glyph)};		
	if(result.val > 0x0FFFFFF) {
		INVALID_PATH;
	}
	return(result);
}

extern inline struct mesh_id asset_FindMesh(struct game_assets *assets, 
											enum asset_type_id id,
											uint8_t rot)
{
		struct mesh_id result = {Find_Asset(assets, id, rot)};		
		return(result);
}

/*

static inline uint32_t asset_GetFirstSlot(struct game_assets *assets,
										  enum asset_type_id id, 
										  uint32_t offset)
{
		uint32_t result = 0;
		
		struct asset_type *type = assets->types + id;
		if(type->firstIndex != type->lastIndexPlus1) {
				result = type->firstIndex + offset;
		}
		return(result);
}

extern inline struct bmp_id asset_GetFirstBMP(struct game_assets *assets, 
											  enum asset_type_id id, 
											  uint32_t offset)
{
		struct bmp_id result = {asset_GetFirstSlot(assets, id, offset)};
		return(result);
}

extern inline struct bmp_id asset_GetChar(struct game_assets *assets, 
											  enum asset_type_id id,
											  uint8_t glyph)
{
		struct bmp_id result = {asset_GetFirstSlot(assets, id + (glyph - 33))};
		return(result);
}
*/

extern inline struct asset_memory_header *GetAsset(struct game_assets *assets,
						   uint32_t id)
{	
	if(id > assets->assetCount) {printf("%d\n", id);}
	assert(id <= assets->assetCount);
	struct asset *asset = assets->assets + id;
	
	struct asset_memory_header *result = 0;
	BeginAssetLock(assets);
	if(asset->state == state_loaded) {
			result = asset->header;
			RemoveAssetHeaderFromList(result);
			InsertAssetHeaderAtFront(assets, result);			
			asm ("sfence");
	}
	EndAssetLock(assets);
	
	return(result);
}

extern inline struct loaded_bmp *assets_GetBMP(struct game_assets *assets,
					       struct bmp_id id)
{
	struct asset_memory_header *header = GetAsset(assets, id.val);
	struct loaded_bmp *result = header ? &header->bmp : 0;
	
	return(result);
}

extern inline struct loaded_mesh *assets_GetMesh(struct game_assets *assets,
						 struct mesh_id id)
{
	struct asset_memory_header *header = GetAsset(assets, id.val);
	struct loaded_mesh *result = header ? &header->mesh : 0;
	
	return(result);
}

extern struct game_assets *assets_Allocate(size_t size,
					   struct temp_state *tState,
					   struct platform_texture_op_queue *textureOpQueue)
{
		printf("Begin assets_Allocate\n");
		
		struct game_assets *assets = BootStrapPushSize(struct game_assets, nonRestoredMem,
				NonRestoredArena(), DEF_PUSH);
		struct memory_arena *arena = &assets->nonRestoredMem;
		
		assets->tState = tState;			
		assets->textureOpQueue = textureOpQueue;
		
		assets->memSentinal.flags = 0;
		assets->memSentinal.size = 0;
		assets->memSentinal.prev = &assets->memSentinal;
		assets->memSentinal.next = &assets->memSentinal;
		InsertBlock(&assets->memSentinal, size, PUSH_SIZE(arena, size, NoClear()));
		
		assets->loadedAssetSentinal.next =
				assets->loadedAssetSentinal.prev =
				&assets->loadedAssetSentinal;
		assets->assetCount = 1;				
		
		struct platform_file_group fileGroup = platform.GetFilesTypeStart(filetype_asset);
		assets->fileCount = fileGroup.fileCount;
		assets->files = PUSH_ARRAY(arena, assets->fileCount, struct asset_file, DEF_PUSH);
		for(uint32_t fileIndex = 0; fileIndex < assets->fileCount; ++fileIndex) {
				struct asset_file *file = assets->files + fileIndex;
				file->bmpIDOffset = 0;
				
				ZERO_STRUCT(file->header);
				file->handle = platform.OpenNextFile(&fileGroup);
				platform.ReadDataFromFile(&file->handle, 0, sizeof(file->header), &file->header);

				uint32_t assetTypeArraySize	= file->header.typeCount * sizeof(struct pfile_type);
				file->assetTypeArray = (struct pfile_type *)PUSH_SIZE(arena, assetTypeArraySize, DEF_PUSH);
				platform.ReadDataFromFile(&file->handle, file->header.types, assetTypeArraySize,
										  file->assetTypeArray);
				if(PlatformNoFileErrors(&file->handle)) {
						assets->assetCount += (file->header.assetCount) - 1;
				}
				printf("Files Read\n");				
		}
		platform.GetFilesTypeEnd(&fileGroup);
		
		assets->assets = PUSH_ARRAY(arena, assets->assetCount, struct asset, DEF_PUSH);
		
		uint32_t assetCount = 0;
		ZERO_STRUCT(*(assets->assets + asset_count));
		++assetCount;
		
		for(uint32_t destTypeID = 0; destTypeID < asset_count; ++destTypeID) {
		struct asset_type *destType = assets->types + destTypeID;
		destType->firstIndex = assetCount;
		for(uint32_t fileIndex = 0; fileIndex < assets->fileCount; ++fileIndex) {
			struct asset_file *file = assets->files + fileIndex;
			if(PlatformNoFileErrors(&file->handle)) {
				for(uint32_t i = 0; i < file->header.typeCount; ++i) {
					struct pfile_type *sourceType = file->assetTypeArray + i;
					if(sourceType->id == destTypeID) {
						uint32_t assetCountForType = (sourceType->nextType -
													  sourceType->firstAsset);
						struct temp_memory tempMem = BeginTempMem(&tState->tempArena);
						struct pfile_asset *pfileAssetArray = PUSH_ARRAY(&tState->tempArena,
									assetCountForType, 
									struct pfile_asset,
									DEF_PUSH);
						platform.ReadDataFromFile(&file->handle, file->header.assets +
												  sourceType->firstAsset * sizeof(struct pfile_asset),
												  assetCountForType * sizeof(struct pfile_asset),
												  pfileAssetArray);
						for(uint32_t assetIndex = 0; assetIndex < assetCountForType; ++assetIndex) {
								struct pfile_asset *pfileAsset = pfileAssetArray + assetIndex;
								assert(assetCount < assets->assetCount);
								struct asset *asset = assets->assets + assetCount++;
								
								asset->fileIndex = fileIndex;
								asset->pubb = *pfileAsset;												
						}		
						EndTempMem(tempMem);
					}
				}
			}
		}
		destType->lastIndexPlus1 = assetCount;
		}
		//assert(assetCount == assets->assetCount);
		printf("End assets_Allocate\n");
		return(assets);
}

