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
        : tpl_(*problem.GetCVRPTWTemplate()),
          treeParameters_(treeParameters)
    {
        customerCount_ = static_cast<int>(tpl_.GetCustomersCount());
        horizon_  = std::max(1.0, static_cast<double>(tpl_.GetDepotDueTime()));
        capacity_ = std::max(1.0, static_cast<double>(tpl_.GetCapacity()));
        maxEdge_  = std::max(1.0, static_cast<double>(tpl_.GetMaxDepotEdge()));

        minVehicles_ = static_cast<double>(tpl_.GetMinVehicles());
        maxVehicles_ = std::max<double>(customerCount_, minVehicles_);
        minDistance_ = static_cast<double>(tpl_.GetMinEdge()) * customerCount_;
        maxDistance_ = std::max<double>(
            static_cast<double>(tpl_.GetMaxDistanceBound()),
            minDistance_ + 1.0);
    }

    BNTGPEvaluation BNTGPCVRPTWDecoder::decodeAndEvaluate(
        const gp::GPTree& tree)
    {
        using gp::FeatureId;

        const int n = customerCount_;
        std::vector<char> done(static_cast<std::size_t>(n) + 1U, 0);

        ::CCVRPTWSolution sol(&tpl_);
        int  remaining  = n;
        bool routeEmpty = true;

        gp::FeatureValues f;

        sol.StartRoute();

        while (remaining > 0)
        {
            int best = -1;
            double bestScore = -std::numeric_limits<double>::infinity();

            const std::size_t cur = sol.GetCurrentPosition();
            const double remCap   = static_cast<double>(sol.GetRemainingCapacity());

            for (int c = 1; c <= n; ++c)
            {
                const std::size_t cu = static_cast<std::size_t>(c);

                if (done[cu]) continue;
                if (!sol.FitsCapacity(cu) || !sol.CanArriveInTime(cu)) continue;

                const auto& city = tpl_.GetCities()[cu];

                f[FeatureId::DIST_CUR]        = tpl_.GetDistance(cur, cu) / maxEdge_;
                f[FeatureId::DIST_DEPOT]      = tpl_.GetDistance(cu, CVRPTW_DEPOT_IDX) / maxEdge_;
                f[FeatureId::DEMAND]          = city.m_Demand / capacity_;
                f[FeatureId::REM_CAP]         = remCap / capacity_;
                f[FeatureId::DEMAND_OVER_REM] = remCap > 0.0 ? city.m_Demand / remCap : 1.0;
                f[FeatureId::READY]           = city.m_ReadyTime / horizon_;
                f[FeatureId::DUE]             = city.m_DueTime / horizon_;
                f[FeatureId::SERVICE]         = city.m_ServiceTime / horizon_;
                f[FeatureId::TW_WIDTH]        = (city.m_DueTime - city.m_ReadyTime) / horizon_;
                f[FeatureId::SLACK]           = sol.Slack(cu) / horizon_;
                f[FeatureId::WAIT]            = sol.WaitingTime(cu) / horizon_;
                f[FeatureId::CUR_TIME]        = sol.GetCurrentTime() / horizon_;
                f[FeatureId::UNROUTED_RATIO]  = static_cast<double>(remaining) / n;

                double score = gp::evaluateTree(tree, f);
                if (!std::isfinite(score))
                    score = -std::numeric_limits<double>::infinity();

                if (best < 0 || score > bestScore)
                {
                    bestScore = score;
                    best = c;
                }
            }

            if (best >= 0)
            {
                sol.Serve(static_cast<std::size_t>(best));
                done[static_cast<std::size_t>(best)] = 1;
                --remaining;
                routeEmpty = false;
                continue;
            }

            if (routeEmpty)
            {
                // Even a fresh vehicle cannot serve the rest: mark unserved, guarantees termination.
                for (int c = 1; c <= n; ++c)
                {
                    const std::size_t cu = static_cast<std::size_t>(c);
                    if (!done[cu])
                    {
                        sol.MarkUnserved(cu);
                        done[cu] = 1;
                    }
                }
                break;
            }

            sol.StartRoute();   // closes the current route, opens a new one
            routeEmpty = true;
        }

        sol.CloseRoute();

        // Objectives (both minimized): vehicles, total distance.
        // Unserved customers are penalized so infeasible solutions are always dominated.
        const int    unserved = sol.GetUnservedCount();
        const double vehicles = static_cast<double>(sol.GetVehiclesUsed())
                              + static_cast<double>(unserved) * n;
        const double distance = static_cast<double>(sol.GetTotalDistance())
                              + static_cast<double>(unserved) * maxDistance_;

        BNTGPEvaluation evaluation{};
        evaluation.makespan = static_cast<int>(vehicles);   // number of vehicles
        evaluation.cost     = distance;                      // total distance

        const double vRange = maxVehicles_ - minVehicles_;
        const double dRange = maxDistance_ - minDistance_;

        evaluation.normalizedMakespan = vRange > 0.0 ? (vehicles - minVehicles_) / vRange : 0.0;
        evaluation.normalizedCost     = dRange > 0.0 ? (distance - minDistance_) / dRange : 0.0;

        evaluation.rawNormalizedMakespan = evaluation.normalizedMakespan;
        evaluation.rawNormalizedCost     = evaluation.normalizedCost;
        evaluation.searchMakespan = vehicles;
        evaluation.searchCost     = distance;

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

                if (vRange > 0.0)
                    evaluation.normalizedMakespan +=
                        excess * treeParameters_.softDepthMakespanPenalty / vRange;
                if (dRange > 0.0)
                    evaluation.normalizedCost +=
                        excess * treeParameters_.softDepthCostPenalty / dRange;
            }
        }

        evaluation.fitness = evaluation.normalizedMakespan + evaluation.normalizedCost;

        return evaluation;
    }
}