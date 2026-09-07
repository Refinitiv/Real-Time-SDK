/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Domain.Internal;
using LSEG.Ema.Rdm;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using System;
using System.Text;

using Buffer = LSEG.Eta.Codec.Buffer;
using Date = LSEG.Eta.Codec.Date;
using Double = LSEG.Eta.Codec.Double;

namespace LSEG.Ema.Domain.Directory
{
    /// <summary>
    /// The RDM Service Data. Contains information provided by the Source
    /// Directory Data filter.
    /// </summary>
    public sealed class DirectoryServiceData : DirectoryServiceFilter<Access.ElementList, DirectoryServiceData>, ICloneable
    {
        private readonly StringBuilder m_ToString = new();
        private Data? m_Data = null;
        private readonly EncodeIterator m_EncIterator = new();
        private readonly DecodeIterator m_DecIterator = new();
        private readonly Buffer m_Buffer = new();
        private ContentType m_Type;

        /// <summary>
        /// Explains the content of the ServiceData.
        /// </summary>
        public enum ContentType
        {
            /// <summary>
            /// Indicates that no options are specified.
            /// </summary>
            NONE = 0,
            /// <summary>
            /// Time content.
            /// </summary>
            TIME = 1,
            /// <summary>
            /// Alert content.
            /// </summary>
            ALERT = 2,
            /// <summary>
            /// Headline content.
            /// </summary>
            HEADLINE = 3,
            /// <summary>
            /// Status content.
            /// </summary>
            STATUS = 4
        }

        /// <inheritdoc />
        public override int FilterId => EmaRdm.SERVICE_DATA_ID;

        /// <summary>
        /// Indicates the presence of the data field.
        /// </summary>
        public bool HasData => m_Data != null;

        /// <summary>
        /// dataType - The OMM type of the data. Populated by <see cref="LSEG.Ema.Access.DataType.DataTypes" />.
        /// </summary>
        public int DataType
        {
            get
            {
                if (m_Data == null)
                    throw new OmmInvalidUsageException(
                        EmaRdm.ENAME_DATA + " element is not set",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                return m_Data.DataType;
            }
        }

        /// <summary>
        /// Directory content type. Populated by <see cref="ContentType" />.
        /// </summary>
        public ContentType Type() => m_Type;
        /// <summary>
        /// Directory content type. Populated by <see cref="ContentType" />.
        /// </summary>
        public DirectoryServiceData Type(ContentType value)
        {
            var intValue = (int)value;
            if (!(intValue is >= 0 and <= 1023))
                throw new OmmInvalidUsageException(
                    $"Invalid element value {value} of {EmaRdm.ENAME_TYPE}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            m_Type = value;
            return this;
        }

        /// <summary>
        /// Data object representing the encoded data, to be applied to all items being provided by this
        /// service.
        /// </summary>
        public Data Data()
        {
            if (m_Data == null)
                throw new OmmInvalidUsageException(
                    EmaRdm.ENAME_DATA + " element is not set",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            return m_Data;
        }

        /// <summary>
        /// Sets encoded data that represents int type for this service.
        /// </summary>
        /// <param name="data">the data.</param>
        public DirectoryServiceData DataAsInt(long data)
        {
            var ommData = new OmmInt();
            var etaData = new Int();
            etaData.Value(data);

            ConvertToOmmDataViaEncoding(ommData, 15, encIter => etaData.Encode(encIter));

            m_Data = ommData;
            return this;
        }

        /// <summary>
        /// Sets encoded data that represents unsigned int type for this service.
        /// </summary>
        /// <param name="data">the data.</param>
        public DirectoryServiceData DataAsUInt(ulong data)
        {
            var ommData = new OmmUInt();
            var etaData = new UInt();
            etaData.Value(data);

            ConvertToOmmDataViaEncoding(ommData, 15, encIter => etaData.Encode(encIter));

            m_Data = ommData;
            return this;
        }

        /// <summary>
        /// Sets encoded data that represents float type for this service.
        /// </summary>
        /// <param name="data">the data.</param>
        public DirectoryServiceData DataAsFloat(float data)
        {
            var ommData = new OmmFloat();
            var etaData = new Float();
            etaData.Value(data);

            ConvertToOmmDataViaEncoding(ommData, 15, encIter => etaData.Encode(encIter));

            m_Data = ommData;
            return this;
        }

        /// <summary>
        /// Sets encoded data that represents double type for this service.
        /// </summary>
        /// <param name="data">the data.</param>
        public DirectoryServiceData DataAsDouble(double data)
        {
            var ommData = new OmmDouble();
            var etaData = new Double();
            etaData.Value(data);

            ConvertToOmmDataViaEncoding(ommData, 15, encIter => etaData.Encode(encIter));

            m_Data = ommData;
            return this;
        }

        /// <summary>
        /// Sets encoded data that represents buffer type for this service.
        /// </summary>
        /// <param name="data">the data</param>
        public DirectoryServiceData DataAsBuffer(ByteBuffer data)
        {
            if (data == null)
                throw new OmmInvalidUsageException("data can not be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            var ommData = new OmmBuffer();
            ommData.Value.AssignFrom(data.Contents, 0, data.Position);

            m_Data = ommData;
            return this;
        }

        /// <summary>
        /// Sets encoded data that represents ascii type for this service.
        /// </summary>
        /// <param name="data">the data</param>
        public DirectoryServiceData DataAsAscii(string data)
        {
            if (data == null)
                throw new OmmInvalidUsageException("data can not be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            m_Data = new OmmAscii();
            m_Data.m_bodyBuffer!.Data(ByteBuffer.Wrap(Encoding.ASCII.GetBytes(data)));
            return this;
        }

        /// <summary>
        /// Sets encoded data that represents utf8 string type for this service.
        /// </summary>
        /// <param name="data">the data</param>
        public DirectoryServiceData DataAsUtf8(string data)
        {
            if (data == null)
                throw new OmmInvalidUsageException("data can not be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            m_Data = new OmmUtf8();
            m_Data.m_bodyBuffer!.Data(ByteBuffer.Wrap(Encoding.UTF8.GetBytes(data)));
            return this;
        }

        /// <summary>
        /// Sets encoded data that represents Rmtes string type for this service.
        /// </summary>
        /// <param name="data">the data</param>
        public DirectoryServiceData DataAsRmtes(Access.RmtesBuffer data)
        {
            if (data == null)
                throw new OmmInvalidUsageException("data can not be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            var ommData = new OmmRmtes();
            ommData.Value.Apply(data);
            m_Data = ommData;
            return this;
        }

        /// <summary>
        /// Sets encoded data that represents real type for this service.
        /// </summary>
        /// <param name="mantissa">the mantissa</param>
        /// <param name="magnitudeType">the magnitude type</param>
        public DirectoryServiceData DataAsReal(long mantissa, int magnitudeType)
        {
            if (magnitudeType < RealHints.EXPONENT_14 || magnitudeType > RealHints.NOT_A_NUMBER)
                throw new OmmInvalidUsageException(
                    "magnitudeType should be in range of MagnitudeType values",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            var ommData = new OmmReal();
            var etaData = new Real();
            etaData.Value(mantissa, magnitudeType);

            ConvertToOmmDataViaEncoding(ommData, 15, encIter => etaData.Encode(encIter));

            m_Data = ommData;
            return this;
        }

        /// <summary>
        /// Sets encoded data that represents date type for this service.
        /// </summary>
        /// <param name="year">the year</param>
        /// <param name="month">the month</param>
        /// <param name="day">the day</param>
        public DirectoryServiceData DataAsDate(int year, int month, int day)
        {
            var ommData = new OmmDate();
            var etaData = new Date();
            etaData.Year(year);
            etaData.Month(month);
            etaData.Day(day);

            ConvertToOmmDataViaEncoding(ommData, 15, encIter => etaData.Encode(encIter));

            m_Data = ommData;
            return this;
        }

        /// <summary>
        /// Sets encoded data that represents date type for this service.
        /// </summary>
        /// <param name="date">the date</param>
        public DirectoryServiceData DataAsDate(DateOnly date)
        {
            var ommData = new OmmDate();
            var etaData = new Date();
            etaData.Year(date.Year);
            etaData.Month(date.Month);
            etaData.Day(date.Day);

            ConvertToOmmDataViaEncoding(ommData, 15, encIter => etaData.Encode(encIter));

            m_Data = ommData;
            return this;
        }

        /// <summary>
        /// Sets encoded data that represents time type for this service.
        /// </summary>
        /// <param name="hour">the hour</param>
        /// <param name="minute">the minute</param>
        /// <param name="second">the second</param>
        /// <param name="millisecond">the millisecond</param>
        /// <param name="microsecond">the microsecond</param>
        /// <param name="nanosecond">the nanosecond</param>
        public DirectoryServiceData DataAsTime(int hour, int minute, int second, int millisecond, int microsecond, int nanosecond)
        {
            var ommData = new OmmTime();
            var etaData = new Time();
            etaData.Hour(hour);
            etaData.Minute(minute);
            etaData.Second(second);
            etaData.Millisecond(millisecond);
            etaData.Microsecond(microsecond);
            etaData.Nanosecond(nanosecond);

            ConvertToOmmDataViaEncoding(ommData, 15, encIter => etaData.Encode(encIter));

            m_Data = ommData;
            return this;
        }

        /// <summary>
        /// Sets encoded data that represents time type for this service.
        /// </summary>
        /// <param name="time">the time</param>
        public DirectoryServiceData DataAsTime(TimeOnly time)
        {
            var ommData = new OmmTime();
            var etaData = new Time();
            etaData.Hour(time.Hour);
            etaData.Minute(time.Minute);
            etaData.Second(time.Second);
            etaData.Millisecond(time.Millisecond);
            etaData.Microsecond(time.Microsecond);
            etaData.Nanosecond(time.Nanosecond);

            ConvertToOmmDataViaEncoding(ommData, 15, encIter => etaData.Encode(encIter));

            m_Data = ommData;
            return this;
        }

        /// <summary>
        /// Sets encoded data that represents date and time type for this service.
        /// </summary>
        /// <param name="year">the year</param>
        /// <param name="month">the month</param>
        /// <param name="day">the day</param>
        /// <param name="hour">the hour</param>
        /// <param name="minute">the minute</param>
        /// <param name="second">the second</param>
        /// <param name="millisecond">the millisecond</param>
        /// <param name="microsecond">the microsecond</param>
        /// <param name="nanosecond">the nanosecond</param>
        public DirectoryServiceData DataAsDateTime(int year, int month, int day, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond)
        {
            var ommData = new OmmTime();
            var etaData = new Eta.Codec.DateTime();
            etaData.Year(year);
            etaData.Month(month);
            etaData.Day(day);
            etaData.Hour(hour);
            etaData.Minute(minute);
            etaData.Second(second);
            etaData.Millisecond(millisecond);
            etaData.Microsecond(microsecond);
            etaData.Nanosecond(nanosecond);

            ConvertToOmmDataViaEncoding(ommData, 15, encIter => etaData.Encode(encIter));

            m_Data = ommData;
            return this;
        }

        /// <summary>
        /// Sets encoded data that represents date and time type for this service.
        /// </summary>
        /// <param name="dateTime">the date time</param>
        public DirectoryServiceData DataAsDateTime(System.DateTime dateTime)
        {
            var ommData = new OmmTime();
            var etaData = new Eta.Codec.DateTime();
            etaData.Year(dateTime.Year);
            etaData.Month(dateTime.Month);
            etaData.Day(dateTime.Day);
            etaData.Hour(dateTime.Hour);
            etaData.Minute(dateTime.Minute);
            etaData.Second(dateTime.Second);
            etaData.Millisecond(dateTime.Millisecond);
            etaData.Microsecond(dateTime.Microsecond);
            etaData.Nanosecond(dateTime.Nanosecond);

            ConvertToOmmDataViaEncoding(ommData, 15, encIter => etaData.Encode(encIter));

            m_Data = ommData;
            return this;
        }

        /// <summary>
        /// Sets encoded data that represents Qos type for this service.
        /// </summary>
        /// <param name="timeliness">the timeliness</param>
        /// <param name="rate">the rate</param>
        public DirectoryServiceData DataAsQos(uint timeliness, uint rate)
        {
            var ommQos = new OmmQos().Populate(timeliness, rate);
            m_Data = ommQos;

            return this;
        }

        /// <summary>
        /// Sets encoded data that represents State type for this service.
        /// </summary>
        /// <param name="streamState">represents OmmState StreamState</param>
        /// <param name="dataState">represents OmmState DataState</param>
        /// <param name="statusCode">represents OmmState StatusCode</param>
        /// <param name="statusText">represents OmmState StatusText</param>
        /// <exception cref="OmmInvalidUsageException"></exception>
        public DirectoryServiceData DataAsState(int streamState, int dataState, int statusCode, string statusText)
        {
            if (statusText == null)
                throw new OmmInvalidUsageException("statusText cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            var rsslState = new State();
            var stateText = new Buffer();

            rsslState.StreamState(streamState);
            rsslState.DataState(dataState);
            rsslState.Code(statusCode);
            stateText.Data(statusText);
            rsslState.Text(stateText);

            var ommState = new OmmState();
            ommState.Decode(rsslState);

            m_Data = ommState;
            return this;
        }

        /// <summary>
        /// Sets encoded data that represents enum type for this service.
        /// </summary>
        /// <param name="data">the data</param>
        public DirectoryServiceData DataAsEnum(ushort data)
        {
            var ommData = new OmmEnum();
            var etaData = new Eta.Codec.Enum();
            etaData.Value(data);

            ConvertToOmmDataViaEncoding(ommData, 15, encIter => etaData.Encode(encIter));

            m_Data = ommData;
            return this;
        }

        /// <summary>
        /// Sets encoded data that represents array type for this service.
        /// </summary>
        /// <param name="data">the data</param>
        public DirectoryServiceData DataAsArray(OmmArray data)
        {
            if (data == null)
                throw new OmmInvalidUsageException("data cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            m_Data = new OmmArray();

            data.m_bodyBuffer!.Overwrite(m_Data.m_bodyBuffer);

            return this;
        }

        /// <summary>
        /// Sets encoded data that inherits from ComplexType for this service.
        /// </summary>
        /// <param name="data">the data</param>
        public DirectoryServiceData DataAsComplexType(ComplexType data)
        {
            if (data == null)
                throw new OmmInvalidUsageException("data cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            var isNoData = data.DataType == Access.DataType.DataTypes.NO_DATA;

            m_Data = data.DataType switch
            {
                Access.DataType.DataTypes.FIELD_LIST => new Access.FieldList(),
                Access.DataType.DataTypes.MAP => new Access.Map(),
                Access.DataType.DataTypes.ELEMENT_LIST => new Access.ElementList(),
                Access.DataType.DataTypes.FILTER_LIST => new Access.FilterList(),
                Access.DataType.DataTypes.VECTOR => new Access.Vector(),
                Access.DataType.DataTypes.SERIES => new Access.Series(),
                Access.DataType.DataTypes.OPAQUE => new OmmOpaque(),
                Access.DataType.DataTypes.ANSI_PAGE => new OmmAnsiPage(),
                Access.DataType.DataTypes.XML => new OmmXml(),
                Access.DataType.DataTypes.JSON => new OmmJson(),
                Access.DataType.DataTypes.REQ_MSG => new RequestMsg(),
                Access.DataType.DataTypes.REFRESH_MSG => new RefreshMsg(),
                Access.DataType.DataTypes.STATUS_MSG => new StatusMsg(),
                Access.DataType.DataTypes.UPDATE_MSG => new UpdateMsg(),
                Access.DataType.DataTypes.ACK_MSG => new AckMsg(),
                Access.DataType.DataTypes.POST_MSG => new PostMsg(),
                Access.DataType.DataTypes.GENERIC_MSG => new GenericMsg(),
                Access.DataType.DataTypes.NO_DATA => new NoData(),
                _ => throw new OmmInvalidUsageException(
                        $"Data type ({Access.DataType.AsString(data.DataType)}) is not supported",
                        OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT),
            };

            if (!isNoData)
            {
                data.m_bodyBuffer!.Overwrite(m_Data.m_bodyBuffer);
            }
            return this;
        }

        /// <inheritdoc />
        public override DirectoryServiceData Clear()
        {
            base.Clear();
            Type(ContentType.NONE);
            m_Data = null;
            return this;
        }

        /// <summary>
        /// Creates a copy of this instance.
        /// </summary>
        /// <returns>A new <see cref="DirectoryServiceData"/> instance that is a copy of this instance.</returns>
        public DirectoryServiceData Clone() => new DirectoryServiceData().CopyFrom(this);

        /// <inheritdoc />
        object ICloneable.Clone() => Clone();

        /// <inheritdoc />
        public override DirectoryServiceData CopyFrom(DirectoryServiceData source)
        {
            base.CopyFrom(source);
            m_Type = source.m_Type;
            if (source.HasData)
            {
                SetData(source.m_Data);
            }

            return this;
        }

        /// <inheritdoc />
        public override string ToString()
        {
            m_ToString.Clear();
            AppendToString(m_ToString, 0);
            return m_ToString.ToString();
        }

        internal override void AppendToString(StringBuilder sb, int indent)
        {
            base.AppendToString(sb, indent);
            sb.AddIndent(indent).AppendLine($"Type: {m_Type}");
            if (HasData)
            {
                sb.AddIndent(indent).AppendLine($"Data:");
                sb.AddIndent(indent + 1).AppendLine(m_Data!.ToString());
                sb.AddIndent(indent).AppendLine($"EndData");
            }
        }

        internal override void DecodeFrom(Access.ElementList elementList)
        {
            Clear();

            var foundType = false;
            var foundData = false;
            foreach (var elementEntry in elementList)
            {
                var elementName = elementEntry.Name;
                switch (elementName)
                {
                    case EmaRdm.ENAME_TYPE:
                        var type = elementEntry.UIntValue();
                        Type((ContentType)type);
                        foundType = true;
                        break;
                    case EmaRdm.ENAME_DATA:
                        var data = elementEntry.Load;
                        if (!SetData(data))
                            throw new OmmInvalidUsageException(
                                $"Invalid element value {data?.DataType} of {elementName}",
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        foundData = true;
                        break;
                    default:
                        break;
                }
            }

            // If Data element is present, type must be too.
            if ((foundData && !foundType) || (foundType && !foundData))
            {
                Clear();
                throw new OmmInvalidUsageException(
                    $"If {EmaRdm.ENAME_DATA} is present, {EmaRdm.ENAME_TYPE} must be too",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            }
        }

        internal override void EncodeTo(Access.ElementList elementList)
        {
            if (!HasData)
                return;

            elementList.AddUInt(EmaRdm.ENAME_TYPE, (ulong)m_Type);

            switch (m_Data?.DataType)
            {
                case Access.DataType.DataTypes.INT:
                    elementList.AddInt(EmaRdm.ENAME_DATA, ((OmmInt)m_Data).Value);
                    break;
                case Access.DataType.DataTypes.UINT:
                    elementList.AddUInt(EmaRdm.ENAME_DATA, ((OmmUInt)m_Data).Value);
                    break;
                case Access.DataType.DataTypes.FLOAT:
                    elementList.AddFloat(EmaRdm.ENAME_DATA, ((OmmFloat)m_Data).Value);
                    break;
                case Access.DataType.DataTypes.DOUBLE:
                    elementList.AddDouble(EmaRdm.ENAME_DATA, ((OmmDouble)m_Data).Value);
                    break;
                case Access.DataType.DataTypes.BUFFER:
                    elementList.AddBuffer(EmaRdm.ENAME_DATA, ((OmmBuffer)m_Data).Value);
                    break;
                case Access.DataType.DataTypes.ASCII:
                    elementList.AddAscii(EmaRdm.ENAME_DATA, ((OmmAscii)m_Data).Value);
                    break;
                case Access.DataType.DataTypes.UTF8:
                    elementList.AddUtf8(EmaRdm.ENAME_DATA, ((OmmUtf8)m_Data).Value);
                    break;
                case Access.DataType.DataTypes.RMTES:
                    elementList.AddRmtes(EmaRdm.ENAME_DATA, ((OmmRmtes)m_Data).AsHex());
                    break;
                case Access.DataType.DataTypes.REAL:
                    elementList.AddReal(EmaRdm.ENAME_DATA, ((OmmReal)m_Data).Mantissa, ((OmmReal)m_Data).MagnitudeType);
                    break;
                case Access.DataType.DataTypes.DATE:
                    elementList.AddDate(EmaRdm.ENAME_DATA, ((OmmDate)m_Data).Year, ((OmmDate)m_Data).Month,
                            ((OmmDate)m_Data).Day);
                    break;
                case Access.DataType.DataTypes.TIME:
                    elementList.AddTime(EmaRdm.ENAME_DATA, ((OmmTime)m_Data).Hour, ((OmmTime)m_Data).Minute,
                            ((OmmTime)m_Data).Second, ((OmmTime)m_Data).Millisecond, ((OmmTime)m_Data).Microsecond,
                            ((OmmTime)m_Data).Nanosecond);
                    break;
                case Access.DataType.DataTypes.DATETIME:
                    elementList.AddDateTime(EmaRdm.ENAME_DATA, ((OmmDateTime)m_Data).Year, ((OmmDateTime)m_Data).Month,
                            ((OmmDateTime)m_Data).Day, ((OmmDateTime)m_Data).Hour, ((OmmDateTime)m_Data).Minute,
                            ((OmmDateTime)m_Data).Second, ((OmmDateTime)m_Data).Millisecond,
                            ((OmmDateTime)m_Data).Microsecond, ((OmmDateTime)m_Data).Nanosecond);
                    break;
                case Access.DataType.DataTypes.QOS:
                    elementList.AddQos(EmaRdm.ENAME_DATA, ((OmmQos)m_Data).Timeliness, ((OmmQos)m_Data).Rate);
                    break;
                case Access.DataType.DataTypes.STATE:
                    elementList.AddState(EmaRdm.ENAME_DATA, ((OmmState)m_Data).StreamState, ((OmmState)m_Data).DataState,
                            ((OmmState)m_Data).StatusCode, ((OmmState)m_Data).StatusText);
                    break;
                case Access.DataType.DataTypes.ENUM:
                    elementList.AddEnum(EmaRdm.ENAME_DATA, ((OmmEnum)m_Data).Value);
                    break;
                case Access.DataType.DataTypes.ARRAY:
                    elementList.AddArray(EmaRdm.ENAME_DATA, ((OmmArray)m_Data));
                    break;
                case Access.DataType.DataTypes.FIELD_LIST:
                    elementList.AddFieldList(EmaRdm.ENAME_DATA, ((Access.FieldList)m_Data));
                    break;
                case Access.DataType.DataTypes.MAP:
                    elementList.AddMap(EmaRdm.ENAME_DATA, ((Access.Map)m_Data));
                    break;
                case Access.DataType.DataTypes.ELEMENT_LIST:
                    elementList.AddElementList(EmaRdm.ENAME_DATA, ((Access.ElementList)m_Data));
                    break;
                case Access.DataType.DataTypes.FILTER_LIST:
                    elementList.AddFilterList(EmaRdm.ENAME_DATA, ((Access.FilterList)m_Data));
                    break;
                case Access.DataType.DataTypes.VECTOR:
                    elementList.AddVector(EmaRdm.ENAME_DATA, ((Access.Vector)m_Data));
                    break;
                case Access.DataType.DataTypes.SERIES:
                    elementList.AddSeries(EmaRdm.ENAME_DATA, ((Access.Series)m_Data));
                    break;
                case Access.DataType.DataTypes.OPAQUE:
                    elementList.AddOpaque(EmaRdm.ENAME_DATA, ((OmmOpaque)m_Data));
                    break;
                case Access.DataType.DataTypes.ANSI_PAGE:
                    elementList.AddAnsiPage(EmaRdm.ENAME_DATA, ((OmmAnsiPage)m_Data));
                    break;
                case Access.DataType.DataTypes.XML:
                    elementList.AddXml(EmaRdm.ENAME_DATA, ((OmmXml)m_Data));
                    break;
                case Access.DataType.DataTypes.JSON:
                    elementList.AddJson(EmaRdm.ENAME_DATA, ((OmmJson)m_Data));
                    break;
                case Access.DataType.DataTypes.REQ_MSG:
                    elementList.AddRequestMsg(EmaRdm.ENAME_DATA, ((RequestMsg)m_Data));
                    break;
                case Access.DataType.DataTypes.REFRESH_MSG:
                    elementList.AddRefreshMsg(EmaRdm.ENAME_DATA, ((RefreshMsg)m_Data));
                    break;
                case Access.DataType.DataTypes.STATUS_MSG:
                    elementList.AddStatusMsg(EmaRdm.ENAME_DATA, ((StatusMsg)m_Data));
                    break;
                case Access.DataType.DataTypes.UPDATE_MSG:
                    elementList.AddUpdateMsg(EmaRdm.ENAME_DATA, ((UpdateMsg)m_Data));
                    break;
                case Access.DataType.DataTypes.ACK_MSG:
                    elementList.AddAckMsg(EmaRdm.ENAME_DATA, ((AckMsg)m_Data));
                    break;
                case Access.DataType.DataTypes.POST_MSG:
                    elementList.AddPostMsg(EmaRdm.ENAME_DATA, ((PostMsg)m_Data));
                    break;
                case Access.DataType.DataTypes.GENERIC_MSG:
                    elementList.AddGenericMsg(EmaRdm.ENAME_DATA, ((GenericMsg)m_Data));
                    break;
                case Access.DataType.DataTypes.NO_DATA:
                default:
                    elementList.AddNoData(EmaRdm.ENAME_DATA);
                    break;
            }
        }

        private void ConvertToOmmDataViaEncoding(Data ommData, int bufferSize, Action<EncodeIterator> encode)
        {
            m_EncIterator.Clear();
            m_Buffer.Data(new ByteBuffer(bufferSize));
            m_EncIterator.SetBufferAndRWFVersion(m_Buffer, Codec.MajorVersion(), Codec.MinorVersion());

            encode(m_EncIterator);

            m_DecIterator.Clear();
            m_DecIterator.SetBufferAndRWFVersion(m_Buffer, Codec.MajorVersion(), Codec.MinorVersion());

            ommData.Decode(m_DecIterator);
        }

        private bool SetData(Data? data)
        {
            switch (data?.DataType)
            {
                case Access.DataType.DataTypes.INT:
                    DataAsInt(((OmmInt)data).Value);
                    break;
                case Access.DataType.DataTypes.UINT:
                    DataAsUInt(((OmmUInt)data).Value);
                    break;
                case Access.DataType.DataTypes.FLOAT:
                    DataAsFloat(((OmmFloat)data).Value);
                    break;
                case Access.DataType.DataTypes.DOUBLE:
                    DataAsDouble(((OmmDouble)data).Value);
                    break;
                case Access.DataType.DataTypes.BUFFER:
                    DataAsBuffer(((OmmBuffer)data)?.m_bodyBuffer?.Data()!);
                    break;
                case Access.DataType.DataTypes.ASCII:
                    DataAsAscii(((OmmAscii)data).Value);
                    break;
                case Access.DataType.DataTypes.UTF8:
                    DataAsUtf8(((OmmUtf8)data).AsString());
                    break;
                case Access.DataType.DataTypes.RMTES:
                    DataAsRmtes(((OmmRmtes)data).Value);
                    break;
                case Access.DataType.DataTypes.REAL:
                    DataAsReal(((OmmReal)data).Mantissa, ((OmmReal)data).MagnitudeType);
                    break;
                case Access.DataType.DataTypes.DATE:
                    DataAsDate(((OmmDate)data).Year, ((OmmDate)data).Month, ((OmmDate)data).Day);
                    break;
                case Access.DataType.DataTypes.TIME:
                    DataAsTime(((OmmTime)data).Hour, ((OmmTime)data).Minute, ((OmmTime)data).Second,
                            ((OmmTime)data).Millisecond, ((OmmTime)data).Microsecond, ((OmmTime)data).Nanosecond);
                    break;
                case Access.DataType.DataTypes.DATETIME:
                    DataAsDateTime(((OmmDateTime)data).Year, ((OmmDateTime)data).Month, ((OmmDateTime)data).Day,
                            ((OmmDateTime)data).Hour, ((OmmDateTime)data).Minute, ((OmmDateTime)data).Second,
                            ((OmmDateTime)data).Millisecond, ((OmmDateTime)data).Microsecond,
                            ((OmmDateTime)data).Nanosecond);
                    break;
                case Access.DataType.DataTypes.QOS:
                    DataAsQos(((OmmQos)data).Timeliness, ((OmmQos)data).Rate);
                    break;
                case Access.DataType.DataTypes.STATE:
                    DataAsState(((OmmState)data).StreamState, ((OmmState)data).DataState, ((OmmState)data).StatusCode,
                            ((OmmState)data).StatusText);
                    break;
                case Access.DataType.DataTypes.ENUM:
                    DataAsEnum(((OmmEnum)data).Value);
                    break;
                case Access.DataType.DataTypes.ARRAY:
                case Access.DataType.DataTypes.FIELD_LIST:
                case Access.DataType.DataTypes.MAP:
                case Access.DataType.DataTypes.ELEMENT_LIST:
                case Access.DataType.DataTypes.FILTER_LIST:
                case Access.DataType.DataTypes.VECTOR:
                case Access.DataType.DataTypes.SERIES:
                case Access.DataType.DataTypes.OPAQUE:
                case Access.DataType.DataTypes.ANSI_PAGE:
                case Access.DataType.DataTypes.XML:
                case Access.DataType.DataTypes.JSON:
                case Access.DataType.DataTypes.REQ_MSG:
                case Access.DataType.DataTypes.REFRESH_MSG:
                case Access.DataType.DataTypes.STATUS_MSG:
                case Access.DataType.DataTypes.UPDATE_MSG:
                case Access.DataType.DataTypes.ACK_MSG:
                case Access.DataType.DataTypes.POST_MSG:
                case Access.DataType.DataTypes.GENERIC_MSG:
                case Access.DataType.DataTypes.NO_DATA:
                    DataAsComplexType((ComplexType)data);
                    break;
                default:
                    return false;
            }
            return true;
        }
    }
}
