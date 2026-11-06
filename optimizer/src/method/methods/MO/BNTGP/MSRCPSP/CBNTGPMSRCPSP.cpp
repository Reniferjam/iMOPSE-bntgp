#include "CBNTGPMSRCPSP.h"

#include "config/BNTGPConfigReader.hpp"
#include "evolution/BNTGPEvolutionEngine.hpp"

#include "decoding/MSRCPSP/BNTGPMSRCPSPDecoder.hpp"
#include "decoding/MSRCPSP/BNTGPMSRCPSPInstanceAdapter.hpp"

#include "problem/problems/MSRCPSP/CMSRCPSP_TA.h"

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
    CScheduler& resolveTaskAssignmentScheduler(
        AProblem& problem)
    {
        auto* taskAssignmentProblem =
            dynamic_cast<CMSRCPSP_TA*>(&problem);

        if (taskAssignmentProblem == nullptr)
        {
            throw std::runtime_error(
                "BNTGP works only with the MSRCPSP_TA2 problem."
            );
        }

        CScheduler* scheduler = taskAssignmentProblem->GetScheduler();
        if (scheduler == nullptr) {
            throw std::runtime_error("BNTGP received null MSRCPSP scheduler.");
        }
        return *scheduler;
    }

    [[nodiscard]]
    bntgp_msrcpsp::BNTGPParameters readParameters(
        const SConfigMap* configuration)
    {
        if (configuration != nullptr)
        {
            return bntgp_msrcpsp::ReadBNTGPConfiguration(
                *configuration
            );
        }

        const SConfigMap emptyConfiguration{};

        return bntgp_msrcpsp::ReadBNTGPConfiguration(
            emptyConfiguration
        );
    }

    [[nodiscard]]
    std::uint64_t resolveSeed(
        const bntgp_msrcpsp::BNTGPParameters& parameters)
    {
        if (parameters.seedOverride.has_value())
        {
            return *parameters.seedOverride;
        }

        return static_cast<std::uint64_t>(
            CRandom::GetSeed()
            );
    }

    void logFinalArchive(
        const bntgp_msrcpsp::BNTGPArchive& archive)
    {
        std::vector<
            const bntgp_msrcpsp::BNTGPArchiveEntry*
        > sortedEntries;

        sortedEntries.reserve(
            archive.size()
        );

        for (const bntgp_msrcpsp::BNTGPArchiveEntry& entry :
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
                const bntgp_msrcpsp::BNTGPArchiveEntry* first,
                const bntgp_msrcpsp::BNTGPArchiveEntry* second)
            {
                const bntgp_msrcpsp::BNTGPEvaluation&
                    firstEvaluation =
                    first
                    ->individual()
                    .evaluation();

                const bntgp_msrcpsp::BNTGPEvaluation&
                    secondEvaluation =
                    second
                    ->individual()
                    .evaluation();

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

        for (const bntgp_msrcpsp::BNTGPArchiveEntry* entry :
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
            const bntgp_msrcpsp::BNTGPEvaluation& evaluation =
                entry
                ->individual()
                .evaluation();

            resultStream
                << evaluation.makespan
                << ';'
                << evaluation.cost
                << '\n';
        }

        CExperimentLogger::LogResult(
            resultStream.str().c_str()
        );
    }
}

CBNTGPMSRCPSP::CBNTGPMSRCPSP(
    AProblem* problem,
    SConfigMap* const configuration)
    : problem_(problem),
    configuration_(configuration)
{
}

void CBNTGPMSRCPSP::RunOptimization()
{
    CScheduler& scheduler =
        resolveTaskAssignmentScheduler(
            *problem_
        );

    bntgp_msrcpsp::BNTGPParameters parameters =
        readParameters(
            configuration_
        );

    const std::uint64_t runSeed =
        resolveSeed(parameters);

    bntgp_msrcpsp::decoding::msrcpsp::domain::Instance instance =
        bntgp_msrcpsp::decoding::msrcpsp::fromScheduler(
            scheduler
        );

    auto decoder = std::make_unique<
        bntgp_msrcpsp::decoding::msrcpsp::BNTGPMSRCPSPDecoder
    >(
        instance,
        scheduler,
        parameters.tree
    );

    bntgp_msrcpsp::BNTGPEvolutionEngine engine{
        std::move(decoder),
        std::move(parameters),
        runSeed
    };

    engine.run();

    logFinalArchive(
        engine.archive()
    );
}