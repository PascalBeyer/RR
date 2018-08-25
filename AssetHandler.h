#ifndef RR_ASSETHANDLER
#define RR_ASSETHANDLER

#include "Bitmap.h"
#include "Sound.h"

enum
{
	Asset_Unit,
	Asset_Building,
	Asset_TileMapTiles,
//	Asset_Map,

	Asset_White,

	Asset_Count, 
	Asset_Invalid
};


struct AssetId
{
	u32 id;
};

class Asset
{
public:
	Asset();
	~Asset();
	Asset(AssetId id);
	Asset(Bitmap **sprites, AssetId id, u32 spriteCount);
	Asset(Bitmap *sprites, AssetId id);

	void Unload();

	Bitmap **sprites;
	Sound **sounds;

	AssetId id;
	u32 spriteCount;
	u32 soundCount;
private:

};

class AssetHandler
{
public:
	AssetHandler();

	Asset *GetAsset(AssetId id);
	Sound **GetSounds(AssetId id);
	Sound *GetSound(AssetId id, u32 soundIndex);
	Bitmap **GetBitmaps(AssetId id);

	Bitmap *GetWhite();

	void LoadAsset(AssetId id);
	void RemoveAsset(AssetId id);

	Asset *assets;
private:
	//u32 *assetLookup;

};

Asset::Asset()
{
	sprites = NULL;
	id.id = Asset_Invalid;
}
Asset::~Asset()
{

}
Asset::Asset(Bitmap **sprites, AssetId id, u32 spriteCount)
{
	this->spriteCount = spriteCount;
	this->sprites = sprites;
	this->id = id;
}
Asset::Asset(Bitmap *sprite, AssetId id)
{
	sprites = new Bitmap*[1];
	spriteCount = 1;
	sprites[0] = sprite;
	this->id = id;
}

Asset::Asset(AssetId id)
{
	this->id = id;
	switch (id.id)
	{
	case (Asset_White):
	{
		u32 *whitePixels = new u32[16];
		for (u32 iWhite = 0; iWhite < 16; iWhite++)
		{
			whitePixels[iWhite] = 0xFFFFFFFF;
		}
		spriteCount = 1;
		sprites = new Bitmap*[1];
		sprites[0] = new Bitmap;
		*sprites[0] = CreateBitmap(whitePixels, 4, 4);

	}break;
	case Asset_Building:
	{
		spriteCount = 1;
		sprites = new Bitmap*[spriteCount];
		for (u32 spriteIndex = 0; spriteIndex < spriteCount; spriteIndex++)
		{
			sprites[spriteIndex] = new Bitmap;
		}
		*sprites[0] = CreateBitmap("../Art/Tree01.bmp");
	}break;
	case Asset_Unit:
	{
		spriteCount = 3;
		sprites = new Bitmap*[spriteCount];
		for (u32 spriteIndex = 0; spriteIndex < spriteCount; spriteIndex++)
		{
			sprites[spriteIndex] = new Bitmap;
		}
		*sprites[0] = CreateBitmap("../Art/dude2/head.bmp");
		*sprites[1] = CreateBitmap("../Art/dude2/torso.bmp");
		*sprites[2] = CreateBitmap("../Art/dude2/leg.bmp");

		soundCount = 1;
		sounds = new Sound*[soundCount];
		sounds[0] = new Sound("../Sound/bloop_00.wav");

	}break;
	case Asset_TileMapTiles:
	{
		spriteCount = 6;
		sprites = new Bitmap*[spriteCount];
		for (u32 spriteIndex = 0; spriteIndex < spriteCount; spriteIndex++)
		{
			sprites[spriteIndex] = new Bitmap;
		}
		*sprites[0] = CreateBitmap("../Art/fds/Empty.bmp");
		*sprites[1] = CreateBitmap("../Art/fds/Square.bmp");
		*sprites[2] = CreateBitmap("../Art/Tilemap/UpperLeft.bmp");
		*sprites[3] = CreateBitmap("../Art/Tilemap/UpperRight.bmp");
		*sprites[4] = CreateBitmap("../Art/Tilemap/LowerLeft.bmp");
		*sprites[5] = CreateBitmap("../Art/Tilemap/LowerRight.bmp");


	}break;
#if 0 
	case Asset_Map:
	{
		spriteCount = 1;
		sprites = new Bitmap*[spriteCount];
		soundCount = 1;
		sounds = new Sound*[soundCount];
		sounds[0] = new Sound("../Sound/music_test.wav");
	}break;
#endif
	
	}
}
void Asset::Unload()
{
	id.id = Asset_Invalid;
	for (u32 i = 0; i < spriteCount; i++)
	{
		FreeBitmap(sprites[i]);
	}
}


AssetHandler::AssetHandler()
{
	assets = new Asset[Asset_Count];
	this->LoadAsset({ Asset_White });
}

struct LoadAssetWork
{
	AssetId id;
	Asset *asset;
};
void LoadAssetsCallback(void *data)
{
	LoadAssetWork *work = (LoadAssetWork *)data;
	if (!work->asset)
	{
		work->asset = new Asset(work->id);
	}
}
void AssetHandler::LoadAsset(AssetId id)
{
	assets[id.id] = Asset(id);
}
Bitmap *AssetHandler::GetWhite()
{
	return this->GetAsset({ Asset_White })->sprites[0];
}

Asset *AssetHandler::GetAsset(AssetId id)
{
	return (assets + id.id);
}
Bitmap **AssetHandler::GetBitmaps(AssetId id)
{
	return GetAsset(id)->sprites;
}

Sound **AssetHandler::GetSounds(AssetId id)
{
	return GetAsset(id)->sounds;
}
Sound *AssetHandler::GetSound(AssetId id, u32 soundIndex)
{
	Asset *asset = GetAsset(id);
	if (soundIndex < asset->soundCount)
	{
		return asset->sounds[soundIndex];
	}
	else
	{
		return NULL;
	}

}

void AssetHandler::RemoveAsset(AssetId id)
{
	assets[id.id].Unload();
}
#endif
