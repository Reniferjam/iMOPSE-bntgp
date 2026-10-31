#pragma once

#include <string>
#include <vector>
#include <cmath>
#include <cfloat>
#include <cstddef>
#include <cstdint>

constexpr int CVRPTW_VEHICLE_DELIMITER = INT32_MAX;
constexpr std::size_t CVRPTW_DEPOT_IDX = 0;

struct SCityCVRPTW
{
    SCityCVRPTW(int id, float x, float y, int demand,
                float readyTime, float dueTime, float serviceTime);

    int m_ID;
    float m_PosX, m_PosY;
    int m_Demand;
    float m_ReadyTime, m_DueTime, m_ServiceTime;
};

class CCVRPTWTemplate
{
public:
    void Clear();
    void SetData(const std::vector<SCityCVRPTW>& cities, int capacity, int vehicleCount);

    const std::string& GetFileName() const { return m_FileName; }
    void SetFileName(const std::string& fileName) { m_FileName = fileName; }

    const std::vector<SCityCVRPTW>& GetCities() const { return m_Cities; }
    const std::vector<std::size_t>& GetCustomers() const { return m_CustomerIndexes; }
    const std::vector<std::vector<float>>& GetDistMtx() const { return m_DistanceMatrix; }
    const std::vector<std::size_t>& GetNearestNeighbors(std::size_t cityIdx) const { return m_NearestNeighbors[cityIdx]; }

    float GetDistance(std::size_t i, std::size_t j) const { return m_DistanceMatrix[i][j]; }
    float GetTravelTime(std::size_t i, std::size_t j) const { return m_DistanceMatrix[i][j]; }

    int GetCapacity() const { return m_Capacity; }
    int GetVehicleCount() const { return m_VehicleCount; }
    std::size_t GetCitiesSize() const { return m_Cities.size(); }
    std::size_t GetCustomersCount() const { return m_CustomerIndexes.size(); }

    float GetDepotReadyTime() const { return m_Cities[CVRPTW_DEPOT_IDX].m_ReadyTime; }
    float GetDepotDueTime() const { return m_Cities[CVRPTW_DEPOT_IDX].m_DueTime; }

    float GetMaxDistanceBound() const;
    int GetMinVehicles() const;

    float GetMinEdge() const { return m_MinEdge; }
    float GetMaxDepotEdge() const { return m_MaxDepotEdge; }

    bool Validate() const;

private:
    void CalculateContextData();

    std::string m_FileName;

    std::vector<SCityCVRPTW> m_Cities;
    std::vector<std::size_t> m_CustomerIndexes;
    int m_Capacity = 0;
    int m_VehicleCount = 0;
    float m_MinEdge = 0.f;
    float m_MaxDepotEdge = 0.f;

    std::vector<std::vector<float>> m_DistanceMatrix;
    std::vector<std::vector<std::size_t>> m_NearestNeighbors; // customers only, sorted by distance
};