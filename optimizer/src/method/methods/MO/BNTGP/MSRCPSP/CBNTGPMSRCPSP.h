#pragma once

#include "method/AMethod.h"
#include "method/configMap/SConfigMap.h"

class CBNTGPMSRCPSP final : public AMethod
{
public:
    CBNTGPMSRCPSP(
        AProblem* problem,
        SConfigMap* configuration
    );

    void RunOptimization() override;

    void Reset() override
    {
    }

private:
    AProblem* problem_{ nullptr };
    SConfigMap* configuration_{ nullptr };
};