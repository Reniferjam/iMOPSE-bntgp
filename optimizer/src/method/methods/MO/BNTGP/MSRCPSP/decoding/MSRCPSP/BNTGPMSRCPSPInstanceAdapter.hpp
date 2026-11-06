#pragma once

#include "domain/Instance.hpp"

class CScheduler;

namespace bntgp_msrcpsp::decoding::msrcpsp
{
    [[nodiscard]]
    domain::Instance fromScheduler(
        const ::CScheduler& scheduler
    );
}
