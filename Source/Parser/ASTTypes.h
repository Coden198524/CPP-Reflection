/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** ASTTypes.h
** --------------------------------------------------------------------------*/

#pragma once

// Custom enums to replace Clang/LLVM types

enum class CursorKind
{
    Unknown,

    // Declarations
    StructDecl,
    ClassDecl,
    EnumDecl,
    FieldDecl,
    EnumConstantDecl,
    FunctionDecl,
    VarDecl,
    ParmDecl,
    TypedefDecl,
    CXXMethod,
    Namespace,
    Constructor,
    Destructor,
    ConversionFunction,
    TemplateTypeParameter,
    NonTypeTemplateParameter,
    TemplateTemplateParameter,
    FunctionTemplate,
    ClassTemplate,
    ClassTemplatePartialSpecialization,
    NamespaceAlias,
    UsingDirective,
    UsingDeclaration,
    TypeAliasDecl,
    CXXAccessSpecifier,

    // References
    TypeRef,
    CXXBaseSpecifier,
    TemplateRef,
    NamespaceRef,
    MemberRef,
    OverloadedDeclRef,
    VariableRef,

    // Expressions
    IntegerLiteral,
    FloatingLiteral,
    StringLiteral,
    CharacterLiteral,

    // Statements
    CompoundStmt,

    // Preprocessing
    MacroDefinition,
    MacroExpansion,
    InclusionDirective,

    // Other
    TranslationUnit,
    UnexposedDecl,
    UnexposedExpr,
    UnexposedStmt
};

enum class AccessSpecifier
{
    Invalid,
    Public,
    Protected,
    Private
};

enum class StorageClass
{
    Invalid,
    None,
    Extern,
    Static,
    PrivateExtern,
    Auto,
    Register
};

enum class TypeKind
{
    Invalid,
    Unexposed,
    Void,
    Bool,
    Char_U,
    UChar,
    Char16,
    Char32,
    UShort,
    UInt,
    ULong,
    ULongLong,
    UInt128,
    Char_S,
    SChar,
    WChar,
    Short,
    Int,
    Long,
    LongLong,
    Int128,
    Float,
    Double,
    LongDouble,
    NullPtr,
    Overload,
    Dependent,
    ObjCId,
    ObjCClass,
    ObjCSel,
    Complex,
    Pointer,
    BlockPointer,
    LValueReference,
    RValueReference,
    Record,
    Enum,
    Typedef,
    FunctionNoProto,
    FunctionProto,
    ConstantArray,
    Vector,
    IncompleteArray,
    VariableArray,
    DependentSizedArray,
    MemberPointer,
    Auto,
    Elaborated
};
