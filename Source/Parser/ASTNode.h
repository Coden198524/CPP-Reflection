/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** ASTNode.h
** --------------------------------------------------------------------------*/

#pragma once

#include "ASTTypes.h"
#include <string>
#include <vector>
#include <memory>

// Forward declarations
class ASTNode;

// Base class for all AST nodes
class ASTNode
{
public:
    ASTNode(CursorKind kind, const std::string &spelling, unsigned line, unsigned column);
    virtual ~ASTNode(void) { }

    CursorKind GetKind(void) const;
    std::string GetSpelling(void) const;
    unsigned GetLine(void) const;
    unsigned GetColumn(void) const;

    void AddChild(std::shared_ptr<ASTNode> child);
    const std::vector<std::shared_ptr<ASTNode>>& GetChildren(void) const;

    void SetParent(ASTNode *parent);
    ASTNode* GetParent(void) const;

    void SetSourceFile(const std::string &sourceFile);
    std::string GetSourceFile(void) const;

    // Public members for direct access (used by CppParser)
    CursorKind kind;
    std::string name;
    std::vector<std::shared_ptr<ASTNode>> children;

protected:
    std::string m_spelling;
    unsigned m_line;
    unsigned m_column;
    std::string m_sourceFile;

    ASTNode *m_parent;
};

// Class/Struct declaration node
class ClassNode : public ASTNode
{
public:
    ClassNode(const std::string &name, unsigned line, unsigned column, bool isStruct = false);
    virtual ~ClassNode(void) { }

    void SetAccessSpecifier(AccessSpecifier access);
    AccessSpecifier GetAccessSpecifier(void) const;

    void AddBaseClass(const std::string &baseName, AccessSpecifier access);
    const std::vector<std::pair<std::string, AccessSpecifier>>& GetBaseClasses(void) const;

    void SetQualifiedName(const std::string &qualifiedName);
    std::string GetQualifiedName(void) const;

    bool IsStruct(void) const;

private:
    AccessSpecifier m_accessSpecifier;
    std::vector<std::pair<std::string, AccessSpecifier>> m_baseClasses;
    std::string m_qualifiedName;
    bool m_isStruct;
};

// Method/Function declaration node
class MethodNode : public ASTNode
{
public:
    MethodNode(const std::string &name, unsigned line, unsigned column);
    virtual ~MethodNode(void) { }

    void SetReturnType(const std::string &returnType);
    std::string GetReturnType(void) const;

    void SetAccessSpecifier(AccessSpecifier access);
    AccessSpecifier GetAccessSpecifier(void) const;

    void SetIsConst(bool isConst);
    bool IsConst(void) const;

    void SetIsStatic(bool isStatic);
    bool IsStatic(void) const;

    void SetIsVirtual(bool isVirtual);
    bool IsVirtual(void) const;

    void AddParameter(const std::string &type, const std::string &name);
    const std::vector<std::pair<std::string, std::string>>& GetParameters(void) const;

private:
    std::string m_returnType;
    AccessSpecifier m_accessSpecifier;
    bool m_isConst;
    bool m_isStatic;
    bool m_isVirtual;
    std::vector<std::pair<std::string, std::string>> m_parameters; // type, name pairs
};

// Field/Member variable declaration node
class FieldNode : public ASTNode
{
public:
    FieldNode(const std::string &name, unsigned line, unsigned column);
    virtual ~FieldNode(void) { }

    void SetType(const std::string &type);
    std::string GetType(void) const;

    void SetAccessSpecifier(AccessSpecifier access);
    AccessSpecifier GetAccessSpecifier(void) const;

    void SetIsStatic(bool isStatic);
    bool IsStatic(void) const;

    void SetIsConst(bool isConst);
    bool IsConst(void) const;

    void SetStorageClass(StorageClass storage);
    StorageClass GetStorageClass(void) const;

private:
    std::string m_type;
    AccessSpecifier m_accessSpecifier;
    bool m_isStatic;
    bool m_isConst;
    StorageClass m_storageClass;
};

// Enum declaration node
class EnumNode : public ASTNode
{
public:
    EnumNode(const std::string &name, unsigned line, unsigned column, bool isEnumClass = false);
    virtual ~EnumNode(void) { }

    void SetAccessSpecifier(AccessSpecifier access);
    AccessSpecifier GetAccessSpecifier(void) const;

    void AddEnumValue(const std::string &key, const std::string &value);
    const std::vector<std::pair<std::string, std::string>>& GetEnumValues(void) const;

    void SetQualifiedName(const std::string &qualifiedName);
    std::string GetQualifiedName(void) const;

    bool IsEnumClass(void) const;

private:
    AccessSpecifier m_accessSpecifier;
    std::vector<std::pair<std::string, std::string>> m_enumValues; // key, value pairs
    std::string m_qualifiedName;
    bool m_isEnumClass;
};
