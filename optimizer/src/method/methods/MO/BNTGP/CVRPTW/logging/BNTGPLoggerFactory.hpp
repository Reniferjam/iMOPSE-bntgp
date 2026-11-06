#pragma once

#include "../config/BNTGPParameters.hpp"
#include "BNTGPLoggerCollection.hpp"

namespace bntgp_cvrptw
{
    [[nodiscard]]
    BNTGPLoggerCollection createBNTGPLoggers(
        const LoggingParameters& parameters
    );
}
