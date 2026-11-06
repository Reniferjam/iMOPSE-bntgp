#pragma once

#include "GPTypes.hpp"

#include <array>
#include <cstddef>
#include <string_view>

namespace bntgp_cvrptw::gp
{
    inline constexpr std::array<FeatureId, FeatureCount>
        FeatureSamplingOrder{
            FeatureId::DIST_CUR,
            FeatureId::DIST_DEPOT,
            FeatureId::DEMAND,
            FeatureId::REM_CAP,
            FeatureId::DEMAND_OVER_REM,
            FeatureId::READY,
            FeatureId::DUE,
            FeatureId::SERVICE,
            FeatureId::TW_WIDTH,
            FeatureId::SLACK,
            FeatureId::WAIT,
            FeatureId::CUR_TIME,
            FeatureId::UNROUTED_RATIO
    };

    [[nodiscard]]
    constexpr std::size_t toFeatureIndex(
        const FeatureId feature) noexcept
    {
        return static_cast<std::size_t>(feature);
    }

    [[nodiscard]]
    constexpr bool isValidFeature(
        const FeatureId feature) noexcept
    {
        return toFeatureIndex(feature) < FeatureCount;
    }

    [[nodiscard]]
    constexpr std::size_t samplingIndex(
        const FeatureId feature) noexcept
    {
        for (std::size_t index = 0U;
            index < FeatureSamplingOrder.size();
            ++index)
        {
            if (FeatureSamplingOrder[index] == feature)
            {
                return index;
            }
        }

        return FeatureCount;
    }

    [[nodiscard]]
    constexpr std::string_view featureShortName(
        const FeatureId feature) noexcept
    {
        switch (feature)
        {
        case FeatureId::DIST_CUR:        return "D_CUR";
        case FeatureId::DIST_DEPOT:      return "D_DEPOT";
        case FeatureId::DEMAND:          return "DEM";
        case FeatureId::REM_CAP:         return "REM_CAP";
        case FeatureId::DEMAND_OVER_REM: return "DEM_REM";
        case FeatureId::READY:           return "READY";
        case FeatureId::DUE:             return "DUE";
        case FeatureId::SERVICE:         return "SERVICE";
        case FeatureId::TW_WIDTH:        return "TW_W";
        case FeatureId::SLACK:           return "SLACK";
        case FeatureId::WAIT:            return "WAIT";
        case FeatureId::CUR_TIME:        return "T_NOW";
        case FeatureId::UNROUTED_RATIO:  return "UNROUTED";
        case FeatureId::COUNT:
            break;
        }

        return "UNKNOWN";
    }

    namespace detail
    {
        [[nodiscard]]
        constexpr bool containsUniqueFeatures() noexcept
        {
            for (std::size_t first = 0U;
                first < FeatureSamplingOrder.size();
                ++first)
            {
                if (!isValidFeature(FeatureSamplingOrder[first]))
                {
                    return false;
                }

                for (std::size_t second = first + 1U;
                    second < FeatureSamplingOrder.size();
                    ++second)
                {
                    if (FeatureSamplingOrder[first] ==
                        FeatureSamplingOrder[second])
                    {
                        return false;
                    }
                }
            }

            return true;
        }

        [[nodiscard]]
        constexpr bool containsEveryFeature() noexcept
        {
            for (std::size_t featureIndex = 0U;
                featureIndex < FeatureCount;
                ++featureIndex)
            {
                const auto feature =
                    static_cast<FeatureId>(featureIndex);

                if (samplingIndex(feature) == FeatureCount)
                {
                    return false;
                }
            }

            return true;
        }
    }

    static_assert(
        FeatureSamplingOrder.size() == FeatureCount,
        "The BNTGP feature pool must contain exactly FeatureCount features.");

    static_assert(
        detail::containsUniqueFeatures(),
        "The BNTGP feature pool contains an invalid or duplicated feature.");

    static_assert(
        detail::containsEveryFeature(),
        "The BNTGP feature pool does not contain every FeatureId.");
}