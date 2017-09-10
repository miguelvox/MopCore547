/*
 * Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "MMapManager.h"
#include "Errors.h"
#include "Log.h"
#include "Config.h"
#include "MMapFactory.h"
#include "Configuration/Config.h"

namespace MMAP
{
    static char const* const MAP_FILE_NAME_FORMAT = "%s/mmaps/%04i.mmap";
    static char const* const TILE_FILE_NAME_FORMAT = "%s/mmaps/%04i%02i%02i.mmtile";

    // ######################## MMapManager ########################
    MMapManager::~MMapManager()
    {
        for (MMapDataSet::iterator i = loadedMMaps.begin(); i != loadedMMaps.end(); ++i)
            delete i->second;

        // by now we should not have maps loaded
        // if we had, tiles in MMapData->mmapLoadedTiles, their actual data is lost!
    }

    void MMapManager::InitializeThreadUnsafe(std::unordered_map<uint32, std::vector<uint32>> const& mapData)
    {
        // the caller must pass the list of all mapIds that will be used in the MMapManager lifetime
        for (auto const& p : mapData)
        {
            loadedMMaps.insert(MMapDataSet::value_type(p.first, nullptr));
            if (!p.second.empty())
            {
                phaseMapData[p.first] = p.second;
                for (uint32 phasedMapId : p.second)
                    _phaseTiles.insert(PhaseTileMap::value_type(phasedMapId, PhaseTileContainer()));
            }
        }

        thread_safe_environment = false;
    }

    MMapDataSet::const_iterator MMapManager::GetMMapData(uint32 mapId) const
    {
        // return the iterator if found or end() if not found/NULL
        MMapDataSet::const_iterator itr = loadedMMaps.find(mapId);
        if (itr != loadedMMaps.cend() && !itr->second)
            itr = loadedMMaps.cend();

        return itr;
    }

    bool MMapManager::loadMapData(uint32 mapId)
    {
        // we already have this map loaded?
        MMapDataSet::iterator itr = loadedMMaps.find(mapId);
        if (itr != loadedMMaps.end())
        {
            if (itr->second)
                return true;
        }
        else
        {
            if (thread_safe_environment)
                itr = loadedMMaps.insert(MMapDataSet::value_type(mapId, nullptr)).first;
            else
            {
                std::ostringstream temp;
                temp << "Invalid mapId " << mapId << " passed to MMapManager after startup in thread unsafe environment";
                ASSERT(false && temp.str().c_str());
            }
        }

        // load and init dtNavMesh - read parameters from file
        char temp[300];
        sprintf(temp, MAP_FILE_NAME_FORMAT, ConfigMgr::GetStringDefault("DataDir", ".").c_str(), mapId);
        std::string fileName = temp;
        FILE* file = fopen(fileName.c_str(), "rb");
        if (!file)
        {
            sLog->outDebug(LOG_FILTER_MAPS, "MMAP:loadMapData: Error: Could not open mmap file '%s'", fileName.c_str());
            return false;
        }

        dtNavMeshParams params;
        uint32 count = uint32(fread(&params, sizeof(dtNavMeshParams), 1, file));
        fclose(file);
        if (count != 1)
        {
            sLog->outDebug(LOG_FILTER_MAPS, "MMAP:loadMapData: Error: Could not read params from file '%s'", fileName.c_str());
            return false;
        }

        dtNavMesh* mesh = dtAllocNavMesh();
        ASSERT(mesh);
        if (dtStatusFailed(mesh->init(&params)))
        {
            dtFreeNavMesh(mesh);
            sLog->outDebug(LOG_FILTER_MAPS, "MMAP:loadMapData: Failed to initialize dtNavMesh for mmap %04u from file %s", mapId, fileName.c_str());
            return false;
        }

        sLog->outDebug(LOG_FILTER_MAPS, "MMAP:loadMapData: Loaded %04i.mmap", mapId);

        // store inside our map list
        MMapData* mmap_data = new MMapData(mesh, mapId);

        itr->second = mmap_data;
        return true;
    }

    uint32 MMapManager::packTileID(int32 x, int32 y)
    {
        return uint32(x << 16 | y);
    }

    bool MMapManager::loadMap(const std::string& /*basePath*/, uint32 mapId, int32 x, int32 y)
    {
        // make sure the mmap is loaded and ready to load tiles
        if (!loadMapData(mapId))
            return false;

        // get this mmap data
        MMapData* mmap = loadedMMaps[mapId];
        ASSERT(mmap->navMesh);

        // check if we already have this tile loaded
        uint32 packedGridPos = packTileID(x, y);
        if (mmap->loadedTileRefs.find(packedGridPos) != mmap->loadedTileRefs.end())
            return false;

        // load this tile :: mmaps/MMMMXXYY.mmtile
        char temp[300];
        sprintf(temp, TILE_FILE_NAME_FORMAT, ConfigMgr::GetStringDefault("DataDir", ".").c_str(), mapId, x, y);
        std::string fileName = temp;
        FILE* file = fopen(fileName.c_str(), "rb");
        if (!file)
        {
            sLog->outDebug(LOG_FILTER_MAPS, "MMAP:loadMap: Could not open mmtile file '%s'", fileName.c_str());
            return false;
        }

        // read header
        MmapTileHeader fileHeader;
        if (fread(&fileHeader, sizeof(MmapTileHeader), 1, file) != 1 || fileHeader.mmapMagic != MMAP_MAGIC)
        {
            sLog->outError(LOG_FILTER_MAPS, "MMAP:loadMap: Bad header in mmap %04u%02i%02i.mmtile", mapId, x, y);
            fclose(file);
            return false;
        }

        if (fileHeader.mmapVersion != MMAP_VERSION)
        {
            sLog->outError(LOG_FILTER_MAPS, "MMAP:loadMap: %04u%02i%02i.mmtile was built with generator v%i, expected v%i",
                mapId, x, y, fileHeader.mmapVersion, MMAP_VERSION);
            fclose(file);
            return false;
        }

        long pos = ftell(file);
        fseek(file, 0, SEEK_END);
        if (int64(fileHeader.size) > ftell(file) - pos)
        {
            sLog->outError(LOG_FILTER_MAPS, "MMAP:loadMap: %04u%02i%02i.mmtile has corrupted data size", mapId, x, y);
            fclose(file);
            return false;
        }

        fseek(file, pos, SEEK_SET);

        unsigned char* data = (unsigned char*)dtAlloc(fileHeader.size, DT_ALLOC_PERM);
        ASSERT(data);

        size_t result = fread(data, fileHeader.size, 1, file);
        if (!result)
        {
            sLog->outError(LOG_FILTER_MAPS, "MMAP:loadMap: Bad header or data in mmap %04u%02i%02i.mmtile", mapId, x, y);
            fclose(file);
            return false;
        }

        fclose(file);

        dtMeshHeader* header = (dtMeshHeader*)data;
        dtTileRef tileRef = 0;

        // memory allocated for data is now managed by detour, and will be deallocated when the tile is removed
        if (dtStatusSucceed(mmap->navMesh->addTile(data, fileHeader.size, 0, 0, &tileRef)))
        {
            mmap->loadedTileRefs.insert(std::pair<uint32, dtTileRef>(packedGridPos, tileRef));
            ++loadedTiles;
            sLog->outDebug(LOG_FILTER_MAPS, "MMAP:loadMap: Loaded mmtile %04i[%02i, %02i] into %04i[%02i, %02i]", mapId, x, y, mapId, header->x, header->y);

            PhaseChildMapContainer::const_iterator phasedMaps = phaseMapData.find(mapId);
            if (phasedMaps != phaseMapData.end())
            {
                mmap->AddBaseTile(packedGridPos, data, fileHeader, fileHeader.size);
                LoadPhaseTiles(phasedMaps, x, y);
            }

            return true;
        }

        sLog->outError(LOG_FILTER_MAPS, "MMAP:loadMap: Could not load %04u%02i%02i.mmtile into navmesh", mapId, x, y);
        dtFree(data);
        return false;
    }

    PhasedTile* MMapManager::LoadTile(uint32 mapId, int32 x, int32 y)
    {
        // load this tile :: mmaps/MMMXXYY.mmtile
        char temp[300];
        sprintf(temp, TILE_FILE_NAME_FORMAT, ConfigMgr::GetStringDefault("DataDir", ".").c_str(), mapId, x, y);
        std::string fileName = temp;
        FILE* file = fopen(fileName.c_str(), "rb");
        if (!file)
        {
            // Not all tiles have phased versions, don't flood this msg
            //sLog->outDebug(LOG_FILTER_PHASE, "MMAP:LoadTile: Could not open mmtile file '%s'", fileName);
            return NULL;
        }

        PhasedTile* pTile = new PhasedTile();

        // read header
        if (fread(&pTile->fileHeader, sizeof(MmapTileHeader), 1, file) != 1 || pTile->fileHeader.mmapMagic != MMAP_MAGIC)
        {
            sLog->outDebug(LOG_FILTER_PHASE, "MMAP:LoadTile: Bad header in mmap %04u%02i%02i.mmtile", mapId, x, y);
            fclose(file);
            delete pTile;
            return nullptr;
        }

        if (pTile->fileHeader.mmapVersion != MMAP_VERSION)
        {
            sLog->outDebug(LOG_FILTER_PHASE, "MMAP:LoadTile: %04u%02i%02i.mmtile was built with generator v%i, expected v%i",
                mapId, x, y, pTile->fileHeader.mmapVersion, MMAP_VERSION);
            fclose(file);
            delete pTile;
            return nullptr;
        }

        pTile->data = (unsigned char*)dtAlloc(pTile->fileHeader.size, DT_ALLOC_PERM);
        ASSERT(pTile->data);

        size_t result = fread(pTile->data, pTile->fileHeader.size, 1, file);
        if (!result)
        {
            sLog->outDebug(LOG_FILTER_PHASE, "MMAP:LoadTile: Bad header or data in mmap %04u%02i%02i.mmtile", mapId, x, y);
            fclose(file);
            delete pTile;
            return nullptr;
        }

        fclose(file);

        return pTile;
    }

    void MMapManager::LoadPhaseTiles(PhaseChildMapContainer::const_iterator phasedMapData, int32 x, int32 y)
    {
        sLog->outDebug(LOG_FILTER_PHASE, "MMAP:LoadPhaseTiles: Loading phased mmtiles for map %u, x: %i, y: %i", phasedMapData->first, x, y);

        uint32 packedGridPos = packTileID(x, y);

        for (uint32 phaseMapId : phasedMapData->second)
        {
            // only a few tiles have terrain swaps, do not write error for them
            if (PhasedTile* data = LoadTile(phaseMapId, x, y))
            {
                sLog->outDebug(LOG_FILTER_PHASE, "MMAP:LoadPhaseTiles: Loaded phased %04u%02i%02i.mmtile for root phase map %u", phaseMapId, x, y, phasedMapData->first);
                _phaseTiles[phaseMapId][packedGridPos] = data;
            }
        }
    }

    void MMapManager::UnloadPhaseTile(PhaseChildMapContainer::const_iterator phasedMapData, int32 x, int32 y)
    {
        sLog->outDebug(LOG_FILTER_PHASE, "MMAP:UnloadPhaseTile: Unloading phased mmtile for map %u, x: %i, y: %i", phasedMapData->first, x, y);

        uint32 packedGridPos = packTileID(x, y);

        for (uint32 phaseMapId : phasedMapData->second)
        {
            auto phasedTileItr = _phaseTiles.find(phaseMapId);
            if (phasedTileItr == _phaseTiles.end())
                continue;

            auto dataItr = phasedTileItr->second.find(packedGridPos);
            if (dataItr != phasedTileItr->second.end())
            {
                sLog->outDebug(LOG_FILTER_PHASE, "MMAP:UnloadPhaseTile: Unloaded phased %04u%02i%02i.mmtile for root phase map %u", phaseMapId, x, y, phasedMapData->first);
                dtFree(dataItr->second->data);
                delete dataItr->second;
                phasedTileItr->second.erase(dataItr);
            }
        }
    }

    bool MMapManager::unloadMap(uint32 mapId, int32 x, int32 y)
    {
        // check if we have this map loaded
        MMapDataSet::const_iterator itr = GetMMapData(mapId);
        if (itr == loadedMMaps.end())
        {
            // file may not exist, therefore not loaded
            sLog->outDebug(LOG_FILTER_MAPS, "MMAP:unloadMap: Asked to unload not loaded navmesh map. %04u%02i%02i.mmtile", mapId, x, y);
            return false;
        }

        MMapData* mmap = itr->second;

        // check if we have this tile loaded
        uint32 packedGridPos = packTileID(x, y);
        if (mmap->loadedTileRefs.find(packedGridPos) == mmap->loadedTileRefs.end())
        {
            // file may not exist, therefore not loaded
            sLog->outDebug(LOG_FILTER_MAPS, "MMAP:unloadMap: Asked to unload not loaded navmesh tile. %04u%02i%02i.mmtile", mapId, x, y);
            return false;
        }

        dtTileRef tileRef = mmap->loadedTileRefs[packedGridPos];

        // unload, and mark as non loaded
        unsigned char* data = NULL;
        if (dtStatusFailed(mmap->navMesh->removeTile(tileRef, &data, NULL)))
        {
            // this is technically a memory leak
            // if the grid is later reloaded, dtNavMesh::addTile will return error but no extra memory is used
            // we cannot recover from this error - assert out
            char temp[350];
            sprintf(temp, "MMAP:unloadMap: Could not unload %04u%02i%02i.mmtile from navmesh", mapId, x, y);
            sLog->outError(LOG_FILTER_MAPS, "%s", temp);
            ASSERT(false && temp);
        }
        else
        {
            mmap->loadedTileRefs.erase(packedGridPos);
            --loadedTiles;
            sLog->outDebug(LOG_FILTER_MAPS, "MMAP:unloadMap: Unloaded mmtile %04i[%02i, %02i] from %04i", mapId, x, y, mapId);

            PhaseChildMapContainer::const_iterator phasedMaps = phaseMapData.find(mapId);
            if (phasedMaps != phaseMapData.end())
            {
                mmap->DeleteBaseTile(packedGridPos);
                UnloadPhaseTile(phasedMaps, x, y);
            }
            else
                dtFree(data);
            return true;
        }

        return false;
    }

    bool MMapManager::unloadMap(uint32 mapId)
    {
        MMapDataSet::iterator itr = loadedMMaps.find(mapId);
        if (itr == loadedMMaps.end() || !itr->second)
        {
            // file may not exist, therefore not loaded
            sLog->outDebug(LOG_FILTER_MAPS, "MMAP:unloadMap: Asked to unload not loaded navmesh map %04u", mapId);
            return false;
        }

        // unload all tiles from given map
        MMapData* mmap = itr->second;
        for (MMapTileSet::iterator i = mmap->loadedTileRefs.begin(); i != mmap->loadedTileRefs.end(); ++i)
        {
            uint32 x = (i->first >> 16);
            uint32 y = (i->first & 0x0000FFFF);
            unsigned char* data = NULL;
            if (dtStatusFailed(mmap->navMesh->removeTile(i->second, &data, NULL)))
                sLog->outError(LOG_FILTER_MAPS, "MMAP:unloadMap: Could not unload %04u%02i%02i.mmtile from navmesh", mapId, x, y);
            else
            {
                PhaseChildMapContainer::const_iterator phasedMaps = phaseMapData.find(mapId);
                if (phasedMaps != phaseMapData.end())
                {
                    mmap->DeleteBaseTile(i->first);
                    UnloadPhaseTile(phasedMaps, x, y);
                }
                else
                    dtFree(data);
                --loadedTiles;
                sLog->outDebug(LOG_FILTER_MAPS, "MMAP:unloadMap: Unloaded mmtile %04i[%02i, %02i] from %04i", mapId, x, y, mapId);
            }
        }

        delete mmap;
        itr->second = nullptr;
        sLog->outDebug(LOG_FILTER_MAPS, "MMAP:unloadMap: Unloaded %04i.mmap", mapId);

        return true;
    }

    bool MMapManager::unloadMapInstance(uint32 mapId, uint32 instanceId)
    {
        // check if we have this map loaded
        MMapDataSet::const_iterator itr = GetMMapData(mapId);
        if (itr == loadedMMaps.end())
        {
            // file may not exist, therefore not loaded
            sLog->outDebug(LOG_FILTER_MAPS, "MMAP:unloadMapInstance: Asked to unload not loaded navmesh map %04u", mapId);
            return false;
        }

        MMapData* mmap = itr->second;
        if (mmap->navMeshQueries.find(instanceId) == mmap->navMeshQueries.end())
        {
            sLog->outDebug(LOG_FILTER_MAPS, "MMAP:unloadMapInstance: Asked to unload not loaded dtNavMeshQuery mapId %04u instanceId %u", mapId, instanceId);
            return false;
        }

        dtNavMeshQuery* query = mmap->navMeshQueries[instanceId];

        dtFreeNavMeshQuery(query);
        mmap->navMeshQueries.erase(instanceId);
        sLog->outDebug(LOG_FILTER_MAPS, "MMAP:unloadMapInstance: Unloaded mapId %04u instanceId %u", mapId, instanceId);

        return true;
    }

    dtNavMesh const* MMapManager::GetNavMesh(uint32 mapId)
    {
        MMapDataSet::const_iterator itr = GetMMapData(mapId);
        if (itr == loadedMMaps.end())
            return NULL;

        return NULL;
    }

    dtNavMeshQuery const* MMapManager::GetNavMeshQuery(uint32 mapId, uint32 instanceId)
    {
        MMapDataSet::const_iterator itr = GetMMapData(mapId);
        if (itr == loadedMMaps.end())
            return NULL;

        MMapData* mmap = itr->second;
        if (mmap->navMeshQueries.find(instanceId) == mmap->navMeshQueries.end())
        {
            // allocate mesh query
            dtNavMeshQuery* query = dtAllocNavMeshQuery();
            if (!query)
                return NULL;

            sLog->outDebug(LOG_FILTER_MAPS, "MMAP:GetNavMeshQuery: created dtNavMeshQuery for mapId %04u instanceId %u", mapId, instanceId);
            mmap->navMeshQueries.insert(std::pair<uint32, dtNavMeshQuery*>(instanceId, query));
        }

        return mmap->navMeshQueries[instanceId];
    }

    MMapData::MMapData(dtNavMesh* mesh, uint32 mapId)
    {
        navMesh = mesh;
        _mapId = mapId;
    }

    MMapData::~MMapData()
    {
        for (NavMeshQuerySet::iterator i = navMeshQueries.begin(); i != navMeshQueries.end(); ++i)
            dtFreeNavMeshQuery(i->second);

        dtFreeNavMesh(navMesh);
    }

    void MMapData::RemoveSwap(PhasedTile* ptile, uint32 swap, uint32 packedXY)
    {
        uint32 x = (packedXY >> 16);
        uint32 y = (packedXY & 0x0000FFFF);

        dtMeshHeader* header = (dtMeshHeader*)ptile->data;

        // remove old tile
        if (dtStatusFailed(navMesh->removeTile(loadedTileRefs[packedXY], NULL, NULL)))
            sLog->outDebug(LOG_FILTER_PHASE, "MMapData::RemoveSwap: Could not unload phased %04u%02i%02i.mmtile from navmesh", swap, x, y);
        else
        {
            sLog->outDebug(LOG_FILTER_PHASE, "MMapData::RemoveSwap: Unloaded phased %04u%02i%02i.mmtile from navmesh", swap, x, y);

            // restore base tile
            if (dtStatusSucceed(navMesh->addTile(_baseTiles[packedXY]->data, _baseTiles[packedXY]->dataSize, 0, 0, &loadedTileRefs[packedXY])))
                sLog->outDebug(LOG_FILTER_PHASE, "MMapData::RemoveSwap: Loaded base mmtile %04u[%02i, %02i] into %04i[%02i, %02i]", _mapId, x, y, _mapId, header->x, header->y);
            else
                sLog->outDebug(LOG_FILTER_PHASE, "MMapData::RemoveSwap: Could not load base %04u%02i%02i.mmtile to navmesh", _mapId, x, y);
        }
    }

    void MMapData::AddSwap(PhasedTile* ptile, uint32 swap, uint32 packedXY)
    {
        uint32 x = (packedXY >> 16);
        uint32 y = (packedXY & 0x0000FFFF);

        if (loadedTileRefs.find(packedXY) == loadedTileRefs.end())
        {
            sLog->outDebug(LOG_FILTER_PHASE, "MMapData::AddSwap: phased mmtile %04u[%02i, %02i] load skipped, due to not loaded base tile on map %u", swap, x, y, _mapId);
            return;
        }

        dtMeshHeader* header = (dtMeshHeader*)ptile->data;

        const dtMeshTile* oldTile = navMesh->getTileByRef(loadedTileRefs[packedXY]);

        if (!oldTile)
        {
            sLog->outDebug(LOG_FILTER_PHASE, "MMapData::AddSwap: phased mmtile %04u[%02i, %02i] load skipped, due to not loaded base tile ref on map %u", swap, x, y, _mapId);
            return;
        }

        // header xy is based on the swap map's tile set, wich doesn't have all the same tiles as root map, so copy the xy from the orignal header
        header->x = oldTile->header->x;
        header->y = oldTile->header->y;

        // remove old tile
        if (dtStatusFailed(navMesh->removeTile(loadedTileRefs[packedXY], NULL, NULL)))
            sLog->outDebug(LOG_FILTER_PHASE, "MMapData::AddSwap: Could not unload %04u%02i%02i.mmtile from navmesh", _mapId, x, y);
        else
        {
            sLog->outDebug(LOG_FILTER_PHASE, "MMapData::AddSwap: Unloaded %04u%02i%02i.mmtile from navmesh", _mapId, x, y);

            _activeSwaps.insert(swap);

            // add new swapped tile
            if (dtStatusSucceed(navMesh->addTile(ptile->data, ptile->fileHeader.size, 0, 0, &loadedTileRefs[packedXY])))
                sLog->outDebug(LOG_FILTER_PHASE, "MMapData::AddSwap: Loaded phased mmtile %04u[%02i, %02i] into %04i[%02i, %02i]", swap, x, y, _mapId, header->x, header->y);
            else
                sLog->outDebug(LOG_FILTER_PHASE, "MMapData::AddSwap: Could not load %04u%02i%02i.mmtile to navmesh", swap, x, y);
        }
    }

    void MMapData::AddBaseTile(uint32 packedGridPos, unsigned char* data, MmapTileHeader const& fileHeader, int32 dataSize)
    {
        auto itr = _baseTiles.find(packedGridPos);
        if (itr == _baseTiles.end())
        {
            PhasedTile* pt = new PhasedTile();
            pt->data = data;
            pt->fileHeader = fileHeader;
            pt->dataSize = dataSize;
            _baseTiles[packedGridPos] = pt;
        }
    }

    void MMapData::DeleteBaseTile(uint32 packedGridPos)
    {
        auto itr = _baseTiles.find(packedGridPos);
        if (itr != _baseTiles.end())
        {
            dtFree(itr->second->data);
            delete itr->second;
            _baseTiles.erase(itr);
        }
    }
}
