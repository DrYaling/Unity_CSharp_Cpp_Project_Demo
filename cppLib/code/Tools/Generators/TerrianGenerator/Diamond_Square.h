#ifndef DIAMOND_SQUARE_H
#define DIAMOND_SQUARE_H
#include "../generator.h"
#include <vector>
NS_GNRT_START
/************************************************************************/
/*							����-���������ɵ���                         */
/*				������Ϊ x=0-x = max Ϊ0-max,y�����ϵ���				*/
/************************************************************************/
class Diamond_Square :public TerrianGenerator
{
public:
	Diamond_Square(int32_t seed, int32_t I, float H, std::vector<float>& heightMap);
	virtual ~Diamond_Square();
	void SetProcessHandler(std::function<void(int32_t)> handler) { m_cbProcessHandler = handler; }
	void Start(const float* corner, const int32_t size = 4, std::function<void(void)> cb = nullptr);
	//���ݴ�����������maxCoord�����ͼ
	void GenerateTerrian(float maxCoord);
	bool IsFinished() { return m_bIsFinished; }
	int32_t GetSquareSize() { return m_nSize; }
	void SetVerticesAndNormal(G3D::Vector3* pV, G3D::Vector3* pN, int mesh);
	int32_t GetVerticesSize(int mesh) {
		if (mesh <0 || mesh > GetMeshCount())
		{
			return 0;
		}
		return m_vVerticesSize[mesh];
	}
private:
	void WorkThread(std::function<void(void)> cb);
	inline void Diamond(int x, int y, int size, float h);
	inline void Square(int x, int y, int size, float h);
	inline float Randomize(float h);
	inline bool IsEdge(int x, int y) { return x == 0 || y == 0 || x == m_nI || y == m_nI; }
	//ƽ�����أ�ʹ֮���Ժ�������ͼƴ��
	void AddEdge(const G3D::Vector3* edge, int32_t size, int32_t edgeType/*x or y*/);
	inline void SetAtXY(int32_t x, int32_t y, float val) { m_vHeightMap[x + y * m_nSize] = val; }
	inline float GetAtXY(int x, int y) { return m_vHeightMap[x + y * m_nSize]; }
	float GetExtendHeight(int x, int y)
	{
		if (x < 0)
		{
			x = 0;
		}
		else if (x > m_nMax)
		{
			x = m_nMax;
		}
		if (y < 0)
		{
			y = 0;
		}
		else if (y > m_nMax)
		{
			y = m_nMax;
		}
		return GetAtXY(x, y);
	}
	inline size_t GetSize() { return m_vHeightMap.size(); }
	size_t GetMeshCount() {
		if (GetSize() % MAX_MESH_VERTICES == 0)
		{
			return GetSize() / MAX_MESH_VERTICES;
		}
		else
		{
			return GetSize() / MAX_MESH_VERTICES + 1;
		}
	}
	inline void	 SetExtendedPoint(int x, int y, float fx, float fy, float fz)/*x y from -1~m_nSize+1*/
	{
		m_vExtendPoints[x + 1 + (y + 1) * (m_nSize + 2)] = G3D::Vector3(fx, fy, fz);
	}
	inline const G3D::Vector3& GetExtendedPoint(int x, int y) const/*x y from -1~m_nSize+1*/
	{
		return m_vExtendPoints[x + 1 + (y + 1) * (m_nSize + 2)];
	}
	inline const G3D::Vector3& GetRealVertice(int x, int y)
	{
		return m_vVertices[x + y * m_nSize];
	}
	inline const G3D::Vector3& GetRealNormal(int x, int y)
	{
		return m_vNormals[x + y * m_nSize];
	}
private:
	std::function<void(int32_t)> m_cbProcessHandler;
	std::vector<float>& m_vHeightMap;
	std::vector<G3D::Vector3> m_vExtendPoints;/*x = -1,y = -1,x = m_nSize,y = m_nSize*/
	std::vector<G3D::Vector3> m_vVertices;
	std::vector<G3D::Vector3> m_vNormals;
	std::vector<int32_t> m_vVerticesSize;
	float m_aPointBuffer[5];
	int32_t m_nSize;//���� 2^(2*i)+1
	float m_nH;//�ֲڶ�
	int32_t	m_nI;//����
	int32_t m_nMax;
	bool m_bIsFinished;
	bool m_bEdgeExtended;
};

NS_GNRT_END
#endif