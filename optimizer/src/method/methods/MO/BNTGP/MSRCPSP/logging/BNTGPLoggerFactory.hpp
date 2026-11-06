#pragma once

#include "../config/BNTGPParameters.hpp"
#include "BNTGPLoggerCollection.hpp"

namespace bntgp_msrcpsp
{
    [[nodiscard]]
    BNTGPLoggerCollection createBNTGPLoggers(
        const LoggingParameters& parameters
    );
}
