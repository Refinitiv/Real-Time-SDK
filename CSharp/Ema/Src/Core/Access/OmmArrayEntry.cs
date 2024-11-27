/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Text;

using LSEG.Eta.Codec;

namespace LSEG.Ema.Access;

/// <summary>
/// OmmArrayEntry represents an entry of OmmArray.
/// </summary>
/// <remarks>
/// OmmArrayEntry associates entry's data and its data type.<br/>
/// Objects of this class are intended to be short lived or rather transitional.<br/>
/// This class is designed to efficiently perform extracting of data from entry.<br/>
/// Objects of this class are not cache-able.<br/>
/// All methods in this class are Single Threaded.<br/>
/// Example usage:
/// <code>
/// void DecodeArray(OmmArray array)
/// {
/// 	foreach (var aEntry in array)
/// 	{
/// 		if (aEntry.Code != Data.DataCode.BlankEnum)
/// 			switch ( aEntry.LoadType )
/// 			{
/// 			case DataType.DataTypes.INT:
/// 				int val = aEntry.OmmIntValue().Value;
/// 				break;
/// 			case DataType.DataTypes.ERROR:
///                 LogError(aEntry.OmmErrorValue());
///                 break;
/// 			}
/// 	}
/// }
/// </code>
/// </remarks>
/// <seealso cref="Entry"/>
/// <seealso cref="Data"/>
/// <seealso cref="OmmArray"/>
/// <seealso cref="EmaBuffer"/>
/// <seealso cref="OmmReal"/>
/// <seealso cref="OmmDate"/>
/// <seealso cref="OmmTime"/>
/// <seealso cref="OmmDateTime"/>
/// <seealso cref="OmmQos"/>
/// <seealso cref="OmmState"/>
/// <seealso cref="OmmError"/>
public sealed class OmmArrayEntry
{

    #region Public members

    /// <summary>
    /// Returns the contained Data based on the DataType.
    /// </summary>
    /// <returns>Data class reference to contained object.</returns>
    public Data Load { get => m_Load!; }


    /// <summary>
    /// Getter returns <see cref="Ema.Access.DataType.DataTypes"/> of the current entry.
    ///
    /// Setter accepts <see cref="Ema.Access.DataType.DataTypes"/>.
    /// </summary>
    public int LoadType
    {
        get => Load.m_dataType;

        internal set
        {
            if (Load is null)
            {
                // init new load instance
                m_Load = GetDataOfType(value);
            }
            else if (Load.m_dataType != value)
            {
                Load.ClearAndReturnToPool_All();
                m_Load = GetDataOfType(value);
            }
        }
    }

    /// <summary>
    /// Returns the Code of the entry's load.
    /// </summary>
    /// <remarks>
    /// The code indicates a special state of a Data.<br/>
    /// Attempts to extract data will cause OmmInvalidUsageException if
    /// <see cref="Data.DataCode.BLANK"/> is returned.
    /// </remarks>
    /// <returns>data code of the contained object.</returns>
    public Data.DataCode Code { get => Load.Code; }

    /// <summary>
    /// Returns the current OMM data represented as <see cref="OmmInt"/>.
    /// </summary>
    /// <returns><see cref="OmmInt"/> value</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if<br/>
    /// 1) if contained object is not <see cref="OmmInt"/><br/>
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/><br/>
    /// </exception>
    public OmmInt OmmIntValue()
    {
        if (DataType.DataTypes.INT != Load.m_dataType)
        {
            string error = $"Attempt to OmmIntValue while actual entry data type is {DataType.AsString(Load.DataType)}";
            throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        else if (Data.DataCode.BLANK == Load.Code)
            throw new OmmInvalidUsageException("Attempt to OmmIntValue while entry data is blank.",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        return (OmmInt)Load;
    }

    /// <summary>
    /// Returns the current OMM data represented as <see cref="OmmUInt"/>.
    /// </summary>
    /// <returns><see cref="OmmUInt"/> value</returns>
    /// /// <exception cref="OmmInvalidUsageException">Thrown if<br/>
    /// 1) if contained object is not <see cref="OmmUInt"/><br/>
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/><br/>
    /// </exception>
    public OmmUInt OmmUIntValue()
    {
        if (Load.m_dataType != DataType.DataTypes.UINT)
        {
            string error = $"Attempt to OmmUIntValue while actual entry data type is {DataType.AsString(Load.DataType)}";
            throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        else if (Data.DataCode.BLANK == Load.Code)
            throw new OmmInvalidUsageException("Attempt to OmmUIntValue while entry data is blank.",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        return (OmmUInt)Load;
    }

    /// <summary>
    /// Returns the current OMM data represented as <see cref="OmmReal"/>.
    /// </summary>
    /// <returns><see cref="OmmReal"/> value</returns>
    /// /// <exception cref="OmmInvalidUsageException">Thrown if<br/>
    /// 1) if contained object is not <see cref="OmmReal"/><br/>
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/><br/>
    /// </exception>
    public OmmReal OmmRealValue()
    {
        if (Load.m_dataType != DataType.DataTypes.REAL)
        {
            string error = $"Attempt to OmmRealValue while actual entry data type is {DataType.AsString(Load.DataType)}";
            throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        else if (Data.DataCode.BLANK == Load.Code)
            throw new OmmInvalidUsageException("Attempt to OmmRealValue while entry data is blank.",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        return (OmmReal)Load;
    }

    /// <summary>
    /// Returns the current OMM data represented as <see cref="OmmFloat"/>.
    /// </summary>
    /// <returns><see cref="OmmFloat"/> value</returns>
    /// /// <exception cref="OmmInvalidUsageException">Thrown if<br/>
    /// 1) if contained object is not <see cref="OmmFloat"/><br/>
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/><br/>
    /// </exception>
    public OmmFloat OmmFloatValue()
    {
        if (Load.m_dataType != DataType.DataTypes.FLOAT)
        {
            string error = $"Attempt to OmmFloatValue while actual entry data type is {DataType.AsString(Load.DataType)}";
            throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        else if (Data.DataCode.BLANK == Load.Code)
            throw new OmmInvalidUsageException("Attempt to OmmFloatValue while entry data is blank.",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        return (OmmFloat)Load;
    }

    /// <summary>
    /// Returns the current OMM data represented as <see cref="OmmDouble"/>.
    /// </summary>
    /// <returns><see cref="OmmDouble"/> value</returns>
    /// /// <exception cref="OmmInvalidUsageException">Thrown if<br/>
    /// 1) if contained object is not <see cref="OmmDouble"/><br/>
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/><br/>
    /// </exception>
    public OmmDouble OmmDoubleValue()
    {
        if (Load.m_dataType != DataType.DataTypes.DOUBLE)
        {
            string error = $"Attempt to OmmDoubleValue while actual entry data type is {DataType.AsString(Load.DataType)}";
            throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        else if (Data.DataCode.BLANK == Load.Code)
            throw new OmmInvalidUsageException("Attempt to OmmDoubleValue while entry data is blank.",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        return (OmmDouble)Load;
    }

    /// <summary>
    /// Returns the current OMM data represented as <see cref="OmmDate"/>.
    /// </summary>
    /// <returns><see cref="OmmDate"/> value</returns>
    /// /// <exception cref="OmmInvalidUsageException">Thrown if<br/>
    /// 1) if contained object is not <see cref="OmmDate"/><br/>
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/><br/>
    /// </exception>
    public OmmDate OmmDateValue()
    {
        if (Load.m_dataType != DataType.DataTypes.DATE)
        {
            string error = $"Attempt to OmmDateValue while actual entry data type is {DataType.AsString(Load.DataType)}";
            throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        else if (Data.DataCode.BLANK == Load.Code)
            throw new OmmInvalidUsageException("Attempt to OmmDateValue while entry data is blank.",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        return (OmmDate)Load;
    }

    /// <summary>
    /// Returns the current OMM data represented as <see cref="OmmTime"/>.
    /// </summary>
    /// <returns><see cref="OmmTime"/> value</returns>
    /// /// <exception cref="OmmInvalidUsageException">Thrown if<br/>
    /// 1) if contained object is not <see cref="OmmTime"/><br/>
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/><br/>
    /// </exception>
    public OmmTime OmmTimeValue()
    {
        if (Load.m_dataType != DataType.DataTypes.TIME)
        {
            string error = $"Attempt to OmmTimeValue while actual entry data type is {DataType.AsString(Load.DataType)}";
            throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        else if (Data.DataCode.BLANK == Load.Code)
            throw new OmmInvalidUsageException("Attempt to OmmTimeValue while entry data is blank.",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        return (OmmTime)Load;
    }

    /// <summary>
    /// Returns the current OMM data represented as <see cref="OmmTime"/>.
    /// </summary>
    /// <returns><see cref="OmmDateTime"/> value</returns>
    /// /// <exception cref="OmmInvalidUsageException">Thrown if<br/>
    /// 1) if contained object is not <see cref="OmmDateTime"/><br/>
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/><br/>
    /// </exception>
    public OmmDateTime OmmDateTimeValue()
    {
        if (Load.m_dataType != DataType.DataTypes.DATETIME)
        {
            string error = $"Attempt to OmmDateTimeValue while actual entry data type is {DataType.AsString(Load.DataType)}";
            throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        else if (Data.DataCode.BLANK == Load.Code)
            throw new OmmInvalidUsageException("Attempt to OmmDateTimeValue while entry data is blank.",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        return (OmmDateTime)Load;
    }

    /// <summary>
    /// Returns the current OMM data represented as <see cref="OmmQos"/>.
    /// </summary>
    /// <returns><see cref="OmmQos"/> value</returns>
    /// /// <exception cref="OmmInvalidUsageException">Thrown if<br/>
    /// 1) if contained object is not <see cref="OmmQos"/><br/>
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/><br/>
    /// </exception>
    public OmmQos OmmQosValue()
    {
        if (Load.m_dataType != DataType.DataTypes.QOS)
        {
            string error = $"Attempt to OmmQosValue while actual entry data type is {DataType.AsString(Load.DataType)}";
            throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        else if (Data.DataCode.BLANK == Load.Code)
            throw new OmmInvalidUsageException("Attempt to OmmQosValue while entry data is blank.",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        return (OmmQos)Load;
    }

    /// <summary>
    /// Returns the current OMM data represented as a <see cref="OmmState"/>.
    /// </summary>
    /// <returns><see cref="OmmState"/> value</returns>
    /// /// <exception cref="OmmInvalidUsageException">Thrown if<br/>
    /// 1) if contained object is not <see cref="OmmState"/><br/>
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/><br/>
    /// </exception>
    public OmmState OmmStateValue()
    {
        if (Load.m_dataType != DataType.DataTypes.STATE)
        {
            string error = $"Attempt to OmmStateValue while actual entry data type is {DataType.AsString(Load.DataType)}";
            throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        else if (Data.DataCode.BLANK == Load.Code)
            throw new OmmInvalidUsageException("Attempt to OmmStateValue while entry data is blank.",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        return (OmmState)Load;
    }

    /// <summary>
    /// Returns the current OMM data represented as <see cref="OmmEnum"/>.
    /// </summary>
    /// <returns><see cref="OmmEnum"/> value</returns>
    /// /// <exception cref="OmmInvalidUsageException">Thrown if<br/>
    /// 1) if contained object is not <see cref="OmmEnum"/><br/>
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/><br/>
    /// </exception>
    public OmmEnum OmmEnumValue()
    {
        if (Load.m_dataType != DataType.DataTypes.ENUM)
        {
            string error = $"Attempt to OmmEnumValue while actual entry data type is {DataType.AsString(Load.DataType)}";
            throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        else if (Data.DataCode.BLANK == Load.Code)
            throw new OmmInvalidUsageException("Attempt to OmmEnumValue while entry data is blank.",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        return (OmmEnum)Load;
    }

    /// <summary>
    /// Returns the current OMM data represented as <see cref="OmmBuffer"/>.
    /// </summary>
    /// <returns><see cref="OmmBuffer"/> value</returns>
    /// /// <exception cref="OmmInvalidUsageException">Thrown if<br/>
    /// 1) if contained object is not <see cref="OmmBuffer"/><br/>
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/><br/>
    /// </exception>
    public OmmBuffer OmmBufferValue()
    {
        if (Load.m_dataType != DataType.DataTypes.BUFFER)
        {
            string error = $"Attempt to OmmBufferValue while actual entry data type is {DataType.AsString(Load.DataType)}";
            throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        else if (Data.DataCode.BLANK == Load.Code)
            throw new OmmInvalidUsageException("Attempt to OmmBufferValue while entry data is blank.",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        return (OmmBuffer)Load;
    }

    /// <summary>
    /// Returns the current OMM data represented as <see cref="OmmAscii"/>.
    /// </summary>
    /// <returns><see cref="OmmAscii"/> value</returns>
    /// /// <exception cref="OmmInvalidUsageException">Thrown if<br/>
    /// 1) if contained object is not <see cref="OmmAscii"/><br/>
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/><br/>
    /// </exception>
    public OmmAscii OmmAsciiValue()
    {
        if (Load.m_dataType != DataType.DataTypes.ASCII)
        {
            string error = $"Attempt to OmmAsciiValue while actual entry data type is {DataType.AsString(Load.DataType)}";
            throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        else if (Data.DataCode.BLANK == Load.Code)
            throw new OmmInvalidUsageException("Attempt to OmmAsciiValue while entry data is blank.",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        return (OmmAscii)Load;
    }

    /// <summary>
    /// Returns the current OMM data represented as <see cref="OmmUtf8"/>.
    /// </summary>
    /// <returns><see cref="OmmUtf8"/> value</returns>
    /// /// <exception cref="OmmInvalidUsageException">Thrown if<br/>
    /// 1) if contained object is not <see cref="OmmUtf8"/><br/>
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/><br/>
    /// </exception>
    public OmmUtf8 OmmUtf8Value()
    {
        if (Load.m_dataType != DataType.DataTypes.UTF8)
        {
            string error = $"Attempt to OmmUtf8Value while actual entry data type is {DataType.AsString(Load.DataType)}";
            throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        else if (Data.DataCode.BLANK == Load.Code)
            throw new OmmInvalidUsageException("Attempt to OmmUtf8Value while entry data is blank.",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        return (OmmUtf8)Load;
    }

    /// <summary>
    /// Returns the current OMM data represented as <see cref="OmmRmtes"/>.
    /// </summary>
    /// <returns><see cref="OmmRmtes"/> value</returns>
    /// /// <exception cref="OmmInvalidUsageException">Thrown if<br/>
    /// 1) if contained object is not <see cref="OmmRmtes"/><br/>
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/><br/>
    /// </exception>
    public OmmRmtes OmmRmtesValue()
    {
        if (Load.m_dataType != DataType.DataTypes.RMTES)
        {
            string error = $"Attempt to OmmRmtesValue while actual entry data type is {DataType.AsString(Load.DataType)}";
            throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }
        else if (Data.DataCode.BLANK == Load.Code)
            throw new OmmInvalidUsageException("Attempt to OmmRmtesValue while entry data is blank.",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        return (OmmRmtes)Load;
    }

    /// <summary>
    /// Returns the current OMM data represented as <see cref="OmmError"/>.
    /// </summary>
    /// <returns><see cref="OmmError"/> value</returns>
    /// /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmError"/><br/>
    /// </exception>
    public OmmError OmmErrorValue()
    {
        if (Load.m_dataType != DataType.DataTypes.ERROR)
        {
            string error = $"Attempt to OmmErrorValue while actual entry data type is {DataType.AsString(Load.DataType)}";
            throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return (OmmError)Load;
    }

    /// <summary>
    /// Returns a string representation of the class instance.
    /// </summary>
    /// <returns>string representation of the class object.</returns>
    public override string ToString()
    {
        if (Load == null)
            return $"{NewLine}ToString() method could not be used for just encoded object.{NewLine}";

        return m_ToString.Clear()
            .Append("OmmArrayEntry ")
            .Append(" dataType=\"").Append(DataType.AsString(Load.DataType)).Append('"')
            .Append(" value=\"").Append(Load.ToString()).Append('"').AppendLine()
            .ToString();
    }

    #endregion

    #region Implementation details

    private StringBuilder m_ToString = new();

    internal Data? m_Load;

    internal readonly EmaObjectManager m_objectManager;

#pragma warning disable CS8618
    internal OmmArrayEntry(EmaObjectManager objectManager)
#pragma warning restore CS8618
    {
        m_objectManager = objectManager;
    }

    internal OmmArrayEntry(Data data)
    {
        m_objectManager = EmaGlobalObjectPool.Instance;
        m_Load = data;
    }

    /// <summary>
    /// Decodes the current array entry from the underlying buffer.
    /// </summary>
    /// <param name="dIter"></param>
    /// <param name="dataType"><see cref="Ema.Access.DataType.DataTypes"/></param>
    /// <returns></returns>
    internal OmmArrayEntry Decode(DecodeIterator dIter, int dataType)
    {
        LoadType = dataType;

        CodecReturnCode retCode = Load.Decode(dIter);
        if (retCode != CodecReturnCode.SUCCESS)
        {
            m_Load = new OmmError() { ErrorCode = OmmError.ErrorCodes.UNKNOWN_ERROR };
        };

        return this;
    }

    /// <summary>
    /// Returns conrete <see cref="Data"/> based on specified data type.
    /// </summary>
    /// <param name="dataType">based on <see cref="Eta.Codec.DataTypes"/></param>
    /// <returns></returns>
    private Data GetDataOfType(int dataType)
    {
        // ensure that the Codec.DataType is correctly converted to the OMM DataType, that
        // is expected by the object manager
        Data? load = m_objectManager.GetDataObjectFromPool(dataType);

        if (load is null)
        {
            return new OmmError() { ErrorCode = OmmError.ErrorCodes.UNSUPPORTED_DATA_TYPE };
        }

        return load;
    }

    /// <summary>
    /// Marks this array entry as errorneus.
    /// </summary>
    /// <param name="errorCode">reported error code</param>
    internal void Error(OmmError.ErrorCodes errorCode)
    {
        m_Load = new OmmError()
        {
            ErrorCode = errorCode
        };
    }

    #endregion
}
