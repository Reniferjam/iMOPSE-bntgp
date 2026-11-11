#pragma once

#include "CCVRPTWTemplate.h"
#include "../../AProblem.h"
#include <vector>

class CCVRPTWSolution;

struct SCVRPTWObjectives
{
    float distance{ 0.f };   // objective 1: total distance + fleet penalty
    float delay{ 0.f };      // objective 2: total delay    + fleet penalty
};

class CCVRPTW : public AProblem
{
public:
    static constexpr int OBJECTIVES_NUMBER = 2;
    static constexpr float FLEET_PENALTY = 1000.f; // per route above the fleet size

    explicit CCVRPTW(CCVRPTWTemplate* base);
    ~CCVRPTW() { delete m_CVRPTWTemplate; }

    SProblemEncoding& GetProblemEncoding() override { return m_ProblemEncoding; }

    void Evaluate(AIndividual& individual) override;
    void LogSolution(AIndividual& individual) override;
    void LogAdditionalData() override;

    CCVRPTWTemplate* GetCVRPTWTemplate() { return m_CVRPTWTemplate; }
    std::vector<int> GetRealPath(AIndividual& individual);

    // objective 1: totalDistance + FLEET_PENALTY per route above the fleet size
    float ComputeFitness(float totalDistance, int vehiclesUsed) const;

    // objective 2: totalDelay + FLEET_PENALTY per route above the fleet size
    float ComputeDelay(float totalDelay, int vehiclesUsed) const;

    // Both objectives of a built solution (unserved customers are penalized too).
    [[nodiscard]] SCVRPTWObjectives GetObjectives(const CCVRPTWSolution& solution) const;

    void Normalize(float fitness, float delay, float& normFitness, float& normDelay) const;

    [[nodiscard]] float GetMinObjectiveValue(int objective) const { return m_MinObjectiveValues[objective]; }
    [[nodiscard]] float GetMaxObjectiveValue(int objective) const { return m_MaxObjectiveValues[objective]; }

protected:
    SProblemEncoding m_ProblemEncoding;
    CCVRPTWTemplate* m_CVRPTWTemplate;
    std::vector<float> m_MaxObjectiveValues;
    std::vector<float> m_MinObjectiveValues;

private:
    void CreateProblemEncoding();
};