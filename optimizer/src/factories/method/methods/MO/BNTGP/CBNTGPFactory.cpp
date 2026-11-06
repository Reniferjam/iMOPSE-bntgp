#include "CBNTGPFactory.h"

#include "../../../../../method/methods/MO/BNTGP/MSRCPSP/CBNTGPMSRCPSP.h"
#include "../../../../../method/methods/MO/BNTGP/CVRPTW/CBNTGPCVRPTW.h"

#include <stdexcept>

// MS-RCPSP: keeps the existing "BNTGP" method name working
AMethod* CBNTGPFactory::CreateBNTGP(
    SConfigMap* const configuration,
    AProblem* problem,
    AInitialization* const initialization)
{
    if (initialization == nullptr)
    {
        throw std::runtime_error(
            "BNTGP requires a valid initialization object."
        );
    }

    return new CBNTGPMSRCPSP(
        problem,
        configuration
    );
}

// CVRPTW
AMethod* CBNTGPFactory::CreateBNTGPCVRPTW(
    SConfigMap* const configuration,
    AProblem* problem,
    AInitialization* const initialization)
{
    if (initialization == nullptr)
    {
        throw std::runtime_error(
            "BNTGP-CVRPTW requires a valid initialization object."
        );
    }

    return new CBNTGPCVRPTW(
        problem,
        configuration
    );
}