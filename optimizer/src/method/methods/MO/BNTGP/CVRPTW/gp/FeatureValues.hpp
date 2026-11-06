#pragma once

#include "FeatureCatalog.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <type_traits>

namespace bntgp_cvrptw::gp
{
    // CVRPTW has a single entity (the candidate customer), so there is no
    // task/resource split: one flat array indexed by FeatureId.
    class FeatureValues final
    {
    public:
        using Storage = std::array<double, FeatureCount>;

        FeatureValues() noexcept
        {
            reset();
        }

        void reset() noexcept
        {
            values_.fill(0.0);
        }

        [[nodiscard]]
        double operator[](const FeatureId feature) const noexcept
        {
            assert(isValidFeature(feature));
            return values_[toFeatureIndex(feature)];
        }

        double& operator[](const FeatureId feature) noexcept
        {
            assert(isValidFeature(feature));
            return values_[toFeatureIndex(feature)];
        }

        [[nodiscard]]
        const Storage& raw() const noexcept
        {
            return values_;
        }

    private:
        Storage values_{};
    };

    static_assert(
        std::is_trivially_copyable_v<FeatureValues>,
        "FeatureValues must remain trivially copyable.");

    static_assert(
        sizeof(FeatureValues) == FeatureCount * sizeof(double),
        "FeatureValues contains unexpected memory overhead.");
}