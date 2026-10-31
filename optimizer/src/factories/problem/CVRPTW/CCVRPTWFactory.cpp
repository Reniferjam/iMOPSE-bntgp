#include "CCVRPTWFactory.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    bool ParseNumbers(const std::string& line, std::vector<double>& out)
    {
        out.clear();
        std::istringstream stream(line);
        std::string token;
        while (stream >> token)
        {
            try
            {
                std::size_t used = 0;
                double value = std::stod(token, &used);
                if (used != token.size()) return false;
                out.push_back(value);
            }
            catch (const std::exception&)
            {
                return false;
            }
        }
        return !out.empty();
    }
}

CCVRPTW* CCVRPTWFactory::CreateCVRPTW(const char* problemDefinitionPath)
{
    auto* cvrptwTemplate = ReadCVRPTWTemplate(problemDefinitionPath);
    return new CCVRPTW(cvrptwTemplate);
}

CCVRPTWTemplate* CCVRPTWFactory::ReadCVRPTWTemplate(const char* problemDefinitionPath)
{
    std::ifstream file(problemDefinitionPath);
    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open CVRPTW instance: " + std::string(problemDefinitionPath));
    }

    int vehicleCount = 0;
    int capacity = 0;
    std::vector<SCityCVRPTW> cities;

    bool vehicleSection = false;
    bool customerSection = false;
    bool vehicleDataRead = false;

    std::string line;
    std::vector<double> numbers;
    while (std::getline(file, line))
    {
        if (line.find("CAPACITY") != std::string::npos)
        {
            vehicleSection = true; // numbers follow on the next data line
            continue;
        }
        if (line.find("CUSTOMER") != std::string::npos)
        {
            customerSection = true;
            vehicleSection = false;
            continue;
        }

        if (!ParseNumbers(line, numbers)) continue;

        if (vehicleSection && !vehicleDataRead && numbers.size() >= 2)
        {
            vehicleCount = (int)numbers[0];
            capacity = (int)numbers[1];
            vehicleDataRead = true;
        }
        else if (customerSection && numbers.size() >= 7)
        {
            cities.emplace_back((int)cities.size(),
                                (float)numbers[1],            // x
                                (float)numbers[2],            // y
                                (int)numbers[3],              // demand
                                (float)numbers[4],            // ready time
                                (float)numbers[5],            // due date
                                (float)numbers[6]);           // service time
        }
    }
    file.close();

    if (!vehicleDataRead) throw std::runtime_error("Error reading vehicle data for CVRPTW");
    if (cities.size() < 2) throw std::runtime_error("Error reading customers for CVRPTW");

    auto* result = new CCVRPTWTemplate();

    std::string pathString(problemDefinitionPath);
    std::size_t nameStart = pathString.find_last_of("/\\");
    nameStart = (nameStart == std::string::npos) ? 0 : nameStart + 1;
    std::size_t nameEnd = pathString.rfind('.');
    if (nameEnd == std::string::npos || nameEnd < nameStart) nameEnd = pathString.size();
    result->SetFileName(pathString.substr(nameStart, nameEnd - nameStart));

    result->SetData(cities, capacity, vehicleCount);

    if (!result->Validate())
    {
        delete result;
        throw std::runtime_error("Instance is invalid: " + std::string(problemDefinitionPath));
    }

    return result;
}