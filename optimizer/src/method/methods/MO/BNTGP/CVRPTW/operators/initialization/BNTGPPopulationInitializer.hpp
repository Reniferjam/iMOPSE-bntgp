#pragma once

#include "../../core/BNTGPIndividual.hpp"

#include <cstddef>
#include <random>
#include <vector>

namespace bntgp_cvrptw
{
    class BNTGPIndividualEvaluator;

    void initializeBNTGPPopulation(
        std::vector<BNTGPIndividual>& population,
        std::size_t populationSize,
        int maximumTreeDepth,
        std::mt19937& randomEngine,
        BNTGPIndividualEvaluator& evaluator,
        BNTGPIndividualId& nextIndividualId
    );
}
