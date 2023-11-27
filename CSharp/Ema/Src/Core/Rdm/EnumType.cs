/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Text;

using LSEG.Eta.ValueAdd.Common;

namespace LSEG.Ema.Rdm;

/// <summary>
/// A single defined enumerated value.
/// </summary>
public sealed class EnumType
{
    #region Public members

    /// <summary>
    /// The actual value representing the type.
    /// </summary>
    /// <value>the value</value>
    public int Value
    {
        get => m_rsslEnumType.Value;
    }

    /// <summary>
    /// A brief string representation describing what the type means (For example,
    /// this may be an abbreviation of a currency to be displayed to a user).
    /// </summary>
    /// <value>the display</value>
    public string Display
    {
        get => m_rsslEnumType.Display.Data() != null
            ? m_rsslEnumType.Display.ToString()
            : string.Empty;
    }

    /// <summary>
    /// A more elaborate description of what the value means. This information is
    /// typically optional and not displayed.
    /// </summary>
    /// <value>the meaning</value>
    public string Meaning
    {
        get => m_rsslEnumType.Meaning.Data() != null
            ? m_rsslEnumType.Meaning.ToString()
            : string.Empty;
    }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="EnumType"/> object.</returns>
    public override string ToString()
    {
        if (m_ToString == null)
        {
            m_ToString = new StringBuilder(64);
        }
        else
        {
            m_ToString.Clear();
        }

        m_ToString.Append("value=").Append(Value)
            .Append(" display=\"").Append(Display)
            .Append("\" meaning=\"").Append(Meaning).Append('"');

        return m_ToString.ToString();
    }

    #endregion

    #region Implementation details

    private Eta.Codec.IEnumType m_rsslEnumType;
    private StringBuilder m_ToString;
    private LinkedListNode<EnumType> m_Node;

#pragma warning disable CS8618 
    internal EnumType()
#pragma warning restore CS8618
    {
        m_Node = new LinkedListNode<EnumType>(this);
    }

    internal EnumType SetEnumType(Eta.Codec.IEnumType enumType)
    {
        m_rsslEnumType = enumType;
        return this;
    }

    internal Eta.Codec.IEnumType GetEnumType()
    {
        return m_rsslEnumType;
    }

    internal void ReturnToPool(LinkedList<EnumType> pool)
    {
        pool.AddLast(m_Node);
    }

    #endregion
}