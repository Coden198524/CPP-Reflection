/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** StringUtils.h
** --------------------------------------------------------------------------*/

#pragma once

#include <string>
#include <sstream>

namespace utils
{
    // Join elements of a container with a delimiter
    template<typename Container>
    inline std::string join(const Container& elements, const std::string& delimiter)
    {
        if (elements.empty())
            return "";

        auto it = elements.begin();
        std::string result = *it++;

        for (; it != elements.end(); ++it)
        {
            result += delimiter;
            result += *it;
        }

        return result;
    }

    // Check if a string starts with a given prefix
    inline bool starts_with(const std::string& str, const std::string& prefix)
    {
        return str.size() >= prefix.size() &&
               str.compare(0, prefix.size(), prefix) == 0;
    }
}
