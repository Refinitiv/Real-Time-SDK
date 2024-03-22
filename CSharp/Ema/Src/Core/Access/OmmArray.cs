/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */


using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;

using LSEG.Eta.Codec;
using LSEG.Eta.Common;


namespace LSEG.Ema.Access;


/// <summary>
/// OmmArray is a homogeneous container of primitive data type entries.
/// </summary>
/// <remarks>
/// Objects of this class are intended to be short lived or rather transitional.<br/>
/// This class is designed to efficiently perform setting and extracting of OmmArray and
/// its content.<br/>
/// Objects of this class are not cache-able.<br/>
/// All methods in this class are single threaded.<br/>
/// <example>
/// The following code snippet shows addition of primitive data type entries to OmmArray.<br/>
/// <code>
/// OmmArray array = new();
///
/// array.FixedWidth = 8;
/// array.AddInt(-16).AddInt(28).AddInt(-35).Complete();
/// </code>
///
/// The following code snippet shows getting data from OmmArray:
/// <code>
/// var eListIt = eList.GetEnumerator();
/// while (eListIt.MoveNext())
/// {
///     ElementEntry eEntry = eListIt.Current;
///     if ( eEntry.LoadType == DataType.DataTypes.ARRAY &amp;&amp;
/// 	    eEntry.Code != Data.DataCode.BLANK )
///     {
/// 	    OmmArray array = eEntry.OmmArrayValue();
///
/// 	    foreach (var aEntry in  array )
/// 	    {
/// 		    switch ( aEntry.LoadType )
/// 		    {
/// 			    case DataType.DataTypes.INT:
/// 				    long val = aEntry.OmmIntValue().Value;
/// 				    break;
/// 			    case DataType.DataTypes.REAL:
/// 				    OmmReal real = aEntry.OmmRealValue();
/// 				    break;
/// 			    ...
/// 		    }
/// 	    }
///     }
/// }
/// </code>
/// </example>
/// </remarks>
/// <seealso cref="Data"/>
/// <seealso cref="OmmArrayEntry"/>
/// <seealso cref="OmmReal"/>
/// <seealso cref="OmmState"/>
/// <seealso cref="OmmQos"/>
/// <seealso cref="EmaBuffer"/>
public sealed class OmmArray : Data, IEnumerable<OmmArrayEntry>
{
    #region Public members

    /// <summary>
    /// Constructor for OmmArray
    /// </summary>
    public OmmArray()
    {
        m_ommArrayEncoder = new OmmArrayEncoder(this);
        Encoder = m_ommArrayEncoder;
        ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
        ClearTypeSpecific_All = ClearInternal;
        m_dataType = Access.DataType.DataTypes.ARRAY;
    }

    /// <summary>
    /// Indicates presence of FixedWidth.
    /// </summary>
    public bool HasFixedWidth { get => FixedWidth > 0; }

    /// <summary>
    /// Gets FixedWidth
    /// </summary>
    public int FixedWidth
    {
        get => m_Array.ItemLength;

        set
        {
            if (m_Array.ItemLength > 0)
            {
                throw new OmmInvalidUsageException(
                    "Attempt to set fixed width of OmmArray after Add***() was already called.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            m_Array.ItemLength = value;
        }
    }

    /// <summary>
    /// Clears the OmmArray. Invoking Clear() method clears all the values and resets all
    /// the defaults.
    /// </summary>
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray Clear()
    {
        Clear_All();
        return this;
    }

    internal void ClearInternal()
    {
        m_Array.Clear();
        m_ErrorCode = OmmError.ErrorCodes.NO_ERROR;
    }

    /// <summary>
    /// Returns an iterator over the entries in this array.
    /// </summary>
    /// <returns>iterator over the entries in this array</returns>
    public IEnumerator<OmmArrayEntry> GetEnumerator()
    {
        if (m_ErrorCode == OmmError.ErrorCodes.NO_ERROR)
        {
            OmmArrayEnumerator enumerator = new OmmArrayEnumerator();
            CodecReturnCode retCode = enumerator.SetRsslData(m_bodyBuffer!, m_MajorVersion, m_MinorVersion);
            if (retCode == CodecReturnCode.SUCCESS)
                return enumerator;

            m_ErrorCode = enumerator.ErrorCode;
        }

        return new ArrayErrorEnumerator(m_ErrorCode);
    }

    IEnumerator IEnumerable.GetEnumerator()
    {
        return GetEnumerator();
    }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmArray"/> object.</returns>
    public override string ToString()
    {
        return ToString(0);
    }

    internal override string ToString(int indent)
    {
        m_ToString.Clear();

        if (Code == DataCode.BLANK)
        {
            Utilities.AddIndent(m_ToString, indent)
                .Append("OmmArray");

            ++indent;
            Utilities.AddIndent(m_ToString.AppendLine(), indent).Append("blank array");
            --indent;
        }
        else
        {
            Utilities.AddIndent(m_ToString, indent)
                .Append("OmmArray with entries of dataType=\"")
                .Append(Access.DataType.AsString(m_Array.PrimitiveType)).Append('"');

            if (HasFixedWidth)
                m_ToString.Append(" fixed width=\"").Append(FixedWidth).Append('"');

            if (m_Array.PrimitiveType == 0)
            {
                Utilities.AddIndent(m_ToString.AppendLine(), indent).AppendLine("OmmArrayEnd");
                return m_ToString.ToString();
            }

            ++indent;

            foreach (OmmArrayEntry arrayEntry in this)
            {
                Data load = (Data)arrayEntry.Load;
                if (load == null)
                    return "Decoding of just encoded object in the same application is not supported";

                Utilities.AddIndent(m_ToString.AppendLine(), indent).Append("value=\"");

                if (load.m_dataType == Access.DataType.DataTypes.BUFFER)
                    m_ToString.AppendLine().Append(load.ToString());
                else if (load.m_dataType == Access.DataType.DataTypes.ERROR)
                    m_ToString.AppendLine().Append(load.ToString(indent));
                else
                    m_ToString.Append('"').Append(load.ToString()).Append('"');
            }

            --indent;
        }

        Utilities.AddIndent(m_ToString.AppendLine(), indent).AppendLine("OmmArrayEnd");

        return m_ToString.ToString();
    }

    /// <summary>
    /// Adds a specific simple type of OMM data to the OmmArray.
    /// </summary>
    ///
    /// <param name="val">value specifies added Int64</param>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddInt(long val)
    {
        m_ommArrayEncoder.AddInt(val);
        return this;
    }

    /// <summary>
    /// Adds a specific simple type of OMM data to the OmmArray.
    /// </summary>
    ///
    /// <param name="val">value specifies added UInt64</param>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddUInt(ulong val)
    {
        m_ommArrayEncoder.AddUInt(val);
        return this;
    }

    /// <summary>
    /// Adds a specific simple type of OMM data to the OmmArray.
    /// </summary>
    ///
    /// <param name="mantissa">added <see cref="OmmReal.Mantissa"/></param>
    /// <param name="magnitudeType">added <see cref="OmmReal.MagnitudeType"/>
    ///   as defined in <see cref="OmmReal.MagnitudeTypes"/></param>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddReal(long mantissa, int magnitudeType)
    {
        m_ommArrayEncoder.AddReal(mantissa, magnitudeType);
        return this;
    }

    /// <summary>
    /// Adds a specific simple type of OMM data to the OmmArray.
    /// </summary>
    ///
    /// <param name="val">added double to be converted to OmmReal</param>
    /// <param name="magnitudeType">added <see cref="OmmReal.MagnitudeType"/>
    ///   as defined in <see cref="OmmReal.MagnitudeTypes"/> (default value is
    ///   <see cref="OmmReal.MagnitudeTypes.EXPONENT_0"/>)</param>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddRealFromDouble(double val, int magnitudeType = OmmReal.MagnitudeTypes.EXPONENT_0)
    {
        m_ommArrayEncoder.AddRealFromDouble(val, magnitudeType);
        return this;
    }

    /// <summary>
    /// Adds a specific simple type of OMM data to the OmmArray.
    /// </summary>
    ///
    /// <param name="val">value specifies added float</param>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddFloat(float val)
    {
        m_ommArrayEncoder.AddFloat(val);
        return this;
    }

    /// <summary>
    /// Adds a specific simple type of OMM data to the OmmArray.
    /// </summary>
    ///
    /// <param name="val">value specifies added double</param>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddDouble(double val)
    {
        m_ommArrayEncoder.AddDouble(val);
        return this;
    }

    /// <summary>
    /// Adds a specific simple type of OMM data to the OmmArray.
    /// </summary>
    ///
    /// <param name="year">added <see cref="OmmDate.Year"/> (0 - 4095 where 0 indicates blank)</param>
    /// <param name="month">added <see cref="OmmDate.Month"/> (0 - 12 where 0 indicates blank)</param>
    /// <param name="day">added <see cref="OmmDate.Day"/> (0 - 31 where 0 indicates blank)</param>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddDate(int year, int month, int day)
    {
        m_ommArrayEncoder.AddDate(year, month, day);
        return this;
    }

    /// <summary>
    /// Adds a specific simple type of OMM data to the OmmArray.
    /// </summary>
    ///
    /// <param name="hour">added <see cref="OmmTime.Hour"/> (0 - 23 where 255 indicates blank)</param>
    /// <param name="minute">added <see cref="OmmTime.Minute"/> (0 - 59 where 255 indicates blank)</param>
    /// <param name="second">added <see cref="OmmTime.Second"/> (0 - 60 where 255 indicates blank)</param>
    /// <param name="millisecond">added <see cref="OmmTime.Millisecond"/> (0 - 999 where 65535 indicates blank)</param>
    /// <param name="microsecond">added <see cref="OmmTime.Microsecond"/> (0 - 999 where 2047 indicates blank)</param>
    /// <param name="nanosecond">added <see cref="OmmTime.Nanosecond"/> (0 - 999 where 2047 indicates blank)</param>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddTime(int hour = 0, int minute = 0, int second = 0, int millisecond = 0, int microsecond = 0,
        int nanosecond = 0)
    {
        m_ommArrayEncoder.AddTime(hour, minute, second, millisecond, microsecond, nanosecond);
        return this;
    }


    /// <summary>
    /// Adds a specific simple type of OMM data to the OmmArray.
    /// </summary>
    ///
    /// <param name="year">added <see cref="OmmDate.Year"/> (0 - 4095 where 0 indicates blank)</param>
    /// <param name="month">added <see cref="OmmDate.Month"/> (0 - 12 where 0 indicates blank)</param>
    /// <param name="day">added <see cref="OmmDate.Day"/> (0 - 31 where 0 indicates blank)</param>
    /// <param name="hour">added <see cref="OmmTime.Hour"/> (0 - 23 where 255 indicates blank)</param>
    /// <param name="minute">added <see cref="OmmTime.Minute"/> (0 - 59 where 255 indicates blank)</param>
    /// <param name="second">added <see cref="OmmTime.Second"/> (0 - 60 where 255 indicates blank)</param>
    /// <param name="millisecond">added <see cref="OmmTime.Millisecond"/> (0 - 999 where 65535 indicates blank)</param>
    /// <param name="microsecond">added <see cref="OmmTime.Microsecond"/> (0 - 999 where 2047 indicates blank)</param>
    /// <param name="nanosecond">added <see cref="OmmTime.Nanosecond"/> (0 - 999 where 2047 indicates blank)</param>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddDateTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0,
        int millisecond = 0, int microsecond = 0, int nanosecond = 0)
    {
        m_ommArrayEncoder.AddDateTime(year, month, day, hour, minute, second, millisecond, microsecond, nanosecond);
        return this;
    }

    /// <summary>
    /// Adds a specific simple type of OMM data to the OmmArray.
    /// </summary>
    ///
    /// <param name="timeliness">added <see cref="OmmQos.Timeliness"/>
    ///   (default value is <see cref="OmmQos.Timelinesses.REALTIME"/>)</param>
    /// <param name="rate">added <see cref="OmmQos.Rate"/>
    ///   (default value is <see cref="OmmQos.Rates.TICK_BY_TICK"/>)</param>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddQos(uint timeliness = OmmQos.Timelinesses.REALTIME, uint rate = OmmQos.Rates.TICK_BY_TICK)
    {
        m_ommArrayEncoder.AddQos(timeliness, rate);
        return this;
    }

    /// <summary>
    /// Adds a specific simple type of OMM data to the OmmArray.
    /// </summary>
    ///
    /// <param name="streamState">added <see cref="OmmState.StreamState"/>
    ///   (default value is <see cref="OmmState.StreamStates.OPEN"/>)</param>
    /// <param name="dataState">added <see cref="OmmState.DataState"/>
    ///   (default value is <see cref="OmmState.DataStates.OK"/>)</param>
    /// <param name="statusCode">added <see cref="OmmState.StatusCode"/>
    ///   (default value is <see cref="OmmState.StatusCodes.NONE"/>)</param>
    /// <param name="statusText">added <see cref="OmmState.StatusText()"/>
    ///   (default value is empty string)</param>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddState(int streamState = OmmState.StreamStates.OPEN,
        int dataState = OmmState.DataStates.OK,
        int statusCode = OmmState.StatusCodes.NONE,
        string statusText = "")
    {
        m_ommArrayEncoder.AddState(streamState, dataState, statusCode, statusText);
        return this;
    }

    /// <summary>
    /// Adds a specific simple type of OMM data to the OmmArray.
    /// </summary>
    ///
    /// <param name="val">added Enum</param>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddEnum(ushort val)
    {
        m_ommArrayEncoder.AddEnum(val);
        return this;
    }

    /// <summary>
    /// Adds a specific simple type of OMM data to the OmmArray.
    /// </summary>
    ///
    /// <param name="val">added EmaBuffer as OmmBuffer</param>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddBuffer(EmaBuffer val)
    {
        m_ommArrayEncoder.AddBuffer(val);
        return this;
    }

    /// <summary>
    /// Adds a specific simple type of OMM data to the OmmArray.
    /// </summary>
    ///
    /// <param name="val">added string as OmmAscii</param>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddAscii(string val)
    {
        m_ommArrayEncoder.AddAscii(val);
        return this;
    }

    /// <summary>
    /// Adds a specific simple type of OMM data to the OmmArray.
    /// </summary>
    ///
    /// <param name="val">added EmaBuffer as OmmUtf8</param>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddUtf8(EmaBuffer val)
    {
        m_ommArrayEncoder.AddUtf8(val);
        return this;
    }

    /// <summary>
    /// Adds a specific simple type of OMM data to the OmmArray.
    /// </summary>
    ///
    /// <param name="val">added EmaBuffer as OmmRmtes</param>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddRmtes(EmaBuffer val)
    {
        m_ommArrayEncoder.AddRmtes(val);
        return this;
    }

    #region Blank entries

    /// <summary>
    /// Adds a blank data code to the OmmArray.
    /// </summary>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddCodeInt()
    {
        m_ommArrayEncoder.AddCodeInt();
        return this;
    }

    /// <summary>
    /// Adds a blank data code to the OmmArray.
    /// </summary>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddCodeUInt()
    {
        m_ommArrayEncoder.AddCodeUInt();
        return this;
    }

    /// <summary>
    /// Adds a blank data code to the OmmArray.
    /// </summary>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddCodeReal()
    {
        m_ommArrayEncoder.AddCodeReal();
        return this;
    }

    /// <summary>
    /// Adds a blank data code to the OmmArray.
    /// </summary>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddCodeFloat()
    {
        m_ommArrayEncoder.AddCodeFloat();
        return this;
    }

    /// <summary>
    /// Adds a blank data code to the OmmArray.
    /// </summary>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddCodeDouble()
    {
        m_ommArrayEncoder.AddCodeDouble();
        return this;
    }

    /// <summary>
    /// Adds a blank data code to the OmmArray.
    /// </summary>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddCodeDate()
    {
        m_ommArrayEncoder.AddCodeDate();
        return this;
    }

    /// <summary>
    /// Adds a blank data code to the OmmArray.
    /// </summary>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddCodeTime()
    {
        m_ommArrayEncoder.AddCodeTime();
        return this;
    }

    /// <summary>
    /// Adds a blank data code to the OmmArray.
    /// </summary>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddCodeDateTime()
    {
        m_ommArrayEncoder.AddCodeDateTime();
        return this;
    }

    /// <summary>
    /// Adds a blank data code to the OmmArray.
    /// </summary>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddCodeQos()
    {
        m_ommArrayEncoder.AddCodeQos();
        return this;
    }

    /// <summary>
    /// Adds a blank data code to the OmmArray.
    /// </summary>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddCodeState()
    {
        m_ommArrayEncoder.AddCodeState();
        return this;
    }

    /// <summary>
    /// Adds a blank data code to the OmmArray.
    /// </summary>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddCodeEnum()
    {
        m_ommArrayEncoder.AddCodeEnum();
        return this;
    }

    /// <summary>
    /// Adds a blank data code to the OmmArray.
    /// </summary>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddCodeBuffer()
    {
        m_ommArrayEncoder.AddCodeBuffer();
        return this;
    }

    /// <summary>
    /// Adds a blank data code to the OmmArray.
    /// </summary>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddCodeAscii()
    {
        m_ommArrayEncoder.AddCodeAscii();
        return this;
    }

    /// <summary>
    /// Adds a blank data code to the OmmArray.
    /// </summary>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddCodeUtf8()
    {
        m_ommArrayEncoder.AddCodeUtf8();
        return this;
    }

    /// <summary>
    /// Adds a blank data code to the OmmArray.
    /// </summary>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if first addition was of different data type
    /// </exception>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    public OmmArray AddCodeRmtes()
    {
        m_ommArrayEncoder.AddCodeRmtes();
        return this;
    }

    #endregion

    /// <summary>
    /// Completes encoding of OmmArray.
    /// </summary>
    ///
    /// <returns>Reference to current <see cref="OmmArray"/> object.</returns>
    ///
    /// <exception cref="OmmInvalidUsageException">Thrown if an error is dectected(exception will specify the cause of the error)</exception>
    public OmmArray Complete()
    {
        m_ommArrayEncoder.Complete();
        return this;
    }

    #endregion

    #region Implementation details

    internal LSEG.Eta.Codec.Array m_Array = new LSEG.Eta.Codec.Array();
    OmmArrayEncoder m_ommArrayEncoder;
    private EncodeIterator? m_EncodeIterator;

    internal OmmError.ErrorCodes m_ErrorCode = OmmError.ErrorCodes.NO_ERROR;

    internal OmmArray StartEncoding(EncodeIterator encodeIterator)
    {
        m_EncodeIterator = encodeIterator;
        m_bodyBuffer = encodeIterator.Buffer();
        return this;
    }

    internal bool SetRsslData(DecodeIterator dIter, Eta.Codec.Buffer rsslBuffer)
    {
        m_MajorVersion = dIter.MajorVersion();

        m_MinorVersion = dIter.MinorVersion();

        m_bodyBuffer = rsslBuffer;

        m_decodeIterator.Clear();
        CodecReturnCode retCode = m_decodeIterator.SetBufferAndRWFVersion(m_bodyBuffer, m_MajorVersion, m_MinorVersion);

        if (retCode != CodecReturnCode.SUCCESS)
        {
            Code = (m_bodyBuffer.Length > 0) ? Data.DataCode.NO_CODE : Data.DataCode.BLANK;
            m_ErrorCode = OmmError.ErrorCodes.ITERATOR_SET_FAILURE;
            return false;
        }

        retCode = m_Array.Decode(m_decodeIterator);

        // shortcut for the likely outcome
        if (retCode == CodecReturnCode.SUCCESS)
        {
            Code = Data.DataCode.BLANK;
            m_ErrorCode = OmmError.ErrorCodes.NO_ERROR;
            return true;
        }

        // dispatch unlikely outcome
        switch (retCode)
        {
            case CodecReturnCode.BLANK_DATA:
                Code = Data.DataCode.BLANK;
                m_ErrorCode = OmmError.ErrorCodes.NO_ERROR;
                return true;

            case CodecReturnCode.ITERATOR_OVERRUN:
                Code = (m_bodyBuffer.Length > 0) ? Data.DataCode.NO_CODE : Data.DataCode.BLANK;
                m_ErrorCode = OmmError.ErrorCodes.ITERATOR_OVERRUN;
                return false;

            case CodecReturnCode.INCOMPLETE_DATA:
                Code = (m_bodyBuffer.Length > 0) ? Data.DataCode.NO_CODE : Data.DataCode.BLANK;
                m_ErrorCode = OmmError.ErrorCodes.INCOMPLETE_DATA;
                return false;

            default:
                Code = (m_bodyBuffer.Length > 0) ? Data.DataCode.NO_CODE : Data.DataCode.BLANK;
                m_ErrorCode = OmmError.ErrorCodes.UNKNOWN_ERROR;
                return false;
        }
    }

    internal bool SetRsslData(int majVer, int minVer, Eta.Codec.Buffer rsslBuffer, DataDictionary? dict = null)
    {
        m_MajorVersion = majVer;

        m_MinorVersion = minVer;

        m_bodyBuffer = rsslBuffer;

        m_decodeIterator.Clear();

        CodecReturnCode retCode = m_decodeIterator.SetBufferAndRWFVersion(m_bodyBuffer, m_MajorVersion, m_MinorVersion);
        if (CodecReturnCode.SUCCESS != retCode)
        {
            Code = (m_bodyBuffer.Length > 0) ? Data.DataCode.NO_CODE : Data.DataCode.BLANK;
            m_ErrorCode = OmmError.ErrorCodes.ITERATOR_SET_FAILURE;
            return false;
        }

        retCode = m_Array.Decode(m_decodeIterator);

        // shortcut for the likely outcome
        if (retCode == CodecReturnCode.SUCCESS)
        {
            Code = Data.DataCode.NO_CODE;
            m_ErrorCode = OmmError.ErrorCodes.NO_ERROR;
            return true;
        }

        // dispatch unlikely outcome
        switch (retCode)
        {
            case CodecReturnCode.BLANK_DATA:
                Code = Data.DataCode.BLANK;
                m_ErrorCode = OmmError.ErrorCodes.NO_ERROR;
                return true;

            case CodecReturnCode.ITERATOR_OVERRUN:
                Code = (m_bodyBuffer.Length > 0) ? Data.DataCode.NO_CODE : Data.DataCode.BLANK;
                m_ErrorCode = OmmError.ErrorCodes.ITERATOR_OVERRUN;
                return false;

            case CodecReturnCode.INCOMPLETE_DATA:
                Code = (m_bodyBuffer.Length > 0) ? Data.DataCode.NO_CODE : Data.DataCode.BLANK;
                m_ErrorCode = OmmError.ErrorCodes.INCOMPLETE_DATA;
                return false;

            default:
                Code = (m_bodyBuffer.Length > 0) ? Data.DataCode.NO_CODE : Data.DataCode.BLANK;
                m_ErrorCode = OmmError.ErrorCodes.UNKNOWN_ERROR;
                return false;
        }
    }

    internal class ArrayErrorEnumerator : IEnumerator<OmmArrayEntry>
    {
        private OmmArrayEntry m_Entry = new OmmArrayEntry();
        private bool m_decodingStarted = false;
        private bool m_atEnd = false;
        private OmmError m_ommError = new OmmError();

        public OmmArrayEntry Current => m_decodingStarted ? m_Entry : null!;

        object? IEnumerator.Current => m_decodingStarted ? m_Entry : null;

        public ArrayErrorEnumerator(OmmError.ErrorCodes errorCode)
        {
            m_ommError.ErrorCode = errorCode;
            m_Entry.Load = m_ommError;
        }

        public void Dispose()
        {
        }

        public bool MoveNext()
        {
            if (m_atEnd) return false;

            m_decodingStarted = true;
            m_atEnd = true;

            return true;
        }

        public void Reset()
        {
            m_decodingStarted = false;
            m_atEnd = false;
        }
    }

    #endregion
}
