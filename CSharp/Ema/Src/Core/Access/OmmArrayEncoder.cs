/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using LSEG.Eta.Codec;
using LSEG.Eta.Common;

using Buffer = LSEG.Eta.Codec.Buffer;
using DateTime = LSEG.Eta.Codec.DateTime;
using Double = LSEG.Eta.Codec.Double;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Ema.Access
{
    internal sealed class OmmArrayEncoder : Encoder
    {
        private Eta.Codec.Array m_Array;
        private Eta.Codec.ArrayEntry m_ArrayEntry = new Eta.Codec.ArrayEntry();

        private Int intObject = new Int();
        private UInt uintObject = new UInt();
        private Float floatObject = new Float();
        private Double doubleObject = new Double();
        private Real realObject = new Real();
        private Buffer bufferObject = new Buffer();
        private Date dateObject = new Date();
        private Time timeObject = new Time();
        private DateTime dateTimeObject = new DateTime();
        private Qos qosObject = new Qos();
        private State stateObject = new State();
        private Enum enumObject = new Enum();


        internal OmmArrayEncoder(OmmArray encoderOwner)
        {
            m_encoderOwner = encoderOwner;
            m_Array = encoderOwner.m_Array;
        }


        private void InitEncode()
        {
            CodecReturnCode retCode = m_Array.EncodeInit(m_encodeIterator);

            while (retCode == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                retCode = m_Array.EncodeComplete(m_encodeIterator, false);
                ReallocateEncodeIteratorBuffer();
                retCode = m_Array.EncodeInit(m_encodeIterator);
            }

            if (retCode < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException(
                    $"Failed to initialize OmmArray encoding. Reason='{retCode.GetAsString()}'",
                    (int)retCode);
            }

            m_ArrayEntry ??= new();
        }


        private void AddPrimitiveEntry(Func<CodecReturnCode> entry)
        {
            if (m_containerComplete)
            {
                throw new OmmInvalidUsageException("Attempt to add an entry after Complete() was called.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            CodecReturnCode retCode = entry();

            if (retCode == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                retCode = entry();
            }

            if (retCode < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException(
                    $"Failed to encode ({DataTypes.ToString(m_Array.PrimitiveType)}) while encoding OmmArray. Reason='{retCode.GetAsString()}'",
                    (int)retCode);
            }
            return;
        }


        internal void AddRmtes(EmaBuffer val)
        {
            if (m_Array.PrimitiveType == 0)
            {
                m_Array.PrimitiveType = DataTypes.RMTES_STRING;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.RMTES_STRING)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddRmtes() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            if (m_Array.ItemLength > 0
                && m_Array.ItemLength < val.Length)
            {
                throw new OmmInvalidUsageException(
                    $"Passed in value is longer than fixed width in AddRmtes(). Fixed width='{m_Array.ItemLength}'.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }


            bufferObject.Clear();
            bufferObject.Data(new ByteBuffer(val.AsByteArray()).Flip());

            AddPrimitiveEntry(() => m_ArrayEntry!.Encode(m_encodeIterator, bufferObject));
        }


        internal void AddInt(long val)
        {
            if (m_Array.PrimitiveType == 0)
            {
                m_Array.PrimitiveType = Eta.Codec.DataTypes.INT;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.INT)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddInt() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            switch (m_Array.ItemLength)
            {
                case 0:
                case 8:
                    break;
                case 1:
                    if (val > 127 || val < -127)
                    {
                        throw new OmmInvalidUsageException(
                            $"Out of range value for the specified fixed width in AddInt(). Fixed width='{m_Array.ItemLength}' value='{val}'. ",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    }
                    break;
                case 2:
                    if (val > 32767 || val < -32767)
                    {
                        throw new OmmInvalidUsageException(
                            $"Out of range value for the specified fixed width in AddInt(). Fixed width='{m_Array.ItemLength}' value='{val}'. ",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    }
                    break;
                case 4:
                    if (val > 2147483647 || val < -2147483647)
                    {
                        throw new OmmInvalidUsageException(
                            $"Out of range value for the specified fixed width in AddInt(). Fixed width='{m_Array.ItemLength}' value='{val}'. ",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    }
                    break;
                default:
                    {
                        throw new OmmInvalidUsageException(
                            $"Unsupported FixedWidth encoding in AddInt(). Fixed width='{m_Array.ItemLength}'. ",
                            OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                    }
            }

            intObject.Value(val);

            AddPrimitiveEntry(() => m_ArrayEntry!.Encode(m_encodeIterator, intObject));
        }


        internal void AddUInt(ulong val)
        {
            if (m_Array.PrimitiveType == 0)
            {
                m_Array.PrimitiveType = DataTypes.UINT;
                AcquireEncodeIterator();
                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.UINT)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to call AddUInt() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            switch (m_Array.ItemLength)
            {
                case 0:
                case 8:
                    break;
                case 1:
                    if (val > 255)
                    {
                        throw new OmmInvalidUsageException(
                            $"Out of range value for the specified fixed width in AddUInt(). Fixed width='{m_Array.ItemLength}' value='{val}'. ",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    }
                    break;
                case 2:
                    if (val > 65535)
                    {
                        throw new OmmInvalidUsageException(
                            $"Out of range value for the specified fixed width in AddUInt(). Fixed width='{m_Array.ItemLength}' value='{val}'. ",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

                    }
                    break;
                case 4:
                    if (val > 4294967295)
                    {
                        throw new OmmInvalidUsageException(
                            $"Out of range value for the specified fixed width in AddUInt(). Fixed width='{m_Array.ItemLength}' value='{val}'. ",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    }
                    break;
                default:
                    {
                        throw new OmmInvalidUsageException(
                            $"Unsupported FixedWidth encoding in AddUInt(). Fixed width='{m_Array.ItemLength}'. ",
                            OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                    }
            };

            uintObject.Clear();
            uintObject.Value((uint)val);
            AddPrimitiveEntry(() => m_ArrayEntry!.Encode(m_encodeIterator, uintObject));
        }


        internal void AddReal(long mantissa, int magnitudeType)
        {
            if (m_Array.PrimitiveType == 0)
            {
                if (m_Array.ItemLength > 0)
                {
                    throw new OmmInvalidUsageException(
                        $"Unsupported FixedWidth encoding in AddReal(). Fixed width='{m_Array.ItemLength}'.",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                m_Array.PrimitiveType = DataTypes.REAL;
                AcquireEncodeIterator();
                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.REAL)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddReal() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            realObject.Clear();
            realObject.Value(mantissa, magnitudeType);

            AddPrimitiveEntry(() => m_ArrayEntry!.Encode(m_encodeIterator, realObject));
        }


        internal void AddRealFromDouble(double val, int magnitudeType)
        {
            if (m_Array.PrimitiveType == 0)
            {
                if (m_Array.ItemLength > 0)
                {
                    throw new OmmInvalidUsageException(
                        $"Unsupported FixedWidth encoding in AddRealFromDouble(). Fixed width='{m_Array.ItemLength}'. ",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                m_Array.PrimitiveType = DataTypes.REAL;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.REAL)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddRealFromDouble() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            doubleObject.Clear();
            doubleObject.Value(val);

            Eta.Codec.Real rel = doubleObject.ToReal(magnitudeType);

            if (rel.IsBlank)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddRealFromDouble() with invalid value or magnitudeType='{OmmReal.MagnitudeTypeAsString(magnitudeType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT); // MagnitudeTypeAsString
            }

            AddPrimitiveEntry(() => m_ArrayEntry!.Encode(m_encodeIterator, rel));
        }


        internal void AddFloat(float val)
        {
            if (m_Array.PrimitiveType == 0)
            {
                if (m_Array.ItemLength > 0 && m_Array.ItemLength != 4)
                {
                    throw new OmmInvalidUsageException(
                        $"Unsupported FixedWidth encoding in AddFloat(). Fixed width='{m_Array.ItemLength}'. ",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                m_Array.PrimitiveType = DataTypes.FLOAT;
                AcquireEncodeIterator();
                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.FLOAT)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddFloat() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            floatObject.Value(val);
            AddPrimitiveEntry(() => m_ArrayEntry!.Encode(m_encodeIterator, floatObject));
        }


        internal void AddDouble(double val)
        {
            if (m_Array.PrimitiveType == 0)
            {
                if (m_Array.ItemLength > 0 && m_Array.ItemLength != 8)
                {
                    throw new OmmInvalidUsageException(
                        $"Unsupported FixedWidth encoding in AddDouble(). Fixed width='{m_Array.ItemLength}'. ",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                m_Array.PrimitiveType = DataTypes.DOUBLE;
                AcquireEncodeIterator();
                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.DOUBLE)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddDouble() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            doubleObject.Clear();
            doubleObject.Value(val);

            AddPrimitiveEntry(() => m_ArrayEntry!.Encode(m_encodeIterator, doubleObject));
        }


        internal void AddDate(int year, int month, int day)
        {
            dateObject.Clear();
            dateObject.Year(year);
            dateObject.Month(month);
            dateObject.Day(day);

            if (!dateObject.IsValid)
            {
                throw new OmmOutOfRangeException($"Attempt to specify invalid date. Passed in value is='{month} / {day} / {year}'.");
            }

            if (m_Array.PrimitiveType == 0)
            {
                if (m_Array.ItemLength > 0 && m_Array.ItemLength != 4)
                {
                    throw new OmmInvalidUsageException(
                        $"Unsupported FixedWidth encoding in AddDate(). Fixed width='{m_Array.ItemLength}'. ",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

                }

                m_Array.PrimitiveType = DataTypes.DATE;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.DATE)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddDate() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            AddPrimitiveEntry(() => m_ArrayEntry!.Encode(m_encodeIterator, dateObject));
        }


        internal void AddTime(int hour = 0, int minute = 0, int second = 0, int millisecond = 0, int microsecond = 0, int nanosecond = 0)
        {
            timeObject.Clear();
            timeObject.Hour(hour);
            timeObject.Minute(minute);
            timeObject.Second(second);
            timeObject.Millisecond(millisecond);
            timeObject.Microsecond(microsecond);
            timeObject.Nanosecond(nanosecond);

            if (!timeObject.IsValid)
            {
                throw new OmmOutOfRangeException($"Attempt to specify invalid time. Passed in value is='{hour}:{minute}:{second}.{millisecond}.{microsecond}.{nanosecond}'.");
            }

            if (m_Array.PrimitiveType == 0)
            {
                m_Array.PrimitiveType = DataTypes.TIME;
                AcquireEncodeIterator();
                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.TIME)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddTime() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            if (m_Array.ItemLength == 0 ||
                m_Array.ItemLength == 5 ||
                (m_Array.ItemLength == 3 && millisecond == 0))
            {
                AddPrimitiveEntry(() => m_ArrayEntry!.Encode(m_encodeIterator, timeObject));
            }
            else
            {
                throw new OmmInvalidUsageException(
                    $"Unsupported FixedWidth encoding in AddTime(). Fixed width='{m_Array.ItemLength}'.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            }
        }


        internal void AddDateTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0,
            int millisecond = 0, int microsecond = 0, int nanosecond = 0)
        {
            dateTimeObject.Clear();
            dateTimeObject.Date().Year(year);
            dateTimeObject.Date().Month(month);
            dateTimeObject.Date().Day(day);
            dateTimeObject.Time().Hour(hour);
            dateTimeObject.Time().Minute(minute);
            dateTimeObject.Time().Second(second);
            dateTimeObject.Time().Millisecond(millisecond);
            dateTimeObject.Time().Microsecond(microsecond);
            dateTimeObject.Time().Nanosecond(nanosecond);

            if (!dateTimeObject.IsValid)
            {
                string msg =
                    $"Attempt to specify invalid date time. Passed in value is='{month} / {day} / {year}  {hour}:{minute}:{second}.{millisecond}.{microsecond}.{nanosecond}'.";
                throw new OmmOutOfRangeException(msg);
            }

            if (m_Array.PrimitiveType == 0)
            {
                m_Array.PrimitiveType = DataTypes.DATETIME;
                AcquireEncodeIterator();
                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.DATETIME)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to addDateTime() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            if (m_Array.ItemLength == 0 ||
                m_Array.ItemLength == 9 ||
                (m_Array.ItemLength == 7 && millisecond == 0))
            {
                AddPrimitiveEntry(() => m_ArrayEntry!.Encode(m_encodeIterator, dateTimeObject));
            }
            else
            {
                throw new OmmInvalidUsageException(
                    $"Unsupported FixedWidth encoding in AddDateTime(). Fixed width='{m_Array.ItemLength}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }


        internal void AddQos(uint timeliness = OmmQos.Timelinesses.REALTIME, uint rate = OmmQos.Rates.TICK_BY_TICK)
        {
            if (m_Array.PrimitiveType == 0)
            {
                if (m_Array.ItemLength > 0)
                {
                    throw new OmmInvalidUsageException(
                        $"Unsupported FixedWidth encoding in AddQos(). Fixed width='{m_Array.ItemLength}'.",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                m_Array.PrimitiveType = DataTypes.QOS;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.QOS)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddQos() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            qosObject.Clear();

            Utilities.ToRsslQos(rate, timeliness, qosObject);

            AddPrimitiveEntry(() => m_ArrayEntry!.Encode(m_encodeIterator, qosObject));
        }


        internal void AddState(int streamState = OmmState.StreamStates.OPEN,
            int dataState = OmmState.DataStates.OK,
            int statusCode = OmmState.StatusCodes.NONE,
            string statusText = "")
        {
            if (m_Array.PrimitiveType == 0)
            {
                if (m_Array.ItemLength > 0)
                {
                    throw new OmmInvalidUsageException(
                        $"Unsupported FixedWidth encoding in AddState(). Fixed width='{m_Array.ItemLength}'.",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                m_Array.PrimitiveType = DataTypes.STATE;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.STATE)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddState() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            stateObject.Clear();
            stateObject.StreamState(streamState);
            stateObject.DataState(dataState);
            stateObject.Code(statusCode);
            stateObject.Text().Data(statusText);

            AddPrimitiveEntry(() => m_ArrayEntry!.Encode(m_encodeIterator, stateObject));
        }


        internal void AddEnum(int val)
        {
            if (m_Array.PrimitiveType == 0)
            {
                m_Array.PrimitiveType = DataTypes.ENUM;
                AcquireEncodeIterator();
                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.ENUM)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to addEnum() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            switch (m_Array.ItemLength)
            {
                case 0:
                    break;
                case 1:
                    if (val > 255)
                    {
                        throw new OmmInvalidUsageException(
                            $"Out of range value for the specified fixed width in AddEnum(). Fixed width='{m_Array.ItemLength}' value='{val}'. ",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    }
                    break;
                case 2:
                    break;
                default:
                    {
                        throw new OmmInvalidUsageException(
                            $"Unsupported FixedWidth encoding in AddEnum(). Fixed width='{m_Array.ItemLength}'. ",
                            OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                    }
            }
            enumObject.Clear();
            enumObject.Value(val);

            AddPrimitiveEntry(() => m_ArrayEntry!.Encode(m_encodeIterator, enumObject));
        }


        internal void AddBuffer(EmaBuffer val)
        {
            if (m_Array.PrimitiveType == 0)
            {
                m_Array.PrimitiveType = DataTypes.BUFFER;
                AcquireEncodeIterator();
                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.BUFFER)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddBuffer() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            if (m_Array.ItemLength > 0
                && m_Array.ItemLength < val.Length)
            {
                throw new OmmInvalidUsageException(
                    $"Passed in value is longer than fixed width in AddBuffer(). Fixed width='{m_Array.ItemLength}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            bufferObject.Clear();
            bufferObject.Data(new ByteBuffer(val.AsByteArray()).Flip());

            AddPrimitiveEntry(() => m_ArrayEntry!.Encode(m_encodeIterator, bufferObject));
        }


        internal void AddAscii(string val)
        {
            if (m_Array.PrimitiveType == 0)
            {
                m_Array.PrimitiveType = DataTypes.ASCII_STRING;
                AcquireEncodeIterator();
                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.ASCII_STRING)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddAscii() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            if (m_Array.ItemLength > 0
                && m_Array.ItemLength < val.Length)
            {
                throw new OmmInvalidUsageException(
                    $"Passed in value is longer than fixed width in AddAscii(). Fixed width='{m_Array.ItemLength}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }

            bufferObject.Clear();
            bufferObject.Data(val);

            AddPrimitiveEntry(() => m_ArrayEntry!.Encode(m_encodeIterator, bufferObject));
        }


        internal void AddUtf8(EmaBuffer val)
        {
            if (m_Array.PrimitiveType == 0)
            {
                m_Array.PrimitiveType = DataTypes.UTF8_STRING;
                AcquireEncodeIterator();
                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.UTF8_STRING)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddUtf8() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            if (m_Array.ItemLength > 0
                && m_Array.ItemLength < val.Length)
            {
                throw new OmmInvalidUsageException(
                    $"Passed in value is longer than fixed width in AddUtf8(). Fixed width='{m_Array.ItemLength}'.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }

            bufferObject.Clear();
            bufferObject.Data(new ByteBuffer(val.AsByteArray()).Flip());

            AddPrimitiveEntry(() => m_ArrayEntry!.Encode(m_encodeIterator, bufferObject));
        }


        internal void AddCodeInt()
        {
            if (m_Array.PrimitiveType == 0)
            {
                switch (m_Array.ItemLength)
                {
                    case 0:
                    case 1:
                    case 2:
                    case 4:
                    case 8:
                        break;
                    default:
                        {
                            throw new OmmInvalidUsageException(
                                $"Unsupported FixedWidth encoding in AddCodeInt(). Fixed width='{m_Array.ItemLength}'. ",
                                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                        }
                }

                m_Array.PrimitiveType = DataTypes.INT;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.INT)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddCodeInt() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            AddPrimitiveEntry(() => m_ArrayEntry!.EncodeBlank(m_encodeIterator));
        }


        internal void AddCodeUInt()
        {
            if (m_Array.PrimitiveType == 0)
            {
                switch (m_Array.ItemLength)
                {
                    case 0:
                    case 1:
                    case 2:
                    case 4:
                    case 8:
                        break;
                    default:
                        {
                            throw new OmmInvalidUsageException(
                                $"Unsupported FixedWidth encoding in AddCodeUInt(). Fixed width='{m_Array.ItemLength}'. ",
                                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                        }
                }

                m_Array.PrimitiveType = DataTypes.UINT;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.UINT)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddCodeUInt() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            AddPrimitiveEntry(() => m_ArrayEntry!.EncodeBlank(m_encodeIterator));
        }


        internal void AddCodeReal()
        {
            if (m_Array.PrimitiveType == 0)
            {
                if (m_Array.ItemLength != 0)
                {
                    throw new OmmInvalidUsageException(
                        $"Unsupported FixedWidth encoding in AddCodeReal(). Fixed width='{m_Array.ItemLength}'. ",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                m_Array.PrimitiveType = DataTypes.REAL;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.REAL)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddCodeReal() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            AddPrimitiveEntry(() => m_ArrayEntry!.EncodeBlank(m_encodeIterator));
        }


        internal void AddCodeFloat()
        {
            if (m_Array.PrimitiveType == 0)
            {
                if (m_Array.ItemLength != 0
                    && m_Array.ItemLength != 4)
                {
                    throw new OmmInvalidUsageException(
                        $"Unsupported FixedWidth encoding in AddCodeFloat(). Fixed width='{m_Array.ItemLength}'. ",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                m_Array.PrimitiveType = DataTypes.FLOAT;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.FLOAT)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddCodeFloat() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            AddPrimitiveEntry(() => m_ArrayEntry!.EncodeBlank(m_encodeIterator));
        }


        internal void AddCodeDouble()
        {
            if (m_Array.PrimitiveType == 0)
            {
                if (m_Array.ItemLength != 0
                    && m_Array.ItemLength != 8)
                {
                    throw new OmmInvalidUsageException(
                        $"Unsupported FixedWidth encoding in AddCodeDouble(). Fixed width='{m_Array.ItemLength}'. ",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                m_Array.PrimitiveType = DataTypes.DOUBLE;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.DOUBLE)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddCodeDouble() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            }

            AddPrimitiveEntry(() => m_ArrayEntry!.EncodeBlank(m_encodeIterator));
        }


        internal void AddCodeDate()
        {
            if (m_Array.PrimitiveType == 0)
            {
                if (m_Array.ItemLength != 0
                    && m_Array.ItemLength != 4)
                {
                    throw new OmmInvalidUsageException(
                        $"Unsupported FixedWidth encoding in AddCodeDate(). Fixed width='{m_Array.ItemLength}'. ",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                m_Array.PrimitiveType = DataTypes.DATE;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.DATE)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddCodeDate() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            AddPrimitiveEntry(() => m_ArrayEntry!.EncodeBlank(m_encodeIterator));
        }


        internal void AddCodeTime()
        {
            if (m_Array.PrimitiveType == 0)
            {
                switch (m_Array.ItemLength)
                {
                    case 0:
                    case 3:
                    case 5:
                        break;
                    default:
                        {
                            throw new OmmInvalidUsageException(
                                $"Unsupported FixedWidth encoding in AddCodeTime(). Fixed width='{m_Array.ItemLength}'. ",
                                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                        }
                }

                m_Array.PrimitiveType = DataTypes.TIME;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.TIME)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddCodeTime() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            AddPrimitiveEntry(() => m_ArrayEntry!.EncodeBlank(m_encodeIterator));
        }


        internal void AddCodeDateTime()
        {
            if (m_Array.PrimitiveType == 0)
            {
                switch (m_Array.ItemLength)
                {
                    case 0:
                    case 7:
                    case 9:
                        break;
                    default:
                        {
                            throw new OmmInvalidUsageException(
                                $"Unsupported FixedWidth encoding in AddCodeDateTime(). Fixed width='{m_Array.ItemLength}'. ",
                                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                        }
                }

                m_Array.PrimitiveType = DataTypes.DATETIME;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.DATETIME)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddCodeDateTime() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            }

            AddPrimitiveEntry(() => m_ArrayEntry!.EncodeBlank(m_encodeIterator));
        }


        internal void AddCodeQos()
        {
            if (m_Array.PrimitiveType == 0)
            {
                if (m_Array.ItemLength != 0)
                {
                    throw new OmmInvalidUsageException(
                        $"Unsupported FixedWidth encoding in AddCodeQos(). Fixed width='{m_Array.ItemLength}'. ",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                m_Array.PrimitiveType = DataTypes.QOS;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.QOS)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddCodeQos() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            AddPrimitiveEntry(() => m_ArrayEntry!.EncodeBlank(m_encodeIterator));
        }


        internal void AddCodeState()
        {
            if (m_Array.PrimitiveType == 0)
            {
                if (m_Array.ItemLength != 0)
                {
                    throw new OmmInvalidUsageException(
                        $"Unsupported FixedWidth encoding in AddCodeState(). Fixed width='{m_Array.ItemLength}'. ",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                m_Array.PrimitiveType = DataTypes.STATE;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.STATE)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddCodeState() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            AddPrimitiveEntry(() => m_ArrayEntry!.EncodeBlank(m_encodeIterator));
        }


        internal void AddCodeEnum()
        {
            if (m_Array.PrimitiveType == 0)
            {
                switch (m_Array.ItemLength)
                {
                    case 0:
                    case 1:
                    case 2:
                        break;
                    default:
                        {
                            throw new OmmInvalidUsageException(
                                $"Unsupported FixedWidth encoding in AddCodeEnum(). Fixed width='{m_Array.ItemLength}'. ",
                                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                        }
                }

                m_Array.PrimitiveType = DataTypes.ENUM;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.ENUM)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddCodeEnum() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            AddPrimitiveEntry(() => m_ArrayEntry!.EncodeBlank(m_encodeIterator));
        }


        internal void AddCodeBuffer()
        {
            if (m_Array.PrimitiveType == 0)
            {
                m_Array.PrimitiveType = DataTypes.BUFFER;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.BUFFER)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddCodeBuffer() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            AddPrimitiveEntry(() => m_ArrayEntry!.EncodeBlank(m_encodeIterator));
        }


        internal void AddCodeAscii()
        {
            if (m_Array.PrimitiveType == 0)
            {
                m_Array.PrimitiveType = DataTypes.ASCII_STRING;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.ASCII_STRING)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddCodeAscii() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            AddPrimitiveEntry(() => m_ArrayEntry!.EncodeBlank(m_encodeIterator));
        }


        internal void AddCodeUtf8()
        {
            if (m_Array.PrimitiveType == 0)
            {
                m_Array.PrimitiveType = DataTypes.UTF8_STRING;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.UTF8_STRING)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddCodeUtf8() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            AddPrimitiveEntry(() => m_ArrayEntry!.EncodeBlank(m_encodeIterator));
        }


        internal void AddCodeRmtes()
        {
            if (m_Array.PrimitiveType == 0)
            {
                m_Array.PrimitiveType = DataTypes.RMTES_STRING;

                AcquireEncodeIterator();

                InitEncode();
            }
            else if (m_Array.PrimitiveType != DataTypes.RMTES_STRING)
            {
                throw new OmmInvalidUsageException(
                    $"Attempt to AddCodeRmtes() while OmmArray contains='{DataTypes.ToString(m_Array.PrimitiveType)}'. ",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            AddPrimitiveEntry(() => m_ArrayEntry!.EncodeBlank(m_encodeIterator));
        }


        internal void Complete()
        {
            if (m_containerComplete)
                return;

            if (m_Array.PrimitiveType == 0)
            {
                throw new OmmInvalidUsageException("Attempt to call Complete() while no entries were added yet.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            CodecReturnCode retCode = m_Array.EncodeComplete(m_encodeIterator, true);

            if (retCode < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to complete OmmArray encoding. Reason='{retCode.GetAsString()}'. ");
            }

            if (!OwnsIterator() && m_iteratorOwner != null)
            {
                m_iteratorOwner!.EndEncodingEntry!();
            }

            m_containerComplete = true;
        }


        public override void Clear()
        {
            base.Clear();
            m_Array.Clear();
            m_ArrayEntry.Clear();
        }
    }
}
