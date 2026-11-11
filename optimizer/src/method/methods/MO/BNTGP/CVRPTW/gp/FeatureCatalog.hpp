#pragma once

#include "GPTypes.hpp"

#include <array>
#include <cstddef>
#include <string_view>

namespace bntgp_cvrptw::gp
{
    inline constexpr std::array<FeatureId, FeatureCount>
        FeatureSamplingOrder{
            FeatureId::DEMAND,
            FeatureId::DISTANCE,
            FeatureId::REMAINING_CAPACITY,
            FeatureId::DISTANCE_FROM_DEPOT,
            FeatureId::CUSTOMERS_LEFT,
            FeatureId::VEHICLES_LEFT,
            FeatureId::REF_CENTER_DISTANCE,
            FeatureId::REF_Q1_DISTANCE,
            FeatureId::REF_Q2_DISTANCE,
            FeatureId::REF_Q3_DISTANCE,
            FeatureId::REF_Q4_DISTANCE,
            FeatureId::CURRENT_TIME,
            FeatureId::TRAVEL_TIME,
            FeatureId::WAITING_TIME,
            FeatureId::DELAY
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
        case FeatureId::DEMAND:              return "DEM";
        case FeatureId::DISTANCE:            return "DIST";
        case FeatureId::REMAINING_CAPACITY:  return "REM_CAP";
        case FeatureId::DISTANCE_FROM_DEPOT: return "D_DEPOT";
        case FeatureId::CUSTOMERS_LEFT:      return "CUST_LEFT";
        case FeatureId::VEHICLES_LEFT:       return "VEH_LEFT";
        case FeatureId::REF_CENTER_DISTANCE: return "REF_C";
        case FeatureId::REF_Q1_DISTANCE:     return "REF_Q1";
        case FeatureId::REF_Q2_DISTANCE:     return "REF_Q2";
        case FeatureId::REF_Q3_DISTANCE:     return "REF_Q3";
        case FeatureId::REF_Q4_DISTANCE:     return "REF_Q4";
        case FeatureId::CURRENT_TIME:        return "T_NOW";
        case FeatureId::TRAVEL_TIME:         return "TRAVEL";
        case FeatureId::WAITING_TIME:        return "WAIT";
        case FeatureId::DELAY:               return "DELAY";
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