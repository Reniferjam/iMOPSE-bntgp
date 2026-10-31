#pragma once

#include <vector>
#include <cstddef>

class CCVRPTWTemplate;

class CCVRPTWSolution
{
public:
    explicit CCVRPTWSolution(const CCVRPTWTemplate* problemTemplate);

    void Reset();

    void Build(const std::vector<int>& permutation);

    void StartRoute();
    void CloseRoute();
    [[nodiscard]] bool IsRouteOpen() const { return m_RouteOpen; }

    [[nodiscard]] bool FitsCapacity(std::size_t customerIdx) const;
    [[nodiscard]] bool CanArriveInTime(std::size_t customerIdx) const;
    [[nodiscard]] bool CanServe(std::size_t customerIdx) const { return FitsCapacity(customerIdx) && CanArriveInTime(customerIdx); }

    [[nodiscard]] float ArrivalTime(std::size_t customerIdx) const;
    [[nodiscard]] float WaitingTime(std::size_t customerIdx) const;
    [[nodiscard]] float Slack(std::size_t customerIdx) const;
    [[nodiscard]] float StartOfServiceTime(std::size_t customerIdx) const;

    bool Serve(std::size_t customerIdx);
    void MarkUnserved(std::size_t customerIdx);

    [[nodiscard]] std::size_t GetCurrentPosition() const { return m_CurrentPosition; }
    [[nodiscard]] int GetRemainingCapacity() const { return m_CurrentLoad; }
    [[nodiscard]] float GetCurrentTime() const { return m_CurrentTime; }

    // ---- results ----
    [[nodiscard]] float GetTotalDistance() const { return m_TotalDistance; }
    [[nodiscard]] int GetUnservedCount() const { return (int)m_Unserved.size(); }
    [[nodiscard]] const std::vector<int>& GetUnserved() const { return m_Unserved; }
    [[nodiscard]] float GetTotalDuration() const;   // sum of route end times
    [[nodiscard]] float GetMakespan() const;        // latest route end time
    [[nodiscard]] int GetVehiclesUsed() const { return (int)m_Routes.size(); }
    [[nodiscard]] const std::vector<std::vector<int>>& GetRoutes() const { return m_Routes; }
    [[nodiscard]] std::vector<int> GetFlatPath() const;  // 0,c,c,0,c,0,...

private:
    const CCVRPTWTemplate* m_Template;

    std::vector<std::vector<int>> m_Routes;
    std::vector<float> m_RouteEndTimes;
    std::vector<int> m_Unserved;

    bool m_RouteOpen = false;
    std::size_t m_CurrentPosition = 0;
    int m_CurrentLoad = 0;
    float m_CurrentTime = 0.f;

    float m_TotalDistance = 0.f;
};