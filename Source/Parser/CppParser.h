/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** CppParser.h
** --------------------------------------------------------------------------*/

#pragma once

#include "ASTNode.h"
#include "TypeInfo.h"
#include "Tokenizer/Tokenizer.h"
#include "Tokenizer/TokenType.h"

#include <string>
#include <vector>
#include <memory>
#include <stack>

class CppParser
{
public:
    typedef std::shared_ptr<ASTNode> ASTNodePtr;
    typedef std::vector<ASTNodePtr> ASTNodeList;

    CppParser(void);
    ~CppParser(void);

    // Parse a C++ source file and return the root AST node
    ASTNodePtr Parse(const std::string &sourceFile);

private:
    // Recursive descent parsing methods
    ASTNodePtr parseTranslationUnit(void);
    ASTNodePtr parseNamespace(void);
    ASTNodePtr parseClass(void);
    ASTNodePtr parseStruct(void);
    ASTNodePtr parseEnum(void);
    ASTNodePtr parseMethod(AccessSpecifier currentAccess);
    ASTNodePtr parseField(AccessSpecifier currentAccess);
    ASTNodePtr parseConstructor(AccessSpecifier currentAccess);
    ASTNodePtr parseFunction(void);
    ASTNodePtr parseGlobal(void);

    // Type parsing
    std::shared_ptr<TypeInfo> parseType(void);
    std::shared_ptr<TypeInfo> parseTemplateType(void);

    // Helper methods
    void parseBaseClasses(ClassNode *classNode);
    void parseClassBody(ClassNode *classNode);
    void parseEnumBody(EnumNode *enumNode);
    void parseMethodParameters(MethodNode *methodNode);
    void parseTemplateParameters(ASTNode *node);

    AccessSpecifier parseAccessSpecifier(void);
    StorageClass parseStorageClass(void);

    bool isTypeKeyword(const std::string &token) const;
    bool isAccessSpecifier(const std::string &token) const;
    bool isStorageClass(const std::string &token) const;
    bool isClassKeyword(const std::string &token) const;

    void skipToSemicolon(void);
    void skipToClosingBrace(void);
    void skipBalancedBraces(void);
    void skipBalancedParens(void);
    void skipBalancedAngles(void);

    std::string readQualifiedName(void);
    std::string readUntil(const std::string &delimiter);

    // Token management
    bool expect(const std::string &token);
    bool match(const std::string &token);
    bool peek(const std::string &token, int offset = 0) const;

    std::string currentToken(void) const;
    std::string nextToken(void);
    void advance(void);
    bool isEOF(void) const;

    // Scope tracking
    struct ScopeContext
    {
        std::string namespaceName;
        std::string className;
        AccessSpecifier currentAccess;
        int braceDepth;

        ScopeContext(void)
            : currentAccess(AccessSpecifier::Public)
            , braceDepth(0) { }
    };

    void pushScope(const std::string &name, bool isClass = false);
    void popScope(void);
    std::string getCurrentNamespace(void) const;
    std::string getCurrentClassName(void) const;
    AccessSpecifier getCurrentAccessSpecifier(void) const;
    void setCurrentAccessSpecifier(AccessSpecifier access);

    std::stack<ScopeContext> m_scopeStack;

    // Tokenization state
    std::vector<std::string> m_tokens;
    size_t m_currentTokenIndex;
    std::string m_sourceFile;
    std::string m_sourceContent;
};
