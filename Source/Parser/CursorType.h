/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** CursorType.h
** --------------------------------------------------------------------------*/

#pragma once

#include "ASTTypes.h"

#include <string>
#include <memory>

class Cursor;
class TypeInfo;

class CursorType
{
public:
    CursorType(TypeInfo* handle);

    std::string GetDisplayName(void) const;

    int GetArgumentCount(void) const;
    CursorType GetArgument(unsigned index) const;

    CursorType GetCanonicalType(void) const;

    Cursor GetDeclaration(void) const;

    TypeKind GetKind(void) const;

    bool IsConst(void) const;

private:
    TypeInfo* m_handle;
};