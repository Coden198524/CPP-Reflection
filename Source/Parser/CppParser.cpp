/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** CppParser.cpp
** --------------------------------------------------------------------------*/

#include "CppParser.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

CppParser::CppParser(void)
    : m_currentTokenIndex(0)
{
}

CppParser::~CppParser(void)
{
}

CppParser::ASTNodePtr CppParser::Parse(const std::string &sourceFile)
{
    m_sourceFile = sourceFile;
    m_currentTokenIndex = 0;
    m_tokens.clear();

    // Read source file
    std::ifstream file(sourceFile);
    if (!file.is_open())
    {
        return nullptr;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    m_sourceContent = buffer.str();
    file.close();

    // TODO: Implement proper tokenization using existing Tokenizer infrastructure
    // For now, use simple whitespace-based tokenization as placeholder
    // This will be replaced with proper lexical analysis

    return parseTranslationUnit();
}

CppParser::ASTNodePtr CppParser::parseTranslationUnit(void)
{
    auto root = std::make_shared<ASTNode>();
    root->kind = CursorKind::TranslationUnit;
    root->name = m_sourceFile;

    // Parse top-level declarations
    while (!isEOF())
    {
        std::string token = currentToken();

        if (token == "namespace")
        {
            auto ns = parseNamespace();
            if (ns)
                root->children.push_back(ns);
        }
        else if (token == "class")
        {
            auto cls = parseClass();
            if (cls)
                root->children.push_back(cls);
        }
        else if (token == "struct")
        {
            auto st = parseStruct();
            if (st)
                root->children.push_back(st);
        }
        else if (token == "enum")
        {
            auto en = parseEnum();
            if (en)
                root->children.push_back(en);
        }
        else
        {
            // Skip unknown tokens for now
            advance();
        }
    }

    return root;
}

CppParser::ASTNodePtr CppParser::parseNamespace(void)
{
    if (!expect("namespace"))
        return nullptr;

    auto node = std::make_shared<ASTNode>();
    node->kind = CursorKind::Namespace;
    node->name = nextToken();

    pushScope(node->name, false);

    if (expect("{"))
    {
        // Parse namespace body
        while (!isEOF() && !peek("}"))
        {
            std::string token = currentToken();

            if (token == "namespace")
            {
                auto ns = parseNamespace();
                if (ns)
                    node->children.push_back(ns);
            }
            else if (token == "class")
            {
                auto cls = parseClass();
                if (cls)
                    node->children.push_back(cls);
            }
            else if (token == "struct")
            {
                auto st = parseStruct();
                if (st)
                    node->children.push_back(st);
            }
            else if (token == "enum")
            {
                auto en = parseEnum();
                if (en)
                    node->children.push_back(en);
            }
            else
            {
                advance();
            }
        }

        expect("}");
    }

    popScope();

    return node;
}

CppParser::ASTNodePtr CppParser::parseClass(void)
{
    if (!expect("class"))
        return nullptr;

    auto classNode = std::make_shared<ClassNode>();
    classNode->kind = CursorKind::ClassDecl;
    classNode->name = nextToken();
    classNode->accessModifier = AccessSpecifier::Private; // default for class

    pushScope(classNode->name, true);

    // Parse base classes
    if (peek(":"))
    {
        parseBaseClasses(classNode.get());
    }

    // Parse class body
    if (expect("{"))
    {
        parseClassBody(classNode.get());
        expect("}");
        expect(";");
    }

    popScope();

    return classNode;
}

CppParser::ASTNodePtr CppParser::parseEnum(void)
{
    if (!expect("enum"))
        return nullptr;

    // Check for enum class or enum struct
    bool isEnumClass = false;
    if (match("class") || match("struct"))
    {
        isEnumClass = true;
        advance();
    }

    // Get enum name
    std::string enumName = nextToken();

    // Create enum node with proper constructor
    auto enumNode = std::make_shared<EnumNode>(enumName, 0, 0, isEnumClass);

    // Set qualified name based on current namespace
    std::string qualifiedName = getCurrentNamespace();
    if (!qualifiedName.empty())
        qualifiedName += "::" + enumName;
    else
        qualifiedName = enumName;

    enumNode->SetQualifiedName(qualifiedName);

    // Parse underlying type if specified (e.g., enum class Foo : int)
    if (peek(":"))
    {
        advance();
        // Skip underlying type for now - parseType() would handle this
        // but we don't store it in EnumNode currently
        auto underlyingType = parseType();
    }

    // Parse enum body
    if (expect("{"))
    {
        parseEnumBody(enumNode.get());
        expect("}");
        expect(";");
    }

    return enumNode;
}

CppParser::ASTNodePtr CppParser::parseStruct(void)
{
    if (!expect("struct"))
        return nullptr;

    auto classNode = std::make_shared<ClassNode>();
    classNode->kind = CursorKind::StructDecl;
    classNode->name = nextToken();
    classNode->accessModifier = AccessSpecifier::Public; // default for struct

    pushScope(classNode->name, true);

    // Parse base classes
    if (peek(":"))
    {
        parseBaseClasses(classNode.get());
    }

    // Parse class body
    if (expect("{"))
    {
        parseClassBody(classNode.get());
        expect("}");
        expect(";");
    }

    popScope();

    return classNode;
}

CppParser::ASTNodePtr CppParser::parseMethod(AccessSpecifier currentAccess)
{
    auto methodNode = std::make_shared<MethodNode>();
    methodNode->kind = CursorKind::CXXMethod;
    methodNode->accessModifier = currentAccess;

    while (isStorageClass(currentToken()))
    {
        if (currentToken() == "static")
            methodNode->isStatic = true;
        else if (currentToken() == "virtual")
            methodNode->isVirtual = true;
        else if (currentToken() == "inline")
            methodNode->isInline = true;

        advance();
    }

    methodNode->returnType = parseType();
    methodNode->name = nextToken();

    if (expect("("))
    {
        parseMethodParameters(methodNode.get());
        expect(")");
    }

    if (match("const"))
    {
        methodNode->isConst = true;
        advance();
    }

    if (match("override"))
    {
        methodNode->isOverride = true;
        advance();
    }

    if (match("final"))
    {
        methodNode->isFinal = true;
        advance();
    }

    if (match("="))
    {
        advance();
        if (match("0"))
        {
            methodNode->isPureVirtual = true;
            advance();
        }
        expect(";");
    }
    else if (match("{"))
    {
        skipBalancedBraces();
    }
    else
    {
        expect(";");
    }

    return methodNode;
}

CppParser::ASTNodePtr CppParser::parseField(AccessSpecifier currentAccess)
{
    auto fieldNode = std::make_shared<FieldNode>();
    fieldNode->kind = CursorKind::FieldDecl;
    fieldNode->accessModifier = currentAccess;

    if (match("static"))
    {
        fieldNode->storageClass = StorageClass::Static;
        advance();
    }
    else if (match("mutable"))
    {
        fieldNode->storageClass = StorageClass::Mutable;
        advance();
    }

    fieldNode->type = parseType();
    fieldNode->name = nextToken();

    if (match("["))
    {
        skipBalancedAngles();
    }

    if (match("="))
    {
        advance();
        skipToSemicolon();
    }
    else
    {
        expect(";");
    }

    return fieldNode;
}

CppParser::ASTNodePtr CppParser::parseConstructor(AccessSpecifier currentAccess)
{
    auto ctorNode = std::make_shared<MethodNode>();
    ctorNode->kind = CursorKind::Constructor;
    ctorNode->accessModifier = currentAccess;
    ctorNode->name = nextToken();

    if (expect("("))
    {
        parseMethodParameters(ctorNode.get());
        expect(")");
    }

    if (match(":"))
    {
        advance();
        while (!match("{") && !match(";") && !isEOF())
        {
            advance();
        }
    }

    if (match("{"))
    {
        skipBalancedBraces();
    }
    else
    {
        expect(";");
    }

    return ctorNode;
}

CppParser::ASTNodePtr CppParser::parseFunction(void)
{
    auto funcNode = std::make_shared<MethodNode>();
    funcNode->kind = CursorKind::FunctionDecl;

    while (isStorageClass(currentToken()))
    {
        if (currentToken() == "static")
            funcNode->isStatic = true;
        else if (currentToken() == "inline")
            funcNode->isInline = true;

        advance();
    }

    funcNode->returnType = parseType();
    funcNode->name = nextToken();

    if (expect("("))
    {
        parseMethodParameters(funcNode.get());
        expect(")");
    }

    if (match("{"))
    {
        skipBalancedBraces();
    }
    else
    {
        expect(";");
    }

    return funcNode;
}

CppParser::ASTNodePtr CppParser::parseGlobal(void)
{
    auto globalNode = std::make_shared<FieldNode>();
    globalNode->kind = CursorKind::VarDecl;

    if (match("static"))
    {
        globalNode->storageClass = StorageClass::Static;
        advance();
    }
    else if (match("extern"))
    {
        globalNode->storageClass = StorageClass::Extern;
        advance();
    }

    globalNode->type = parseType();
    globalNode->name = nextToken();

    if (match("="))
    {
        advance();
        skipToSemicolon();
    }
    else
    {
        expect(";");
    }

    return globalNode;
}

std::shared_ptr<TypeInfo> CppParser::parseType(void)
{
    auto typeInfo = std::make_shared<TypeInfo>();
    std::string typeName;

    while (match("const") || match("volatile"))
    {
        if (match("const"))
            typeInfo->SetIsConst(true);
        typeName += currentToken() + " ";
        advance();
    }

    if (match("template") || peek("<"))
    {
        return parseTemplateType();
    }

    typeName += readQualifiedName();

    while (match("*") || match("&") || match("&&"))
    {
        typeName += currentToken();
        advance();
    }

    typeInfo->SetDisplayName(typeName);
    typeInfo->SetKind(TypeKind::Unexposed);

    return typeInfo;
}

std::shared_ptr<TypeInfo> CppParser::parseTemplateType(void)
{
    auto typeInfo = std::make_shared<TypeInfo>();
    std::string typeName = readQualifiedName();

    if (expect("<"))
    {
        typeName += "<";
        int depth = 1;

        while (depth > 0 && !isEOF())
        {
            std::string token = currentToken();
            if (token == "<")
                depth++;
            else if (token == ">")
                depth--;

            typeName += token;
            advance();
        }
    }

    typeInfo->SetDisplayName(typeName);
    typeInfo->SetKind(TypeKind::Unexposed);

    return typeInfo;
}

void CppParser::parseBaseClasses(ClassNode *classNode)
{
    if (!expect(":"))
        return;

    while (!peek("{") && !isEOF())
    {
        if (isAccessSpecifier(currentToken()))
        {
            advance();
        }

        std::string baseName = readQualifiedName();
        classNode->baseClasses.push_back(baseName);

        if (match(","))
            advance();
        else
            break;
    }
}

void CppParser::parseClassBody(ClassNode *classNode)
{
    AccessSpecifier currentAccess = classNode->accessModifier;

    while (!peek("}") && !isEOF())
    {
        std::string token = currentToken();

        // Handle access specifiers
        if (isAccessSpecifier(token))
        {
            currentAccess = parseAccessSpecifier();
            expect(":");
            continue;
        }

        // Handle nested class/struct
        if (token == "class" || token == "struct")
        {
            auto nested = (token == "class") ? parseClass() : parseStruct();
            if (nested)
                classNode->children.push_back(nested);
            continue;
        }

        // Handle nested enum
        if (token == "enum")
        {
            auto enumNode = parseEnum();
            if (enumNode)
                classNode->children.push_back(enumNode);
            continue;
        }

        // Check if this is a constructor (name matches class name)
        if (token == classNode->name)
        {
            auto ctorNode = parseConstructor(currentAccess);
            if (ctorNode)
                classNode->children.push_back(ctorNode);
            continue;
        }

        // Check if this is a destructor
        if (token == "~" && peek(classNode->name, 1))
        {
            advance(); // skip ~
            auto dtorNode = parseConstructor(currentAccess);
            if (dtorNode)
            {
                dtorNode->kind = CursorKind::Destructor;
                classNode->children.push_back(dtorNode);
            }
            continue;
        }

        // Check for storage class specifiers
        bool isStatic = false;
        bool isVirtual = false;
        bool isMutable = false;

        while (isStorageClass(token) || token == "virtual" || token == "inline" || token == "explicit")
        {
            if (token == "static")
                isStatic = true;
            else if (token == "virtual")
                isVirtual = true;
            else if (token == "mutable")
                isMutable = true;

            advance();
            token = currentToken();
        }

        // Try to determine if this is a method or field
        // Look ahead to see if there's a function signature
        int lookahead = 0;
        bool isMethod = false;

        // Skip type tokens to find potential function name and parentheses
        while (lookahead < 10)
        {
            size_t index = m_currentTokenIndex + lookahead;
            if (index >= m_tokens.size())
                break;

            std::string ahead = m_tokens[index];
            if (ahead == "(")
            {
                isMethod = true;
                break;
            }
            else if (ahead == ";" || ahead == "=" || ahead == "{" || ahead == "}")
            {
                break;
            }
            lookahead++;
        }

        if (isMethod)
        {
            // Parse as method
            auto methodNode = parseMethod(currentAccess);
            if (methodNode)
            {
                if (isStatic)
                {
                    methodNode->kind = CursorKind::CXXMethod;
                    methodNode->isStatic = true;
                }
                classNode->children.push_back(methodNode);
            }
        }
        else
        {
            // Parse as field
            auto fieldNode = parseField(currentAccess);
            if (fieldNode)
            {
                if (isStatic)
                {
                    fieldNode->kind = CursorKind::VarDecl;
                    fieldNode->storageClass = StorageClass::Static;
                }
                else if (isMutable)
                {
                    fieldNode->storageClass = StorageClass::Mutable;
                }
                classNode->children.push_back(fieldNode);
            }
        }
    }
}

void CppParser::parseEnumBody(EnumNode *enumNode)
{
    while (!peek("}") && !isEOF())
    {
        std::string key = currentToken();
        std::string value;

        advance();

        // Check if there's an explicit value assignment
        if (match("="))
        {
            advance();
            // Collect the value expression until we hit a comma or closing brace
            std::string valueExpr;
            while (!match(",") && !peek("}") && !isEOF())
            {
                if (!valueExpr.empty())
                    valueExpr += " ";
                valueExpr += currentToken();
                advance();
            }
            value = valueExpr;
        }
        else
        {
            // No explicit value, use qualified enum value name
            std::string qualifiedName = enumNode->GetQualifiedName();
            if (!qualifiedName.empty())
                value = qualifiedName + "::" + key;
            else
                value = key;
        }

        // Add the enum value using the proper API
        enumNode->AddEnumValue(key, value);

        // Skip comma if present
        if (match(","))
            advance();
    }
}

void CppParser::parseMethodParameters(MethodNode *methodNode)
{
    while (!peek(")") && !isEOF())
    {
        auto paramType = parseType();
        std::string paramName;

        if (!peek(")") && !match(","))
        {
            paramName = currentToken();
            advance();
        }

        if (match("="))
        {
            advance();
            while (!match(",") && !peek(")") && !isEOF())
            {
                advance();
            }
        }

        if (match(","))
            advance();
    }
}

void CppParser::parseTemplateParameters(ASTNode *node)
{
    if (!expect("<"))
        return;

    int depth = 1;
    while (depth > 0 && !isEOF())
    {
        std::string token = currentToken();
        if (token == "<")
            depth++;
        else if (token == ">")
            depth--;

        advance();
    }
}

AccessSpecifier CppParser::parseAccessSpecifier(void)
{
    std::string token = currentToken();
    advance();

    if (token == "public")
        return AccessSpecifier::Public;
    else if (token == "protected")
        return AccessSpecifier::Protected;
    else if (token == "private")
        return AccessSpecifier::Private;

    return AccessSpecifier::Public;
}

StorageClass CppParser::parseStorageClass(void)
{
    std::string token = currentToken();
    advance();

    if (token == "static")
        return StorageClass::Static;
    else if (token == "extern")
        return StorageClass::Extern;
    else if (token == "mutable")
        return StorageClass::Mutable;

    return StorageClass::None;
}

bool CppParser::isTypeKeyword(const std::string &token) const
{
    return token == "void" || token == "bool" || token == "char" ||
           token == "int" || token == "float" || token == "double" ||
           token == "short" || token == "long" || token == "signed" ||
           token == "unsigned" || token == "wchar_t" || token == "char16_t" ||
           token == "char32_t";
}

bool CppParser::isAccessSpecifier(const std::string &token) const
{
    return token == "public" || token == "protected" || token == "private";
}

bool CppParser::isStorageClass(const std::string &token) const
{
    return token == "static" || token == "extern" || token == "mutable" ||
           token == "virtual" || token == "inline";
}

bool CppParser::isClassKeyword(const std::string &token) const
{
    return token == "class" || token == "struct" || token == "union";
}

void CppParser::skipToSemicolon(void)
{
    while (!match(";") && !isEOF())
    {
        advance();
    }
    if (match(";"))
        advance();
}

void CppParser::skipToClosingBrace(void)
{
    int depth = 1;
    while (depth > 0 && !isEOF())
    {
        if (match("{"))
            depth++;
        else if (match("}"))
            depth--;
        advance();
    }
}

void CppParser::skipBalancedBraces(void)
{
    if (!expect("{"))
        return;

    int depth = 1;
    while (depth > 0 && !isEOF())
    {
        if (match("{"))
            depth++;
        else if (match("}"))
            depth--;
        advance();
    }
}

void CppParser::skipBalancedParens(void)
{
    if (!expect("("))
        return;

    int depth = 1;
    while (depth > 0 && !isEOF())
    {
        if (match("("))
            depth++;
        else if (match(")"))
            depth--;
        advance();
    }
}

void CppParser::skipBalancedAngles(void)
{
    if (!expect("<"))
        return;

    int depth = 1;
    while (depth > 0 && !isEOF())
    {
        if (match("<"))
            depth++;
        else if (match(">"))
            depth--;
        advance();
    }
}

std::string CppParser::readQualifiedName(void)
{
    std::string name = currentToken();
    advance();

    while (match("::"))
    {
        name += "::";
        advance();
        name += currentToken();
        advance();
    }

    return name;
}

std::string CppParser::readUntil(const std::string &delimiter)
{
    std::string result;

    while (!match(delimiter) && !isEOF())
    {
        result += currentToken() + " ";
        advance();
    }

    return result;
}

bool CppParser::expect(const std::string &token)
{
    if (match(token))
    {
        advance();
        return true;
    }
    return false;
}

bool CppParser::match(const std::string &token)
{
    return currentToken() == token;
}

bool CppParser::peek(const std::string &token, int offset) const
{
    size_t index = m_currentTokenIndex + offset;
    if (index < m_tokens.size())
        return m_tokens[index] == token;
    return false;
}

std::string CppParser::currentToken(void) const
{
    if (m_currentTokenIndex < m_tokens.size())
        return m_tokens[m_currentTokenIndex];
    return "";
}

std::string CppParser::nextToken(void)
{
    std::string token = currentToken();
    advance();
    return token;
}

void CppParser::advance(void)
{
    if (m_currentTokenIndex < m_tokens.size())
        m_currentTokenIndex++;
}

bool CppParser::isEOF(void) const
{
    return m_currentTokenIndex >= m_tokens.size();
}

void CppParser::pushScope(const std::string &name, bool isClass)
{
    ScopeContext ctx;
    ctx.namespaceName = name;
    if (isClass)
        ctx.className = name;
    m_scopeStack.push(ctx);
}

void CppParser::popScope(void)
{
    if (!m_scopeStack.empty())
        m_scopeStack.pop();
}

std::string CppParser::getCurrentNamespace(void) const
{
    std::string ns;
    std::stack<ScopeContext> temp = m_scopeStack;

    while (!temp.empty())
    {
        if (!temp.top().namespaceName.empty())
        {
            if (!ns.empty())
                ns = temp.top().namespaceName + "::" + ns;
            else
                ns = temp.top().namespaceName;
        }
        temp.pop();
    }

    return ns;
}
