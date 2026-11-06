#include "BNTGPNodeStatisticsLogger.hpp"

// Temporary stub: node-statistics logging is disabled for the CVRPTW port.
// The original MSRCPSP version printed task/resource feature names and
// needs to be rewritten for the 13 CVRPTW features before use.

namespace bntgp_cvrptw
{
    BNTGPNodeStatisticsLogger::BNTGPNodeStatisticsLogger(
        NodeStatisticsLoggerParameters parameters) noexcept
        : parameters_(parameters)
    {
    }

    BNTGPLogEventMask BNTGPNodeStatisticsLogger::subscribedEvents()
        const noexcept
    {
        return BNTGPLogEventMask{ 0U };   // subscribes to nothing
    }

    void BNTGPNodeStatisticsLogger::onRunStarted(
        const BNTGPLoggingContext&)
    {
    }

    void BNTGPNodeStatisticsLogger::onGenerationCompleted(
        const BNTGPLoggingContext&)
    {
    }
}