#ifndef RR_ASSETHANDLER
#define RR_ASSETHANDLER

enum AssetType
{
	// Asset_Mesh, I have decided these are not assets, as their use depends alot of the context and there might be a lot of 'em
	Asset_Texture,
	Asset_Material,

	Asset_Type_Count,
};
#define Asset_Type_Offset 25
#define Asset_Texture_Amount 25
struct AssetCatalogEntry
{
	String name; // for now this is a c-string, not sure if that has to change
	AssetType type;
	bool currentlyLoaded;
	u32 id;
	u32 index;
};

DefineDynamicArray(AssetCatalogEntry);
struct LoadedTexture
{
	Bitmap bitmap;
	AssetCatalogEntry *entry;
};

DefineArray(LoadedTexture);

typedef AssetCatalogEntryDynamicArray AssetCatalog;

struct AssetHandler
{
	union
	{
		struct
		{
			AssetCatalog meshCatalog;
			AssetCatalog textureCatalog;
			AssetCatalog materialCatalog;
		};
		AssetCatalog catalogs[Asset_Type_Count];
	};
	u32 serializer;

	LoadedTextureArray textures;
	u32 rollingTextureId;
};

#define AssetBitmapSize 2048
static AssetHandler *CreateAssetHandler(Arena *arena)
{
	AssetHandler *ret = PushStruct(arena, AssetHandler);
	for (u32 i = 0; i < Asset_Type_Count; i++)
	{
		ret->catalogs[i] = AssetCatalogEntryCreateDynamicArray();
	}
	ret->serializer = 0;
	ret->textures = PushArray(arena, LoadedTexture, Asset_Texture_Amount);
	For(ret->textures)
	{
		it->bitmap.height = AssetBitmapSize;
		it->bitmap.width  = AssetBitmapSize;
		it->bitmap.pixels = PushData(arena, u32, it->bitmap.height * it->bitmap.width);
		it->bitmap.textureHandle = RegisterWrapingTexture(it->bitmap.width, it->bitmap.height, it->bitmap.pixels);
	}

	return ret;
}

static u32 RegisterAsset(AssetHandler *handler, AssetType type, char *fileName)
{
	Assert(type < Asset_Type_Count);
	auto catalog = handler->catalogs[type];
	For(catalog)
	{
		if (it->name == fileName)
		{
			return it->id;
		}
	}

	AssetCatalogEntry entry;	
	entry.id = handler->serializer++ | (type << Asset_Type_Offset); 

	Assert(handler->serializer < (1 << Asset_Type_Offset));

	entry.name = S(fileName, constantArena, true);
	entry.currentlyLoaded = false;
	entry.type = type;
	ArrayAdd(handler->catalogs + type, entry);

	return entry.id;
}

static Bitmap *GetTexture(AssetHandler *handler, u32 id)
{
	u32 type = (id >> Asset_Type_Offset);
	Assert(type == Asset_Texture);
	AssetCatalogEntry *entry = NULL;
	For(handler->catalogs[Asset_Texture])
	{
		if (it->id == id)
		{
			entry = it;
			break;
		}
	}

	Assert(entry);
	if (entry->currentlyLoaded)
	{
		Assert(entry->index < handler->textures.amount);
		return &handler->textures[entry->index].bitmap;
	}

	// todo for now this is a rolling buffer....

	if (handler->textures[handler->rollingTextureId].entry)
	{
		handler->textures[handler->rollingTextureId].entry->currentlyLoaded = false;
	}
		

	handler->textures[handler->rollingTextureId].entry = entry;
	entry->index = handler->rollingTextureId;
	handler->rollingTextureId = (handler->rollingTextureId + 1) % handler->textures.amount;
	
	File file = LoadFile((char *)entry->name.data, handler->textures[entry->index].bitmap.pixels, AssetBitmapSize * AssetBitmapSize * sizeof(u32));
	Assert(file.fileSize);
	entry->currentlyLoaded = true;
	UpdateWrapingTexture(handler->textures[entry->index].bitmap);

	return &handler->textures[entry->index].bitmap;
}


#undef Asset_Type_Offset
#undef Asset_Texture_Amount 

#endif
