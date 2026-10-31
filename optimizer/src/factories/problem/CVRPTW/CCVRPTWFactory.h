#pragma once

#include "problem/problems/CVRPTW/CCVRPTW.h"

class CCVRPTWFactory
{
public:
    static CCVRPTW* CreateCVRPTW(const char* problemDefinitionPath);

private:
    static CCVRPTWTemplate* ReadCVRPTWTemplate(const char* problemDefinitionPath);
};