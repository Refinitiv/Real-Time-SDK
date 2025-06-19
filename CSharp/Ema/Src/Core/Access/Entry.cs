/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Text;

namespace LSEG.Ema.Access;

/// <summary>
/// A variant class representing a primitive data entry in another container.<br/>
/// This public interface provides only read-only access to the contained data.
/// </summary>
/// <seealso cref="OmmArrayEntry"/>
public abstract class Entry
{
    internal StringBuilder? m_errorString;
    internal StringBuilder m_toString = new StringBuilder();

    internal Entry() { }
    internal Entry(Data load)
    {
        Load = load;
    }

    internal StringBuilder GetErrorString()
    {
        if (m_errorString == null)
        {
            m_errorString = new StringBuilder();
        } else
        {
            m_errorString.Length = 0;
        }

        return m_errorString;
    }

    /// <summary>
    /// Returns the contained Data based on the DataType.
    /// </summary>
    /// <returns>Reference to the contained Data object.</returns>
    public Data? Load;


    /// <summary>
    /// Returns the DataType of the entry's load.
    /// Return of <see cref="DataType.DataTypes.NO_DATA"/> signifies no data present in load.
    /// Return of <see cref="DataType.DataTypes.ERROR"/> signifies error while extracting content of load.
    /// </summary>
    /// <returns>Data type of the contained object.</returns>
    public int LoadType { get => Load != null ? Load!.m_dataType : DataType.DataTypes.NO_DATA; }

    /// <summary>
    /// Returns the Code of the entry's load.<br/>
    /// The code indicates a special state of a Data.<br/>
    /// Attempts to extract data will cause OmmInvalidUsageException if <see cref="Data.DataCode.BLANK"/> is returned.<br/>
    /// </summary>
    /// <returns>Data code of the contained object.</returns>
    public Data.DataCode Code { get => Load != null ? Load!.Code : Data.DataCode.BLANK; }

    /// <summary>
    /// Returns the current OMM data represented as a specific simple type.
    /// </summary>
    /// <returns><see cref="OmmError"/> object containing the error information.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmError"/></exception>
    public OmmError OmmErrorValue()
    {
        if (Load!.m_dataType != DataType.DataTypes.ERROR)
        {
            string error = $"Attempt to error() while actual entry data type is {DataType.AsString(Load.DataType)}";
            throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return (OmmError)Load;
    }

    #region Methods for accessing objects of complex type

    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.RequestMsg"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.RequestMsg"/></exception>
    public RequestMsg RequestMsg()  => GetData<RequestMsg>(DataType.DataTypes.REQ_MSG);
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.RefreshMsg"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.RefreshMsg"/></exception>
    public RefreshMsg RefreshMsg() => GetData<RefreshMsg>(DataType.DataTypes.REFRESH_MSG);
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.UpdateMsg"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.UpdateMsg"/></exception>
    public UpdateMsg UpdateMsg() => GetData<UpdateMsg>(DataType.DataTypes.UPDATE_MSG);
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.StatusMsg"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.StatusMsg"/></exception>
    public StatusMsg StatusMsg() => GetData<StatusMsg>(DataType.DataTypes.STATUS_MSG);
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.PostMsg"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.PostMsg"/></exception>
    public PostMsg PostMsg() => GetData<PostMsg>(DataType.DataTypes.POST_MSG);
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.AckMsg"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.AckMsg"/></exception>
    public AckMsg AckMsg() => GetData<AckMsg>(DataType.DataTypes.ACK_MSG);
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.GenericMsg"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.GenericMsg"/></exception>
    public GenericMsg GenericMsg() => GetData<GenericMsg>(DataType.DataTypes.GENERIC_MSG);
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.FieldList"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.FieldList"/></exception>
    public FieldList FieldList() => GetData<FieldList>(DataType.DataTypes.FIELD_LIST);
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.ElementList"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.ElementList"/></exception>
    public ElementList ElementList() => GetData<ElementList>(DataType.DataTypes.ELEMENT_LIST);
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.Map"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.Map"/></exception>
    public Map Map() => GetData<Map>(DataType.DataTypes.MAP);
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.Vector"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.Vector"/></exception>
    public Vector Vector() => GetData<Vector>(Access.DataType.DataTypes.VECTOR);
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.Series"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.Series"/></exception>
    public Series Series() => GetData<Series>(DataType.DataTypes.SERIES);
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.FilterList"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.FilterList"/></exception>
    public FilterList FilterList() => GetData<FilterList>(DataType.DataTypes.FILTER_LIST);
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="OmmOpaque"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.OmmOpaque"/></exception>
    public OmmOpaque Opaque() => GetData<OmmOpaque>(DataType.DataTypes.OPAQUE);
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="OmmXml"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.OmmXml"/></exception>
    public OmmXml Xml() => GetData<OmmXml>(DataType.DataTypes.XML);
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="OmmJson"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.OmmJson"/></exception>
    public OmmJson Json() => GetData<OmmJson>(DataType.DataTypes.JSON);
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="OmmAnsiPage"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.OmmAnsiPage"/></exception>
    public OmmAnsiPage AnsiPage() => GetData<OmmAnsiPage>(DataType.DataTypes.ANSI_PAGE);

    private T GetData<T>(int dataType)
        where T : Data
    {
        if (Load!.m_dataType != dataType)
        {
            throw new OmmInvalidUsageException($"Attempt to use {DataType.AsString(dataType)} while actual entry data type is {DataType.AsString(Load.DataType)}",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        return (T)Load;
    }

    #endregion

    internal void ClearLoad()
    {
        Load = null;
    }
}