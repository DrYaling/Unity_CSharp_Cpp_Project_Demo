#ifndef TERRIAN_MESH_H
#define TERRIAN_MESH_H
#include "Generators/generator.h"
#include "Generators/TerrianGenerator/Diamond_Square.h"
NS_GNRT_START

class TerrainMesh
{
public:
	TerrainMesh(int32_t ins);
	~TerrainMesh();
	void Init(int32_t* args, int32_t argsize,float* heightMap,int32_t heightMapSize, MeshInitilizerCallBack callback);
	void InitVerticesWithNeighbor(NeighborType position = NeighborType::neighborPositionAll);
	void InitNeighbor(NeighborType edge, TerrainMesh* mesh);
	void Start();
	void Release();
	void GetHeightMap(float* heightMap, int32_t size1, int32_t size2);
private:
	bool GetNeighborVertice(int32_t x, int32_t y, NeighborType neighbor, float & p);
	static bool StaticGetNeighborVertice(int32_t x, int32_t y, NeighborType neighbor, float & p,void* owner);
	void WorkThread();
private:
	TerrianDataBinding * m_pTerrianData;
	MeshInitilizerCallBack m_cbMeshInitilizer;
	TerrainMesh* m_pLeftNeighbor;
	TerrainMesh* m_pRightNeighbor;
	TerrainMesh* m_pBottomNeighbor;
	TerrainMesh* m_pTopNeighbor;
	Diamond_Square* m_pGenerator;
	std::vector<int32_t> m_vInitilizeArgs;
	/*std::vector<float>*/float* m_vHeightMap;
	int32_t m_nheightMapSize;
	int32_t m_nSize;
	bool m_bInitilized;
	bool m_bGenerated;
	int32_t m_nInstanceId;
};
NS_GNRT_END
#endif
