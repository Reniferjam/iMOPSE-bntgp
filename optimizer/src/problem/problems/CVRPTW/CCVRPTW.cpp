#include "CCVRPTW.h"
#include "CCVRPTWSolution.h"
#include "utils/logger/CExperimentLogger.h"
#include <algorithm>
#include <limits>
#include <sstream>
#include <string>

CCVRPTW::CCVRPTW(CCVRPTWTemplate* base)
    : m_CVRPTWTemplate(base)
{
    CreateProblemEncoding();

    const float n = (float)m_CVRPTWTemplate->GetCitiesSize();
    const float fleet = (float)m_CVRPTWTemplate->GetVehicleCount();

    float totalService = 0.f;
    float latestReady = 0.f;
    float earliestDue = std::numeric_limits<float>::max();
    for (const auto& city : m_CVRPTWTemplate->GetCities())
    {
        totalService += city.m_ServiceTime;
        latestReady = std::max(latestReady, city.m_ReadyTime);
        earliestDue = std::min(earliestDue, city.m_DueTime);
    }

    const float maxFitness =
        std::max(0.f, n - fleet) * FLEET_PENALTY + 2.f * m_CVRPTWTemplate->GetMaxDepotEdge() * n;
    const float maxTimeFitness = maxFitness + totalService + latestReady;
    const float maxDelay = std::max(1.f, (maxTimeFitness - earliestDue) * n);

    m_MinObjectiveValues = { m_CVRPTWTemplate->GetMinEdge() * n, 0.f };
    m_MaxObjectiveValues = { maxFitness, maxDelay };
}

void CCVRPTW::CreateProblemEncoding()
{
    const std::size_t customers = m_CVRPTWTemplate->GetCustomersCount();

    SEncodingSection customersSection = SEncodingSection
    {
        std::vector<SEncodingDescriptor>(customers, SEncodingDescriptor{ 1.f, (float)customers }),
        EEncodingType::PERMUTATION
    };

    m_ProblemEncoding = SProblemEncoding{ OBJECTIVES_NUMBER, { customersSection }, m_CVRPTWTemplate->GetDistMtx() };
}

float CCVRPTW::ComputeFitness(float totalDistance, int vehiclesUsed) const
{
    const int extraRoutes = std::max(0, vehiclesUsed - m_CVRPTWTemplate->GetVehicleCount());
    return totalDistance + FLEET_PENALTY * (float)extraRoutes;
}

float CCVRPTW::ComputeDelay(float totalDelay, int vehiclesUsed) const
{
    const int extraRoutes = std::max(0, vehiclesUsed - m_CVRPTWTemplate->GetVehicleCount());
    return totalDelay + FLEET_PENALTY * (float)extraRoutes;
}

SCVRPTWObjectives CCVRPTW::GetObjectives(const CCVRPTWSolution& solution) const
{
    // Every customer must be served; otherwise the solution is infeasible and heavily penalized.
    const float infeasible = FLEET_PENALTY * (float)solution.GetUnservedCount();

    SCVRPTWObjectives objectives;
    objectives.distance = ComputeFitness(solution.GetTotalDistance(), solution.GetVehiclesUsed()) + infeasible;
    objectives.delay = ComputeDelay(solution.GetTotalDelay(), solution.GetVehiclesUsed()) + infeasible;
    return objectives;
}

void CCVRPTW::Normalize(float fitness, float delay, float& normFitness, float& normDelay) const
{
    const float fitRange = m_MaxObjectiveValues[0] - m_MinObjectiveValues[0];
    const float delRange = m_MaxObjectiveValues[1] - m_MinObjectiveValues[1];

    normFitness = fitRange > 0.f ? (fitness - m_MinObjectiveValues[0]) / fitRange : 0.f;
    normDelay = delRange > 0.f ? (delay - m_MinObjectiveValues[1]) / delRange : 0.f;
}

void CCVRPTW::Evaluate(AIndividual& individual)
{
    CCVRPTWSolution solution(m_CVRPTWTemplate);
    solution.Build(individual.m_Genotype.m_IntGenotype);

    const SCVRPTWObjectives objectives = GetObjectives(solution);
    individual.m_Evaluation[0] = objectives.distance;
    individual.m_Evaluation[1] = objectives.delay;

    Normalize(individual.m_Evaluation[0], individual.m_Evaluation[1],
              individual.m_NormalizedEvaluation[0], individual.m_NormalizedEvaluation[1]);
}

std::vector<int> CCVRPTW::GetRealPath(AIndividual& individual)
{
    CCVRPTWSolution solution(m_CVRPTWTemplate);
    solution.Build(individual.m_Genotype.m_IntGenotype);
    return solution.GetFlatPath();
}

void CCVRPTW::LogSolution(AIndividual& individual)
{
    const auto realPath = GetRealPath(individual);
    std::string line;
    for (std::size_t i = 0; i < realPath.size(); ++i)
    {
        line += std::to_string(realPath[i]);
        if (i + 1 != realPath.size()) line += ";";
    }
    CExperimentLogger::AddLine(line.c_str());
}

void CCVRPTW::LogAdditionalData()
{
    std::ostringstream pointsData;
    for (const auto& city : m_CVRPTWTemplate->GetCities())
    {
        const char type = (city.m_ID == (int)CVRPTW_DEPOT_IDX) ? 'd' : 'c';
        pointsData << city.m_PosX << ';' << city.m_PosY << ';' << type << std::endl;
    }
    CExperimentLogger::LogResult(pointsData.str().c_str(), "points.csv");
}