/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** TypeInfo.h
** --------------------------------------------------------------------------*/

#pragma once

#include "ASTTypes.h"

#include <string>
#include <memory>

class TypeInfo
{
public:
    TypeInfo(void);
    TypeInfo(const std::string &displayName, TypeKind kind);

    std::string GetDisplayName(void) const;

    int GetArgumentCount(void) const;
    std::shared_ptr<TypeInfo> GetArgument(unsigned index) const;

    std::shared_ptr<TypeInfo> GetCanonicalType(void) const;

    TypeKind GetKind(void) const;

    bool IsConst(void) const;

    void SetDisplayName(const std::string &name);
    void SetKind(TypeKind kind);
    void SetConst(bool isConst);
    void SetCanonicalType(std::shared_ptr<TypeInfo> canonical);
    void AddArgument(std::shared_ptr<TypeInfo> argument);

private:
    std::string m_displayName;
    TypeKind m_kind;
    bool m_isConst;
    std::shared_ptr<TypeInfo> m_canonicalType;
    std::vector<std::shared_ptr<TypeInfo>> m_arguments;
};
