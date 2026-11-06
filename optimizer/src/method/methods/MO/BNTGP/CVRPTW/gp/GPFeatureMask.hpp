#pragma once
#include "FeatureCatalog.hpp"
#include "GPTree.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>

namespace bntgp_cvrptw::gp {
    using GPFeatureMask = std::array<bool, FeatureCount>;

    inline constexpr std::array<std::string_view, FeatureCount> GPFeatureNames{
        "DIST_CUR", "DIST_DEPOT", "DEMAND", "REM_CAP", "DEMAND_OVER_REM",
        "READY", "DUE", "SERVICE", "TW_WIDTH", "SLACK", "WAIT", "CUR_TIME",
        "UNROUTED_RATIO"
    };

    [[nodiscard]] inline GPFeatureMask allGPFeatures() {
        GPFeatureMask mask{}; mask.fill(true); return mask;
    }

    // Default CVRPTW feature set. Disable weak features here if you want.
    [[nodiscard]] inline GPFeatureMask defaultGPFeatures() {
        return allGPFeatures();
    }

    [[nodiscard]] inline std::string normalizedFeatureName(std::string value) {
        std::string result;
        for (unsigned char c : value) {
            if (!std::isspace(c) && c != '-') result.push_back(static_cast<char>(std::toupper(c)));
        }
        return result;
    }

    [[nodiscard]] inline FeatureId featureFromName(const std::string& name) {
        const std::string canonical = normalizedFeatureName(name);
        for (std::size_t i = 0; i < FeatureCount; ++i) {
            auto id = static_cast<FeatureId>(i);
            if (canonical == GPFeatureNames[i] || canonical == featureShortName(id)) return id;
        }
        throw std::invalid_argument("BNTGP: unknown GP feature: " + name);
    }

    inline void applyFeatureList(GPFeatureMask& mask, const std::string& names, bool enabled) {
        std::size_t pos = 0;
        while (pos < names.size()) {
            std::size_t end = names.find(',', pos);
            if (end == std::string::npos) end = names.size();
            std::string name = names.substr(pos, end - pos);
            if (!normalizedFeatureName(name).empty()) mask[toFeatureIndex(featureFromName(name))] = enabled;
            pos = end + 1;
        }
        if (std::none_of(mask.begin(), mask.end(), [](bool active){return active;}))
            throw std::invalid_argument("BNTGP: at least one GP feature must be enabled");
    }

    struct ActiveGPFeatures {
        std::array<FeatureId, FeatureCount> ordered{};
        std::size_t count{0};
        constexpr ActiveGPFeatures() noexcept = default;
        void set(const GPFeatureMask& mask) {
            count = 0;
            for (auto feature : FeatureSamplingOrder)
                if (mask[toFeatureIndex(feature)]) ordered[count++] = feature;
            if (count == 0) throw std::invalid_argument("BNTGP: feature mask is empty");
        }
    };

    extern thread_local ActiveGPFeatures CurrentGPFeatures;

    inline void configureGPFeatures(const GPFeatureMask& mask) {CurrentGPFeatures.set(mask);}

    [[nodiscard]] inline FeatureId sampleActiveGPFeature(std::mt19937& randomEngine) {
        std::uniform_int_distribution<int> distribution(0, static_cast<int>(CurrentGPFeatures.count) - 1);
        return CurrentGPFeatures.ordered[static_cast<std::size_t>(distribution(randomEngine))];
    }

    [[nodiscard]] inline bool treeRespectsFeatureMask(const GPTree& tree, const GPFeatureMask& mask) {
        for (std::size_t i = 0; i < tree.nodeCount(); ++i) {
            const GPNode& n = tree.node(static_cast<NodeIndex>(i));
            if (n.kind == NodeKind::FEATURE && (!isValidFeature(n.feature) || !mask[toFeatureIndex(n.feature)]))
                return false;
        }
        return true;
    }
}