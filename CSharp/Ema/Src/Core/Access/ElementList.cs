/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using Buffer = LSEG.Eta.Codec.Buffer;
using DateTime = LSEG.Eta.Codec.DateTime;
using Double = LSEG.Eta.Codec.Double;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// ElementList is a heterogeneous container of complex and primitive data type entries.
    /// </summary>
    /// <remarks>
    /// ElementList entries are identified by name.<br/>
    /// Objects of this class are intended to be short lived or rather transitional.<br/>
    /// This class is designed to efficiently perform setting and extracting of ElementList and its content.<br/>
    /// Objects of this class are not cache-able.<br/>
    /// </remarks>
    public sealed class ElementList : ComplexType, IEnumerable<ElementEntry>
    {
        internal Eta.Codec.ElementList m_rsslElementList = new Eta.Codec.ElementList();
        private ElementListEncoder m_elementListEncoder;
        private LocalElementSetDefDb? m_localElementSetDefDb;

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
        private ByteBuffer byteBuffer = new ByteBuffer(BYTE_BUFFER_INIT_SIZE);

        /// <summary>
        /// Constructor for ElementList
        /// </summary>
        public ElementList()
        {
            var encoder = new ElementListEncoder(this);

            Encoder = encoder;
            m_elementListEncoder = encoder;

            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            ClearTypeSpecific_All = ClearElementList_All;
            ClearTypeSpecific_Decode = ClearElementList_Decode;
            DecodeComplexType = DecodeElementList;
            m_dataType = Access.DataType.DataTypes.ELEMENT_LIST;
        }

        /// <summary>
        /// Indicates presence of Info.
        /// </summary>
        /// <returns>True if the element list has the info parameters, false otherwise.</returns>

        public bool HasInfo { get => m_rsslElementList.CheckHasInfo(); }

        /// <summary>
        /// Returns the Element List Record Template Number value.<br/>
        /// A record template contains information about all possible entries contained
		/// in the stream and is typically used by caching mechanisms to pre-allocate
		/// storage. Must be in the range of -32768 - 32767.
        /// </summary>
        /// <returns>ElementList number</returns>
        public int InfoElementListNum()
        {
            return m_rsslElementList.ElementListNum;
        }

        /// <summary>
        /// Clears the ElementList. Allows the re-use of ElementList instance during encoding.
        /// </summary>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        public ElementList Clear()
        {
            Clear_All();
            return this;
        }

        internal void ClearElementList_All()
        {
            m_rsslElementList.Clear();
            m_localElementSetDefDb = null;
            m_dataDictionary = null;
        }

        internal void ClearElementList_Decode()
        {
            m_localElementSetDefDb = null;
            m_dataDictionary = null;
        }

        /// <summary>
        /// Specifies that the Element List Info parameter contains an Element List Record Number.
        /// The ElementList Info is optional. If used, it must be specified before adding anything to ElementList.
        /// </summary>
        /// <param name="elementListNum">Element List record template number</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        public ElementList Info(int elementListNum)
        {
            m_elementListEncoder.Info(elementListNum);
            return this;
        }

        /// <summary>
        /// Returns <see cref="IEnumerator{ElementEntry}"/> instance that iterates through current ElementList's entries.
        /// </summary>
        /// <returns><see cref="IEnumerator{ElementEntry}"/> object.</returns>
        public IEnumerator<ElementEntry> GetEnumerator()
        {
            if (m_errorCode != OmmError.ErrorCodes.NO_ERROR || !m_hasDecodedDataSet)
            {
                var errorEnumerator = m_objectManager.GetElementListErrorEnumerator();
                errorEnumerator.SetError(m_errorCode);
                return errorEnumerator;
            }

            var decoder = m_objectManager.GetElementListEnumerator();
            if (!decoder.SetData(m_MajorVersion, m_MinorVersion, m_bodyBuffer!, m_dataDictionary, m_localElementSetDefDb))
            {
                decoder.Dispose();
                var errorEnumerator = m_objectManager!.GetElementListErrorEnumerator();
                errorEnumerator.SetError(decoder.m_decodingError.ErrorCode);
                return errorEnumerator;
            }
            return decoder;
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        /// <summary>
        /// Performs intital decoding of ElementList
        /// </summary>
        /// <param name="majorVersion">the major version</param>
        /// <param name="minorVersion">the minor version</param>
        /// <param name="body"><see cref="Buffer"/> instance representing the ElementList</param>
        /// <param name="dictionary"><see cref="DataDictionary"/> instance used to decode ElementList</param>
        /// <param name="localDb"><see cref="LocalElementSetDefDb"/> instance used to decode current ElementList</param>
        /// <returns><see cref="CodecReturnCode"/> value indicating the status of the operation</returns>
        internal CodecReturnCode DecodeElementList(int majorVersion, int minorVersion, Buffer body, DataDictionary? dictionary, object? localDb)
        {
            m_hasDecodedDataSet = true;

            m_MajorVersion = majorVersion;
            m_MinorVersion = minorVersion;
            m_bodyBuffer = body;
            m_localElementSetDefDb = (LocalElementSetDefDb?)localDb;
            m_dataDictionary = dictionary;

            CodecReturnCode ret;
            m_decodeIterator.Clear();
            if ((ret = m_decodeIterator.SetBufferAndRWFVersion(m_bodyBuffer, m_MajorVersion, m_MinorVersion)) != CodecReturnCode.SUCCESS)
            {
                m_errorCode = OmmError.ErrorCodes.ITERATOR_SET_FAILURE;
                return ret;
            }
            ret = m_rsslElementList.Decode(m_decodeIterator, m_localElementSetDefDb);
            switch (ret)
            {
                case CodecReturnCode.NO_DATA:
                    m_errorCode = OmmError.ErrorCodes.NO_ERROR;
                    m_rsslElementList.Flags = 0;
                    break;

                case CodecReturnCode.SUCCESS:
                    m_errorCode = OmmError.ErrorCodes.NO_ERROR;
                    break;

                case CodecReturnCode.ITERATOR_OVERRUN:
                    m_errorCode = OmmError.ErrorCodes.ITERATOR_OVERRUN;
                    break;

                case CodecReturnCode.INCOMPLETE_DATA:
                    m_errorCode = OmmError.ErrorCodes.INCOMPLETE_DATA;
                    break;

                case CodecReturnCode.SET_SKIPPED:
                    m_errorCode = OmmError.ErrorCodes.NO_SET_DEFINITION;
                    break;

                default:
                    m_errorCode = OmmError.ErrorCodes.UNKNOWN_ERROR;
                    break;
            }
            return ret;
        }

        #region Methods for adding Msg type entries to ElementList

        /// <summary>
        /// Adds a Request message as an element of the ElementEntry.
        /// Assumes that the message is either pre-encoded or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added RequestMsg</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddRequestMsg(string name, RequestMsg value)
        {
            m_elementListEncoder.AddRequestMsg(name, value);
            return this;
        }

        /// <summary>
        /// Adds a Refresh message as an element of the ElementEntry.
        /// Assumes that the message is either pre-encoded or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added <see cref="RefreshMsg"/> object</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddRefreshMsg(string name, RefreshMsg value)
        {
            m_elementListEncoder.AddRefreshMsg(name, value);
            return this;
        }

        /// <summary>
        /// Adds a Status message as an element of the ElementEntry.
        /// Assumes that the message is either pre-encoded or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added <see cref="StatusMsg"/> object</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddStatusMsg(string name, StatusMsg value)
        {
            m_elementListEncoder.AddStatusMsg(name, value);
            return this;
        }

        /// <summary>
        /// Adds an Update message as an element of the ElementEntry.
        /// Assumes that the message is either pre-encoded or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added <see cref="UpdateMsg"/> object</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddUpdateMsg(string name, UpdateMsg value)
        {
            m_elementListEncoder.AddUpdateMsg(name, value);
            return this;
        }

        /// <summary>
        /// Adds a Post message as an element of the ElementEntry.
        /// Assumes that the message is either pre-encoded or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added <see cref="PostMsg"/> object</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddPostMsg(string name, PostMsg value)
        {
            m_elementListEncoder.AddPostMsg(name, value);
            return this;
        }

        /// <summary>
        /// Adds an Ack message as an element of the ElementEntry.
        /// Assumes that the message is either pre-encoded or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added <see cref="AckMsg"/> object</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddAckMsg(string name, AckMsg value)
        {
            m_elementListEncoder.AddAckMsg(name, value);
            return this;
        }

        /// <summary>
        /// Adds a Generic message as an element of the ElementEntry.
        /// Assumes that the message is either pre-encoded or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added <see cref="GenericMsg"/> object</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddGenericMsg(string name, GenericMsg value)
        {
            m_elementListEncoder.AddGenericMsg(name, value);
            return this;
        }

        #endregion Methods for adding Msg type entries to ElementList

        #region Methods for adding Container type entries to ElementList

        /// <summary>
        /// Adds a complex type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added <see cref="FieldList"/> object</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddFieldList(string name, FieldList value)
        {
            m_elementListEncoder.AddFieldList(name, value);
            return this;
        }

        /// <summary>
        /// Adds a complex type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added <see cref="ElementList"/> object</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddElementList(string name, ElementList value)
        {
            m_elementListEncoder.AddElementList(name, value);
            return this;
        }

        /// <summary>
        /// Adds a complex type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added <see cref="Map"/> object</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddMap(string name, Map value)
        {
            m_elementListEncoder.AddMap(name, value);
            return this;
        }

        /// <summary>
        /// Adds a complex type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added <see cref="Vector"/> object</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddVector(string name, Vector value)
        {
            m_elementListEncoder.AddVector(name, value);
            return this;
        }

        /// <summary>
        /// Adds a complex type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added <see cref="Series"/> object</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddSeries(string name, Series value)
        {
            m_elementListEncoder.AddSeries(name, value);
            return this;
        }

        /// <summary>
        /// Adds a complex type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added <see cref="FilterList"/> object</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddFilterList(string name, FilterList value)
        {
            m_elementListEncoder.AddFilterList(name, value);
            return this;
        }

        /// <summary>
        /// Adds a complex type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added <see cref="OmmOpaque"/> object</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddOpaque(string name, OmmOpaque value)
        {
            m_elementListEncoder.AddOpaque(name, value);
            return this;
        }

        /// <summary>
        /// Adds a complex type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added <see cref="OmmXml"/> object</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddXml(string name, OmmXml value)
        {
            m_elementListEncoder.AddXml(name, value);
            return this;
        }

        /// <summary>
        /// Adds a complex type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added <see cref="OmmAnsiPage"/> object</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddAnsiPage(string name, OmmAnsiPage value)
        {
            m_elementListEncoder.AddAnsiPage(name, value);
            return this;
        }

        #endregion Methods for adding Container type entries to ElementList

        #region Methods for adding Primitive type entries to current ElementList

        /// <summary>
        /// Adds a simple type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added <see cref="long"/> value</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddInt(string name, long value)
        {
            intObject.Clear();
            intObject.Value(value);
            m_elementListEncoder.AddInt(name, intObject);
            return this;
        }

        /// <summary>
        /// Adds a simple type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added <see cref="uint"/> value</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddUInt(string name, ulong value)
        {
            uintObject.Clear();
            uintObject.Value(value);
            m_elementListEncoder.AddUInt(name, uintObject);
            return this;
        }

        /// <summary>
        /// Adds a OmmReal type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="mantissa">added OmmReal mantissa</param>
        /// <param name="magnitudeType">magnitudeType added</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddReal(string name, long mantissa, int magnitudeType)
        {
            realObject.Clear();
            if (realObject.Value(mantissa, magnitudeType) != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException("Attempt to set invalid Real value.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_elementListEncoder.AddReal(name, realObject);
            return this;
        }

        /// <summary>
        /// Adds a OmmReal type of OMM data to the ElementEntry.
        /// Default magnitudeType is <see cref="OmmReal.MagnitudeTypes.EXPONENT_0"/>
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added double to be converted to OmmReal</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddRealFromDouble(string name, double value)
        {
            realObject.Clear();
            if (realObject.Value(value, OmmReal.MagnitudeTypes.EXPONENT_0) != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException("Attempt to set invalid Real value.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_elementListEncoder.AddReal(name, realObject);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added double to be converted to OmmReal</param>
        /// <param name="magnitudeType">the <see cref="OmmReal.MagnitudeType"/> used for the conversion</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddRealFromDouble(string name, double value, int magnitudeType)
        {
            realObject.Clear();
            if (realObject.Value(value, magnitudeType) != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException("Attempt to set invalid Real value.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_elementListEncoder.AddReal(name, realObject);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">name string object containing ElementEntry name</param>
        /// <param name="value">added float value</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddFloat(string name, float value)
        {
            floatObject.Clear();
            floatObject.Value(value);
            m_elementListEncoder.AddFloat(name, floatObject);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added double</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddDouble(string name, double value)
        {
            doubleObject.Clear();
            doubleObject.Value(value);
            m_elementListEncoder.AddDouble(name, doubleObject);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="year">added OmmDate year (0 - 4095 where 0 indicates blank)</param>
        /// <param name="month">added OmmDate month (0 - 12 where 0 indicates blank)</param>
        /// <param name="day">OmmDate day (0 - 31 where 0 indicates blank)</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">throws if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddDate(string name, int year, int month, int day)
        {
            dateObject.Clear();
            if (dateObject.Year(year) != CodecReturnCode.SUCCESS
                || dateObject.Month(month) != CodecReturnCode.SUCCESS
                || dateObject.Day(day) != CodecReturnCode.SUCCESS
                || !dateObject.IsValid)
            {
                throw new OmmInvalidUsageException("Attempt to specify invalid date.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_elementListEncoder.AddDate(name, dateObject);
            return this;
        }

        /// <summary>
        /// Adds a <see cref="OmmTime"/> simple type of OMM data to the ElementEntry.<br/>
        /// Defaults: hour=0, minute=0, second=0, millisecond=0, microsecond=0, nanosecond=0
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="hour">added OmmTime hour (0 - 23 where 255 indicates blank)</param>
        /// <param name="minute">added OmmTime minute (0 - 59 where 255 indicates blank)</param>
        /// <param name="second">added OmmTime second (0 - 60 where 255 indicates blank)</param>
        /// <param name="millisecond">added OmmTime millisecond (0 - 999 where 65535 indicates blank)</param>
        /// <param name="microsecond">added OmmTime microsecond (0 - 999 where 2047 indicates blank)</param>
        ///<param name="nanosecond">added OmmTime nanosecond (0 - 999 where 2047 indicates blank)</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddTime(string name, int hour = 0, int minute = 0, int second = 0, int millisecond = 0,
            int microsecond = 0, int nanosecond = 0)
        {
            return AddTimeEntry(name, hour, minute, second, millisecond, microsecond, nanosecond);
        }

        /// <summary>
        /// Adds a <see cref="OmmDateTime"/> simple type of OMM data to the ElementEntry.<br/>
        /// Defaults: hour=0, minute=0, second=0, millisecond=0, microsecond=0, nanosecond=0
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="year">added OmmDateTime year (0 - 4095 where 0 indicates blank)</param>
        /// <param name="month">added OmmDateTime month (0 - 12 where 0 indicates blank)</param>
        /// <param name="day">added OmmDateTime day (0 - 31 where 0 indicates blank)</param>
        /// <param name="hour">OmmDateTime hour (0 - 23 where 255 indicates blank)</param>
        /// <param name="minute">added OmmDateTime minute (0 - 59 where 255 indicates blank)</param>
        /// <param name="second">added OmmDateTime second (0 - 60 where 255 indicates blank)</param>
        /// <param name="millisecond">added OmmDateTime millisecond (0 - 999 where 65535 indicates blank)</param>
        /// <param name="microsecond">added OmmDateTime microsecond (0 - 999 where 2047 indicates blank)</param>
        /// <param name="nanosecond">added OmmDateTime nanosecond (0 - 999 where 2047 indicates blank)</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddDateTime(string name, int year, int month, int day = 0, int hour = 0, int minute = 0,
                                        int second = 0, int millisecond = 0, int microsecond = 0, int nanosecond = 0)
        {
            return AddDateTimeEntry(name, year, month, day, hour, minute, second, millisecond, microsecond, nanosecond);
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the ElementEntry<br/>
        /// Default timeliness is <see cref="OmmQos.Timelinesses.REALTIME"/>
        /// Default rate is <see cref="OmmQos.Rates.TICK_BY_TICK"/>
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="timeliness">added <see cref="OmmQos.Timelinesses"/></param>
        /// <param name="rate">added <see cref="OmmQos.Rates"/></param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddQos(string name, uint timeliness = OmmQos.Timelinesses.REALTIME,
            uint rate = OmmQos.Rates.TICK_BY_TICK)
        {
            qosObject.Clear();
            SetEtaRate(rate, qosObject);
            SetEtaTimeliness(timeliness, qosObject);
            m_elementListEncoder.AddQos(name, qosObject);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the ElementEntry.<br/>
        /// Default streamState is <see cref="OmmState.StreamStates.OPEN"/><br/>
        /// Default dataState is <see cref="OmmState.DataStates.OK"/><br/>
        /// Default statusCode is <see cref="OmmState.StatusCodes.NONE"/><br/>
        /// Default statusText is an empty string
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="streamState">added <see cref="OmmState.StreamStates"/> value</param>
        /// <param name="dataState">added <see cref="OmmState.DataStates"/> value</param>
        /// <param name="statusCode">added <see cref="OmmState.StatusCodes"/> value</param>
        /// <param name="statusText">the status text</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddState(string name, int streamState = OmmState.StreamStates.OPEN, int dataState = OmmState.DataStates.OK,
            int statusCode = OmmState.StatusCodes.NONE, string statusText = "")
        {
            return AddStateValue(name, streamState, dataState, statusCode, statusText);
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added Enum value</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddEnum(string name, ushort value)
        {
            enumObject.Clear();
            if (enumObject.Value(value) != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException("Attempt to set invalid Enum value.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_elementListEncoder.AddEnum(name, enumObject);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added EmaBuffer as <see cref="Access.DataType.DataTypes.BUFFER"/></param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddBuffer(string name, EmaBuffer value)
        {
            bufferObject.Clear();
            if (byteBuffer.Capacity > value.Length)
            {
                byteBuffer.Clear();
                byteBuffer.Put(value.Contents).Flip();
            }
            else
            {
                byteBuffer = new ByteBuffer(value.Length);
                byteBuffer.Put(value.Contents).Flip();
            }
            bufferObject.Data(byteBuffer);
            m_elementListEncoder.AddBuffer(name, bufferObject, Access.DataType.DataTypes.BUFFER);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added EmaBuffer as <see cref="Access.DataType.DataTypes.ASCII"/></param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddAscii(string name, string value)
        {
            bufferObject.Clear();
            bufferObject.Data(value);
            m_elementListEncoder.AddBuffer(name, bufferObject, Access.DataType.DataTypes.ASCII);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added EmaBuffer as <see cref="Access.DataType.DataTypes.UTF8"/></param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddUtf8(string name, EmaBuffer value)
        {
            bufferObject.Clear();
            if (byteBuffer.Capacity > value.Length)
            {
                byteBuffer.Clear();
                byteBuffer.Put(value.Contents).Flip();
            }
            else
            {
                byteBuffer = new ByteBuffer(value.Length);
                byteBuffer.Put(value.Contents).Flip();
            }
            bufferObject.Data(byteBuffer);
            m_elementListEncoder.AddBuffer(name, bufferObject, Access.DataType.DataTypes.UTF8);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added string as <see cref="Access.DataType.DataTypes.UTF8"/></param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddUtf8(string name, string value)
        {
            bufferObject.Clear();
            bufferObject.Data(value);
            m_elementListEncoder.AddBuffer(name, bufferObject, Access.DataType.DataTypes.UTF8);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added EmaBuffer as <see cref="Access.DataType.DataTypes.RMTES"/></param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddRmtes(string name, EmaBuffer value)
        {
            bufferObject.Clear();
            if (byteBuffer.Capacity > value.Length)
            {
                byteBuffer.Clear();
                byteBuffer.Put(value.Contents).Flip();
            }
            else
            {
                byteBuffer = new ByteBuffer(value.Length);
                byteBuffer.Put(value.Contents).Flip();
            }
            bufferObject.Data(byteBuffer);
            m_elementListEncoder.AddBuffer(name, bufferObject, Access.DataType.DataTypes.RMTES);
            return this;
        }

        /// <summary>
        /// Adds an OmmArray of OMM data to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <param name="value">added OmmArray as as <see cref="Access.DataType.DataTypes.ARRAY"/></param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddArray(string name, OmmArray value)
        {
            m_elementListEncoder.AddArray(name, value);
            return this;
        }

        #endregion Methods for adding Primitive type entries to current ElementList

        #region Methods for blank empty entries to ElementList

        /// <summary>
        /// Adds a blank data code to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddCodeInt(string name)
        {
            m_elementListEncoder.AddBlankPrimitive(name, Access.DataType.DataTypes.INT);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddCodeUInt(string name)
        {
            m_elementListEncoder.AddBlankPrimitive(name, Access.DataType.DataTypes.UINT);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddCodeReal(string name)
        {
            m_elementListEncoder.AddBlankPrimitive(name, Access.DataType.DataTypes.REAL);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddCodeFloat(string name)
        {
            m_elementListEncoder.AddBlankPrimitive(name, Access.DataType.DataTypes.FLOAT);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddCodeDouble(string name)
        {
            m_elementListEncoder.AddBlankPrimitive(name, Access.DataType.DataTypes.DOUBLE);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddCodeDate(string name)
        {
            m_elementListEncoder.AddBlankPrimitive(name, Access.DataType.DataTypes.DATE);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddCodeTime(string name)
        {
            m_elementListEncoder.AddBlankPrimitive(name, Access.DataType.DataTypes.TIME);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddCodeDateTime(string name)
        {
            m_elementListEncoder.AddBlankPrimitive(name, Access.DataType.DataTypes.DATETIME);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddCodeQos(string name)
        {
            m_elementListEncoder.AddBlankPrimitive(name, Access.DataType.DataTypes.QOS);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddCodeState(string name)
        {
            m_elementListEncoder.AddBlankPrimitive(name, Access.DataType.DataTypes.STATE);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the ElementEntry.
        /// </summary>
        /// <param name="name">string object containing ElementEntry name</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddCodeEnum(string name)
        {
            m_elementListEncoder.AddBlankPrimitive(name, Access.DataType.DataTypes.ENUM);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the ElementEntry.
        /// </summary>
        /// <param name="name">string identifying blank data</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddCodeBuffer(string name)
        {
            m_elementListEncoder.AddBlankPrimitive(name, Access.DataType.DataTypes.BUFFER);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the ElementEntry.
        /// </summary>
        /// <param name="name">string identifying blank data</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddCodeAscii(string name)
        {
            m_elementListEncoder.AddBlankPrimitive(name, Access.DataType.DataTypes.ASCII);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the ElementEntry.
        /// </summary>
        /// <param name="name">string identifying blank data</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddCodeUtf8(string name)
        {
            m_elementListEncoder.AddBlankPrimitive(name, Access.DataType.DataTypes.UTF8);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the ElementEntry.
        /// </summary>
        /// <param name="name">string identifying blank data</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddCodeRmtes(string name)
        {
            m_elementListEncoder.AddBlankPrimitive(name, Access.DataType.DataTypes.RMTES);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the ElementEntry.
        /// </summary>
        /// <param name="name">string identifying blank data</param>
        /// <returns>Reference to the current <see cref="ElementList"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ElementList AddCodeArray(string name)
        {
            m_elementListEncoder.AddBlankPrimitive(name, Access.DataType.DataTypes.ARRAY);
            return this;
        }

        /// <summary>
        /// Adds no payload to the ElementEntry
        /// </summary>
        /// <param name="name">string identifying no payload entry</param>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected (exception will specify the cause of the error)</exception>
        public ElementList AddNoData(string name)
        {
            m_elementListEncoder.AddNoData(name);
            return this;
        }

        #endregion Methods for blank empty entries to ElementList

        /// <summary>
        /// Completes encoding of the ElementList entries
        /// </summary>
        /// <returns>Reference to current <see cref="ElementList"/> object.</returns>
        public ElementList Complete()
        {
            m_elementListEncoder.Complete();
            return this;
        }

        #region Private methods

        private ElementList AddTimeEntry(string name, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond)
        {
            timeObject.Clear();
            if (timeObject.Hour(hour) != CodecReturnCode.SUCCESS
                || timeObject.Minute(minute) != CodecReturnCode.SUCCESS
                || timeObject.Second(second) != CodecReturnCode.SUCCESS
                || timeObject.Millisecond(millisecond) != CodecReturnCode.SUCCESS
                || timeObject.Microsecond(microsecond) != CodecReturnCode.SUCCESS
                || timeObject.Nanosecond(nanosecond) != CodecReturnCode.SUCCESS
                || !timeObject.IsValid)
            {
                throw new OmmInvalidUsageException("Attempt to specify invalid time.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_elementListEncoder.AddTime(name, timeObject);
            return this;
        }

        private ElementList AddDateTimeEntry(string name,
            int year, int month, int day,
            int hour, int minute, int second, int millisecond, int microsecond, int nanosecond)
        {
            dateTimeObject.Clear();
            if (dateTimeObject.Year(year) != CodecReturnCode.SUCCESS
                || dateTimeObject.Month(month) != CodecReturnCode.SUCCESS
                || dateTimeObject.Day(day) != CodecReturnCode.SUCCESS
                || dateTimeObject.Hour(hour) != CodecReturnCode.SUCCESS
                || dateTimeObject.Minute(minute) != CodecReturnCode.SUCCESS
                || dateTimeObject.Second(second) != CodecReturnCode.SUCCESS
                || dateTimeObject.Millisecond(millisecond) != CodecReturnCode.SUCCESS
                || dateTimeObject.Microsecond(microsecond) != CodecReturnCode.SUCCESS
                || dateTimeObject.Nanosecond(nanosecond) != CodecReturnCode.SUCCESS
                || !dateTimeObject.IsValid)
            {
                throw new OmmInvalidUsageException("Attempt to specify invalid DateTime.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_elementListEncoder.AddDateTime(name, dateTimeObject);
            return this;
        }

        private ElementList AddStateValue(string name, int streamState, int dataState, int statusCode, string statusText)
        {
            stateObject.Clear();
            if (stateObject.StreamState(streamState) != CodecReturnCode.SUCCESS
                || stateObject.DataState(dataState) != CodecReturnCode.SUCCESS
                || stateObject.Code(statusCode) != CodecReturnCode.SUCCESS
                || stateObject.Text().Data(statusText) != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException("Attempted to set invalid State value.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_elementListEncoder.AddState(name, stateObject);
            return this;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal override string FillString(int indent)
        {
            m_ToString.Length = 0;
            Utilities.AddIndent(m_ToString, indent).Append("ElementList");
            if (m_rsslElementList.CheckHasInfo()) m_ToString.Append(" ElementListNum=\"").Append(m_rsslElementList.ElementListNum).Append("\"");

            ++indent;

            int loadDataType;
            foreach (ElementEntry elementEntry in this)
            {
                var load = elementEntry.Load;

                if(load == null)
                    return "\nToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.\n";

                loadDataType = elementEntry.LoadType;

                m_ToString.AppendLine();
                Utilities.AddIndent(m_ToString, indent).Append("ElementEntry name=\"")
                        .Append(elementEntry.Name).Append("\" dataType=\"").Append(Access.DataType.AsString(loadDataType));

                if (DataTypes.FIELD_LIST <= loadDataType || DataTypes.ARRAY == loadDataType || Access.DataType.DataTypes.ERROR == loadDataType && load != null)
                {
                    ++indent;
                    m_ToString.Append("\"").AppendLine().Append(load!.ToString(indent));
                    --indent;
                    Utilities.AddIndent(m_ToString, indent).Append("ElementEntryEnd");
                }
                else if (loadDataType == DataTypes.BUFFER)
                {
                    if (load == null || load.Code == DataCode.BLANK)
                        m_ToString.Append("\" value=\"").Append(load!.ToString()).Append("\"");
                    else
                        m_ToString.Append("\"").AppendLine().Append(load.ToString());
                }
                else
                    m_ToString.Append("\" value=\"").Append(load?.ToString() ?? string.Empty).Append("\"");
            }

            --indent;

            m_ToString.AppendLine();
            Utilities.AddIndent(m_ToString, indent).Append("ElementListEnd").AppendLine();

            return m_ToString.ToString();
        }

        /// <summary>
        /// String representation of the current instance.
        /// </summary>
        /// <returns>String representation of the current instance</returns>
        public override string ToString()
        {
            return ToString(0);
        }

        #endregion Private methods
    }

    internal class ElementListErrorEnumerator : Decoder, IEnumerator<ElementEntry>
    {
        private ElementEntry m_elementEntry = new ElementEntry();
        public ElementEntry Current => m_decodingStarted ? m_elementEntry : null!;

        object? IEnumerator.Current => m_decodingStarted ? m_elementEntry : null;

        public ElementListErrorEnumerator()
        {
            m_dataType = DataType.DataTypes.ELEMENT_LIST;
            m_isErrorDecoder = true;
        }

        public ElementListErrorEnumerator(OmmError.ErrorCodes errorCode) : this()
        {
            m_decodingError.ErrorCode = errorCode;
            m_elementEntry!.Load = m_decodingError;
        }

        internal void SetErrorCode(OmmError.ErrorCodes errorCode)
        {
            m_decodingError.ErrorCode = errorCode;
            m_elementEntry!.Load = m_decodingError;
        }

        public void Dispose()
        {
            Reset();
            ReturnElementListErrorEnumeratorToPool();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ReturnElementListErrorEnumeratorToPool()
        {
            if (!m_inPool)
            {
                m_objectManager!.ReturnToEnumeratorPool(this);
            }
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
            m_elementEntry.Clear();
            m_decodingStarted = false;
            m_atEnd = false;
        }
    }
}
