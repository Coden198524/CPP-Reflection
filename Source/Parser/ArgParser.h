/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** ArgParser.h
** --------------------------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

class ArgParser
{
public:
    ArgParser(void);
    ~ArgParser(void);

    void addOption(const std::string &name, bool required = false, const std::string &defaultValue = "");
    void addMultiOption(const std::string &name);
    void addFlag(const std::string &name);

    void parse(int argc, char *argv[]);

    bool has(const std::string &name) const;
    std::string get(const std::string &name) const;
    std::vector<std::string> getMulti(const std::string &name) const;

private:
    struct OptionInfo
    {
        bool required;
        bool isMulti;
        bool isFlag;
        std::string defaultValue;
    };

    void printUsage(void) const;
    std::string normalizeOptionName(const std::string &arg) const;

    std::unordered_map<std::string, OptionInfo> m_optionInfo;
    std::unordered_map<std::string, std::string> m_values;
    std::unordered_map<std::string, std::vector<std::string>> m_multiValues;
    std::string m_programName;
};

#include "Impl/ArgParser.hpp"
