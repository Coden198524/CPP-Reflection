/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** CommandLineParser.h
** --------------------------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include <sstream>

namespace cmdline
{
    class VariablesMap;
    class OptionsDescription;

    // Represents a single command-line option
    class Option
    {
    public:
        Option() = default;  // Add default constructor
        Option(const std::string& name, const std::string& description);

        Option& required();
        Option& default_value(const std::string& value);
        Option& multitoken();

        std::string getName() const { return m_name; }
        std::string getDescription() const { return m_description; }
        bool isRequired() const { return m_required; }
        bool hasDefault() const { return m_hasDefault; }
        std::string getDefault() const { return m_defaultValue; }
        bool isMultitoken() const { return m_multitoken; }

    private:
        std::string m_name;
        std::string m_description;
        bool m_required = false;
        bool m_hasDefault = false;
        std::string m_defaultValue;
        bool m_multitoken = false;
    };

    // Builder for option values
    template<typename T>
    class ValueBuilder
    {
    public:
        ValueBuilder() = default;

        ValueBuilder& required()
        {
            m_required = true;
            return *this;
        }

        ValueBuilder& default_value(const T& value)
        {
            m_hasDefault = true;
            std::stringstream ss;
            ss << value;
            m_defaultValue = ss.str();
            return *this;
        }

        ValueBuilder& multitoken()
        {
            m_multitoken = true;
            return *this;
        }

        bool isRequired() const { return m_required; }
        bool hasDefault() const { return m_hasDefault; }
        std::string getDefault() const { return m_defaultValue; }
        bool isMultitoken() const { return m_multitoken; }

    private:
        bool m_required = false;
        bool m_hasDefault = false;
        std::string m_defaultValue;
        bool m_multitoken = false;
    };

    // Factory for creating value builders
    template<typename T>
    inline ValueBuilder<T>* value()
    {
        return new ValueBuilder<T>();
    }

    // Proxy for adding options with fluent interface
    class OptionAdder
    {
    public:
        OptionAdder(OptionsDescription* desc, const std::string& name);

        template<typename T>
        OptionAdder& operator()(const std::string& name, ValueBuilder<T>* builder, const std::string& description);

        OptionAdder& operator()(const std::string& name, const std::string& description);

    private:
        OptionsDescription* m_desc;
    };

    // Container for option definitions
    class OptionsDescription
    {
    public:
        OptionsDescription(const std::string& caption);

        OptionAdder add_options();

        void addOption(const std::string& name, const std::string& description,
                      bool required = false, bool hasDefault = false,
                      const std::string& defaultValue = "", bool multitoken = false);

        const std::vector<Option>& getOptions() const { return m_options; }
        std::string getCaption() const { return m_caption; }

        friend std::ostream& operator<<(std::ostream& os, const OptionsDescription& desc);

    private:
        std::string m_caption;
        std::vector<Option> m_options;
    };

    // Container for parsed variable values
    class VariablesMap
    {
    public:
        void set(const std::string& name, const std::string& value);
        void add(const std::string& name, const std::string& value);

        size_t count(const std::string& name) const;

        template<typename T>
        T as(const std::string& name) const;

        const std::string& at(const std::string& name) const;

    private:
        std::map<std::string, std::string> m_values;
        std::map<std::string, std::vector<std::string>> m_multiValues;
    };

    // Parse command line arguments
    void store(const VariablesMap& parsed, VariablesMap& target);
    VariablesMap parse_command_line(int argc, char* argv[], const OptionsDescription& desc);
    void notify(const VariablesMap& vm);
    void validate_required(const VariablesMap& vm, const OptionsDescription& desc);

    // Template implementations
    template<typename T>
    OptionAdder& OptionAdder::operator()(const std::string& name, ValueBuilder<T>* builder, const std::string& description)
    {
        std::unique_ptr<ValueBuilder<T>> builderPtr(builder);
        m_desc->addOption(name, description, builderPtr->isRequired(),
                         builderPtr->hasDefault(), builderPtr->getDefault(),
                         builderPtr->isMultitoken());
        return *this;
    }

    template<typename T>
    T VariablesMap::as(const std::string& name) const
    {
        // Specializations will be provided in the .cpp file
        throw std::runtime_error("Unsupported type conversion");
    }

    // Specialization declarations
    template<>
    std::string VariablesMap::as<std::string>(const std::string& name) const;

    template<>
    std::vector<std::string> VariablesMap::as<std::vector<std::string>>(const std::string& name) const;
}
