/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** MetaUtils.hpp
** --------------------------------------------------------------------------*/

namespace utils
{
    template<typename Container>
    std::string Join(const Container &container, const std::string &delimiter)
    {
        std::ostringstream oss;
        auto it = container.begin( );
        if (it != container.end( ))
        {
            oss << *it;
            ++it;
        }
        while (it != container.end( ))
        {
            oss << delimiter << *it;
            ++it;
        }
        return oss.str( );
    }

    template<typename A, typename B>
    bool RangeEqual(A startA, A endA, B startB, B endB)
    {
        while (startA != endA && startB != endB)
        {
            if (*startA != *startB)
                return false;

            ++startA;
            ++startB;
        }

        return (startA == endA) && (startB == endB);
    }
}