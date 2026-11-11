#pragma once

#include "../IBNTGPDecoder.hpp"
#include "../../config/BNTGPParameters.hpp"

#include "problem/problems/CVRPTW/CCVRPTW.h"
#include "problem/problems/CVRPTW/CCVRPTWTemplate.h"

#include <array>
#include <cstddef>
#include <vector>

namespace bntgp_cvrptw::decoding::cvrptw
{
    class BNTGPCVRPTWDecoder final : public IBNTGPDecoder
    {
    public:
        BNTGPCVRPTWDecoder(
            ::CCVRPTW& problem,
            TreeParameters treeParameters);

        [[nodiscard]]
        BNTGPEvaluation decodeAndEvaluate(
            const gp::GPTree& tree) override;

    private:
        [[nodiscard]]
        std::vector<std::vector<int>> buildRoutes(
            const gp::GPTree& tree) const;

        ::CCVRPTW& problem_;
        const ::CCVRPTWTemplate& tpl_;
        TreeParameters treeParameters_;

        std::size_t nodeCount_{ 0 };      // customers + depot
        int maxVehicles_{ 1 };
        int capacity_{ 1 };

        // normalization constants
        double distanceConst_{ 1.0 };     // 1 / max edge
        double timeConst_{ 1.0 };         // 1 / max due time
        double demandConst_{ 1.0 };       // 1 / capacity
        double clientsConst_{ 1.0 };      // 1 / (customers + depot)
        double vehiclesConst_{ 1.0 };     // 1 / fleet size

        // distances from every node to 5 reference points: centre, q1, q2, q3, q4
        std::array<std::vector<double>, 5> referenceDistances_{};

        // objective ranges used for normalization
        double minDistance_{ 0.0 };
        double maxDistance_{ 1.0 };
        double minDelay_{ 0.0 };
        double maxDelay_{ 1.0 };
    };
}