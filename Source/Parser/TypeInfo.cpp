/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** TypeInfo.cpp
** --------------------------------------------------------------------------*/

#include "Precompiled.h"

#include "TypeInfo.h"

TypeInfo::TypeInfo(void)
    : m_kind(TypeKind::Invalid)
    , m_isConst(false)
    , m_canonicalType(nullptr)
{
}

TypeInfo::TypeInfo(const std::string &displayName, TypeKind kind)
    : m_displayName(displayName)
    , m_kind(kind)
    , m_isConst(false)
    , m_canonicalType(nullptr)
{
}

std::string TypeInfo::GetDisplayName(void) const
{
    return m_displayName;
}

int TypeInfo::GetArgumentCount(void) const
{
    return static_cast<int>(m_arguments.size());
}

std::shared_ptr<TypeInfo> TypeInfo::GetArgument(unsigned index) const
{
    if (index < m_arguments.size())
    {
        return m_arguments[index];
    }

    return nullptr;
}

std::shared_ptr<TypeInfo> TypeInfo::GetCanonicalType(void) const
{
    if (m_canonicalType)
    {
        return m_canonicalType;
    }

    // Return self if no canonical type is set
    return std::make_shared<TypeInfo>(*this);
}

TypeKind TypeInfo::GetKind(void) const
{
    return m_kind;
}

bool TypeInfo::IsConst(void) const
{
    return m_isConst;
}

void TypeInfo::SetDisplayName(const std::string &name)
{
    m_displayName = name;
}

void TypeInfo::SetKind(TypeKind kind)
{
    m_kind = kind;
}

void TypeInfo::SetConst(bool isConst)
{
    m_isConst = isConst;
}

void TypeInfo::SetCanonicalType(std::shared_ptr<TypeInfo> canonical)
{
    m_canonicalType = canonical;
}

void TypeInfo::AddArgument(std::shared_ptr<TypeInfo> argument)
{
    m_arguments.push_back(argument);
}
