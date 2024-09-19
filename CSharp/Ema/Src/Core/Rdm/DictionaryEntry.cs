/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Collections;
using System.Collections.Generic;
using System.Text;

using LSEG.Eta.ValueAdd.Common;

namespace LSEG.Ema.Rdm;


/// <summary>
/// A data dictionary entry, containing field information and an enumeration table
/// reference if present.
/// </summary>
/// <seealso cref="LSEG.Ema.Access.DataType.DataTypes"/>
public sealed class DictionaryEntry
{
    #region Public members

    /// <summary>
    /// Create a new DictionaryEntry object.
    /// </summary>
    public DictionaryEntry() : this(true)
    {
    }

    /// <summary>
    /// Acronym field for the current dictionary entry.
    /// </summary>
    /// <value>the acronym</value>
    public string Acronym
    {
        get => m_rsslDictionaryEntry.GetAcronym().Data() != null
            ? m_rsslDictionaryEntry.GetAcronym().ToString()
            : string.Empty;
    }

    /// <summary>
    /// DDE Acronym field for the current dictionary entry.
    /// </summary>
    /// <value>the ddeAcronym</value>
    public string DdeAcronym
    {
        get => m_rsslDictionaryEntry.GetDdeAcronym().Data() != null
            ? m_rsslDictionaryEntry.GetDdeAcronym().ToString()
            : string.Empty;

    }

    /// <summary>
    /// The fieldId the current dictionary entry corresponds to.
    /// </summary>
    /// <value>the fid</value>
    public int FieldId
    {
        get => m_rsslDictionaryEntry.GetFid();
    }

    /// <summary>
    /// The ripple to field for the current dictionary entry, containing the next FID to ripple to.
    /// </summary>
    /// <value>the rippleToField</value>
    public int RippleToField
    {
        get => m_rsslDictionaryEntry.GetRippleToField();
    }

    /// <summary>
    /// Marketfeed Field Type field of the current dictionary entry.<br/>
    /// This type is defined in Eta.Codec.MfFieldTypes
    /// /// </summary>
    /// <value>the fieldType</value>
    public int FieldType
    {
        get => m_rsslDictionaryEntry.GetFieldType();
    }

    /// <summary>
    /// Marketfeed length of the current dictionary entry.
    /// </summary>
    /// <value>the length</value>
    public int Length
    {
        get => m_rsslDictionaryEntry.GetLength();
    }

    /// <summary>
    /// Marketfeed enum length of the current dictionary entry.
    /// </summary>
    /// <value>the enumLength</value>
    public int EnumLength
    {
        get => m_rsslDictionaryEntry.GetEnumLength();
    }

    /// <summary>
    /// RWF type of the current dictionary entry.<br/>
    /// This type is defined in <see cref="Access.DataType.DataTypes"/> and less than 256.
    /// </summary>
    /// <value>the rwfType</value>
    public int RwfType
    {
        get => m_rsslDictionaryEntry.GetRwfType();
    }

    /// <summary>
    /// RWF Length of the current dictionary entry.
    /// </summary>
    ///
    /// <value>the rwfLength</value>
    public int RwfLength
    {
        get => m_rsslDictionaryEntry.GetRwfLength();
    }

    /// <summary>
    /// Check whether the EnumType exists.
    /// </summary>
    /// <param name="val">the value of the enumerated type to check</param>
    /// <returns>the enumerated type if it exists</returns>
    public bool HasEnumType(int val)
    {
        return m_DataDictionaryImpl.HasEnumType(m_rsslDictionaryEntry.GetFid(), val);
    }

    /// <summary>
    /// Returns the corresponding enumerated type in the dictionary entry's
    /// table, if the type exists.
    /// </summary>
    /// <param name="val">the value of the enumerated type to get</param>
    /// <returns>the enumerated type if it exists</returns>
    /// <exception cref="Access.OmmInvalidUsageException">Thrown if <see cref="HasEnumType"/> with val returns false</exception>
    public EnumType GetEnumType(int val)
    {
        if (!HasEnumType(val))
        {
            throw new Access.OmmInvalidUsageException(
                $"The enum value {val} for the Field ID {m_rsslDictionaryEntry.GetFid()} does not exist in enumerated type definitions",
                Access.OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
        }

        m_EnumTypeImpl ??= new EnumType();

        return m_EnumTypeImpl.SetEnumType(m_DataDictionaryImpl.EnumType(m_rsslDictionaryEntry.GetFid(), val).GetEnumType());
    }

    /// <summary>
    /// Returns the corresponding enumerated type in the dictionary entry's
    /// table, if the type exists.
    /// </summary>
    /// <param name="val">the value of the enumerated type to get</param>
    /// <returns>the enumerated type if it exists</returns>
    /// <exception cref="Access.OmmInvalidUsageException">Thrown if <see cref="HasEnumType"/> with val returns false</exception>
    public EnumType EnumType(int val)
    {
        if (!HasEnumType(val))
        {
            throw new Access.OmmInvalidUsageException(
                $"The enum value {val} for the Field ID {m_rsslDictionaryEntry.GetFid()} does not exist in enumerated type definitions",
                Access.OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
        }

        if (m_EnumTypeImpl == null)
        {
            m_EnumTypeImpl = new();
        }

        return m_EnumTypeImpl.SetEnumType((m_DataDictionaryImpl.EnumType(m_rsslDictionaryEntry.GetFid(), val)).GetEnumType());
    }

    /// <summary>
    /// Check whether the EnumTypeTable exists.
    /// </summary>
    /// <returns><c>true</c> if EnumTypeTable exists, otherwise <c>false</c></returns>
    public bool HasEnumTypeTable
    {
        get => m_rsslDictionaryEntry.GetEnumTypeTable() != null ? true : false;
    }

    /// <summary>
    /// Returns the list of EnumTypeTable that is used by this DictionaryEntry,
    /// if the type exists.
    /// </summary>
    /// <returns>the array of EnumTypeTable if it exists</returns>
    /// <exception cref="Access.OmmInvalidUsageException">Thrown if <see cref="HasEnumTypeTable"/> returns false</exception>
    public EnumTypeTable GetEnumTypeTable()
    {
        if (!HasEnumTypeTable)
        {
            throw new Access.OmmInvalidUsageException($"The EnumTypeTable does not exist for the Field ID {m_rsslDictionaryEntry.GetFid()}",
                Ema.Access.OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return EnumTypeTableImpl();
    }

    /// <summary>
    /// Convert information contained in the dictionary entry to a string.
    /// </summary>
    /// <returns>the string representation of this <see cref="DictionaryEntry"/></returns>
    public override string ToString()
    {
        if (m_ToString == null)
        {
            m_ToString = new StringBuilder(512);
        }
        else
        {
            m_ToString.Clear();
        }

        m_ToString.Append("Fid=").Append(FieldId).Append(" '").Append(Acronym).
            Append("' '").Append(DdeAcronym).
            Append("' Type=").Append(FieldType).
            Append(" RippleTo=").Append(RippleToField).Append(" Len=").Append(Length).
            Append(" EnumLen=").Append(EnumLength).
            Append(" RwfType=").Append(RwfType).Append(" RwfLen=").Append(RwfLength);

        if (HasEnumTypeTable)
        {
            m_ToString.AppendLine().AppendLine().AppendLine("Enum Type Table:");

            m_ToString.Append(EnumTypeTableImpl());
        }

        return m_ToString.ToString();
    }

    #endregion

    #region Constructors

#pragma warning disable CS8618
    internal DictionaryEntry(bool isManagedByUser)
#pragma warning restore CS8618
    {
        this.m_IsManagedByUser = isManagedByUser;
        m_Node = new LinkedListNode<DictionaryEntry>(this);
    }

    #endregion

    #region Implementation details

    private Eta.Codec.IDictionaryEntry m_rsslDictionaryEntry;
    private DataDictionary m_DataDictionaryImpl;
    private EnumTypeTable m_EnumTypeTableImpl;
    private EnumType m_EnumTypeImpl;
    private StringBuilder m_ToString;
    internal bool m_IsManagedByUser;
    // node in the pool
    internal LinkedListNode<DictionaryEntry> m_Node;

    private EnumTypeTable EnumTypeTableImpl()
    {
        if (m_EnumTypeTableImpl == null)
        {
            m_EnumTypeTableImpl = new EnumTypeTable();
        }

        m_EnumTypeTableImpl.rsslEnumTypeTable = m_rsslDictionaryEntry.GetEnumTypeTable();

        return m_EnumTypeTableImpl;
    }

    internal DictionaryEntry UpdateDictionaryEntry(DataDictionary dataDictionary, Eta.Codec.IDictionaryEntry dictionaryEntry)
    {
        m_rsslDictionaryEntry = dictionaryEntry;
        m_DataDictionaryImpl = dataDictionary;

        return this;
    }

    internal void ReturnToPool(LinkedList<DictionaryEntry> pool)
    {
        m_EnumTypeTableImpl?.Clear();

        pool.AddLast(m_Node);
    }

    #endregion
}
