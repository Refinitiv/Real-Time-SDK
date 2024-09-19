/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
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
    public RequestMsg RequestMsg() 
    {
        if (Load!.m_dataType != DataType.DataTypes.REQ_MSG)
        {
            throw new OmmInvalidUsageException($"Attempt to use RequestMsg while actual entry data type is {DataType.AsString(Load.DataType)}", 
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        return (RequestMsg)Load;
    }
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.RefreshMsg"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.RefreshMsg"/></exception>
    public RefreshMsg RefreshMsg()
    {
        if (Load!.m_dataType != DataType.DataTypes.REFRESH_MSG)
        {
            throw new OmmInvalidUsageException($"Attempt to use RefreshMsg while actual entry data type is {DataType.AsString(Load.DataType)}",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        return (RefreshMsg)Load;
    }
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.UpdateMsg"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.UpdateMsg"/></exception>
    public UpdateMsg UpdateMsg()
    {
        if (Load!.m_dataType != DataType.DataTypes.UPDATE_MSG)
        {
            throw new OmmInvalidUsageException($"Attempt to use UpdateMsg while actual entry data type is {DataType.AsString(Load.DataType)}",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        return (UpdateMsg)Load;
    }
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.StatusMsg"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.StatusMsg"/></exception>
    public StatusMsg StatusMsg()
    {
        if (Load!.m_dataType != DataType.DataTypes.STATUS_MSG)
        {
            throw new OmmInvalidUsageException($"Attempt to use StatusMsg while actual entry data type is {DataType.AsString(Load.DataType)}",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        return (StatusMsg)Load;
    }
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.PostMsg"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.PostMsg"/></exception>
    public PostMsg PostMsg()
    {
        if (Load!.m_dataType != DataType.DataTypes.POST_MSG)
        {
            throw new OmmInvalidUsageException($"Attempt to use PostMsg while actual entry data type is {DataType.AsString(Load.DataType)}",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        return (PostMsg)Load;
    }
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.AckMsg"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.AckMsg"/></exception>
    public AckMsg AckMsg()
    {
        if (Load!.m_dataType != DataType.DataTypes.ACK_MSG)
        {
            throw new OmmInvalidUsageException($"Attempt to use AckMsg while actual entry data type is {DataType.AsString(Load.DataType)}",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        return (AckMsg)Load;
    }
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.GenericMsg"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.GenericMsg"/></exception>
    public GenericMsg GenericMsg()
    {
        if (Load!.m_dataType != DataType.DataTypes.GENERIC_MSG)
        {
            throw new OmmInvalidUsageException($"Attempt to use GenericMsg while actual entry data type is {DataType.AsString(Load.DataType)}",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        return (GenericMsg)Load;
    }
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.FieldList"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.FieldList"/></exception>
    public FieldList FieldList()
    {
        if (Load!.m_dataType != DataType.DataTypes.FIELD_LIST)
        {
            throw new OmmInvalidUsageException($"Attempt to use FiledList while actual entry data type is {DataType.AsString(Load.DataType)}",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        return (FieldList)Load;
    }
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.ElementList"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.ElementList"/></exception>
    public ElementList ElementList()
    {
        if (Load!.m_dataType != DataType.DataTypes.ELEMENT_LIST)
        {
            throw new OmmInvalidUsageException($"Attempt to use ElementList while actual entry data type is {DataType.AsString(Load.DataType)}",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        return (ElementList)Load;
    }
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.Map"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.Map"/></exception>
    public Map Map()
    {
        if (Load!.m_dataType != DataType.DataTypes.MAP)
        {
            throw new OmmInvalidUsageException($"Attempt to use Map while actual entry data type is {DataType.AsString(Load.DataType)}",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        return (Map)Load;
    }
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.Vector"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.Vector"/></exception>
    public Vector Vector()
    {
        if (Load!.m_dataType != Access.DataType.DataTypes.VECTOR)
        {
            throw new OmmInvalidUsageException($"Attempt to use Vector while actual entry data type is {DataType.AsString(Load.DataType)}",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        return (Vector)Load;
    }
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.Series"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.Series"/></exception>
    public Series Series()
    {
        if (Load!.m_dataType != DataType.DataTypes.SERIES)
        {
            throw new OmmInvalidUsageException($"Attempt to use Series while actual entry data type is {DataType.AsString(Load.DataType)}",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        return (Series)Load;
    }
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="Access.FilterList"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.FilterList"/></exception>
    public FilterList FilterList()
    {
        if (Load!.m_dataType != DataType.DataTypes.FILTER_LIST)
        {
            throw new OmmInvalidUsageException($"Attempt to use FilterList while actual entry data type is {DataType.AsString(Load.DataType)}",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        return (FilterList)Load;
    }
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="OmmOpaque"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.OmmOpaque"/></exception>
    public OmmOpaque Opaque()
    {
        if (Load!.m_dataType != DataType.DataTypes.OPAQUE)
        {
            throw new OmmInvalidUsageException($"Attempt to use Opaque while actual entry data type is {DataType.AsString(Load.DataType)}",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        return (OmmOpaque)Load;
    }
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="OmmXml"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.OmmXml"/></exception>
    public OmmXml Xml()
    {
        if (Load!.m_dataType != DataType.DataTypes.XML)
        {
            throw new OmmInvalidUsageException($"Attempt to use Xml while actual entry data type is {DataType.AsString(Load.DataType)}",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        return (OmmXml)Load;
    }
    /// <summary>
    /// Returns the current OMM data represented as a specific complex type.
    /// </summary>
    /// <returns><see cref="OmmAnsiPage"/> class reference to contained object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.OmmAnsiPage"/></exception>
    public OmmAnsiPage AnsiPage()
    {
        if (Load!.m_dataType != DataType.DataTypes.ANSI_PAGE)
        {
            throw new OmmInvalidUsageException($"Attempt to use AnsiPage while actual entry data type is {DataType.AsString(Load.DataType)}",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        return (OmmAnsiPage)Load;
    }

    #endregion

    internal void ClearLoad()
    {
        Load = null;
    }
}