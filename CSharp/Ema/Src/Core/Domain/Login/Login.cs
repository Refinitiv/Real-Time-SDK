/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Text;

using LSEG.Ema.Access;


namespace LSEG.Ema.Domain.Login;

/// <summary>
/// Base class for Login domain representation.
/// </summary>
public abstract class Login
{
    internal int m_DomainType;

    internal int m_NameType;

    internal string? m_Name;

    internal bool m_Changed;

    internal bool m_NameSet;

    internal bool m_NameTypeSet;

    internal ElementList? m_ElementList;

    internal StringBuilder m_ToString = new StringBuilder();
}