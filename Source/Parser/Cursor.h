/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** Cursor.h
** --------------------------------------------------------------------------*/

#pragma once

#include "CursorType.h"
#include "ASTNode.h"
#include "ASTTypes.h"
#include <functional>

class ASTNode;

class Cursor
{
public:
    typedef std::vector<Cursor> List;
    typedef std::function<void(const Cursor&, const Cursor&)> Visitor;

    Cursor(ASTNode *node);

    CursorKind GetKind(void) const;

    Cursor GetLexicalParent(void) const;
    Cursor GetTemplateSpecialization(void) const;

    std::string GetSpelling(void) const;
    std::string GetDisplayName(void) const;
    std::string GetMangledName(void) const;
    std::string GetUSR(void) const;

    std::string GetSourceFile(void) const;

    bool IsDefinition(void) const;
    bool IsConst(void) const;
    bool IsStatic(void) const;

    AccessSpecifier GetAccessModifier(void) const;
    StorageClass GetStorageClass(void) const;

    CursorType GetType(void) const;
    CursorType GetReturnType(void) const;
    CursorType GetTypedefType(void) const;

    List GetChildren(void) const;
    void VisitChildren(Visitor visitor, void *data = nullptr);

    unsigned GetHash(void) const;

private:
    ASTNode *m_handle;
};