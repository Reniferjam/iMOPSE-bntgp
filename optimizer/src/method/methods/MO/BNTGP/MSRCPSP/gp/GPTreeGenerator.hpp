#pragma once

#include "GPTree.hpp"

#include <random>

namespace bntgp_msrcpsp::gp
{
    [[nodiscard]]
    GPTree generateRandomPairTree(
        std::mt19937& randomEngine,
        int maximumDepth
    );
}