/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using System;
using System.Collections;
using System.Collections.Generic;
using Buffer = LSEG.Eta.Codec.Buffer;
using DateTime = LSEG.Eta.Codec.DateTime;
using Double = LSEG.Eta.Codec.Double;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// FieldList is an interface for a heterogeneous container of complex and primitive data type entries. 
    /// Objects of this class are intended to be short lived or rather transitional. 
    /// This class is designed to efficiently perform setting and extracting of FieldList and its content. 
    /// Objects of this class are not cache-able.
    /// </summary>
    public sealed class FieldList : ComplexType, IEnumerable<FieldEntry>
    {
        internal Eta.Codec.FieldList m_rsslFieldList = new Eta.Codec.FieldList();
        internal Ema.Rdm.DataDictionary? m_dataDictionary;
        private LocalFieldSetDefDb? m_localFieldSetDefDb;
        private OmmError.ErrorCodes m_errorCode;

        internal FieldListEncoder m_fieldListEncoder;

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

        /// <summary>
        /// Constructor for FieldList
        /// </summary>
        public FieldList()
        {
            m_fieldListEncoder = new FieldListEncoder(this);
            Encoder = m_fieldListEncoder;
        }

        internal FieldList(EmaObjectManager objManager) : this()
        {
            m_objectManager = objManager;
            m_dataDictionary = new Rdm.DataDictionary(false);
        }

        /// <summary>
        /// Indicates presence of Info.
        /// </summary>
        public bool HasInfo { get => m_rsslFieldList.CheckHasInfo(); }

        /// <summary>
        ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
        /// </summary>
        public override int DataType => Access.DataType.DataTypes.FIELD_LIST;

        /// <summary>
        /// Returns InfoFieldListNum.
        /// </summary>
        /// <returns>FieldList Number</returns>
        public int InfoFieldListNum() { return m_rsslFieldList.FieldListNum; }
        
        /// <summary>
        /// Returns InfoDictionaryId.
        /// </summary>
        /// <returns>DictionaryId associated with this FieldList</returns>
        public int InfoDictionaryId() { return m_rsslFieldList.DictionaryId; }
        
        /// <summary>
        /// Clears the FieldList. Invoking Clear() method clears all the values and resets all the defaults.
        /// </summary>
        public FieldList Clear() 
        {
            ClearInt();
 
            return this;
        }

        internal override void ClearInt()
        {
            base.ClearInt();
            m_rsslFieldList.Clear();
            m_decodeIterator.Clear();
            m_fieldListEncoder.Clear();
            m_dataDictionary = null;
            m_localFieldSetDefDb = null;
        }

        /// <summary>
        /// Implementation of <see cref="IEnumerable{FieldEntry}"/> interface method
        /// </summary>
        /// <returns><see cref="IEnumerator{FieldEntry}"/> instance that iterates through current FieldList's entries.</returns>
        public IEnumerator<FieldEntry> GetEnumerator()
        {
            if (m_errorCode != OmmError.ErrorCodes.NO_ERROR || !m_hasDecodedDataSet)
            {
                var errorEnumerator = m_objectManager != null ? m_objectManager!.GetFieldListErrorEnumerator() : new FieldListErrorEnumerator();
                errorEnumerator.SetError(m_errorCode);
                return errorEnumerator;
            }

            var decoder = m_objectManager != null ? m_objectManager.GetFieldListEnumerator() : new FieldListEnumerator();
            if (!decoder.SetData(m_MajorVersion, m_MinorVersion, m_bodyBuffer!, m_dataDictionary?.rsslDataDictionary(), m_localFieldSetDefDb))
            {
                decoder.Dispose();
                var errorEnumerator = m_objectManager!.GetFieldListErrorEnumerator();
                errorEnumerator.SetError(decoder.m_decodingError.ErrorCode);
                return errorEnumerator;
            }
            return decoder;
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        internal override CodecReturnCode Decode(DecodeIterator dIter)
        {
            return CodecReturnCode.FAILURE;
        }

        internal override CodecReturnCode Decode(int majorVersion, int minorVersion, Buffer body, DataDictionary? dictionary, object? localDb)
        {
            m_hasDecodedDataSet = true;

            m_MajorVersion = majorVersion;
            m_MinorVersion = minorVersion;
            m_bodyBuffer = body;
            m_localFieldSetDefDb = (LocalFieldSetDefDb?)localDb;
            m_dataDictionary ??= new Rdm.DataDictionary(false);
            m_dataDictionary.rsslDataDictionary(dictionary!);

            if (m_dataDictionary == null)
            {
                m_errorCode = OmmError.ErrorCodes.NO_DICTIONARY;
                return CodecReturnCode.FAILURE;
            }

            m_decodeIterator.Clear();
            CodecReturnCode ret;
            if ((ret = m_decodeIterator.SetBufferAndRWFVersion(m_bodyBuffer, m_MajorVersion, m_MinorVersion)) < CodecReturnCode.SUCCESS)
            {
                m_errorCode = OmmError.ErrorCodes.ITERATOR_SET_FAILURE;
                return ret;
            }
            ret = m_rsslFieldList.Decode(m_decodeIterator, m_localFieldSetDefDb);
            switch (ret)
            {
                case CodecReturnCode.NO_DATA:
                    m_errorCode = OmmError.ErrorCodes.NO_ERROR;
                    m_rsslFieldList.Flags = 0;
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

        /// <summary>
        /// Specifies Info. 
        /// The FieldList Info is optional. If used, it must be set prior to adding anything to FieldList.
        /// </summary>
        /// <param name="dictionaryId">dictionary id of the RdmFieldDictioanry associated with this FieldList</param>
        /// <param name="fieldListNum">FieldList template number</param>
        /// <returns>reference to the current <see cref="FieldList"/> value</returns>
        public FieldList Info(int dictionaryId, int fieldListNum) 
        { 
            m_fieldListEncoder.Info(dictionaryId, fieldListNum);
            return this; 
        }

        #region Methods for adding Msg type entries

        /// <summary>
        /// Adds an Request message as an element of the FieldList. 
        /// Assumes that the message is either pre-encoded or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added RequestMsg</param>
        /// <returns>reference to current <see cref="FieldList"/> instance</returns>
        public FieldList AddRequsetMsg(int fieldId, RequestMsg value)
        {
            m_fieldListEncoder.AddRequestMsg(fieldId, value);
            return this;
        }

        /// <summary>
        /// Adds an Refresh message as an element of the FieldList. 
        /// Assumes that the message is either pre-encoded or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added RefreshMsg</param>
        /// <returns>reference to current <see cref="FieldList"/> instance</returns>
        public FieldList AddRefreshMsg(int fieldId, RefreshMsg value)
        {
            m_fieldListEncoder.AddRefreshMsg(fieldId, value);
            return this;
        }

        /// <summary>
        /// Adds an Status message as an element of the FieldList. 
        /// Assumes that the message is either pre-encoded or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added StatusMsg</param>
        /// <returns>reference to current <see cref="FieldList"/> instance</returns>
        public FieldList AddStatusMsg(int fieldId, StatusMsg value)
        {
            m_fieldListEncoder.AddStatusMsg(fieldId, value);
            return this;
        }

        /// <summary>
        /// Adds an Update message as an element of the FieldList. 
        /// Assumes that the message is either pre-encoded or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added UpdateMsg</param>
        /// <returns>reference to current <see cref="FieldList"/> instance</returns>
        public FieldList AddUpdateMsg(int fieldId, UpdateMsg value)
        {
            m_fieldListEncoder.AddUpdateMsg(fieldId, value);
            return this;
        }

        /// <summary>
        /// Adds an Post message as an element of the FieldList. 
        /// Assumes that the message is either pre-encoded or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added PostMsg</param>
        /// <returns>reference to current <see cref="FieldList"/> instance</returns>
        public FieldList AddPostMsg(int fieldId, PostMsg value)
        {
            m_fieldListEncoder.AddPostMsg(fieldId, value);
            return this;
        }

        /// <summary>
        /// Adds an Ack message as an element of the FieldList. 
        /// Assumes that the message is either pre-encoded or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added AckMsg</param>
        /// <returns>reference to current <see cref="FieldList"/> instance</returns>
        public FieldList AddAckMsg(int fieldId, AckMsg value)
        {
            m_fieldListEncoder.AddAckMsg(fieldId, value);
            return this;
        }

        /// <summary>
        /// Adds an Generic message as an element of the FieldList. 
        /// Assumes that the message is either pre-encoded or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added GenericMsg</param>
        /// <returns>reference to current <see cref="FieldList"/> instance</returns>
        public FieldList AddGenericMsg(int fieldId, GenericMsg value)
        {
            m_fieldListEncoder.AddGenericMsg(fieldId, value);
            return this;
        }

        #endregion

        #region Methods for adding Container type entries

        /// <summary>
        /// Adds a <see cref="FieldList"/> of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added FieldList</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddFieldList(int fieldId, FieldList value)
        {
            m_fieldListEncoder.AddFieldList(fieldId, value);
            return this;
        }

        /// <summary>
        /// Adds a <see cref="ElementList"/> of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added ElementList</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddElementList(int fieldId, ElementList value)
        {
            m_fieldListEncoder.AddElementList(fieldId, value);
            return this;
        }

        /// <summary>
        /// Adds a <see cref="Map"/> of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added Map</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddMap(int fieldId, Map value)
        {
            m_fieldListEncoder.AddMap(fieldId, value);
            return this;
        }

        /// <summary>
        /// Adds a <see cref="Vector"/> of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added Vector</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddVector(int fieldId, Vector value)
        {
            m_fieldListEncoder.AddVector(fieldId, value);
            return this;
        }

        /// <summary>
        /// Adds a <see cref="Series"/> of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added Series</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddSeries(int fieldId, Series value)
        {
            m_fieldListEncoder.AddSeries(fieldId, value);
            return this;
        }

        /// <summary>
        /// Adds a <see cref="FilterList"/> of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added FilterList</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddFilterList(int fieldId, FilterList value)
        {
            m_fieldListEncoder.AddFilterList(fieldId, value);
            return this;
        }

        /// <summary>
        /// Adds a <see cref="OmmOpaque"/> of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added OmmOpaque</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddOpaque(int fieldId, OmmOpaque value)
        {
            m_fieldListEncoder.AddOpaque(fieldId, value);
            return this;
        }

        /// <summary>
        /// Adds a <see cref="OmmXml"/> of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added OmmXml</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddXml(int fieldId, OmmXml value)
        {
            m_fieldListEncoder.AddXml(fieldId, value);
            return this;
        }

        /// <summary>
        /// Adds a <see cref="OmmAnsiPage"/> of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added OmmAnsiPage</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddAnsiPage(int fieldId, OmmAnsiPage value)
        {
            m_fieldListEncoder.AddAnsiPage(fieldId, value);
            return this;
        }

        #endregion

        #region Methods for adding Primitive type entries

        /// <summary>
        /// Adds a <see cref="long"/> of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added long</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddInt(int fieldId, long value)
        {
            intObject.Clear();
            intObject.Value(value);
            m_fieldListEncoder.AddInt(fieldId, intObject);
            return this;
        }

        /// <summary>
        /// Adds a <see cref="ulong"/> of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added ulong</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddUInt(int fieldId, ulong value)
        {
            uintObject.Clear();
            uintObject.Value(value);
            m_fieldListEncoder.AddUInt(fieldId, uintObject);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="mantissa">OmmReal mantissa</param>
        /// <param name="magnitudeType">added <see cref="OmmReal.MagnitudeType"/></param>
        /// <returns></returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddReal(int fieldId, long mantissa, int magnitudeType)
        {
            realObject.Clear();
            if (realObject.Value(mantissa, magnitudeType) != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException("Attempt to set invalid Real value.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_fieldListEncoder.AddReal(fieldId, realObject);
            return this;
        }

        /// <summary>
        /// Adds a <see cref="double"/> of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added double to be converted to <see cref="OmmReal"/></param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddRealFromDouble(int fieldId, double value)
        {
            realObject.Clear();
            if (realObject.Value(value, OmmReal.MagnitudeTypes.EXPONENT_0) != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException("Attempt to set invalid Real value.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_fieldListEncoder.AddReal(fieldId, realObject);
            return this;
        }

        /// <summary>
        /// Adds a <see cref="double"/> of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added double to be converted to <see cref="OmmReal"/></param>
        /// <param name="magnitudeType">added <see cref="OmmReal.MagnitudeType"/></param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddRealFromDouble(int fieldId, double value, int magnitudeType)
        {
            realObject.Clear();
            if (realObject.Value(value, magnitudeType) != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException("Attempt to set invalid Real value.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_fieldListEncoder.AddReal(fieldId, realObject);
            return this;
        }

        /// <summary>
        /// Adds a <see cref="float"/> of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added float</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddFloatValue(int fieldId, float value)
        {
            floatObject.Clear();
            floatObject.Value(value);
            m_fieldListEncoder.AddFloat(fieldId, floatObject);
            return this;
        }

        /// <summary>
        /// Adds a <see cref="double"/> of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added double</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddDoubleValue(int fieldId, double value)
        {
            doubleObject.Clear();
            doubleObject.Value(value);
            m_fieldListEncoder.AddDouble(fieldId, doubleObject);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="year">added OmmDate year (0 - 4095 where 0 indicates blank)</param>
        /// <param name="month">added OmmDate month (0 - 12 where 0 indicates blank)</param>
        /// <param name="day">added OmmDate day (0 - 31 where 0 indicates blank)</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddDate(int fieldId, int year, int month, int day)
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
            m_fieldListEncoder.AddDate(fieldId, dateObject);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the FieldList.
        /// Defaults: hour=0, minute=0, second=0, millisecond=0, microsecond=0, nanosecond=0
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="hour">added OmmTime hour (0 - 23 where 255 indicates blank)</param>
        /// <param name="minute">added OmmTime minute (0 - 59 where 255 indicates blank)</param>
        /// <param name="second">added OmmTime second (0 - 60 where 255 indicates blank)</param>
        /// <param name="millisecond">added OmmTime millisecond (0 - 999 where 65535 indicates blank)</param>
        /// <param name="microsecond">added OmmTime microsecond (0 - 999 where 2047 indicates blank)</param>
        /// <param name="nanosecond"> added OmmTime nanosecond (0 - 999 where 2047 indicates blank)</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddTime(int fieldId, int hour = 0, int minute = 0, int second = 0, int millisecond = 0, int microsecond = 0, 
            int nanosecond = 0)
        {
            return AddTimeEntry(fieldId, hour, minute, second, millisecond, microsecond, nanosecond);
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the FieldList.<br/>
        /// Defaults: hour=0, minute=0, second=0, millisecond=0, microsecond=0, nanosecond=0
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="year">added OmmDateTime year (0 - 4095 where 0 indicates  blank)</param>
        /// <param name="month">added OmmDateTime month (0 - 12 where 0 indicates blank)</param>
        /// <param name="day">added OmmDateTime day (0 - 31 where 0 indicates blank)</param>
        /// <param name="hour">added OmmDateTime hour (0 - 23 where 255 indicates blank)</param>
        /// <param name="minute">added OmmDateTime minute (0 - 59 where 255 indicates blank)</param>
        /// <param name="second">added OmmDateTime second (0 - 60 where 255 indicates blank)</param>
        /// <param name="millisecond">added OmmDateTime millisecond (0 - 999 where 65535 indicates blank)</param>
        /// <param name="microsecond">added OmmDateTime microsecond (0 - 999 where 2047 indicates blank)</param>
        /// <param name="nanosecond">added OmmDateTime nanosecond (0 - 999 where 2047 indicates blank)</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddDateTime(int fieldId, int year, int month, int day, int hour = 0, int minute = 0, int second = 0,
            int millisecond = 0, int microsecond = 0, int nanosecond = 0)
        {
            return AddDateTimeEntry(fieldId, year, month, day, hour, minute, second, millisecond, microsecond, nanosecond);
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the FieldList.<br/>
        /// Default timeliness is <see cref="OmmQos.Timelinesses.REALTIME"/>
        /// Default rate is <see cref="OmmQos.Rates.TICK_BY_TICK"/>
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="timeliness">added <see cref="OmmQos.Timelinesses"/></param>
        /// <param name="rate">added <see cref="OmmQos.Rates"/></param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddQos(int fieldId, uint timeliness = OmmQos.Timelinesses.REALTIME, 
            uint rate = OmmQos.Rates.TICK_BY_TICK)
        {
            qosObject.Clear();
            SetEtaRate(rate, qosObject);
            SetEtaTimeliness(timeliness, qosObject);
            m_fieldListEncoder.AddQos(fieldId, qosObject);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the FieldList.
        /// Default streamState is <see cref="OmmState.StreamStates.OPEN"/><br/>
        /// Default dataState is <see cref="OmmState.DataStates.OK"/><br/>
        /// Default statusCode is <see cref="OmmState.StatusCodes.NONE"/><br/>
        /// Default statusText is an empty string
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="streamState">added <see cref="OmmState.StreamStates"/></param>
        /// <param name="dataState">added <see cref="OmmState.DataStates"/></param>
        /// <param name="statusCode">added <see cref="OmmState.StatusCodes"/></param>
        /// <param name="statusText">added status text</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddState(int fieldId, int streamState = OmmState.StreamStates.OPEN, int dataState = OmmState.DataStates.OK,
            int statusCode = OmmState.StatusCodes.NONE, string statusText = "")
        {
            return AddStateValue(fieldId, streamState, dataState, statusCode, statusText);
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added Enum</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddEnumValue(int fieldId, int value)
        {
            enumObject.Clear();
            if (enumObject.Value(value) != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException("Attempt to set invalid Enum value.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_fieldListEncoder.AddEnum(fieldId, enumObject);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added EmaBuffer as <see cref="DataTypes.BUFFER"/></param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddBuffer(int fieldId, EmaBuffer value)
        {
            bufferObject.Clear();
            bufferObject.Data(new ByteBuffer(value.AsByteArray()).Flip());
            m_fieldListEncoder.AddBuffer(fieldId, bufferObject, Access.DataType.DataTypes.BUFFER);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added string as <see cref="DataTypes.ASCII_STRING"/></param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddAscii(int fieldId, string value)
        {
            bufferObject.Clear();
            bufferObject.Data(value);
            m_fieldListEncoder.AddBuffer(fieldId, bufferObject, Access.DataType.DataTypes.ASCII);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added EmaBuffer as <see cref="DataTypes.UTF8_STRING"/></param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddUtf8(int fieldId, EmaBuffer value)
        {
            bufferObject.Clear();
            bufferObject.Data(new ByteBuffer(value.AsByteArray()).Flip());
            m_fieldListEncoder.AddBuffer(fieldId, bufferObject, Access.DataType.DataTypes.UTF8);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added string as <see cref="DataTypes.UTF8_STRING"/> (string has to be Utf8 charset)</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddUtf8(int fieldId, string value)
        {
            bufferObject.Clear();
            bufferObject.Data(value);
            m_fieldListEncoder.AddBuffer(fieldId, bufferObject, Access.DataType.DataTypes.UTF8);
            return this;
        }

        /// <summary>
        /// Adds a specific simple type of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added EmaBuffer as <see cref="DataTypes.RMTES_STRING"/></param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddRmtes(int fieldId, EmaBuffer value)
        {
            bufferObject.Clear();
            bufferObject.Data(new ByteBuffer(value.AsByteArray()).Flip());
            m_fieldListEncoder.AddBuffer(fieldId, bufferObject, Access.DataType.DataTypes.RMTES);
            return this;
        }

        /// <summary>
        /// Adds an OmmArray of OMM data to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <param name="value">added OmmArray</param>
        /// <returns>reference to this object</returns>
        /// <exception cref="OmmInvalidUsageException">if an error is detected (exception will 
        /// specify the cause of the error)
        /// </exception>
        public FieldList AddArray(int fieldId, OmmArray value) 
        {
            m_fieldListEncoder.AddArray(fieldId, value);
            return this;
        }

        #endregion

        /// <summary>
        /// Adds a blank data code to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <returns>reference to this object</returns>
        public FieldList AddCodeInt(int fieldId)
        {
            m_fieldListEncoder.AddBlankPrimitive(fieldId, Access.DataType.DataTypes.INT);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <returns>reference to this object</returns>
        public FieldList AddCodeUInt(int fieldId)
        {
            m_fieldListEncoder.AddBlankPrimitive(fieldId, Access.DataType.DataTypes.UINT);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <returns>reference to this object</returns>
        public FieldList AddCodeReal(int fieldId)
        {
            m_fieldListEncoder.AddBlankPrimitive(fieldId, Access.DataType.DataTypes.REAL);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <returns>reference to this object</returns>
        public FieldList AddCodeFloat(int fieldId)
        {
            m_fieldListEncoder.AddBlankPrimitive(fieldId, Access.DataType.DataTypes.FLOAT);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <returns>reference to this object</returns>
        public FieldList AddCodeDouble(int fieldId)
        {
            m_fieldListEncoder.AddBlankPrimitive(fieldId, Access.DataType.DataTypes.DOUBLE);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <returns>reference to this object</returns>
        public FieldList AddCodeDate(int fieldId)
        {
            m_fieldListEncoder.AddBlankPrimitive(fieldId, Access.DataType.DataTypes.DATE);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <returns>reference to this object</returns>
        public FieldList AddCodeTime(int fieldId)
        {
            m_fieldListEncoder.AddBlankPrimitive(fieldId, Access.DataType.DataTypes.TIME);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <returns>reference to this object</returns>
        public FieldList AddCodeDateTime(int fieldId)
        {
            m_fieldListEncoder.AddBlankPrimitive(fieldId, Access.DataType.DataTypes.DATETIME);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <returns>reference to this object</returns>
        public FieldList AddCodeQos(int fieldId)
        {
            m_fieldListEncoder.AddBlankPrimitive(fieldId, Access.DataType.DataTypes.QOS);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <returns>reference to this object</returns>
        public FieldList AddCodeState(int fieldId)
        {
            m_fieldListEncoder.AddBlankPrimitive(fieldId, Access.DataType.DataTypes.STATE);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <returns>reference to this object</returns>
        public FieldList AddCodeEnum(int fieldId)
        {
            m_fieldListEncoder.AddBlankPrimitive(fieldId, Access.DataType.DataTypes.ENUM);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <returns>reference to this object</returns>
        public FieldList AddCodeBuffer(int fieldId)
        {
            m_fieldListEncoder.AddBlankPrimitive(fieldId, Access.DataType.DataTypes.BUFFER);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <returns>reference to this object</returns>
        public FieldList AddCodeAscii(int fieldId)
        {
            m_fieldListEncoder.AddBlankPrimitive(fieldId, Access.DataType.DataTypes.ASCII);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <returns>reference to this object</returns>
        public FieldList AddCodeUtf8(int fieldId)
        {
            m_fieldListEncoder.AddBlankPrimitive(fieldId, Access.DataType.DataTypes.UTF8);
            return this;
        }

        /// <summary>
        /// Adds a blank data code to the FieldList.
        /// </summary>
        /// <param name="fieldId">field id value</param>
        /// <returns>reference to this object</returns>
        public FieldList AddCodeRmtes(int fieldId)
        {
            m_fieldListEncoder.AddBlankPrimitive(fieldId, Access.DataType.DataTypes.RMTES);
            return this;
        }

        /// <summary>
        /// Completes encoding of the FieldList entries
        /// </summary>
        /// <returns>reference to current <see cref="FieldList"/> object</returns>
        public FieldList Complete()
        {
            m_fieldListEncoder.Complete();
            return this;
        }

        #region Private methods
        private FieldList AddTimeEntry(int fieldId, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond)
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
            m_fieldListEncoder.AddTime(fieldId, timeObject);
            return this;
        }

        private FieldList AddDateTimeEntry(int fieldId,
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
            m_fieldListEncoder.AddDateTime(fieldId, dateTimeObject);
            return this;
        }

        private FieldList AddStateValue(int fieldId, int streamState, int dataState, int statusCode, string statusText)
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
            m_fieldListEncoder.AddState(fieldId, stateObject);
            return this;
        }

        internal override string ToString(int indent)
        {
            m_ToString.Length = 0;
            Utilities.AddIndent(m_ToString, indent).Append("FieldList");

            if (m_rsslFieldList.CheckHasInfo())
                m_ToString.Append(" FieldListNum=\"").Append(m_rsslFieldList.FieldListNum).Append("\" DictionaryId=\"")
                         .Append(m_rsslFieldList.DictionaryId).Append("\"");

            ++indent;

            int loadDataType;
            foreach (FieldEntry fieldEntry in this)
            {
                var load = fieldEntry.Load;
                if (load == null)
                    return "\nDecoding of just encoded object in the same application is not supported\n";

                loadDataType = load.DataType;
                Utilities.AddIndent(m_ToString.AppendLine(), indent).Append("FieldEntry fid=\"")
                                                                      .Append(fieldEntry.FieldId)
                                                                      .Append("\" name=\"")
                                                                      .Append(fieldEntry.Name)
                                                                      .Append("\" dataType=\"")
                                                                      .Append(Access.DataType.AsString(loadDataType));

                if (DataTypes.FIELD_LIST <= loadDataType || DataTypes.ARRAY == loadDataType || Access.DataType.DataTypes.ERROR == loadDataType)
                {
                    ++indent;
                    m_ToString.Append("\"").AppendLine().Append(load.ToString(indent));
                    --indent;
                    Utilities.AddIndent(m_ToString, indent).Append("FieldEntryEnd");
                }
                else if (loadDataType == DataTypes.BUFFER)
                {
                    if (load.Code == DataCode.BLANK)
                        m_ToString.Append("\" value=\"").Append(load.ToString()).Append("\"");
                    else
                        m_ToString.Append("\"").AppendLine().Append(load.ToString());
                }
                else
                    m_ToString.Append("\" value=\"").Append(load.ToString()).Append("\"");
            }

            --indent;

            Utilities.AddIndent(m_ToString.Append("\n"), indent).Append("FieldListEnd").AppendLine();

            return m_ToString.ToString();
        }

        /// <summary>
        /// Provides string representation of the current instance
        /// </summary>
        /// <returns>string representing current <see cref="FieldList"/> instance.</returns>
        public override string ToString()
        {
            return ToString(0);
        }

        #endregion
    }

    internal class FieldListErrorEnumerator : Decoder, IEnumerator<FieldEntry>
    {
        private FieldEntry m_fieldEntry = new FieldEntry();
        
        public FieldEntry Current => m_decodingStarted ? m_fieldEntry : null!;

        object? IEnumerator.Current => m_decodingStarted ? m_fieldEntry : null;

        public FieldListErrorEnumerator()
        {
            m_dataType = DataType.DataTypes.FIELD_LIST;
            m_isErrorDecoder = true;
        }

        public FieldListErrorEnumerator(OmmError.ErrorCodes errorCode) : this()
        {
            m_decodingError.ErrorCode = errorCode;
            m_fieldEntry!.Load = m_decodingError;
        }

        internal void SetErrorCode(OmmError.ErrorCodes errorCode)
        {
            m_decodingError.ErrorCode = errorCode;
            m_fieldEntry!.Load = m_decodingError;
        }

        public void Dispose()
        {
            Reset();
            ReturnToPool();
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
            m_fieldEntry.Clear();
        }
    }
}
