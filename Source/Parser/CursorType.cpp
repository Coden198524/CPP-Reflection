/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** CursorType.cpp
** --------------------------------------------------------------------------*/

#include "Precompiled.h"

#include "CursorType.h"
#include "TypeInfo.h"
#include "Cursor.h"

CursorType::CursorType(TypeInfo* handle)
    : m_handle( handle )
{

}

std::string CursorType::GetDisplayName(void) const
{
    if (!m_handle)
        return "";

    return m_handle->GetDisplayName();
}

int CursorType::GetArgumentCount(void) const
{
    if (!m_handle)
        return 0;

    return m_handle->GetArgumentCount();
}

CursorType CursorType::GetArgument(unsigned index) const
{
    if (!m_handle)
        return CursorType(nullptr);

    auto arg = m_handle->GetArgument(index);
    return CursorType(arg.get());
}

CursorType CursorType::GetCanonicalType(void) const
{
    if (!m_handle)
        return CursorType(nullptr);

    auto canonical = m_handle->GetCanonicalType();
    return CursorType(canonical.get());
}

Cursor CursorType::GetDeclaration(void) const
{
    // TODO: Implement type-to-cursor mapping
    return Cursor(nullptr);
}

TypeKind CursorType::GetKind(void) const
{
    if (!m_handle)
        return TypeKind::Invalid;

    return m_handle->GetKind();
}

bool CursorType::IsConst(void) const
{
    if (!m_handle)
        return false;

    return m_handle->IsConst();
}