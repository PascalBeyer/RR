#include "AssetHandler.h"

#include "Fonts.h"

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
Asset::Asset(Bitmap *sprite,  AssetId id)
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
	case Asset_Font:
	{
		spriteCount = Font_Count;
		sprites = new Bitmap*[spriteCount];
		for (u32 spriteIndex = 0; spriteIndex < spriteCount; spriteIndex++)
		{
			sprites[spriteIndex] = new Bitmap;
		}
		*sprites[0] = CreateBitmap("../Art/Font/A.bmp");
		*sprites[1] = CreateBitmap("../Art/Font/B.bmp");
		*sprites[2] = CreateBitmap("../Art/Font/C.bmp");
		*sprites[3] = CreateBitmap("../Art/Font/D.bmp");
		*sprites[4] = CreateBitmap("../Art/Font/E.bmp");
		*sprites[5] = CreateBitmap("../Art/Font/F.bmp");
		*sprites[6] = CreateBitmap("../Art/Font/G.bmp");
		*sprites[7] = CreateBitmap("../Art/Font/H.bmp");
		*sprites[8] = CreateBitmap("../Art/Font/I.bmp");
		*sprites[9] = CreateBitmap("../Art/Font/J.bmp");
		*sprites[10] = CreateBitmap("../Art/Font/K.bmp");
		*sprites[11] = CreateBitmap("../Art/Font/L.bmp");
		*sprites[12] = CreateBitmap("../Art/Font/M.bmp");
		*sprites[13] = CreateBitmap("../Art/Font/N.bmp");
		*sprites[14] = CreateBitmap("../Art/Font/O.bmp");
		*sprites[15] = CreateBitmap("../Art/Font/P.bmp");
		*sprites[16] = CreateBitmap("../Art/Font/Q.bmp");
		*sprites[17] = CreateBitmap("../Art/Font/R.bmp");
		*sprites[18] = CreateBitmap("../Art/Font/S.bmp");
		*sprites[19] = CreateBitmap("../Art/Font/T.bmp");
		*sprites[20] = CreateBitmap("../Art/Font/U.bmp");
		*sprites[21] = CreateBitmap("../Art/Font/V.bmp");
		*sprites[22] = CreateBitmap("../Art/Font/W.bmp");
		*sprites[23] = CreateBitmap("../Art/Font/X.bmp");
		*sprites[24] = CreateBitmap("../Art/Font/Y.bmp");
		*sprites[25] = CreateBitmap("../Art/Font/Z.bmp");
		
		*sprites[26] = CreateBitmap("../Art/Font/0.bmp");
		*sprites[27] = CreateBitmap("../Art/Font/1.bmp");
		*sprites[28] = CreateBitmap("../Art/Font/2.bmp");
		*sprites[29] = CreateBitmap("../Art/Font/3.bmp");
		*sprites[30] = CreateBitmap("../Art/Font/4.bmp");
		*sprites[31] = CreateBitmap("../Art/Font/5.bmp");
		*sprites[32] = CreateBitmap("../Art/Font/6.bmp");
		*sprites[33] = CreateBitmap("../Art/Font/7.bmp");
		*sprites[34] = CreateBitmap("../Art/Font/8.bmp");
		*sprites[35] = CreateBitmap("../Art/Font/9.bmp");
		
		*sprites[36] = CreateBitmap("../Art/Font/anfuerzeichen.bmp");
		*sprites[37] = CreateBitmap("../Art/Font/ausrufezeichen.bmp");
		*sprites[38] = CreateBitmap("../Art/Font/doppelPunkt.bmp");
		*sprites[39] = CreateBitmap("../Art/Font/fragezeichen.bmp");
		*sprites[40] = CreateBitmap("../Art/Font/groeser.bmp");
		*sprites[41] = CreateBitmap("../Art/Font/kleiner.bmp");
		*sprites[42] = CreateBitmap("../Art/Font/komma.bmp");
		*sprites[43] = CreateBitmap("../Art/Font/leerzeichen.bmp");
		*sprites[44] = CreateBitmap("../Art/Font/punkt.bmp");
		*sprites[45] = CreateBitmap("../Art/Font/stern.bmp");
		*sprites[46] = CreateBitmap("../Art/Font/minus.bmp");
		soundCount = 0;
	}break;
	default:
	{

	}break;
	}
}
void Asset::Unload()
{
	id.id = Asset_Invalid;
	for (u32 i = 0; i < spriteCount; i++)
	{
		Free(sprites[i]);
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