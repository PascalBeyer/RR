#ifndef RR_ENTITY
#define RR_ENTITY

enum EntityType
{
	Entity_Invalid,
	Entity_Building,
	Entity_Unit,
};

struct Entity
{
	EntityType type;

	f32 radius; // ignored for now
	v2 pos;
	TriangleMeshArray meshes;
	Quaternion oriantation;
};


#endif
