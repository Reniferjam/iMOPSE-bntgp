#include "BNTGPCVRPTWDecoder.hpp"

#include "../../gp/GPTreeEvaluator.hpp"

#include "problem/problems/CVRPTW/CCVRPTWSolution.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace bntgp_cvrptw::decoding::cvrptw
{
    BNTGPCVRPTWDecoder::BNTGPCVRPTWDecoder(
        ::CCVRPTW& problem,
        TreeParameters treeParameters)
        : problem_(problem),
          tpl_(*problem.GetCVRPTWTemplate()),
          treeParameters_(treeParameters)
    {
        const auto& cities = tpl_.GetCities();
        nodeCount_ = cities.size();
        maxVehicles_ = std::max(1, tpl_.GetVehicleCount());
        capacity_ = std::max(1, tpl_.GetCapacity());

        double maxEdge = 0.0;
        for (std::size_t i = 0; i < nodeCount_; ++i)
            for (std::size_t j = 0; j < nodeCount_; ++j)
                maxEdge = std::max(maxEdge, static_cast<double>(tpl_.GetDistance(i, j)));

        double maxTime = 0.0;
        double minX = std::numeric_limits<double>::infinity();
        double maxX = -std::numeric_limits<double>::infinity();
        double minY = std::numeric_limits<double>::infinity();
        double maxY = -std::numeric_limits<double>::infinity();
        for (const auto& city : cities)
        {
            maxTime = std::max(maxTime, static_cast<double>(city.m_DueTime));
            minX = std::min(minX, static_cast<double>(city.m_PosX));
            maxX = std::max(maxX, static_cast<double>(city.m_PosX));
            minY = std::min(minY, static_cast<double>(city.m_PosY));
            maxY = std::max(maxY, static_cast<double>(city.m_PosY));
        }

        distanceConst_ = maxEdge > 0.0 ? 1.0 / maxEdge : 1.0;
        timeConst_     = maxTime > 0.0 ? 1.0 / maxTime : 1.0;
        demandConst_   = 1.0 / static_cast<double>(capacity_);
        clientsConst_  = 1.0 / static_cast<double>(nodeCount_);
        vehiclesConst_ = 1.0 / static_cast<double>(maxVehicles_);

        const double cx = (minX + maxX) / 2.0;
        const double cy = (minY + maxY) / 2.0;
        const std::array<std::pair<double, double>, 5> points{ {
            { cx, cy },                                   // centre
            { (minX + cx) / 2.0, (minY + cy) / 2.0 },     // q1
            { (minX + cx) / 2.0, (maxY + cy) / 2.0 },     // q2
            { (maxX + cx) / 2.0, (maxY + cy) / 2.0 },     // q3
            { (maxX + cx) / 2.0, (minY + cy) / 2.0 }      // q4
        } };

        for (std::size_t p = 0; p < points.size(); ++p)
        {
            referenceDistances_[p].resize(nodeCount_);
            for (std::size_t i = 0; i < nodeCount_; ++i)
            {
                const double dx = static_cast<double>(cities[i].m_PosX) - points[p].first;
                const double dy = static_cast<double>(cities[i].m_PosY) - points[p].second;
                referenceDistances_[p][i] = std::sqrt(dx * dx + dy * dy);
            }
        }

        minDistance_ = problem_.GetMinObjectiveValue(0);
        maxDistance_ = problem_.GetMaxObjectiveValue(0);
        minDelay_    = problem_.GetMinObjectiveValue(1);
        maxDelay_    = problem_.GetMaxObjectiveValue(1);
    }

    std::vector<std::vector<int>> BNTGPCVRPTWDecoder::buildRoutes(
        const gp::GPTree& tree) const
    {
        using gp::FeatureId;

        const auto& cities = tpl_.GetCities();
        const std::size_t depot = static_cast<std::size_t>(CVRPTW_DEPOT_IDX);

        std::vector<std::vector<int>> routes;
        std::vector<int> route;
        route.reserve(nodeCount_);

        std::vector<char> completed(nodeCount_, 0);
        std::size_t completedCount = 0;

        int remainingCapacity = capacity_;
        int remainingCustomers = static_cast<int>(nodeCount_);
        int remainingVehicles = maxVehicles_;
        std::size_t current = depot;

        constexpr double stateTime = 0.0;
        constexpr double nextReady = 0.0;
        constexpr double nextDue = 0.0;

        gp::FeatureValues f;

        while (completedCount < nodeCount_)
        {
            int best = -1;
            double bestScore = std::numeric_limits<double>::infinity();

            for (std::size_t c = 0; c < nodeCount_; ++c)
            {
                if (completed[c] || c == current) continue;
                if (c == depot && route.empty()) continue;   // no empty routes

                const auto& city = cities[c];
                const double distance = static_cast<double>(tpl_.GetDistance(current, c));
                const double arrival = stateTime + distance;   // velocity = 1

                f[FeatureId::DEMAND]              = city.m_Demand * demandConst_;
                f[FeatureId::DISTANCE]            = distance * distanceConst_;
                f[FeatureId::REMAINING_CAPACITY]  = remainingCapacity * demandConst_;
                f[FeatureId::DISTANCE_FROM_DEPOT] = static_cast<double>(tpl_.GetDistance(depot, c)) * distanceConst_;
                f[FeatureId::CUSTOMERS_LEFT]      = remainingCustomers * clientsConst_;
                f[FeatureId::VEHICLES_LEFT]       = remainingVehicles * vehiclesConst_;

                f[FeatureId::REF_CENTER_DISTANCE] = referenceDistances_[0][c] * distanceConst_;
                f[FeatureId::REF_Q1_DISTANCE]     = referenceDistances_[1][c] * distanceConst_;
                f[FeatureId::REF_Q2_DISTANCE]     = referenceDistances_[2][c] * distanceConst_;
                f[FeatureId::REF_Q3_DISTANCE]     = referenceDistances_[3][c] * distanceConst_;
                f[FeatureId::REF_Q4_DISTANCE]     = referenceDistances_[4][c] * distanceConst_;

                f[FeatureId::CURRENT_TIME]        = stateTime * timeConst_;
                f[FeatureId::TRAVEL_TIME]         = distance * timeConst_;
                f[FeatureId::WAITING_TIME]        = nextReady - arrival > 0.0 ? (nextReady - arrival) * timeConst_ : 0.0;
                f[FeatureId::DELAY]               = arrival - nextDue > 0.0 ? (arrival - nextDue) * timeConst_ : 0.0;

                double score = gp::evaluateTree(tree, f);
                if (std::isnan(score))
                    score = std::numeric_limits<double>::infinity();

                if (best < 0 || score < bestScore)
                {
                    bestScore = score;
                    best = static_cast<int>(c);
                }
            }

            if (best < 0) break;   // nothing selectable

            const std::size_t chosen = static_cast<std::size_t>(best);

            if (remainingCapacity >= cities[chosen].m_Demand)
            {
                remainingCapacity -= cities[chosen].m_Demand;
                route.push_back(best);
                current = chosen;

                if (chosen == depot)
                {
                    routes.push_back(route);
                    remainingCapacity = capacity_;
                    --remainingCustomers;
                    --remainingVehicles;

                    if (completedCount + 1 == nodeCount_)
                    {
                        completed[depot] = 1;
                        ++completedCount;
                        route.clear();
                        break;
                    }
                    route.clear();
                }
                else
                {
                    completed[chosen] = 1;
                    ++completedCount;
                }
            }
            else
            {
                route.push_back(static_cast<int>(depot));
                routes.push_back(route);
                route.clear();
                remainingCapacity = capacity_;
                --remainingVehicles;
            }
        }

        if (!route.empty()) routes.push_back(route);
        return routes;
    }

    BNTGPEvaluation BNTGPCVRPTWDecoder::decodeAndEvaluate(
        const gp::GPTree& tree)
    {
        const std::vector<std::vector<int>> routes = buildRoutes(tree);

        ::CCVRPTWSolution solution(&tpl_);
        solution.BuildFromRoutes(routes);
        const SCVRPTWObjectives result = problem_.GetObjectives(solution);

        BNTGPEvaluation evaluation{};
        evaluation.makespan = result.distance;   // objective 1: distance (needs double, see BNTGPIndividual.hpp)
        evaluation.cost     = result.delay;      // objective 2: delay

        const double range1 = maxDistance_ - minDistance_;
        const double range2 = maxDelay_ - minDelay_;

        evaluation.normalizedMakespan = range1 > 0.0 ? (result.distance - minDistance_) / range1 : 0.0;
        evaluation.normalizedCost     = range2 > 0.0 ? (result.delay - minDelay_) / range2 : 0.0;

        evaluation.rawNormalizedMakespan = evaluation.normalizedMakespan;
        evaluation.rawNormalizedCost     = evaluation.normalizedCost;
        evaluation.searchMakespan = result.distance;
        evaluation.searchCost     = result.delay;

        if (treeParameters_.softDepthEnabled && !tree.isEmpty())
        {
            const std::size_t edgeDepth = tree.depth() - 1U;
            const std::size_t freeEdges =
                static_cast<std::size_t>(treeParameters_.softDepthFreeEdges);

            if (edgeDepth > freeEdges)
            {
                const double excess = static_cast<double>(edgeDepth - freeEdges);
                evaluation.searchMakespan += excess * treeParameters_.softDepthMakespanPenalty;
                evaluation.searchCost     += excess * treeParameters_.softDepthCostPenalty;

                if (range1 > 0.0)
                    evaluation.normalizedMakespan +=
                        excess * treeParameters_.softDepthMakespanPenalty / range1;
                if (range2 > 0.0)
                    evaluation.normalizedCost +=
                        excess * treeParameters_.softDepthCostPenalty / range2;
            }
        }

        evaluation.fitness = evaluation.normalizedMakespan + evaluation.normalizedCost;

        return evaluation;
    }
}