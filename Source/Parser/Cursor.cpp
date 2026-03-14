/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** Cursor.cpp
** --------------------------------------------------------------------------*/

#include "Precompiled.h"

#include "Cursor.h"

#include "MetaUtils.h"

Cursor::Cursor(ASTNode *node)
    : m_handle( node ) { }

CursorKind Cursor::GetKind(void) const
{
    if (!m_handle)
        return CursorKind::Unknown;

    return m_handle->GetKind();
}

Cursor Cursor::GetLexicalParent(void) const
{
    if (!m_handle)
        return Cursor(nullptr);

    return Cursor(m_handle->GetParent());
}

Cursor Cursor::GetTemplateSpecialization(void) const
{
    // Template specialization not yet implemented in AST
    return Cursor(nullptr);
}

std::string Cursor::GetSpelling(void) const
{
    if (!m_handle)
        return "";

    return m_handle->GetSpelling();
}

std::string Cursor::GetDisplayName(void) const
{
    if (!m_handle)
        return "";

    return m_handle->GetSpelling();
}

std::string Cursor::GetMangledName(void) const
{
    // Mangled names not yet implemented in AST
    return "";
}

std::string Cursor::GetUSR(void) const
{
    // USR not yet implemented in AST
    return "";
}

std::string Cursor::GetSourceFile(void) const
{
    if (!m_handle)
        return "";

    return m_handle->GetSourceFile();
}

bool Cursor::IsDefinition(void) const
{
    // For now, assume all nodes are definitions
    return m_handle != nullptr;
}

bool Cursor::IsConst(void) const
{
    if (!m_handle)
        return false;

    if (auto method = dynamic_cast<MethodNode*>(m_handle))
        return method->IsConst();

    if (auto field = dynamic_cast<FieldNode*>(m_handle))
        return field->IsConst();

    return false;
}

bool Cursor::IsStatic(void) const
{
    if (!m_handle)
        return false;

    if (auto method = dynamic_cast<MethodNode*>(m_handle))
        return method->IsStatic();

    if (auto field = dynamic_cast<FieldNode*>(m_handle))
        return field->IsStatic();

    return false;
}

AccessSpecifier Cursor::GetAccessModifier(void) const
{
    if (!m_handle)
        return AccessSpecifier::Invalid;

    if (auto classNode = dynamic_cast<ClassNode*>(m_handle))
        return classNode->GetAccessSpecifier();

    if (auto method = dynamic_cast<MethodNode*>(m_handle))
        return method->GetAccessSpecifier();

    if (auto field = dynamic_cast<FieldNode*>(m_handle))
        return field->GetAccessSpecifier();

    if (auto enumNode = dynamic_cast<EnumNode*>(m_handle))
        return enumNode->GetAccessSpecifier();

    return AccessSpecifier::Invalid;
}

StorageClass Cursor::GetStorageClass(void) const
{
    if (!m_handle)
        return StorageClass::Invalid;

    if (auto field = dynamic_cast<FieldNode*>(m_handle))
        return field->GetStorageClass();

    return StorageClass::None;
}

CursorType Cursor::GetType(void) const
{
    // Type information not yet fully implemented
    // Return a placeholder CursorType
    return CursorType(CXType());
}

CursorType Cursor::GetReturnType(void) const
{
    // Return type information not yet fully implemented
    return CursorType(CXType());
}

CursorType Cursor::GetTypedefType(void) const
{
    // Typedef type information not yet fully implemented
    return CursorType(CXType());
}

Cursor::List Cursor::GetChildren(void) const
{
    List children;

    if (!m_handle)
        return children;

    const auto& astChildren = m_handle->GetChildren();
    for (const auto& child : astChildren)
    {
        children.emplace_back(child.get());
    }

    return children;
}

void Cursor::VisitChildren(Visitor visitor, void *data)
{
    if (!m_handle || !visitor)
        return;

    const auto& children = m_handle->GetChildren();
    for (const auto& child : children)
    {
        Cursor childCursor(child.get());
        Cursor parentCursor(m_handle);
        visitor(childCursor, parentCursor);
    }
}

unsigned Cursor::GetHash(void) const
{
    // Simple hash based on pointer address
    return static_cast<unsigned>(reinterpret_cast<uintptr_t>(m_handle));
}