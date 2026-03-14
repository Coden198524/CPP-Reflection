/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** ArgParser.hpp
** --------------------------------------------------------------------------*/

#pragma once

inline ArgParser::ArgParser(void)
    : m_programName( "" )
{
}

inline ArgParser::~ArgParser(void)
{
}

inline void ArgParser::addOption(const std::string &name, bool required, const std::string &defaultValue)
{
    OptionInfo info;
    info.required = required;
    info.isMulti = false;
    info.isFlag = false;
    info.defaultValue = defaultValue;

    m_optionInfo[name] = info;

    if (!defaultValue.empty( ))
    {
        m_values[name] = defaultValue;
    }
}

inline void ArgParser::addMultiOption(const std::string &name)
{
    OptionInfo info;
    info.required = false;
    info.isMulti = true;
    info.isFlag = false;
    info.defaultValue = "";

    m_optionInfo[name] = info;
}

inline void ArgParser::addFlag(const std::string &name)
{
    OptionInfo info;
    info.required = false;
    info.isMulti = false;
    info.isFlag = true;
    info.defaultValue = "";

    m_optionInfo[name] = info;
}

inline void ArgParser::parse(int argc, char *argv[])
{
    if (argc > 0)
    {
        m_programName = argv[0];
    }

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h")
        {
            printUsage( );
            std::exit(0);
        }

        if (arg.substr(0, 2) == "--" || arg.substr(0, 1) == "-")
        {
            std::string optionName = normalizeOptionName(arg);

            auto it = m_optionInfo.find(optionName);
            if (it == m_optionInfo.end( ))
            {
                throw std::runtime_error("Unknown option: " + arg);
            }

            const OptionInfo &info = it->second;

            if (info.isFlag)
            {
                m_values[optionName] = "true";
            }
            else if (info.isMulti)
            {
                if (i + 1 >= argc)
                {
                    throw std::runtime_error("Option " + arg + " requires a value");
                }

                ++i;
                m_multiValues[optionName].push_back(argv[i]);
            }
            else
            {
                if (i + 1 >= argc)
                {
                    throw std::runtime_error("Option " + arg + " requires a value");
                }

                ++i;
                m_values[optionName] = argv[i];
            }
        }
    }

    for (const auto &pair : m_optionInfo)
    {
        const std::string &name = pair.first;
        const OptionInfo &info = pair.second;

        if (info.required)
        {
            if (info.isMulti)
            {
                if (m_multiValues.find(name) == m_multiValues.end( ) || m_multiValues[name].empty( ))
                {
                    throw std::runtime_error("Missing required option: --" + name);
                }
            }
            else
            {
                if (m_values.find(name) == m_values.end( ))
                {
                    throw std::runtime_error("Missing required option: --" + name);
                }
            }
        }
    }
}

inline bool ArgParser::has(const std::string &name) const
{
    auto it = m_optionInfo.find(name);
    if (it == m_optionInfo.end( ))
    {
        return false;
    }

    const OptionInfo &info = it->second;

    if (info.isMulti)
    {
        auto multiIt = m_multiValues.find(name);
        return multiIt != m_multiValues.end( ) && !multiIt->second.empty( );
    }
    else
    {
        return m_values.find(name) != m_values.end( );
    }
}

inline std::string ArgParser::get(const std::string &name) const
{
    auto it = m_values.find(name);
    if (it != m_values.end( ))
    {
        return it->second;
    }

    return "";
}

inline std::vector<std::string> ArgParser::getMulti(const std::string &name) const
{
    auto it = m_multiValues.find(name);
    if (it != m_multiValues.end( ))
    {
        return it->second;
    }

    return std::vector<std::string>( );
}

inline void ArgParser::printUsage(void) const
{
    std::cout << "Usage: " << m_programName << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;

    for (const auto &pair : m_optionInfo)
    {
        const std::string &name = pair.first;
        const OptionInfo &info = pair.second;

        std::cout << "  --" << name;

        if (info.isFlag)
        {
            std::cout << " (flag)";
        }
        else if (info.isMulti)
        {
            std::cout << " <value> (can be specified multiple times)";
        }
        else
        {
            std::cout << " <value>";
        }

        if (info.required)
        {
            std::cout << " (required)";
        }
        else if (!info.defaultValue.empty( ))
        {
            std::cout << " (default: " << info.defaultValue << ")";
        }

        std::cout << std::endl;
    }

    std::cout << "  --help, -h  Display this help message" << std::endl;
}

inline std::string ArgParser::normalizeOptionName(const std::string &arg) const
{
    if (arg.substr(0, 2) == "--")
    {
        return arg.substr(2);
    }
    else if (arg.substr(0, 1) == "-")
    {
        std::string shorthand = arg.substr(1);

        for (const auto &pair : m_optionInfo)
        {
            const std::string &name = pair.first;

            size_t commaPos = name.find(',');
            if (commaPos != std::string::npos)
            {
                std::string shortForm = name.substr(commaPos + 1);
                if (shortForm == shorthand)
                {
                    return name.substr(0, commaPos);
                }
            }
        }

        return shorthand;
    }

    return arg;
}
