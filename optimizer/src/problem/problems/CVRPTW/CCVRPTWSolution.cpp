#include "CCVRPTWSolution.h"
#include "CCVRPTWTemplate.h"
#include <algorithm>

namespace
{
    constexpr float EPS = 1e-4f;
}

CCVRPTWSolution::CCVRPTWSolution(const CCVRPTWTemplate* problemTemplate)
    : m_Template(problemTemplate)
{
    Reset();
}

void CCVRPTWSolution::Reset()
{
    m_Routes.clear();
    m_RouteEndTimes.clear();
    m_Unserved.clear();
    m_RouteOpen = false;
    m_CurrentPosition = CVRPTW_DEPOT_IDX;
    m_CurrentLoad = 0;
    m_CurrentTime = 0.f;
    m_TotalDistance = 0.f;
    m_TotalDelay = 0.f;
}

void CCVRPTWSolution::Build(const std::vector<int>& permutation)
{
    Reset();
    const int nodeCount = (int)m_Template->GetCitiesSize();

    for (int value : permutation)
    {
        if (value == CVRPTW_VEHICLE_DELIMITER || value <= 0 || value >= nodeCount)
        {
            CloseRoute();
            continue;
        }

        const std::size_t customer = (std::size_t)value;

        if (!m_RouteOpen)
        {
            StartRoute();
        }
        if (!FitsCapacity(customer))
        {
            StartRoute(); // capacity exhausted: closes the current route, opens a fresh one
        }
        Serve(customer);  // always served; lateness only adds to the delay
    }

    CloseRoute();
}

void CCVRPTWSolution::BuildFromRoutes(const std::vector<std::vector<int>>& routes)
{
    Reset();
    const int nodeCount = (int)m_Template->GetCitiesSize();

    for (const auto& route : routes)
    {
        StartRoute();
        for (int value : route)
        {
            if (value <= 0 || value >= nodeCount) continue;   // depot / invalid ids are skipped
            Serve((std::size_t)value);
        }
        CloseRoute();                                         // an empty route is dropped here
    }
}

void CCVRPTWSolution::StartRoute()
{
    if (m_RouteOpen)
    {
        CloseRoute();
    }
    m_Routes.emplace_back();
    m_RouteOpen = true;
    m_CurrentPosition = CVRPTW_DEPOT_IDX;
    m_CurrentLoad = m_Template->GetCapacity();
    m_CurrentTime = m_Template->GetDepotReadyTime();
}

void CCVRPTWSolution::CloseRoute()
{
    if (!m_RouteOpen) return;

    m_RouteOpen = false;

    if (m_Routes.back().empty())
    {
        m_Routes.pop_back();
        m_CurrentPosition = CVRPTW_DEPOT_IDX;
        return;
    }

    // return leg: distance, time, and lateness against the depot due time
    m_TotalDistance += m_Template->GetDistance(m_CurrentPosition, CVRPTW_DEPOT_IDX);
    m_CurrentTime += m_Template->GetTravelTime(m_CurrentPosition, CVRPTW_DEPOT_IDX);
    m_TotalDelay += std::max(0.f, m_CurrentTime - m_Template->GetDepotDueTime());

    m_RouteEndTimes.push_back(m_CurrentTime);
    m_CurrentPosition = CVRPTW_DEPOT_IDX;
}

float CCVRPTWSolution::ArrivalTime(std::size_t customerIdx) const
{
    return m_CurrentTime + m_Template->GetTravelTime(m_CurrentPosition, customerIdx);
}

float CCVRPTWSolution::WaitingTime(std::size_t customerIdx) const
{
    return std::max(0.f, m_Template->GetCities()[customerIdx].m_ReadyTime - ArrivalTime(customerIdx));
}

float CCVRPTWSolution::Slack(std::size_t customerIdx) const
{
    return m_Template->GetCities()[customerIdx].m_DueTime - ArrivalTime(customerIdx);
}

float CCVRPTWSolution::Delay(std::size_t customerIdx) const
{
    return std::max(0.f, ArrivalTime(customerIdx) - m_Template->GetCities()[customerIdx].m_DueTime);
}

float CCVRPTWSolution::StartOfServiceTime(std::size_t customerIdx) const
{
    return std::max(ArrivalTime(customerIdx), m_Template->GetCities()[customerIdx].m_ReadyTime);
}

bool CCVRPTWSolution::FitsCapacity(std::size_t customerIdx) const
{
    return m_RouteOpen && m_Template->GetCities()[customerIdx].m_Demand <= m_CurrentLoad;
}

bool CCVRPTWSolution::CanArriveInTime(std::size_t customerIdx) const
{
    if (!m_RouteOpen) return false;

    const auto& city = m_Template->GetCities()[customerIdx];
    if (ArrivalTime(customerIdx) > city.m_DueTime + EPS) return false;

    // The vehicle must still be able to get back to the depot before it closes.
    const float backAtDepot = StartOfServiceTime(customerIdx) + city.m_ServiceTime
                              + m_Template->GetTravelTime(customerIdx, CVRPTW_DEPOT_IDX);
    return backAtDepot <= m_Template->GetDepotDueTime() + EPS;
}

bool CCVRPTWSolution::Serve(std::size_t customerIdx)
{
    if (!m_RouteOpen)
    {
        StartRoute();
    }

    if (!FitsCapacity(customerIdx))
    {
        MarkUnserved(customerIdx);   // only possible if demand > capacity: infeasible instance
        return false;
    }

    const auto& city = m_Template->GetCities()[customerIdx];

    m_TotalDistance += m_Template->GetDistance(m_CurrentPosition, customerIdx);
    m_TotalDelay += Delay(customerIdx);
    m_CurrentTime = StartOfServiceTime(customerIdx) + city.m_ServiceTime; // wait if early
    m_CurrentLoad -= city.m_Demand;
    m_CurrentPosition = customerIdx;
    m_Routes.back().push_back((int)customerIdx);
    return true;
}

void CCVRPTWSolution::MarkUnserved(std::size_t customerIdx)
{
    m_Unserved.push_back((int)customerIdx);
}

float CCVRPTWSolution::GetTotalDuration() const
{
    float sum = 0.f;
    for (float t : m_RouteEndTimes) sum += t;
    return sum;
}

float CCVRPTWSolution::GetMakespan() const
{
    float latest = 0.f;
    for (float t : m_RouteEndTimes) latest = std::max(latest, t);
    return latest;
}

std::vector<int> CCVRPTWSolution::GetFlatPath() const
{
    std::vector<int> path;
    path.push_back((int)CVRPTW_DEPOT_IDX);
    for (const auto& route : m_Routes)
    {
        path.insert(path.end(), route.begin(), route.end());
        path.push_back((int)CVRPTW_DEPOT_IDX);
    }
    return path;
}