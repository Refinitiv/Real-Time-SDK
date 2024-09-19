/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Text;

namespace LSEG.Ema.Rdm;

/// <summary>
/// A table of enumerated types.  A field that uses this table will contain a value
/// corresponding to an enumerated type in this table.
/// </summary>
public sealed class EnumTypeTable
{
    #region Public members

    /// <summary>
    /// Returns the list of EnumType that is belonged to this EnumTypeTable.
    /// </summary>
    /// <value>the list of EnumType</value>
    public List<EnumType> EnumTypes
    {
        get
        {
            if (m_EnumTypeList == null)
            {
                m_EnumTypeList = new(m_RsslEnumTypeTable.MaxValue);
            }
            else if (m_RefreshEnumTypeList)
            {
                ClearEnumTypeList();
            }
            else
            {
                return m_EnumTypeList;
            }

            Eta.Codec.IEnumType rsslEnumType;

            for (int index = 0; index <= m_RsslEnumTypeTable.MaxValue; index++)
            {
                rsslEnumType = m_RsslEnumTypeTable.EnumTypes[index];

                if (rsslEnumType != null)
                {
                    m_EnumTypeList.Add(GetEnumType(rsslEnumType));
                }
            }

            m_RefreshEnumTypeList = false;

            return m_EnumTypeList;
        }
    }

    /// <summary>
    /// Returns the list of Field ID that references to this EnumTypeTable.
    /// </summary>
    /// <value>the list of FID</value>
    public List<int> FidReferences
    {
        get
        {

            if (m_FidsList == null)
            {
                m_FidsList = new(m_RsslEnumTypeTable.FidReferences.Count);
            }
            else
            {
                m_FidsList.Clear();
            }

            for (int index = 0; index < m_RsslEnumTypeTable.FidReferences.Count; index++)
            {
                m_FidsList.Add(m_RsslEnumTypeTable.FidReferences[index]);
            }

            return m_FidsList;
        }
    }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="EnumTypeTable"/> object.</returns>
    public override string ToString()
    {

        if (m_ToStringValue == null)
        {
            m_ToStringValue = new StringBuilder(256);
        }
        else
        {
            m_ToStringValue.Clear();
        }

        for (int index = 0; index < m_RsslEnumTypeTable.FidReferences.Count; index++)
        {
            m_ToStringValue.Append("(Referenced by Fid ")
                .Append(m_RsslEnumTypeTable.FidReferences[index])
                .AppendLine(")");
        }

        Eta.Codec.IEnumType rsslEnumType;
        Rdm.EnumType enumTypImpl = new();

        for (int index = 0; index <= m_RsslEnumTypeTable.MaxValue; index++)
        {
            rsslEnumType = m_RsslEnumTypeTable.EnumTypes[index];

            if (rsslEnumType != null)
            {
                m_ToStringValue.Append(enumTypImpl.SetEnumType(rsslEnumType)).AppendLine();
            }
        }

        return m_ToStringValue.ToString();
    }

    #endregion

    #region Implementation details

    private LSEG.Eta.Codec.IEnumTypeTable m_RsslEnumTypeTable;
    private List<EnumType> m_EnumTypeList;
    private List<int> m_FidsList;
    private StringBuilder m_ToStringValue;
    private bool m_RefreshEnumTypeList;

    private LinkedList<EnumType> m_EnumTypePool = new LinkedList<EnumType>();

    private LinkedListNode<EnumTypeTable> m_Node;

    internal LSEG.Eta.Codec.IEnumTypeTable rsslEnumTypeTable
    {
        set
        {
            m_RefreshEnumTypeList = true;
            m_RsslEnumTypeTable = value;
        }
    }

#pragma warning disable CS8618
    internal EnumTypeTable()
#pragma warning restore CS8618
    {
        m_Node = new LinkedListNode<EnumTypeTable>(this);
    }

    internal EnumTypeTable Clear()
    {
        m_RefreshEnumTypeList = true;

        ClearEnumTypeList();

        if (m_FidsList != null)
        {
            m_FidsList.Clear();
        }

        return this;
    }

    private void ClearEnumTypeList()
    {
        if (m_EnumTypeList != null && m_EnumTypeList.Count != 0)
        {
            for (int index = 0; index < m_EnumTypeList.Count; index++)
            {
                m_EnumTypeList[index].ReturnToPool(m_EnumTypePool);
            }

            m_EnumTypeList.Clear();
        }
    }

    private Rdm.EnumType GetEnumType(Eta.Codec.IEnumType enumType)
    {
        EnumType? enumTypeImpl = m_EnumTypePool.First?.Value;

        if (enumTypeImpl == null)
        {
            // pool is empty, create new entry
            enumTypeImpl = new EnumType();
        }
        else
        {
            // pool wasn't empty, remove entry from the pool
            m_EnumTypePool.RemoveFirst();
        }

        enumTypeImpl.SetEnumType(enumType);

        return enumTypeImpl;
    }

    internal void ReturnToPool(LinkedList<EnumTypeTable> pool)
    {
        pool.AddLast(m_Node);
    }

    #endregion
}
