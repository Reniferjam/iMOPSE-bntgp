#include "CBNTGPCVRPTW.h"

#include "config/BNTGPConfigReader.hpp"
#include "evolution/BNTGPEvolutionEngine.hpp"

#include "decoding/CVRPTW/BNTGPCVRPTWDecoder.hpp"

#include "problem/problems/CVRPTW/CCVRPTW.h"

#include "utils/logger/CExperimentLogger.h"
#include "utils/random/CRandom.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
    [[nodiscard]]
    CCVRPTW& resolveCvrptwProblem(
        AProblem& problem)
    {
        auto* cvrptwProblem =
            dynamic_cast<CCVRPTW*>(&problem);

        if (cvrptwProblem == nullptr)
        {
            throw std::runtime_error(
                "BNTGP works only with the CVRPTW problem."
            );
        }

        if (cvrptwProblem->GetCVRPTWTemplate() == nullptr)
        {
            throw std::runtime_error(
                "BNTGP received null CVRPTW template."
            );
        }

        return *cvrptwProblem;
    }

    [[nodiscard]]
    bntgp_cvrptw::BNTGPParameters readParameters(
        const SConfigMap* configuration)
    {
        if (configuration != nullptr)
        {
            return bntgp_cvrptw::ReadBNTGPConfiguration(
                *configuration
            );
        }

        const SConfigMap emptyConfiguration{};

        return bntgp_cvrptw::ReadBNTGPConfiguration(
            emptyConfiguration
        );
    }

    [[nodiscard]]
    std::uint64_t resolveSeed(
        const bntgp_cvrptw::BNTGPParameters& parameters)
    {
        if (parameters.seedOverride.has_value())
        {
            return *parameters.seedOverride;
        }

        return static_cast<std::uint64_t>(
            CRandom::GetSeed()
            );
    }

    // Output format: "vehicles;distance" per non-dominated solution.
    void logFinalArchive(
        const bntgp_cvrptw::BNTGPArchive& archive)
    {
        std::vector<
            const bntgp_cvrptw::BNTGPArchiveEntry*
        > sortedEntries;

        sortedEntries.reserve(
            archive.size()
        );

        for (const bntgp_cvrptw::BNTGPArchiveEntry& entry :
            archive.entries())
        {
            sortedEntries.push_back(
                &entry
            );
        }

        std::sort(
            sortedEntries.begin(),
            sortedEntries.end(),
            [](
                const bntgp_cvrptw::BNTGPArchiveEntry* first,
                const bntgp_cvrptw::BNTGPArchiveEntry* second)
            {
                const bntgp_cvrptw::BNTGPEvaluation&
                    firstEvaluation =
                    first->individual().evaluation();

                const bntgp_cvrptw::BNTGPEvaluation&
                    secondEvaluation =
                    second->individual().evaluation();

                if (firstEvaluation.makespan !=
                    secondEvaluation.makespan)
                {
                    return
                        firstEvaluation.makespan
                        <
                        secondEvaluation.makespan;
                }

                return
                    firstEvaluation.cost
                    <
                    secondEvaluation.cost;
            }
        );

        std::ostringstream resultStream;

        for (const bntgp_cvrptw::BNTGPArchiveEntry* entry :
            sortedEntries)
        {
            const auto& current = entry->individual().evaluation();
            bool dominatedRaw = false;
            for (const auto* otherEntry : sortedEntries) {
                if (otherEntry == entry) continue;
                const auto& other = otherEntry->individual().evaluation();
                if (other.makespan <= current.makespan && other.cost <= current.cost &&
                    (other.makespan < current.makespan || other.cost < current.cost)) {
                    dominatedRaw = true;
                    break;
                }
            }
            if (dominatedRaw) continue;

            resultStream
                << current.makespan
                << ';'
                << current.cost
                << '\n';
        }

        CExperimentLogger::LogResult(
            resultStream.str().c_str()
        );
    }
}

CBNTGPCVRPTW::CBNTGPCVRPTW(
    AProblem* problem,
    SConfigMap* const configuration)
    : problem_(problem),
    configuration_(configuration)
{
}

void CBNTGPCVRPTW::RunOptimization()
{
    CCVRPTW& cvrptw =
        resolveCvrptwProblem(
            *problem_
        );

    bntgp_cvrptw::BNTGPParameters parameters =
        readParameters(
            configuration_
        );

    const std::uint64_t runSeed =
        resolveSeed(parameters);

    auto decoder = std::make_unique<
        bntgp_cvrptw::decoding::cvrptw::BNTGPCVRPTWDecoder
    >(
        cvrptw,
        parameters.tree
    );

    bntgp_cvrptw::BNTGPEvolutionEngine engine{
        std::move(decoder),
        std::move(parameters),
        runSeed
    };

    engine.run();

    logFinalArchive(
        engine.archive()
    );
}