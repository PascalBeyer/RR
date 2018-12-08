#ifndef RR_ASSETHANDLER
#define RR_ASSETHANDLER

enum AssetType
{
	Asset_Texture,
	Asset_Material,
	Asset_Mesh,

	Asset_Type_Count,
};
#define Asset_Type_Offset 25
#define Asset_Texture_Amount 10
#define Asset_Mesh_Amount 100
#define Asset_Bitmap_Size 1024

struct AssetInfo
{
	String name;
	AssetType type;
	bool currentlyLoaded;
	u32 id;
	u32 loadedIndex;
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
		};
		AssetCatalog catalogs[Asset_Type_Count];
	};
	u32 serializer;

	LoadedTextureList textureList;
	LoadedMeshList meshList;

};

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


// load everything always only for a frame?
static AssetHandler CreateAssetHandler(Arena *arena)
{
	AssetHandler ret;

	ret.textureCatalog = LoadAssetCatalog("textures/", ".texture", Asset_Texture, arena);

	ret.meshCatalog = LoadAssetCatalog("mesh/", ".mesh", Asset_Mesh, arena);

	ret.serializer = 0;
	
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

	return ret;
}

static u32 RegisterAsset(AssetHandler *handler, AssetType type, char *fileName)
{
	String tail = CreateString(fileName);
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

	ConsoleOutputError("Tried finding Asset \"%c*\" but it does not exist.", fileName);

	return (type << Asset_Type_Offset);
}

static u32 RegisterAsset(AssetHandler *handler, AssetType type, String fileName)
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

	return (type << Asset_Type_Offset);
}

static void UnloadMesh(TriangleMesh *mesh)
{
	u32 *pointer = (u32 *)mesh->vertices.data;
	pointer -= 2;
	DynamicFree(pointer);
}

static TriangleMesh LoadMesh(AssetHandler *assetHandler, char *fileName)
{
	//return {};
	TriangleMesh ret;

	File file = LoadFile(fileName);
	if (!file.fileSize) return {};
	u8 *cur = (u8 *)file.memory;
	ret.type = *(u32 *)cur;
	cur += sizeof(u32);
	ret.vertices.amount = *(u32 *)cur;
	cur += sizeof(u32);
	ret.vertices.data = (VertexFormat *)cur;
	cur += ret.vertices.amount * sizeof(VertexFormat);

	ret.indices.amount = *(u32 *)cur;
	cur += sizeof(u32);
	ret.indices.data = (u16 *)cur;
	cur += ret.indices.amount * sizeof(u16);

	ret.indexSets.amount = *(u32 *)cur;
	cur += sizeof(u32);
	ret.indexSets.data = (IndexSet *)cur;
	cur += ret.indexSets.amount * sizeof(IndexSet);

	For(ret.indexSets)
	{
		it->mat.name.length = *(u32 *)cur;
		cur += sizeof(u32);
		it->mat.name.data = (Char *)cur;
		cur += it->mat.name.length * sizeof(Char);
		cur++; // to get it zero Terminated

		it->mat.texturePath.length = *(u32 *)cur;
		cur += sizeof(u32);
		it->mat.texturePath.data = (Char *)cur;
		cur += it->mat.texturePath.length * sizeof(Char);
		cur++; // to get it zero Terminated

		it->mat.bitmapID = RegisterAsset(assetHandler, Asset_Texture, (char *)it->mat.texturePath.data);
	}

	AABB aabb = InvertedInfinityAABB();

	For(ret.vertices) // todo  we could save that
	{
		aabb.maxDim.x = Max(it->p.x, aabb.maxDim.x);
		aabb.maxDim.y = Max(it->p.y, aabb.maxDim.y);
		aabb.maxDim.z = Max(it->p.z, aabb.maxDim.z);

		aabb.minDim.x = Min(it->p.x, aabb.minDim.x);
		aabb.minDim.y = Min(it->p.y, aabb.minDim.y);
		aabb.minDim.z = Min(it->p.z, aabb.minDim.z);
	}

	ret.aabb = aabb;

	// todo update mesh!
	RegisterTriangleMesh(&ret);

	return ret;
}


static void WriteTriangleMesh(TriangleMesh mesh, char *fileName) // todo : when we feel bored, we can make this a bit mor efficient
{
	File file;
	// begin of daisy chain
	file.memory = PushData(frameArena, u8, 0);
	*PushStruct(frameArena, u32) = mesh.type;
	*PushStruct(frameArena, u32) = mesh.vertices.amount;
	For(mesh.vertices)
	{
		/*
		*PushStruct(frameArena, v3) = it->p;
		*PushStruct(frameArena, v2) = it->uv;
		*PushStruct(frameArena, v3) = it->n;
		*PushStruct(frameArena, u32) = it->c;
		*/
		*PushStruct(frameArena, VertexFormat) = *it;
	}

	*PushStruct(frameArena, u32) = mesh.indices.amount;
	For(mesh.indices)
	{
		*PushStruct(frameArena, u16) = *it;
	}

	*PushStruct(frameArena, u32) = mesh.indexSets.amount;
	For(mesh.indexSets)
	{
		*PushStruct(frameArena, IndexSet) = *it;
	}

	For(mesh.indexSets)
	{
		Material mat = it->mat;

		*PushStruct(frameArena, u32) = mat.name.length;
		Char *dest1 = PushData(frameArena, Char, mat.name.length);
		memcpy(dest1, mat.name.data, mat.name.length * sizeof(Char));
		PushZeroStruct(frameArena, Char); // to get it zero Terminated

		*PushStruct(frameArena, u32) = mat.texturePath.length;
		Char *dest2 = PushData(frameArena, Char, mat.texturePath.length);
		memcpy(dest2, mat.texturePath.data, mat.texturePath.length * sizeof(Char));
		PushZeroStruct(frameArena, Char); // to get it zero Terminated
	}

	file.fileSize = (u32)(frameArena->current - (u8 *)file.memory);
	WriteEntireFile(fileName, file);
}


static AssetInfo GetAssetInfo(AssetHandler *handler, u32 id)
{
	u32 type = (id >> Asset_Type_Offset);
	u32 index = id - (type << Asset_Type_Offset);

	return handler->catalogs[type][index];
}

static TriangleMesh *GetMesh(AssetHandler *handler, u32 id)
{
	TimedBlock;

	u32 type = (id >> Asset_Type_Offset);
	Assert(type == Asset_Mesh);
	u32 index = id - (type << Asset_Type_Offset);

	AssetInfo *entry = &handler->meshCatalog[index];

	if (entry->currentlyLoaded)
	{
		Assert(entry->loadedIndex < Asset_Mesh_Amount);

		LoadedMesh *mesh = handler->meshList.base + entry->loadedIndex;

		if (mesh->prev) mesh->prev->next = mesh->next;
		if (mesh->next) mesh->next->prev = mesh->prev;

		mesh->prev = NULL;
		mesh->next = handler->meshList.front;
		handler->meshList.front->prev = mesh;
		handler->meshList.front = mesh;

		return &mesh->mesh;
	}

	if (handler->meshList.back->entry)
	{
		UnloadMesh(&handler->meshList.back->mesh);
		handler->meshList.back->entry->currentlyLoaded = false;
	}

	LoadedMesh *toAlter = handler->meshList.back;
	handler->meshList.back = handler->meshList.back->prev;

	toAlter->entry = entry;
	entry->currentlyLoaded = true;
	toAlter->entry->loadedIndex = (u32)(toAlter - handler->meshList.base);

	handler->meshList.front->prev = toAlter;
	toAlter->next = handler->meshList.front;
	handler->meshList.front = toAlter;
	toAlter->prev = NULL;

	String filePath = FormatCString("mesh/%s", entry->name);

	toAlter->mesh = LoadMesh(handler, (char *)filePath.data);

	return &toAlter->mesh;

}

static Bitmap DownSampleTexture(Bitmap bitmap)
{
	u32 bitmapWidth  = (bitmap.width  >> 1);
	u32 bitmapHeight = (bitmap.height >> 1);

	u32* data = PushData(frameArena, u32, bitmapWidth * bitmapHeight);

	for (u32 h = 0; h < bitmapHeight; h++)
	{
		for (u32 w = 0; w < bitmapWidth; w++)
		{
			v4 ul = Unpack4x8(*GetPixel(bitmap, 2 * w, 2 * h));
			v4 ur = Unpack4x8(*GetPixel(bitmap, 2 * w + 1, 2 * h));
			v4 ll = Unpack4x8(*GetPixel(bitmap, 2 * w, 2 * h + 1));
			v4 lr = Unpack4x8(*GetPixel(bitmap, 2 * w + 1, 2 * h + 1));
			data[h * bitmapWidth + w] = Pack4x8(0.25f * (ul + ur + ll + lr));
		}
	}
	return CreateBitmap(data, bitmapWidth, bitmapHeight);
}
static bool LoadBitmapIntoBitmap(char *filefile, Bitmap *texture)
{
	File file = LoadFile(filefile);
	defer(FreeFile(file));
	if (!file.fileSize) return false;
	u8 *at = (u8 *)file.memory;
	u32 version = *(u32 *)at;
	Assert(version == 1);
	at += sizeof(u32);
	u32 width = *(u32 *)at;
	at += sizeof(u32);
	u32 height = *(u32 *)at;
	at += sizeof(u32);

	memcpy(texture->pixels, at, width * height * sizeof(u32));

	texture->height = height;
	texture->width = width;
	return true;
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

		if(tex->prev) tex->prev->next = tex->next;
		if(tex->next) tex->next->prev = tex->prev;

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

	String filePath = FormatCString("textures/%s", entry->name);
	if (!LoadBitmapIntoBitmap(filePath.cstr, &listBase[entry->loadedIndex].bitmap))
	{
		return NULL;
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


static void WriteTexture(char *fileName, Bitmap bitmap);

// look at 1:50 in vertex vbo
static MaterialDynamicArray LoadMTL(AssetHandler *assetHandler, String path, String fileName, Arena *arena)
{
	MaterialDynamicArray ret = MaterialCreateDynamicArray();
	DeferRestore(frameArena);

	File file = LoadFile((char *)FormatCString("%s%s\0", path, fileName).data, frameArena);
	Assert(file.fileSize);
	String string = CreateString((Char *)file.memory, file.fileSize);

	b32 success = true;

	while (string.length)
	{
		Material cur = {};
		String line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// newmtl
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "newmtl");
			EatSpaces(&line);

			cur.name = CopyString(line, arena);
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// Ns
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "Ns");
			EatSpaces(&line);
			cur.spectularExponent = StoF(line, &success);
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// Ka
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "Ka");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			f32 a1 = StoF(head, &success);

			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a2 = StoF(head, &success);

			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a3 = StoF(head, &success);

			cur.ambientColor = V3(a1, a2, a3);
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// Kd
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "Kd");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			f32 a1 = StoF(head, &success);

			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a2 = StoF(head, &success);

			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a3 = StoF(head, &success);

			cur.diffuseColor = V3(a1, a2, a3);
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// Ks
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "Ks");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			f32 a1 = StoF(head, &success);

			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a2 = StoF(head, &success);

			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a3 = StoF(head, &success);

			cur.specularColor = V3(a1, a2, a3);
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// Ke
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "Ke");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			f32 a1 = StoF(head, &success);

			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a2 = StoF(head, &success);

			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a3 = StoF(head, &success);

			cur.ke = V3(a1, a2, a3);
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// Ni
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "Ni");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			f32 a1 = StoF(head, &success);

			cur.indexOfReflection = a1;
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// d
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "d");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			f32 a1 = StoF(head, &success);

			cur.dissolved = a1;
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// illum
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "illum");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			u32 a1 = StoU(head, &success);

			cur.illuminationModel = a1;
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// map_Kd
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "map_Kd");
			EatSpaces(&line);
			String tail = EatToCharFromBackReturnTail(&line, '\\');
			cur.texturePath = CopyString(FormatCString("%s.texture", GetToChar(tail, '.')), arena);

			//if(assetHandler) cur.bitmapID = RegisterAsset(assetHandler, Asset_Texture, (char *)cur.texturePath.data);
		}

		ArrayAdd(&ret, cur);
	}

	Assert(success);

	return ret;
}

static void WriteTexture(char *fileName, Bitmap bitmap)
{
	DeferRestore(frameArena);

	Assert(bitmap.height == Asset_Bitmap_Size);
	Assert(bitmap.width == Asset_Bitmap_Size);
	Assert(bitmap.pixels);

	u8 *data = PushData(frameArena, u8, 0);

	*PushStruct(frameArena, u32) = 1; // version
	*PushStruct(frameArena, u32) = bitmap.width;
	*PushStruct(frameArena, u32) = bitmap.height;
	u32 *pixels = PushData(frameArena, u32, bitmap.width * bitmap.height);
	memcpy(pixels, bitmap.pixels, bitmap.width * bitmap.height * sizeof(u32));

	u32 fileSize = (u32)(frameArena->current - data);

	File file = CreateFile(data, fileSize);
	WriteEntireFile(fileName, file);
}

static String GetFilePathFromName(String fileName)
{
	u32 positionOfLastSlash = 0;
	for (u32 i = fileName.length - 1; i < MAXU32; i--)
	{
		if (fileName[i] == '/')
		{
			positionOfLastSlash = i;
			break;
		}
	}
	if (!positionOfLastSlash)
	{
		return {};
	}

	String ret;
	ret.data = fileName.data;
	ret.length = positionOfLastSlash + 1;
	return ret;
}


static TriangleMesh ReadObj(AssetHandler *assetHandler, char *fileName, Arena *arena, Arena *workingArena)
{
	String filename = CreateString(fileName);
	String path = GetFilePathFromName(filename);
	TriangleMesh ret = {};

	u8 *frameArenaReset = frameArena->current;
	defer(frameArena->current = frameArenaReset);

	File file = LoadFile(fileName, frameArena);
	if (!file.fileSize) return {};
	String string = CreateString((Char *)file.memory, file.fileSize);

	// here mtllib "bla.mtl"
	String mtllib = ConsumeNextLineSanitize(&string);
	while (string.length && mtllib.length == 0 || mtllib[0] == '#') { mtllib = ConsumeNextLineSanitize(&string); }
	Assert(string.length);
	String ident = EatToNextSpaceReturnHead(&mtllib);
	Assert(ident == "mtllib");
	EatSpaces(&mtllib);

	MaterialDynamicArray materials = LoadMTL(assetHandler, path, mtllib, arena);
	// todo make sure that multiples get handled, i.e make this assets.

	b32 success = true;
	u32 amountOfVertsBefore = 1;
	u32 amountOfNormalsBefore = 1;
	u32 amountOfUVsBefore = 1;
	u32 amountOfIndeciesBefore = 0;
	String line = ConsumeNextLineSanitize(&string);

	u16PtrDynamicArray indexPointerArray = u16PtrCreateDynamicArray();
	IndexSetDynamicArray indexSets = IndexSetCreateDynamicArray();

	// begin on vertex daisy chain
	ret.vertices.data = (VertexFormat *)arena->current;

	while (string.length)
	{
		IndexSet cur;

		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;

		// o
		{
			String head = EatToNextSpaceReturnHead(&line);
			Assert(head == "o");
			EatSpaces(&line);
			String o = line; // ignored for now
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;

		// todo  speed, here we first load and then copy later when we read in the triangles, not sure if we could make this better...

		BeginArray(frameArena, v3, tempVertexArray); // could silent fill and the push, which is not as "save".
		while (line[0] == 'v' && line[1] == ' ')
		{
			Eat1(&line);
			EatSpaces(&line);
			String f1 = EatToNextSpaceReturnHead(&line);
			EatSpaces(&line);
			String f2 = EatToNextSpaceReturnHead(&line);
			EatSpaces(&line);
			String f3 = EatToNextSpaceReturnHead(&line);

			f32 s1 = StoF(f1, &success);
			f32 s2 = StoF(f2, &success);
			f32 s3 = StoF(f3, &success);
			v3 *val = PushStruct(frameArena, v3);
			*val = V3(s1, s2, s3);

			line = ConsumeNextLineSanitize(&string);
		}
		EndArray(frameArena, v3, tempVertexArray);

		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;

		BeginArray(frameArena, v2, textrueCoordinates);
		while (line[0] == 'v' && line[1] == 't')
		{
			Eat1(&line);
			Eat1(&line);
			EatSpaces(&line);
			String f1 = EatToNextSpaceReturnHead(&line);
			EatSpaces(&line);
			String f2 = EatToNextSpaceReturnHead(&line);

			f32 s1 = StoF(f1, &success);
			f32 s2 = StoF(f2, &success);
			v2 *val = PushStruct(frameArena, v2);
			*val = V2(s1, s2);

			line = ConsumeNextLineSanitize(&string);
		}
		EndArray(frameArena, v2, textrueCoordinates);

		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;
		BeginArray(frameArena, v3, normals);
		while (line[0] == 'v' && line[1] == 'n')
		{
			Eat1(&line);
			Eat1(&line);
			EatSpaces(&line);
			String f1 = EatToNextSpaceReturnHead(&line);
			EatSpaces(&line);
			String f2 = EatToNextSpaceReturnHead(&line);
			EatSpaces(&line);
			String f3 = EatToNextSpaceReturnHead(&line);

			f32 s1 = StoF(f1, &success);
			f32 s2 = StoF(f2, &success);
			f32 s3 = StoF(f3, &success);
			v3 *val = PushStruct(frameArena, v3);
			*val = V3(s1, s2, s3);

			line = ConsumeNextLineSanitize(&string);
		}
		EndArray(frameArena, v3, normals);

		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;

		// usemtl
		{
			String head = EatToNextSpaceReturnHead(&line);
			Assert(head == "usemtl");
			EatSpaces(&line);
			String name = line;
			For(materials)
			{
				if (it->name == name)
				{
					cur.mat = *it;
					break;
				}
			}
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;

		// s -smoothening, what ever that means
		{
			String head = EatToNextSpaceReturnHead(&line);
			Assert(head == "s");
			EatSpaces(&line);

			//u32 s = StoU(line, &success); // ignored for now
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;

		struct UVList
		{
			u16 uvIndex;
			u16 flattendIndex;
			u16 normalIndex;
			UVList *next;
		};
		typedef UVList * UVListPtr;
		DefineArray(UVListPtr);

		Clear(workingArena);
		UVListPtrArray flatten = PushZeroArray(workingArena, UVListPtr, tempVertexArray.amount);
		u32 vertexArrayLength = 0;

		cur.offset = amountOfIndeciesBefore;

		BeginArray(frameArena, u16, indecies);
		ArrayAdd(&indexPointerArray, indecies.data);
		while (line[0] == 'f')
		{
			Eat1(&line);

			// we assume they are all here

			for (u32 parseIndex = 0; parseIndex < 3; parseIndex++)
			{
				EatSpaces(&line);
				String s1 = EatToCharReturnHead(&line, '/', ' ');
				Eat1(&line);
				String s2 = EatToCharReturnHead(&line, '/', ' ');
				Eat1(&line);
				String s3 = EatToCharReturnHead(&line, ' ');


				u32 u1 = StoU(s1, &success) - amountOfVertsBefore;		// this has to be relative to be an index, but also saved absolute wrt the flattend array
				u32 u2 = StoU(s2, &success) - amountOfUVsBefore;		// this is relative as we just need it to build our array
				u32 u3 = StoU(s3, &success) - amountOfNormalsBefore;	// this is also relative 

																		//todo no handling for meshes, that have more then 0xFFFE verticies

																		// we are flattening trough an array of vertex positions
				u16 flattenedIndex = 0xFFFF;
				for (UVListPtr it = flatten[u1]; it; it = it->next)
				{
					if (it->uvIndex == u2 && it->normalIndex == u3)
					{
						flattenedIndex = it->flattendIndex;
						break;
					}
				}

				if (flattenedIndex == 0xFFFF)
				{
					UVList *append = PushStruct(workingArena, UVList);
					append->flattendIndex = vertexArrayLength++;
					append->next = flatten[u1];
					append->uvIndex = (u16)u2;
					append->normalIndex = (u16)u3;
					flatten[u1] = append;

					flattenedIndex = append->flattendIndex;
				}

				*PushStruct(frameArena, u16) = flattenedIndex + ret.vertices.amount;
			}

			line = ConsumeNextLineSanitize(&string);
		}
		EndArray(frameArena, u16, indecies);
		cur.amount = indecies.amount;

		ArrayAdd(&indexSets, cur);

		amountOfIndeciesBefore += indecies.amount;
		amountOfVertsBefore += tempVertexArray.amount;
		amountOfNormalsBefore += normals.amount;
		amountOfUVsBefore += textrueCoordinates.amount;
		ret.vertices.amount += vertexArrayLength;

		// vertexArrayLength allready incrented by flattening
		VertexFormatArray save = PushArray(arena, VertexFormat, vertexArrayLength);
		//
		// WARNING, we are daisy chaining here into constantArena and into frameArena
		//
		For(flatten)
		{
			u32 it_index = (u32)(it - &flatten[0]);
			for (UVListPtr i = *it; i; i = i->next)
			{
				save[i->flattendIndex].p = tempVertexArray[it_index];
				save[i->flattendIndex].uv = textrueCoordinates[i->uvIndex];
				save[i->flattendIndex].n = normals[i->normalIndex];
				save[i->flattendIndex].c = 0xFFFFFFFF;
			}
		}
	}


	ret.indices.data = (u16 *)arena->current;
	Assert(indexPointerArray.amount == indexSets.amount);
	for (u32 i = 0; i < indexPointerArray.amount; i++)
	{
		IndexSet set = indexSets[i];
		u16 *source = indexPointerArray[i];
		u16 *dest = PushData(arena, u16, set.amount);
		memcpy(dest, source, set.amount * sizeof(u16));
	}
	ret.indices.amount = (u32)((u16 *)arena->current - ret.indices.data);

	ret.indexSets = PushArray(arena, IndexSet, indexSets.amount);
	memcpy(ret.indexSets.data, indexSets.data, indexSets.amount * sizeof(IndexSet));

	ret.type = TriangleMeshType_List;

	RegisterTriangleMesh(&ret);

	Assert(success);

	return ret;
}


static void ConvertNewAssets(AssetHandler *handler, Arena *currentStateArena, Arena *workingArena)
{
	StringArray fileNames = FindAllFiles("import\\", "", frameArena);

	For(fileNames)
	{
		String type = GetBackToChar(*it, '.');

		String nameToLoad = FormatCString("import/%s", *it);

		if (type == "bmp")
		{
			if (AssetExits(handler, *it, Asset_Texture)) continue;
			String nameToSave = FormatCString("textures/%s.texture", GetToChar(*it, '.'));
			Bitmap bitmap = CreateBitmap(nameToLoad);
			
			if (!bitmap.pixels)
			{
				ConsoleOutputError("Could not read bitmap %s.", nameToLoad);
				continue;
			}

			if (!IsPowerOfTwo(bitmap.height)|| !IsPowerOfTwo(bitmap.width))
			{
				ConsoleOutputError("Could not import new Asset. Bitmap %s is not quatdratic with size power of two!", nameToLoad);
				continue;
			}

			while (bitmap.height > Asset_Bitmap_Size)
			{
				bitmap = DownSampleTexture(bitmap);
			}
			
			WriteTexture((char *)nameToSave.data, bitmap);
			ConsoleOutput("Converted bitmap %s to texture %s", nameToLoad, nameToSave);
			continue;
		}
		else if (type == "obj")
		{
			if (AssetExits(handler, *it, Asset_Mesh)) continue;
			String nameToSave = FormatCString("mesh/%s.mesh", GetToChar(*it, '.'));
			TriangleMesh mesh = ReadObj(NULL, (char *)nameToLoad.data, currentStateArena, workingArena);
			if (!mesh.vertices.amount)
			{
				ConsoleOutputError("Probably file name wrong. Might have loaded a mesh w/0 vertices.");
				continue;
			}
			WriteTriangleMesh(mesh, (char *)nameToSave.data);
			ConsoleOutput("Converted .obj %s to .mesh %s", nameToLoad, nameToSave);
			continue;
		}

	}
}

#undef Asset_Type_Offset

#endif
