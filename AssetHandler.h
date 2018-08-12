#ifndef RR_ASSETHANDLER
#define RR_ASSETHANDLER

#include "Bitmap.h"
#include "Sound.h"
#include "Animation.h"

enum
{
	Asset_Unit,
	Asset_Building,
	Asset_TileMapTiles,
//	Asset_Map,
	Asset_Font,

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

#endif
