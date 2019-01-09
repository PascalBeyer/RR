#ifndef RR_ASSETHANDLER
#define RR_ASSETHANDLER

enum AssetType
{
	Asset_Texture,
	Asset_Material,
	Asset_Mesh,
	Asset_Animation,
   
	Asset_Type_Count,
};
#define Asset_Type_Offset 25
#define Asset_Texture_Amount 10
#define Asset_Mesh_Amount 100
#define Asset_Bitmap_Size 1024

struct MeshInfo
{
	u32 type;
	void *fileLocation;
	AABB aabb;
};

struct TextureInfo
{
	u32 width;
	u32 height;
};

struct AnimationInfo
{
	// f32 length;
	// void *fileLocation;
};

struct AssetInfo
{
	String name;
	AssetType type;
	bool currentlyLoaded;
	u32 id;
	u32 loadedIndex;
	
	union
	{
		MeshInfo	*meshInfo;
		TextureInfo *textureInfo;
		AnimationInfo *animationInfo;
	};
};

DefineArray(AssetInfo);
struct LoadedTexture
{
	Bitmap bitmap;
	AssetInfo *entry;
	LoadedTexture *next;
	LoadedTexture *prev;
};

struct LoadedTextureList
{
	LoadedTexture* base;
   
	LoadedTexture* front;
	LoadedTexture* back;
};

struct LoadedMesh
{
	TriangleMesh mesh;
	AssetInfo *entry;
	LoadedMesh *next;
	LoadedMesh *prev;
};

struct LoadedMeshList
{
	LoadedMesh* base;
   
	LoadedMesh* front;
	LoadedMesh* back;
};

typedef AssetInfoArray AssetCatalog;


struct AssetHandler
{
	union
	{
		struct
		{
			AssetCatalog textureCatalog;
			AssetCatalog materialCatalog;
			AssetCatalog meshCatalog;
			AssetCatalog animationCatalog;
		};
		AssetCatalog catalogs[Asset_Type_Count];
	};
   
	Arena *infoArena;
   
	LoadedTextureList textureList;
	LoadedMeshList meshList;
	KeyFramedAnimationDynamicArray animations;
};


static u32 RegisterAsset(AssetHandler *handler, AssetType type, String fileName, b32 *succsess = NULL)
{
	String tail = fileName;
	tail = EatToCharFromBackReturnTail(&tail, '/');
   
	Assert(type < Asset_Type_Count);
	auto catalog = handler->catalogs[type];
	For(catalog)
	{
		if (it->name == tail)
		{
			return it->id;
		}
	}
   
	ConsoleOutputError("Tried finding Asset \"%s\" but it does not exist.", fileName);
   
	if (succsess) *succsess = false;
   
	return (type << Asset_Type_Offset);
}


static u32 RegisterAsset(AssetHandler *handler, AssetType type, char *fileName, b32 *succsess = NULL)
{
	return RegisterAsset(handler, type, CreateString(fileName), succsess);
}

#include "AssetLoading.h"

//
// catalog stuff
//

static AssetCatalog LoadAssetCatalog(char *path, char *fileType, AssetType assetType, Arena *arena)
{
	AssetCatalog ret;
	StringArray meshFiles = FindAllFiles(path, fileType, arena);
   
	ret = PushArray(arena, AssetInfo, meshFiles.amount);
	For(ret)
	{
		u32 it_index = (u32)(it - ret.data);
		it->currentlyLoaded = false;
		it->id = it_index | (assetType << Asset_Type_Offset);
		it->name = meshFiles[it_index];
		it->type = assetType;
		it->loadedIndex = it_index;
		it->meshInfo = NULL;
	}
   
	return ret;
}

static bool AssetExits(AssetHandler *handler, String assetName, AssetType type)
{
	Assert(type < Asset_Type_Count);
	String name = EatToCharReturnHead(&assetName, '.');
	For(handler->catalogs[type])
	{
		if (GetToChar(it->name, '.') == name)
		{
			return true;
		}
	}
   
	return false;
}

// returns whether we imported something
static bool ConvertNewAssets(AssetHandler *handler, Arena *currentStateArena)
{
	bool imported = false;
	StringArray fileNames = FindAllFiles("import\\", "", frameArena);
   
	For(fileNames)
	{
		String type = GetBackToChar(*it, '.');
      
		char* nameToLoad = FormatCString("import/%s", *it);
      
		if (type == "bmp")
		{
			if (AssetExits(handler, *it, Asset_Texture)) continue;
         
			char* nameToSave = FormatCString("textures/%s.texture", GetToChar(*it, '.'));
			Bitmap bitmap = CreateBitmap(nameToLoad, frameArena);
         
			if (!bitmap.pixels)
			{
				ConsoleOutputError("Could not read bitmap %c*.", nameToLoad);
				continue;
			}
         
			if (!IsPowerOfTwo(bitmap.height) || !IsPowerOfTwo(bitmap.width))
			{
				ConsoleOutputError("Could not import new Asset. Bitmap %c* is not quatdratic with size power of two!", nameToLoad);
				continue;
			}
         
			while (bitmap.height > Asset_Bitmap_Size)
			{
				bitmap = DownSampleTexture(bitmap);
			}
         
			WriteTexture(nameToSave, bitmap);
			ConsoleOutput("Converted bitmap %c* to texture %c*", nameToLoad, nameToSave);
			imported = true;
		}
		else if (type == "obj")
		{
			if (AssetExits(handler, *it, Asset_Mesh)) continue;
			char *nameToSave = FormatCString("mesh/%s.mesh", GetToChar(*it, '.'));
			TriangleMesh mesh = ReadObj(nameToLoad, frameArena);
			if (!mesh.positions.amount)
			{
				ConsoleOutputError("Probably file name wrong. Might have loaded a mesh w/0 vertices.");
				continue;
			}
         
			WriteTriangleMesh(mesh, nameToSave);
			ConsoleOutput("Converted .obj %c* to .mesh %c*", nameToLoad, nameToSave);
			imported = true;
		}
		else if (type == "dae")
		{
			if (AssetExits(handler, *it, Asset_Animation)) continue;
			DAEReturn dae = ReadDAE(frameArena, nameToLoad);
			if (!dae.success)
			{
				ConsoleOutputError("Tried loading DAE %c*, but we failed!", nameToLoad);
				continue;
			}
         
			char *meshNameToSave = FormatCString("mesh/%s.mesh", GetToChar(*it, '.'));
			WriteTriangleMesh(dae.mesh, meshNameToSave);
         
			char *nameToSave = FormatCString("animation/%s.animation", GetToChar(*it, '.'));
			WriteAnimation(nameToSave, dae.animation);
         
			ConsoleOutput("Converted .dae %c* to .animation %c*", nameToLoad, nameToSave);
			imported = true;
		}
	}
   
	return imported;
}

// todo loading everything only for a frame?
static AssetHandler CreateAssetHandler(Arena *arena)
{
	AssetHandler ret;
   
	u8 *arenaReset = arena->current;
   
	ret.textureCatalog = LoadAssetCatalog("textures/", ".texture", Asset_Texture, arena);
	ret.meshCatalog = LoadAssetCatalog("mesh/", ".mesh", Asset_Mesh, arena);
	ret.animationCatalog = LoadAssetCatalog("animation/", ".animation", Asset_Animation, arena);
   
	if (ConvertNewAssets(&ret, arena))
	{
		arena->current = arena->current;
      
		ret.textureCatalog = LoadAssetCatalog("textures/", ".texture", Asset_Texture, arena);
		ret.meshCatalog = LoadAssetCatalog("mesh/", ".mesh", Asset_Mesh, arena);
		ret.animationCatalog = LoadAssetCatalog("animation/", ".animation", Asset_Animation, arena);
	}
   
	// todo growing arena?
	ret.infoArena = PushArena(arena, ret.textureCatalog.amount * sizeof(TextureInfo) + ret.meshCatalog.amount * sizeof(MeshInfo) + ret.animationCatalog.amount * sizeof(AnimationInfo));
   
	ret.textureList.base = PushData(arena, LoadedTexture, Asset_Texture_Amount);
	ret.textureList.front = ret.textureList.base;
	ret.textureList.back = ret.textureList.front + Asset_Texture_Amount - 1;
	for(auto it = ret.textureList.front; it < ret.textureList.front + Asset_Texture_Amount; it++)
	{
		it->bitmap.height = Asset_Bitmap_Size;
		it->bitmap.width  = Asset_Bitmap_Size;
		it->bitmap.pixels = PushData(arena, u32, it->bitmap.height * it->bitmap.width + 3) + 3;
		it->bitmap.textureHandle = RegisterWrapingTexture(it->bitmap.width, it->bitmap.height, it->bitmap.pixels);
		it->entry = NULL;
		it->next = it + 1;
		it->prev = it - 1;
	}
	ret.textureList.front->prev = NULL;
	ret.textureList.back->next = NULL;
	
	ret.meshList.base = PushData(arena, LoadedMesh, Asset_Mesh_Amount);
	ret.meshList.front = ret.meshList.base;
	ret.meshList.back = ret.meshList.front + Asset_Mesh_Amount - 1;
	for (auto it = ret.meshList.front; it < ret.meshList.front + Asset_Mesh_Amount; it++)
	{
		it->mesh = {};
		it->entry = NULL;
		it->next = it + 1;
		it->prev = it - 1;
	}
	ret.meshList.front->prev = NULL;
	ret.meshList.back->next = NULL;
   
	ret.animations = KeyFramedAnimationCreateDynamicArray();
   
	return ret;
}


static AssetInfo GetAssetInfo(AssetHandler *handler, u32 id)
{
	u32 type = (id >> Asset_Type_Offset);
	u32 index = id - (type << Asset_Type_Offset);
   
	return handler->catalogs[type][index]; // do fail?
}

static MeshInfo *GetMeshInfo(AssetHandler *handler, u32 id)
{
	AssetInfo info = GetAssetInfo(handler, id);
   
	return info.meshInfo;
}

static TextureInfo *GetTextureInfo(AssetHandler *handler, u32 id)
{
	AssetInfo info = GetAssetInfo(handler, id);
   
	return info.textureInfo;
}

static AnimationInfo *GetAnimationInfo(AssetHandler *handler, u32 id)
{
	AssetInfo info = GetAssetInfo(handler, id);
   
	return info.animationInfo;
}

static Bitmap *GetTexture(AssetHandler *handler, u32 id)
{
	TimedBlock;
	u32 type = (id >> Asset_Type_Offset);
	Assert(type == Asset_Texture);
	u32 index = id - (type << Asset_Type_Offset);
   
	AssetInfo *entry = &handler->textureCatalog[id];
	LoadedTexture *listBase = handler->textureList.base;
   
	if (entry->currentlyLoaded)
	{
		Assert(entry->loadedIndex < Asset_Texture_Amount);
      
		LoadedTexture *tex = listBase + entry->loadedIndex;
      
		if (tex->prev) tex->prev->next = tex->next;
		if (tex->next) tex->next->prev = tex->prev;
      
		tex->prev = NULL;
		tex->next = handler->textureList.front;
		handler->textureList.front->prev = tex;
		handler->textureList.front = tex;
      
		return &tex->bitmap;
	}
   
	LoadedTexture *toAlter = handler->textureList.back;
	handler->textureList.back = handler->textureList.back->prev;
   
	toAlter->entry = entry;
	toAlter->entry->loadedIndex = (u32)(toAlter - handler->textureList.base);
   
	char *filePath = FormatCString("textures/%s", entry->name);
	if (!LoadBitmapIntoBitmap(filePath, &listBase[entry->loadedIndex].bitmap))
	{
		return NULL;
	}
   
	if (!entry->textureInfo)
	{
		TextureInfo *info = PushStruct(handler->infoArena, TextureInfo);
		info->width = toAlter->bitmap.width;
		info->height = toAlter->bitmap.height;
		entry->textureInfo = info;
	}
   
	if (handler->textureList.back->entry)
	{
		handler->textureList.back->entry->currentlyLoaded = false;
	}
   
	handler->textureList.front->prev = toAlter;
	toAlter->next = handler->textureList.front;
	handler->textureList.front = toAlter;
	toAlter->prev = NULL;
   
	entry->currentlyLoaded = true;
	UpdateWrapingTexture(handler->textureList.base[entry->loadedIndex].bitmap);
   
	return &handler->textureList.base[entry->loadedIndex].bitmap;
}

// todo  for now animations and Meshes are dynamically allocated...
// todo not sure about this, this feels a bit stupid... on the other hand, we have to have an array somewhere :)
static KeyFramedAnimation *GetAnimation(AssetHandler *handler, u32 id)
{
	TimedBlock;
	Assert((id >> Asset_Type_Offset) == Asset_Animation);
	u32 index = id - (Asset_Animation << Asset_Type_Offset);
   
	AssetInfo *entry = &handler->animationCatalog[index];
	if (entry->currentlyLoaded)
	{
		KeyFramedAnimation *ret = handler->animations + entry->loadedIndex;
      
		return ret;
	}
   
	entry->currentlyLoaded = true;
   
	char* filePath = FormatCString("animation/%s", entry->name);
	KeyFramedAnimation animation = LoadAnimation(filePath);
   
	return handler->animations + ArrayAdd(&handler->animations, animation);
}

static TriangleMesh *GetMesh(AssetHandler *handler, u32 id)
{
	TimedBlock;
   
	Assert((id >> Asset_Type_Offset) == Asset_Mesh);
	u32 index = id - (Asset_Mesh << Asset_Type_Offset);
   
	AssetInfo *entry = &handler->meshCatalog[index];
   
	if (entry->currentlyLoaded)
	{
		Assert(entry->loadedIndex < Asset_Mesh_Amount);
      
		LoadedMesh *mesh = handler->meshList.base + entry->loadedIndex;
      
		if (mesh != handler->meshList.front)
		{
			mesh->prev->next = mesh->next;
			if (mesh->next) mesh->next->prev = mesh->prev;
         
			mesh->prev = NULL;
			mesh->next = handler->meshList.front;
			handler->meshList.front->prev = mesh;
			handler->meshList.front = mesh;
		}
      
		return &mesh->mesh;
	}
   
	if (handler->meshList.back->entry)
	{
		UnloadMesh(handler->meshList.back->entry);
		handler->meshList.back->entry->currentlyLoaded = false;
	}
   
   
	LoadedMesh *toAlter = handler->meshList.back;
	handler->meshList.back = handler->meshList.back->prev;
   
	entry->currentlyLoaded = true;
	entry->loadedIndex = (u32)(toAlter - handler->meshList.base);
   
	toAlter->entry = entry;
	
	handler->meshList.front->prev = toAlter;
	toAlter->next = handler->meshList.front;
	handler->meshList.front = toAlter;
	toAlter->prev = NULL;
   
	char* filePath = FormatCString("mesh/%s", entry->name);
   
	void *filePtr = NULL;
   
	toAlter->mesh = LoadMesh(handler, filePath, &filePtr);
   
	if (!entry->meshInfo)
	{
		MeshInfo *info = PushStruct(handler->infoArena, MeshInfo);
		info->aabb = toAlter->mesh.aabb;
		info->type = toAlter->mesh.type;
		entry->meshInfo = info;
		entry->meshInfo->fileLocation = filePtr;
	}
   
	return &toAlter->mesh;
   
}

#undef Asset_Type_Offset

#endif
