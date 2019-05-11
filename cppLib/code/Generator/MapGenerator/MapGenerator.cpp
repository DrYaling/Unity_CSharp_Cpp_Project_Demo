#include "MapGenerator.h"
#include "Logger/leakHelper.h"
#include "Logger/Logger.h"
#include "G3D/Vector4.h"
#include "Threading/ThreadManager.h"
using namespace logger;
namespace generator
{
#define break_if_stopped if(!m_bRun) break
#define return_if_stopped if(!m_bRun) return
	static std::shared_ptr<MapGenerator>  instance = nullptr;
	MapGenerator::MapGenerator() :
		m_pWorldMap(nullptr),
		m_pGenerator(nullptr),
		m_pPainter(nullptr),
		m_worldMapHeightMap(nullptr),
		m_aHeightMap(nullptr),
		m_aSplatMap(nullptr),
		m_aHeightMapCopy(nullptr),
		m_aSplatMapCopy(nullptr),
		m_cbTerrainGenFinish(nullptr),
		m_nCurrentTerrain(0),
		m_stData(),
		m_bRun(false),
		m_bThreadExited(true),
		m_nTotalMapCount(0)
	{
		m_mTerrainData.clear();
		m_pGenerator = new Diamond_Square();
		m_pGenerator->SetGetNeighborHeightCallBack(MapGenerator::GetHeightOnWorldMap);
		m_pPainter = new AutomaticPainter();
	}

	MapGenerator::~MapGenerator()
	{
		Stop();
		safe_delete(m_pGenerator);
		safe_delete(m_pPainter);
		safe_delete_array(m_aHeightMap);
		safe_delete_array(m_aHeightMapCopy);
		safe_delete_array(m_aSplatMap);
		safe_delete_array(m_aSplatMapCopy);
		m_pWorldMap = nullptr;
		LogFormat("map generator exit");
	}

	void MapGenerator::Init(MapGeneratorData data)
	{
		m_pGenerator->Initilize(0, data.seed, data.I, 0, nullptr);
		data.singleMapSize = m_pGenerator->GetSquareSize();
		m_stData = data;
		m_pGenerator->Initilize(0, data.seed, GetWorldMapSize(), 0, nullptr);
		uint32_t worldMapSize = m_pGenerator->GetSquareSize();
		m_nTotalMapCount = data.worldMapSize * data.worldMapSize;
		m_worldMapHeightMap = new float[worldMapSize*worldMapSize];
		m_aHeightMap = new float[data.singleMapSize*data.singleMapSize];
		m_aHeightMapCopy = new float[data.singleMapSize*data.singleMapSize];
		m_aSplatMap = new float[data.splatWidth*data.splatWidth*data.splatCount];
		m_aSplatMapCopy = new float[data.splatWidth*data.splatWidth*data.splatCount];
		m_pPainter->Init(m_aHeightMap, m_stData.singleMapSize, m_aSplatMap, m_stData.splatWidth, m_stData.splatCount);
		LogFormat("MapGeneratorData seed %d,i %d,h %d,worldSize %d,single size %d,map count per edge %d,splat size %d,splat count %d,flat %d", data.seed, data.I, data.H, worldMapSize, data.singleMapSize, data.worldMapSize, data.splatWidth, data.splatCount, data.flags);
	}

	std::shared_ptr<MapGenerator>  MapGenerator::GetInstance()
	{
		if (!instance)
		{
			instance = std::make_shared<MapGenerator>();
		}
		return instance;
	}

	void MapGenerator::Destroy()
	{
		instance = nullptr;
	}

	void MapGenerator::StartRun()
	{
		m_bRun = true;
		LogFormat("MapGenerator start run");
		threading::ThreadManager::GetInstance()->AddTask(threading::ThreadTask(std::bind(&MapGenerator::WorkThread, instance)));
	}

	void MapGenerator::Stop()
	{
		m_bRun = false;
		LogFormat("MapGenerator WorkThread exited");
	}

	void MapGenerator::UpdateInMainThread(int32_t diff)
	{
		uint32_t next;
		while (m_finishQueue.next(next))
		{
			if (m_cbTerrainGenFinish)
			{
				m_cbTerrainGenFinish(next, m_stData.singleMapSize, G3D::Vector4(m_pCurrentMap->m_Position.x, m_pCurrentMap->m_Position.y, m_pCurrentMap->m_Position.z, m_pCurrentMap->GetRealSize()));
			}
		}
	}
	void MapGenerator::InitTerrainInMainThread(uint32_t terrain, float* heightMap, int32_t heightMapSize, float* splatMap, int32_t splatWidth, int32_t splatCount)
	{
		if (!heightMap)
		{
			return;
		}
		if (!splatMap)
		{
			return;
		}
		if (heightMapSize != m_stData.singleMapSize)
		{
			return;
		}
		if (splatWidth != m_stData.splatWidth)
		{
			return;
		}
		if (splatCount != m_stData.splatCount)
		{
			return;
		}
		LogFormat("InitTerrainInMainThread %d,%d,%d", m_pCurrentMap->m_nInstanceId, heightMap, splatMap);
		std::lock_guard<std::mutex> lock(m_generatorMtx);
		auto itr = m_mTerrainData.find(terrain);
		if (itr != m_mTerrainData.end())
		{
			if (itr->second->m_nInstanceId != terrain)
			{
				LogErrorFormat("fail to init terrain %d", terrain);
				return;
			}
			itr->second->Init(heightMap, heightMapSize);
			memcpy(heightMap, m_aHeightMapCopy, m_stData.singleMapSize*m_stData.singleMapSize * sizeof(float));
			memcpy(splatMap, m_aSplatMapCopy, m_stData.splatWidth*m_stData.splatWidth * m_stData.splatCount * sizeof(float));
		}
		else
		{

			LogFormat("Init terrain fail ,terrain  %d not found", terrain);
		}
	}
	std::shared_ptr<Terrain> MapGenerator::GetTerrain(uint32_t terr)
	{
		std::lock_guard<std::mutex> lock(m_generatorMtx);
		auto itr = m_mTerrainData.find(terr);
		if (itr != m_mTerrainData.end())
		{
			return itr->second;
		}
		return nullptr;
	}
	void MapGenerator::SaveTerrain(uint32_t terr)
	{
	}
	void MapGenerator::WorkThreadEntry()
	{
		LogError("WorkThreadEntry is not avalible any more");
		/*if (!m_bRun && m_stData.flags & 0x3)
		{
			WorkThread();
		}*/
	}
	bool MapGenerator::HasInstance()
	{
		return instance != nullptr;
	}
	void MapGenerator::WorkThread()
	{
		LogFormat("WorkThread start ");
		GenWorldMap();
		m_bThreadExited = false;
		while (m_bRun)
		{
			//sleep(10);
			uint32_t current = InitilizeNext();
			LogFormat("current gen terrain %d ,m_nTotalMapCount %d", current, m_nTotalMapCount);
			if (current > 0)
			{
				Generate(current);
			}
			/*LogFormat("flag %d", (m_stData.flags & 0x3));
			if ((m_stData.flags & 0x3) == 2)
			{
				break_if_stopped;
				UpdateInMainThread(0);
			}*/
			while (!m_finishQueue.empty())
			{
				break_if_stopped;
				sleep(2);//hold on while finish queue is empty
			}
			break_if_stopped;
			if (current >= m_nTotalMapCount)
			{
				break;;
			}
		}
		m_bRun = false;
		m_bThreadExited = true;
		LogFormat("Gen WorkThread Exited");
	}
	uint32 MapGenerator::InitilizeNext()
	{
		std::lock_guard<std::mutex> lock(m_generatorMtx);
		return (uint32)m_mTerrainData.size() + 1;
	}
	void MapGenerator::Generate(uint32 terr)
	{
		m_nCurrentTerrain = terr;
		//LogFormat("Generate terrain %d start", terr);
		m_pCurrentMap = std::make_shared<Terrain>(terr, m_stData.I, m_stData.singleMapSize);
		m_pGenerator->Reset();
		uint32_t mapIndex = terr - 1;
		uint32_t x = mapIndex % m_stData.worldMapSize;
		uint32_t y = mapIndex / m_stData.worldMapSize;
		float fx = x * m_pCurrentMap->GetRealSize();
		float fy = y * m_pCurrentMap->GetRealSize();
		m_pCurrentMap->m_Position = G3D::Vector3(fx, 0, fy);
		LogFormat("map %d size x %f,size z %f, x %,y %d", terr, fx, fy, x, y);
		//initilize points based on world map
		if (m_pWorldMap->IsValidWorldMap())
		{
			float scale = m_pCurrentMap->GetHeightMapSize()*m_stData.worldMapSize / (float)m_pWorldMap->GetHeightMapSize();
			uint32_t offsetX, offsetY;
			uint32_t maxX, maxY;
			offsetX = x * m_stData.singleMapSize;
			offsetY = y * m_stData.singleMapSize;
			maxX = (x + 1) * m_stData.singleMapSize;
			maxY = (y + 1) * m_stData.singleMapSize;
			uint32_t mapX, mapY;
			//LogFormat("maxX %d,maxY %d,offset x %d,offset y %d,x %d,y %d,scale %f", maxX, maxY, offsetX, offsetY, x, y, scale);
			for (uint32_t wy = 0; wy < m_pWorldMap->GetHeightMapSize(); wy++)
			{
				for (uint32_t wx = 0; wx < m_pWorldMap->GetHeightMapSize(); wx++)
				{
					mapX = wx * scale - offsetX;
					mapY = wy * scale - offsetY;
					//LogFormat("mapX %d,mapY %d,height %f", mapX, mapY, m_pWorldMap->GetHeight(wx, wy));
					if (mapX >= 0 && mapX < maxX && mapY >= 0 && mapY < maxY)
					{
						m_pGenerator->SetPulse(mapX, mapY, m_pWorldMap->GetHeight(wx, wy));
					}
				}
			}
		}
		uint32_t neighbor[4] = { 0 };
		//initilize edge same to neighbor
		for (size_t i = 0; i < 4; i++)
		{
			return_if_stopped;
			auto dir = (NeighborType)i;
			neighbor[i] = GetNeighborID(dir, terr);
			std::shared_ptr<Terrain> s_ptr = nullptr;
			{
				std::lock_guard<std::mutex> lock(m_generatorMtx);
				auto itr = m_mTerrainData.find(neighbor[i]);
				if (itr != m_mTerrainData.end())
				{
					s_ptr = m_mTerrainData[neighbor[i]];
				}
			}
			if (s_ptr != nullptr)
			{
				//wait until neighbor was initilized or time out
				uint32 costtime = 0;
				while (!s_ptr->IsInitilized() && costtime < 15 * 1000)//time out at 45s later
				{
					return_if_stopped;
					sleep(10);
					costtime += 10;
				}
				return_if_stopped;
				if (costtime < 15 * 1000)
				{
					m_pCurrentMap->InitNeighbor(dir, s_ptr);
					InitHeightMapBaseOnNeighbor(dir, s_ptr);
				}
				else
				{
					LogErrorFormat("terrain %d wait for neighbor %d initialize time out!", terr, s_ptr->m_nInstanceId);
				}
			}
		}
		return_if_stopped;
		if (!LoadFromNative(terr))
		{
			return_if_stopped;
			GenerateTerrain();
			return_if_stopped;
			AutoGenSplatMap();
			return_if_stopped;
			std::lock_guard<std::mutex> lock(m_generatorMtx);
			m_mTerrainData.insert(std::make_pair(terr, m_pCurrentMap));
			memcpy(m_aHeightMapCopy, m_aHeightMap, m_stData.singleMapSize*m_stData.singleMapSize * sizeof(float));
			memcpy(m_aSplatMapCopy, m_aSplatMap, m_stData.splatWidth*m_stData.splatWidth * m_stData.splatCount * sizeof(float));
			m_finishQueue.add(terr);
			LogFormat("gen terrain %d finish ,current terrain count %d ", m_pCurrentMap->m_nInstanceId, m_mTerrainData.size());
		}
		else
		{
			//todo
		}
	}
	void MapGenerator::GenWorldMap()
	{
		setRandomSeed(m_stData.seed);
		uint32_t i = GetWorldMapSize();
		m_pWorldMap = std::make_shared<Terrain>(0xffffffff, i, 1);
		m_pGenerator->Initilize(m_pWorldMap->m_nInstanceId, std::rand(), m_pWorldMap->GetI(), m_stData.H, m_worldMapHeightMap);
		m_pWorldMap->Init(m_worldMapHeightMap, m_pGenerator->GetSquareSize());
		LogFormat("GenWorldMap size %d,i %d", m_pWorldMap->GetHeightMapSize(), m_pWorldMap->GetI());
		if (!LoadFromNative(0xffffffff) && m_pWorldMap->GetI() > 1)
		{
			float worldMapConor[] = { m_stData.height0,m_stData.height1,m_stData.height2,m_stData.height3 };
			m_pGenerator->Start(worldMapConor, 4, 0);
		}
	}
	void MapGenerator::AutoGenSplatMap()
	{
		return_if_stopped;
		m_pPainter->DrawSplatMap();
		LogFormat("Generate terrain %d splat map finished", m_pCurrentMap->m_nInstanceId);
	}

	void MapGenerator::GenerateTerrain()
	{
		//setRandomSeed(time(nullptr));
		m_pGenerator->Initilize(m_pCurrentMap->m_nInstanceId, std::rand(), m_stData.I, m_stData.H, m_aHeightMap);
		static float cornor[] = { m_stData.height0,m_stData.height1,m_stData.height2,m_stData.height3 };
		return_if_stopped;
		m_pGenerator->Start(cornor, 4, m_stData.flags & 0x4 ? m_pCurrentMap->GetRealSize() : 0);
		LogFormat("Generate terrain %d height map finished", m_pCurrentMap->m_nInstanceId);
	}
	uint32_t MapGenerator::GetNeighborID(NeighborType dir, uint32_t who)
	{
		if (who == 0)
		{
			return 0;
		}
		uint32_t worldMapSize = m_stData.worldMapSize;
		uint32_t index = who - 1;
		uint32_t x = index % m_stData.worldMapSize;
		uint32_t y = index / m_stData.worldMapSize;
		int32_t idx;
		switch (dir)
		{
			case NeighborType::neighborPositionLeft:
				idx = x - 1 + y * worldMapSize + 1;
				return idx > 0 ? idx : 0;
			case NeighborType::neighborPositionBottom:
				idx = x + (y - 1) * worldMapSize + 1;
				return idx > 0 ? idx : 0;
			case NeighborType::neighborPositionRight:
				idx = x + 1 + y * worldMapSize + 1;
				return idx <= m_nTotalMapCount ? idx : 0;
			case NeighborType::neighborPositionTop:
				idx = x + (y + 1) * worldMapSize + 1;
				return idx <= m_nTotalMapCount ? idx : 0;
			default:
				return 0;
		}
	}
	bool MapGenerator::LoadFromNative(uint32_t terr) const
	{
		//todo load raw map
		return false;
	}

	bool MapGenerator::GetHeightOnWorldMap(int32_t x, int32_t y, NeighborType neighbor, uint32_t owner, float & p)
	{
		return false;
		/*if (!instance || !instance->m_pCurrentMap || instance->m_pCurrentMap->m_nInstanceId != owner)
		{
			return false;
		}
		//owner is not added to m_mTerrainData yet,get it from m_pCurrentMap
		//LogWarningFormat("terrain %d generator get neighbor %d at x %d,y %d,map size %d", owner, neighbor, x, y,instance->m_pCurrentMap->m_nInstanceId);
		bool ret = instance->m_pCurrentMap->GetNeighborHeight(x, y, neighbor, p);
		return ret;*/
	}
	void MapGenerator::InitHeightMapBaseOnNeighbor(NeighborType position, std::shared_ptr<Terrain> neighbor)
	{
		int m_nSize = m_stData.singleMapSize;
		int nMax = m_nSize - 1;
		//LogFormat("InitHeightMapBaseOnNeighbor pos %d,for terrain %d", position, m_pCurrentMap->m_nInstanceId);
		if (position == NeighborType::neighborPositionLeft)
		{
			if (m_pCurrentMap->m_pLeftNeighbor == neighbor)
			{
				//add left edge
				int x = 0;
				int nx(x + nMax);
				for (int y = 0; y <= nMax; y++)
				{
					float nheight = m_pCurrentMap->m_pLeftNeighbor->GetHeight(nx, y);
					m_pGenerator->SetPulse(x, y, nheight);
				}
			}
		}

		if (position == NeighborType::neighborPositionRight)
		{
			if (m_pCurrentMap->m_pRightNeighbor == neighbor)
			{
				int x = nMax;
				int nx(x - nMax);
				for (int y = 0; y <= nMax; y++)
				{
					float nheight = m_pCurrentMap->m_pRightNeighbor->GetHeight(nx, y);
					m_pGenerator->SetPulse(x, y, nheight);
				}
			}
		}

		if (position == NeighborType::neighborPositionBottom)
		{
			if (m_pCurrentMap->m_pBottomNeighbor == neighbor)
			{
				int y = 0;
				int nY(y + nMax);
				for (int x = 0; x <= nMax; x++)
				{
					float nheight = m_pCurrentMap->m_pBottomNeighbor->GetHeight(x, nY);
					m_pGenerator->SetPulse(x, y, nheight);
				}
			}
		}

		if (position == NeighborType::neighborPositionTop)
		{
			if (m_pCurrentMap->m_pTopNeighbor == neighbor)
			{
				int y = nMax;
				int nY(y - nMax);
				for (int x = 0; x <= nMax; x++)
				{
					const float nheight = m_pCurrentMap->m_pTopNeighbor->GetHeight(x, nY);
					m_pGenerator->SetPulse(x, y, nheight);
				}
			}
		}
	}
}