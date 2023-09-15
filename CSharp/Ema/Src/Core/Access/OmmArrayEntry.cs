/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Text;

using LSEG.Eta.Codec;

namespace LSEG.Ema.Access;

/// <summary>
/// OmmArrayEntry represents an entry of OmmArray.
/// </summary>
/// <remarks>
/// <para>
/// OmmArrayEntry associates entry's data and its data type.</para>
///
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
///
/// <para>
/// Objects of this class are intended to be short lived or rather transitional.
/// This class is designed to efficiently perform extracting of data from entry.
/// Objects of this class are not cache-able.</para>
/// <para>
/// All methods in this class are Single Threaded.</para>
///
/// </remarks>
///
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
///
public sealed class OmmArrayEntry
{

    #region Public members

    /// <summary>
    /// Returns the contained Data based on the DataType.
    /// </summary>
    /// <returns>Data class reference to contained object</returns>
    public Data Load { get; internal set; }


    /// <summary>
    /// Getter returns <see cref="Data.DataType"/> of the current entry.
    ///
    /// Setter accepts <see cref="Eta.Codec.DataTypes"/>.
    /// </summary>
    public int LoadType
    {
        get => Load.DataType;

        internal set
        {
            if (Load == null)
            {
                // init new load instance
                Load = GetDataOfType(value);
            }
            else if (Load.DataType != value)
            {
                // todo: return old Load to the pool, pull a fresh instance of required type
                // better do this in the Load property setter
                Load = GetDataOfType(value);
            }
        }
    }

    /// <summary>
    /// Returns the Code of the entry's load.
    /// The code indicates a special state of a Data.
    /// Attempts to extract data will cause OmmInvalidUsageException if 
    /// <see cref="Data.DataCode.BLANK"/> is returned.
    /// </summary>
    /// <returns>data code of the contained object</returns>
    public Data.DataCode Code { get => Load.Code; }

    /// <summary>
    /// Returns the current OMM data represented as <see cref="OmmInt"/>.
    /// </summary>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// 1) if contained object is not OmmInt
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/>
    /// </exception>
    ///
    /// <return><see cref="OmmInt"/> value</return>
    public OmmInt OmmIntValue()
    {
        if (DataType.DataTypes.INT != Load.DataType)
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
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// 1) if contained object is not OmmUInt
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/>
    /// </exception>
    ///
    /// <return><see cref="OmmUInt"/> value</return>
    public OmmUInt OmmUIntValue()
    {
        if (Load.DataType != DataType.DataTypes.UINT)
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
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// 1) if contained object is not OmmReal
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/>
    /// </exception>
    ///
    /// <return><see cref="OmmReal"/> value</return>
    public OmmReal OmmRealValue()
    {
        if (Load.DataType != DataType.DataTypes.REAL)
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
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// 1) if contained object is not OmmFloat
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/>
    /// </exception>
    ///
    /// <return><see cref="OmmFloat"/> value</return>
    public OmmFloat OmmFloatValue()
    {
        if (Load.DataType != DataType.DataTypes.FLOAT)
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
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// 1) if contained object is not OmmDouble
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/>
    /// </exception>
    ///
    /// <return><see cref="OmmDouble"/> value</return>
    public OmmDouble OmmDoubleValue()
    {
        if (Load.DataType != DataType.DataTypes.DOUBLE)
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
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// 1) if contained object is not OmmDate
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/>
    /// </exception>
    ///
    /// <return><see cref="OmmDate"/> value</return>
    public OmmDate OmmDateValue()
    {
        if (Load.DataType != DataType.DataTypes.DATE)
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
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// 1) if contained object is not OmmTime
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/>
    /// </exception>
    ///
    /// <return><see cref="OmmTime"/> value</return>
    public OmmTime OmmTimeValue()
    {
        if (Load.DataType != DataType.DataTypes.TIME)
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
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// 1) if contained object is not OmmDateTime
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/>
    /// </exception>
    ///
    /// <return><see cref="OmmDateTime"/> value</return>
    public OmmDateTime OmmDateTimeValue()
    {
        if (Load.DataType != DataType.DataTypes.DATETIME)
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
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// 1) if contained object is not OmmQos
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/>
    /// </exception>
    ///
    /// <return><see cref="OmmQos"/> value</return>
    public OmmQos OmmQosValue()
    {
        if (Load.DataType != DataType.DataTypes.QOS)
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
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// 1) if contained object is not OmmState
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/>
    /// </exception>
    ///
    /// <return><see cref="OmmState"/> value</return>
    public OmmState OmmStateValue()
    {
        if (Load.DataType != DataType.DataTypes.STATE)
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
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// 1) if contained object is not OmmEnum
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/>
    /// </exception>
    ///
    /// <return><see cref="OmmEnum"/> value</return>
    public OmmEnum OmmEnumValue()
    {
        if (Load.DataType != DataType.DataTypes.ENUM)
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
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// 1) if contained object is not OmmBuffer
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/>
    /// </exception>
    ///
    /// <return><see cref="OmmBuffer"/> value</return>
    public OmmBuffer OmmBufferValue()
    {
        if (Load.DataType != DataType.DataTypes.BUFFER)
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
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// 1) if contained object is not OmmAscii
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/>
    /// </exception>
    ///
    /// <return><see cref="OmmAscii"/> value</return>
    public OmmAscii OmmAsciiValue()
    {
        if (Load.DataType != DataType.DataTypes.ASCII)
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
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// 1) if contained object is not OmmUtf8
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/>
    /// </exception>
    ///
    /// <return><see cref="OmmUtf8"/> value</return>
    public OmmUtf8 OmmUtf8Value()
    {
        if (Load.DataType != DataType.DataTypes.UTF8)
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
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// 1) if contained object is not OmmRmtes
    /// 2) if <see cref="Code"/> is <see cref="Data.DataCode.BLANK"/>
    /// </exception>
    ///
    /// <return><see cref="OmmRmtes"/> value</return>
    public OmmRmtes OmmRmtesValue()
    {
        if (Load.DataType != DataType.DataTypes.RMTES)
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
    ///
    /// <exception cref="OmmInvalidUsageException">if contained object is not OmmError
    /// </exception>
    ///
    /// <return><see cref="OmmError"/> value</return>
    public OmmError OmmErrorValue()
    {
        if (Load.DataType != DataType.DataTypes.ERROR)
        {
            string error = $"Attempt to OmmErrorValue while actual entry data type is {DataType.AsString(Load.DataType)}";
            throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return (OmmError)Load;
    }

    /// <summary>
    /// Returns a string representation of the class instance.
    /// </summary>
    ///
    /// <returns>string representation of the class instance</returns>
    public override string ToString()
    {
        return m_ToString.Clear()
            .Append("OmmArrayEntry ")
            .Append(" dataType=\"").Append(DataType.AsString(Load.DataType)).Append('"')
            .Append(" value=\"").Append(Load.ToString()).Append('"').AppendLine()
            .ToString();
    }

    #endregion

    #region Implementation details

    private StringBuilder m_ToString = new();

#pragma warning disable CS8618
    internal OmmArrayEntry()
#pragma warning restore CS8618
    { }

    internal OmmArrayEntry(Data data)
    {
        Load = data;
    }

    /// <summary>
    /// Decodes the current array entry from the underlying buffer.
    /// </summary>
    /// <param name="dIter"></param>
    /// <param name="dataType"><see cref="Eta.Codec.DataTypes"/></param>
    /// <returns></returns>
    internal OmmArrayEntry Decode(DecodeIterator dIter, int dataType)
    {
        LoadType = dataType;

        CodecReturnCode retCode = Load.Decode(dIter);
        if (retCode != CodecReturnCode.SUCCESS)
        {
            Load = new OmmError() { ErrorCode = OmmError.ErrorCodes.UNKNOWN_ERROR };
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
        return dataType switch
        {
            Eta.Codec.DataTypes.INT => new OmmInt(),
            Eta.Codec.DataTypes.UINT => new OmmUInt(),
            Eta.Codec.DataTypes.FLOAT => new OmmFloat(),
            Eta.Codec.DataTypes.DOUBLE => new OmmDouble(),
            Eta.Codec.DataTypes.REAL => new OmmReal(),
            Eta.Codec.DataTypes.DATE => new OmmDate(),
            Eta.Codec.DataTypes.TIME => new OmmTime(),
            Eta.Codec.DataTypes.DATETIME => new OmmDateTime(),
            Eta.Codec.DataTypes.QOS => new OmmQos(),
            Eta.Codec.DataTypes.STATE => new OmmState(),
            Eta.Codec.DataTypes.ENUM => new OmmEnum(),
            Eta.Codec.DataTypes.BUFFER => new OmmBuffer(),
            Eta.Codec.DataTypes.ASCII_STRING => new OmmAscii(),
            Eta.Codec.DataTypes.UTF8_STRING => new OmmUtf8(),
            Eta.Codec.DataTypes.RMTES_STRING => new OmmRmtes(),
            _ => new OmmError() { ErrorCode = OmmError.ErrorCodes.UNSUPPORTED_DATA_TYPE }
        };
    }

    /// <summary>
    /// Marks this array entry as errorneus.
    /// </summary>
    /// <param name="errorCode">reported error code</param>
    internal void Error(OmmError.ErrorCodes errorCode)
    {
        // todo: existing load should be returned to pool
        Load = new OmmError()
        {
            ErrorCode = errorCode
        };
    }

    #endregion
}
