#pragma once

#include "CCVRPTWTemplate.h"
#include "../../AProblem.h"
#include <vector>

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

    float ComputeFitness(float totalDistance, int vehiclesUsed) const;

    void Normalize(float fitness, int unservedCount, float& normFitness, float& normUnserved) const;

protected:
    SProblemEncoding m_ProblemEncoding;
    CCVRPTWTemplate* m_CVRPTWTemplate;
    std::vector<float> m_MaxObjectiveValues;
    std::vector<float> m_MinObjectiveValues;

private:
    void CreateProblemEncoding();
};