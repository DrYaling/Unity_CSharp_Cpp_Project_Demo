#ifndef GENERATOR_H
#define GENERATOR_H
#include <thread>
#include <stdlib.h>
#include <vector>
#include <map>
#include <mutex>
#include <stdio.h>
#include "G3D/Vector3.h"
#include "define.h"
#define NS_GNRT_START namespace generator{\

#define NS_GNRT_END }\

#define  UNITY_CORE 1

static inline int _irandom(int min, int max)
{
	if (min >= max)
		return min;
	return std::rand() % (max - min) + min;
}
static inline float _frandom(int min, int max)
{
	if (min >= max)
		return (float)min;
	return (float)(std::rand() % (max - min) + min) + (std::rand() % 10000) / 10000.0f;
}
static inline float _frandom_f(float min, float max)
{
	if (min >= max)
	{
		return min;
	}
	int i(min * 10000), a(max * 10000);
	return (float)(_irandom(i, a) / 10000.0f);
}
static inline void setRandomSeed(int seed)
{
	std::srand(seed);
}
//����
enum eGeographyType :int16_t
{
	e_geo_trench = 0x0000,
	e_geo_deepSea = 0x0001,
	e_geo_shallowSea = 0x0002,
	e_geo_sea_land = 0x0004,
	e_geo_flatlands = 0x0008,
	e_geo_basin = 0x0010,
	e_geo_foothill = 0x0020,
	e_geo_montains = 0x0040,
	e_geo_high_montains = 0x0080,
	e_geo_heightLand = 0x0100,
};
//sub geography property
enum eGeographySubType :int16_t
{
	eg_sub_volcano = 1,//��ɽ
	eg_sub_marsh = 2,//����
	eg_sub_forest = 3,//ɭ��
	eg_sub_desert = 4,//ɳĮ
	eg_sub_grassland = 5,//��ԭ
	eg_sub_lake = 6,//����
};
//��������
typedef struct geographyProperties
{
	uint64_t gType : 2;//eGeographyType
	uint64_t gSubType : 2;//eGeographySubType
	uint64_t e : 8;

}*pGeographyProperties;
#if 0
[StructLayout(LayoutKind.Sequential)]
class TerrianData
{
	private bool _useUV;
	private int _meshCount;
	private int _lodCount;
	Vector3[][] _vertices;
	Vector3[][] _normals;
	Vector2[][][] _uvs;
	int[][][] _triangles;
}
#endif
class TerrianDataBinding
{
public:
	bool isReadable;
	bool useUv;
	int32_t meshCount;
	int32_t lodCount;
	int32_t currentLod;
	std::vector<int32_t> triangleSize;/*size == meshCount*/
	TerrianDataBinding(int32_t totalMeshCount,int32_t maxLodSize,bool _useUv):
		useUv(_useUv),
		meshCount(totalMeshCount),
		lodCount(maxLodSize),
		currentLod(0),
		triangleSize(totalMeshCount)
	{

	}

};
class TerrianGenerator
{
public:
	TerrianGenerator() {};
	virtual ~TerrianGenerator() {};

private:

};
#define MAX_MESH_COUNT 36
#define  MAX_MESH_VERTICES 65000
typedef int32_t*(__stdcall * MeshInitilizerCallBack)(int32_t type, int32_t mesh, int32_t lod, int32_t size);
typedef void(__stdcall * GeneratorNotifier)(int32_t type, int32_t arg0, int32_t arg1);
#define  meshTopologyVertice 0
#define  meshTopologyTriangle 1
#define meshTopologyUV 2
// typedef geography* pGeography;
#endif