#pragma once

#include "BNTGPParameters.hpp"

struct SConfigMap;

namespace bntgp_cvrptw
{
    [[nodiscard]]
    BNTGPParameters ReadBNTGPConfiguration(
        const SConfigMap& sourceConfig
    );
}