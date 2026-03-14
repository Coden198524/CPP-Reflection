/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** CommandLineParser.cpp
** --------------------------------------------------------------------------*/

#include "Precompiled.h"

#include "CommandLineParser.h"

#include <iostream>
#include <algorithm>

namespace cmdline
{
    // Option implementation
    Option::Option(const std::string& name, const std::string& description)
        : m_name(name)
        , m_description(description)
    {
    }

    Option& Option::required()
    {
        m_required = true;
        return *this;
    }

    Option& Option::default_value(const std::string& value)
    {
        m_hasDefault = true;
        m_defaultValue = value;
        return *this;
    }

    Option& Option::multitoken()
    {
        m_multitoken = true;
        return *this;
    }

    // OptionAdder implementation
    OptionAdder::OptionAdder(OptionsDescription* desc, const std::string& name)
        : m_desc(desc)
    {
    }

    OptionAdder& OptionAdder::operator()(const std::string& name, const std::string& description)
    {
        m_desc->addOption(name, description, false, false, "", false);
        return *this;
    }

    // OptionsDescription implementation
    OptionsDescription::OptionsDescription(const std::string& caption)
        : m_caption(caption)
    {
    }

    OptionAdder OptionsDescription::add_options()
    {
        return OptionAdder(this, "");
    }

    void OptionsDescription::addOption(const std::string& name, const std::string& description,
                                      bool required, bool hasDefault,
                                      const std::string& defaultValue, bool multitoken)
    {
        Option opt(name, description);
        if (required)
            opt.required();
        if (hasDefault)
            opt.default_value(defaultValue);
        if (multitoken)
            opt.multitoken();

        m_options.push_back(opt);
    }

    std::ostream& operator<<(std::ostream& os, const OptionsDescription& desc)
    {
        os << desc.m_caption << ":\n";
        for (const auto& opt : desc.m_options)
        {
            os << "  --" << opt.getName();
            if (opt.isRequired())
                os << " (required)";
            if (opt.hasDefault())
                os << " (default: " << opt.getDefault() << ")";
            os << "\n      " << opt.getDescription() << "\n";
        }
        return os;
    }

    // VariablesMap implementation
    void VariablesMap::set(const std::string& name, const std::string& value)
    {
        m_values[name] = value;
    }

    void VariablesMap::add(const std::string& name, const std::string& value)
    {
        m_multiValues[name].push_back(value);
    }

    size_t VariablesMap::count(const std::string& name) const
    {
        return m_values.find(name) != m_values.end() ? 1 : 0;
    }

    const std::string& VariablesMap::at(const std::string& name) const
    {
        auto it = m_values.find(name);
        if (it == m_values.end())
            throw std::runtime_error("Variable not found: " + name);
        return it->second;
    }

    template<>
    std::string VariablesMap::as<std::string>(const std::string& name) const
    {
        return at(name);
    }

    template<>
    std::vector<std::string> VariablesMap::as<std::vector<std::string>>(const std::string& name) const
    {
        auto it = m_multiValues.find(name);
        if (it == m_multiValues.end())
            return std::vector<std::string>();
        return it->second;
    }

    // Parse command line
    VariablesMap parse_command_line(int argc, char* argv[], const OptionsDescription& desc)
    {
        VariablesMap result;

        // Build option lookup
        std::map<std::string, Option> optionMap;
        for (const auto& opt : desc.getOptions())
        {
            optionMap[opt.getName()] = opt;
        }

        // Parse arguments
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];

            // Check if it's an option (starts with --)
            if (arg.size() > 2 && arg[0] == '-' && arg[1] == '-')
            {
                std::string optName = arg.substr(2);

                // Find the option definition
                auto optIt = optionMap.find(optName);
                if (optIt == optionMap.end())
                {
                    throw std::runtime_error("Unknown option: " + arg);
                }

                const Option& opt = optIt->second;

                // Check if this is a flag (no value)
                if (!opt.isRequired() && !opt.hasDefault() && !opt.isMultitoken())
                {
                    // It's a flag, set it to "1"
                    result.set(optName, "1");
                }
                else
                {
                    // It requires a value
                    if (opt.isMultitoken())
                    {
                        // Collect all following non-option arguments
                        while (i + 1 < argc && (argv[i + 1][0] != '-' || argv[i + 1][1] != '-'))
                        {
                            ++i;
                            result.add(optName, argv[i]);
                        }
                    }
                    else
                    {
                        // Single value
                        if (i + 1 >= argc)
                        {
                            throw std::runtime_error("Option requires a value: " + arg);
                        }
                        ++i;
                        result.set(optName, argv[i]);
                    }
                }
            }
            else
            {
                throw std::runtime_error("Unexpected argument: " + arg);
            }
        }

        // Apply defaults
        for (const auto& opt : desc.getOptions())
        {
            if (opt.hasDefault() && result.count(opt.getName()) == 0)
            {
                result.set(opt.getName(), opt.getDefault());
            }
        }

        // Validate required options
        validate_required(result, desc);

        return result;
    }

    void store(const VariablesMap& parsed, VariablesMap& target)
    {
        target = parsed;
    }

    void notify(const VariablesMap& vm)
    {
        // Validation happens during parsing
        // This is kept for API compatibility
    }

    void validate_required(const VariablesMap& vm, const OptionsDescription& desc)
    {
        // Check for required options
        for (const auto& opt : desc.getOptions())
        {
            if (opt.isRequired() && vm.count(opt.getName()) == 0)
            {
                throw std::runtime_error("Required option missing: --" + opt.getName());
            }
        }
    }
}
