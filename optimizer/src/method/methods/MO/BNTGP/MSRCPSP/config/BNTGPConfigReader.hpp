#pragma once

#include "BNTGPParameters.hpp"

struct SConfigMap;

namespace bntgp_msrcpsp
{
    [[nodiscard]]
    BNTGPParameters ReadBNTGPConfiguration(
        const SConfigMap& sourceConfig
    );
}