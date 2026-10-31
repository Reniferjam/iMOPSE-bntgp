#include "CCVRPTWTemplate.h"
#include <algorithm>
#include <iostream>
#include <numeric>

SCityCVRPTW::SCityCVRPTW(int id, float x, float y, int demand,
                         float readyTime, float dueTime, float serviceTime)
    : m_ID(id), m_PosX(x), m_PosY(y), m_Demand(demand),
      m_ReadyTime(readyTime), m_DueTime(dueTime), m_ServiceTime(serviceTime)
{
}

void CCVRPTWTemplate::Clear()
{
    m_Cities.clear();
    m_CustomerIndexes.clear();
    m_DistanceMatrix.clear();
    m_NearestNeighbors.clear();
    m_Capacity = 0;
    m_VehicleCount = 0;
}

void CCVRPTWTemplate::SetData(const std::vector<SCityCVRPTW>& cities, int capacity, int vehicleCount)
{
    Clear();

    m_Cities = cities;
    m_Capacity = capacity;
    m_VehicleCount = vehicleCount;

    for (std::size_t i = 1; i < m_Cities.size(); ++i)
    {
        m_CustomerIndexes.push_back(i);
    }

    CalculateContextData();
}

void CCVRPTWTemplate::CalculateContextData()
{
    const std::size_t dim = m_Cities.size();
    m_DistanceMatrix = std::vector<std::vector<float>>(dim, std::vector<float>(dim, 0.f));

    for (std::size_t i = 0; i < dim; ++i)
    {
        for (std::size_t j = i + 1; j < dim; ++j)
        {
            const float dx = m_Cities[i].m_PosX - m_Cities[j].m_PosX;
            const float dy = m_Cities[i].m_PosY - m_Cities[j].m_PosY;
            m_DistanceMatrix[i][j] = m_DistanceMatrix[j][i] = std::sqrt(dx * dx + dy * dy);
        }
    }

    m_MinEdge = FLT_MAX;
    m_MaxDepotEdge = 0.f;
    for (std::size_t i = 0; i < dim; ++i)
    {
        for (std::size_t j = 0; j < dim; ++j)
        {
            if (i == j) continue;
            if (m_DistanceMatrix[i][j] > 0.f) m_MinEdge = std::min(m_MinEdge, m_DistanceMatrix[i][j]);
        }
        m_MaxDepotEdge = std::max(m_MaxDepotEdge, m_DistanceMatrix[CVRPTW_DEPOT_IDX][i]);
    }
    if (m_MinEdge == FLT_MAX) m_MinEdge = 0.f;

    m_NearestNeighbors.assign(dim, {});
    for (std::size_t i = 0; i < dim; ++i)
    {
        std::vector<std::pair<float, std::size_t>> byDistance;
        byDistance.reserve(dim);
        for (std::size_t j = 1; j < dim; ++j) // skip depot
        {
            if (i != j)
            {
                byDistance.emplace_back(m_DistanceMatrix[i][j], j);
            }
        }
        std::sort(byDistance.begin(), byDistance.end());

        m_NearestNeighbors[i].reserve(byDistance.size());
        for (const auto& entry : byDistance)
        {
            m_NearestNeighbors[i].push_back(entry.second);
        }
    }
}

float CCVRPTWTemplate::GetMaxDistanceBound() const
{
    float bound = 0.f;
    for (std::size_t c : m_CustomerIndexes)
    {
        bound += 2.f * m_DistanceMatrix[CVRPTW_DEPOT_IDX][c];
    }
    return bound;
}

int CCVRPTWTemplate::GetMinVehicles() const
{
    long totalDemand = 0;
    for (std::size_t c : m_CustomerIndexes)
    {
        totalDemand += m_Cities[c].m_Demand;
    }
    if (m_Capacity <= 0) return 1;
    return std::max(1, (int)((totalDemand + m_Capacity - 1) / m_Capacity));
}

bool CCVRPTWTemplate::Validate() const
{
    bool isValid = true;
    for (std::size_t c : m_CustomerIndexes)
    {
        if (m_Cities[c].m_Demand > m_Capacity)
        {
            std::cout << "Customer [" << c << "] demand exceeds vehicle capacity!" << std::endl;
            isValid = false;
        }
    }
    return isValid;
}