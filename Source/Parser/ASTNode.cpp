/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** ASTNode.cpp
** --------------------------------------------------------------------------*/

#include "Precompiled.h"

#include "ASTNode.h"

// ASTNode implementation
ASTNode::ASTNode(CursorKind kind, const std::string &spelling, unsigned line, unsigned column)
    : m_kind(kind)
    , m_spelling(spelling)
    , m_line(line)
    , m_column(column)
    , m_parent(nullptr)
{
}

CursorKind ASTNode::GetKind(void) const
{
    return m_kind;
}

std::string ASTNode::GetSpelling(void) const
{
    return m_spelling;
}

unsigned ASTNode::GetLine(void) const
{
    return m_line;
}

unsigned ASTNode::GetColumn(void) const
{
    return m_column;
}

void ASTNode::AddChild(std::shared_ptr<ASTNode> child)
{
    if (child)
    {
        child->SetParent(this);
        m_children.push_back(child);
    }
}

const std::vector<std::shared_ptr<ASTNode>>& ASTNode::GetChildren(void) const
{
    return m_children;
}

void ASTNode::SetParent(ASTNode *parent)
{
    m_parent = parent;
}

ASTNode* ASTNode::GetParent(void) const
{
    return m_parent;
}

// ClassNode implementation
ClassNode::ClassNode(const std::string &name, unsigned line, unsigned column, bool isStruct)
    : ASTNode(isStruct ? CursorKind::StructDecl : CursorKind::ClassDecl, name, line, column)
    , m_accessSpecifier(isStruct ? AccessSpecifier::Public : AccessSpecifier::Private)
    , m_isStruct(isStruct)
{
}

void ClassNode::SetAccessSpecifier(AccessSpecifier access)
{
    m_accessSpecifier = access;
}

AccessSpecifier ClassNode::GetAccessSpecifier(void) const
{
    return m_accessSpecifier;
}

void ClassNode::AddBaseClass(const std::string &baseName, AccessSpecifier access)
{
    m_baseClasses.push_back(std::make_pair(baseName, access));
}

const std::vector<std::pair<std::string, AccessSpecifier>>& ClassNode::GetBaseClasses(void) const
{
    return m_baseClasses;
}

void ClassNode::SetQualifiedName(const std::string &qualifiedName)
{
    m_qualifiedName = qualifiedName;
}

std::string ClassNode::GetQualifiedName(void) const
{
    return m_qualifiedName;
}

bool ClassNode::IsStruct(void) const
{
    return m_isStruct;
}

// MethodNode implementation
MethodNode::MethodNode(const std::string &name, unsigned line, unsigned column)
    : ASTNode(CursorKind::CXXMethod, name, line, column)
    , m_accessSpecifier(AccessSpecifier::Private)
    , m_isConst(false)
    , m_isStatic(false)
    , m_isVirtual(false)
{
}

void MethodNode::SetReturnType(const std::string &returnType)
{
    m_returnType = returnType;
}

std::string MethodNode::GetReturnType(void) const
{
    return m_returnType;
}

void MethodNode::SetAccessSpecifier(AccessSpecifier access)
{
    m_accessSpecifier = access;
}

AccessSpecifier MethodNode::GetAccessSpecifier(void) const
{
    return m_accessSpecifier;
}

void MethodNode::SetIsConst(bool isConst)
{
    m_isConst = isConst;
}

bool MethodNode::IsConst(void) const
{
    return m_isConst;
}

void MethodNode::SetIsStatic(bool isStatic)
{
    m_isStatic = isStatic;
}

bool MethodNode::IsStatic(void) const
{
    return m_isStatic;
}

void MethodNode::SetIsVirtual(bool isVirtual)
{
    m_isVirtual = isVirtual;
}

bool MethodNode::IsVirtual(void) const
{
    return m_isVirtual;
}

void MethodNode::AddParameter(const std::string &type, const std::string &name)
{
    m_parameters.push_back(std::make_pair(type, name));
}

const std::vector<std::pair<std::string, std::string>>& MethodNode::GetParameters(void) const
{
    return m_parameters;
}

// FieldNode implementation
FieldNode::FieldNode(const std::string &name, unsigned line, unsigned column)
    : ASTNode(CursorKind::FieldDecl, name, line, column)
    , m_accessSpecifier(AccessSpecifier::Private)
    , m_isStatic(false)
    , m_isConst(false)
    , m_storageClass(StorageClass::None)
{
}

void FieldNode::SetType(const std::string &type)
{
    m_type = type;
}

std::string FieldNode::GetType(void) const
{
    return m_type;
}

void FieldNode::SetAccessSpecifier(AccessSpecifier access)
{
    m_accessSpecifier = access;
}

AccessSpecifier FieldNode::GetAccessSpecifier(void) const
{
    return m_accessSpecifier;
}

void FieldNode::SetIsStatic(bool isStatic)
{
    m_isStatic = isStatic;
}

bool FieldNode::IsStatic(void) const
{
    return m_isStatic;
}

void FieldNode::SetIsConst(bool isConst)
{
    m_isConst = isConst;
}

bool FieldNode::IsConst(void) const
{
    return m_isConst;
}

void FieldNode::SetStorageClass(StorageClass storage)
{
    m_storageClass = storage;
}

StorageClass FieldNode::GetStorageClass(void) const
{
    return m_storageClass;
}

// EnumNode implementation
EnumNode::EnumNode(const std::string &name, unsigned line, unsigned column, bool isEnumClass)
    : ASTNode(CursorKind::EnumDecl, name, line, column)
    , m_accessSpecifier(AccessSpecifier::Public)
    , m_isEnumClass(isEnumClass)
{
}

void EnumNode::SetAccessSpecifier(AccessSpecifier access)
{
    m_accessSpecifier = access;
}

AccessSpecifier EnumNode::GetAccessSpecifier(void) const
{
    return m_accessSpecifier;
}

void EnumNode::AddEnumValue(const std::string &key, const std::string &value)
{
    m_enumValues.push_back(std::make_pair(key, value));
}

const std::vector<std::pair<std::string, std::string>>& EnumNode::GetEnumValues(void) const
{
    return m_enumValues;
}

void EnumNode::SetQualifiedName(const std::string &qualifiedName)
{
    m_qualifiedName = qualifiedName;
}

std::string EnumNode::GetQualifiedName(void) const
{
    return m_qualifiedName;
}

bool EnumNode::IsEnumClass(void) const
{
    return m_isEnumClass;
}
