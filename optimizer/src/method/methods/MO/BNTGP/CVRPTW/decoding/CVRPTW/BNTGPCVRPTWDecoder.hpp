#pragma once

#include "../IBNTGPDecoder.hpp"
#include "../../config/BNTGPParameters.hpp"

// Adjust these two include paths to where your CVRPTW classes live.
#include "problem/problems/CVRPTW/CCVRPTW.h"
#include "problem/problems/CVRPTW/CCVRPTWTemplate.h"

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
        const ::CCVRPTWTemplate& tpl_;
        TreeParameters treeParameters_;

        int    customerCount_{ 0 };
        double horizon_{ 1.0 };
        double capacity_{ 1.0 };
        double maxEdge_{ 1.0 };

        double minVehicles_{ 1.0 };
        double maxVehicles_{ 1.0 };
        double minDistance_{ 0.0 };
        double maxDistance_{ 1.0 };
    };
}