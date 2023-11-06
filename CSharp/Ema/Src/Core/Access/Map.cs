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
using System.Data;
using System.Net.Http.Headers;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Xml.Linq;
using Buffer = LSEG.Eta.Codec.Buffer;
using DateTime = LSEG.Eta.Codec.DateTime;
using Double = LSEG.Eta.Codec.Double;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// Map is a homogeneous container of complex data type entries.
    /// </summary>
    /// <remarks>
    /// Map is a collection which provides iterator over the elements in this collection.<br/>
    /// Map entries are identified by a map key.<br/>
    /// All entries must have key of the same primitive data type.<br/> 
    /// All entries must have same complex data type (except for delete action).<br/>
    /// Objects of this class are intended to be short lived or rather transitional.<br/>
    /// This class is designed to efficiently perform setting and extracting of Map and its content.<br/>
    /// Objects of this class are not cache-able.
    /// </remarks>
    public sealed class Map : ComplexType, IEnumerable<MapEntry>
    {
		internal Eta.Codec.Map m_rsslMap = new Eta.Codec.Map();
		private ComplexTypeData m_summaryData;
		private DataDictionary? m_dataDictionary;

        private LocalFieldSetDefDb? m_localFieldSetDefDb;
        private LocalElementSetDefDb? m_localElementSetDefDb;
		private object? m_localDb;

		private MapEncoder m_mapEncoder;

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

        internal void SetObjectManager(EmaObjectManager objectManager)
        {
            m_objectManager = objectManager;
            m_summaryData.m_objectManager = objectManager;
        }

        /// <summary>
        /// Constructor for Map container
        /// </summary>
        public Map()
		{
			m_summaryData = new ComplexTypeData(m_objectManager!);
            m_mapEncoder = new MapEncoder(this);
			Encoder = m_mapEncoder;
            ClearTypeSpecific_All = ClearMap_All;
            ClearTypeSpecific_Decode = ClearMap_Decode;
            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            DecodeComplexType = DecodeMap;
            m_dataType = Access.DataType.DataTypes.MAP;
        }

        /// <summary>
        /// Indicates presence of KeyFieldId.
        /// </summary>
        public bool HasKeyFieldId { get => m_rsslMap.CheckHasKeyFieldId(); }

        /// <summary>
        /// Indicates presence of TotalCountHint
        /// </summary>
        public bool HasTotalCountHint { get => m_rsslMap.CheckHasTotalCountHint(); }

        /// <summary>
        /// Returns KeyFieldId
        /// </summary>
        /// <returns>the key fieldId value</returns>
        public int KeyFieldId() 
		{ 
			if (!m_rsslMap.CheckHasKeyFieldId())
			{
				throw new OmmInvalidUsageException("KeyFieldId is not present.", OmmInvalidUsageException.ErrorCodes.INVALID_USAGE);
			}
			return m_rsslMap.KeyFieldId; 
		}

        /// <summary>
        /// Getter for TotalCountHint
        /// </summary>
        /// <returns>TotalCountHint value</returns>
        public int TotalCountHint() 
		{
            if (!m_rsslMap.CheckHasTotalCountHint())
            {
                throw new OmmInvalidUsageException("TotalCountHint is not present.", OmmInvalidUsageException.ErrorCodes.INVALID_USAGE);
            }
            return m_rsslMap.TotalCountHint; 
		}

        /// <summary>
        /// Returns the contained summaryData Data based on the summaryData DataType.
        /// SummaryData contains no data if <see cref="ComplexTypeData.DataType"/>  returns
        /// <see cref="DataType.DataTypes.NO_DATA"/>
        /// </summary>
        /// <returns>Summary data that the map holds</returns>
        public ComplexTypeData SummaryData() 
		{
            return m_summaryData; 
		}

		/// <summary>
		/// Returns the primitive key type.
		/// </summary>
		/// <returns>int value representing the DataType of the key.</returns>
		public int KeyType()
		{
			return m_rsslMap.KeyPrimitiveType;
		}

        /// <summary>
        /// Clears the Map. Invoking Clear() method clears all the values and resets all the defaults
        /// </summary>
        /// <returns>Reference to the current <see cref="Map"/> object.</returns>      
        public Map Clear() 
		{
            Clear_All();
            return this;
        }

        internal void ClearMap_All()
        {
            m_rsslMap.Clear();
            m_summaryData.Clear();
            m_dataDictionary = null;
            m_localDb = null;
        }

        internal void ClearMap_Decode()
        {
            m_summaryData.Clear();
            m_dataDictionary = null;
            m_localDb = null;
        }

        /// <summary>
        /// Specifies KeyFieldId.
        /// </summary>
        /// <param name="fieldId">the fieldId to be set</param>
        /// <returns>Reference to the current <see cref="Map"/> object.</returns>      
        public Map KeyFieldId(int fieldId) 
		{
			m_mapEncoder.KeyFieldId(fieldId);
			return this; 
		}

        /// <summary>
        /// Specifies TotalCountHint.
        /// </summary>
        /// <param name="totalCountHint">the totalCountHint to be set</param>
        /// <returns>Reference to the current <see cref="Map"/> object.</returns>      
        public Map TotalCountHint(int totalCountHint)
        {
            m_mapEncoder.TotalCountHint(totalCountHint);
            return this;
        }

        /// <summary>
        /// Specifies the SummaryData
        /// </summary>
        /// <param name="summaryData">specifies complex type as summaryData</param>
        /// <returns>Reference to the current <see cref="Map"/> object.</returns>      
        public Map SummaryData(ComplexType summaryData) 
		{
			m_summaryData.Clear();
            m_summaryData.SetExternalData(summaryData);
			m_mapEncoder.SummaryData(summaryData);
			return this;
		}

        /// <summary>
        /// Specifies a primitive type for Map Entry key. 
        /// This is used to override the default <see cref="DataType.DataTypes.BUFFER"/> type.
        /// </summary>
        /// <param name="keyPrimitiveType">specifies a key primitive type defined in <see cref="DataType.DataTypes"/></param>
        /// <returns>Reference to the current <see cref="Map"/> object.</returns>      
        public Map KeyType(int keyPrimitiveType)
        {
            m_mapEncoder.KeyType(keyPrimitiveType);
            return this;
        }

        /// <summary>
        /// Implementation of <see cref="IEnumerable{MapEntry}"/> interface method
        /// </summary>
        /// <returns><see cref="IEnumerator{MapEntry}"/> instance that iterates through current Map's entries.</returns>
        public IEnumerator<MapEntry> GetEnumerator()
        {
            if (m_errorCode != OmmError.ErrorCodes.NO_ERROR || !m_hasDecodedDataSet)
            {
                var errorEnumerator = m_objectManager!.GetMapErrorEnumerator();

                errorEnumerator.SetError(m_errorCode);
				return errorEnumerator;
            }

            var enumerator = m_objectManager!.GetMapEnumerator();
            if (!enumerator.SetData(m_MajorVersion, 
				m_MinorVersion, 
				m_bodyBuffer!,
				m_dataDictionary, 
				m_rsslMap.ContainerType, 
				m_rsslMap.KeyPrimitiveType,
				m_localDb))
			{
				enumerator.Dispose();
                var errorEnumerator = m_objectManager!.GetMapErrorEnumerator();
                errorEnumerator.SetError(enumerator.m_decodingError.ErrorCode);
                return errorEnumerator;
            }

			return enumerator;
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        internal CodecReturnCode DecodeMap(int majorVersion, int minorVersion, Buffer body, DataDictionary? dictionary, object? localSetDb)
        {
			m_hasDecodedDataSet = true;

            m_MajorVersion = majorVersion;
			m_MinorVersion = minorVersion;
			m_bodyBuffer = body;
			m_dataDictionary = dictionary;
			m_decodeIterator.Clear();
			CodecReturnCode ret;
            if ((ret = m_decodeIterator.SetBufferAndRWFVersion(m_bodyBuffer, m_MajorVersion, m_MinorVersion)) != CodecReturnCode.SUCCESS)
			{
				m_errorCode = OmmError.ErrorCodes.ITERATOR_SET_FAILURE;
				return ret;
			}
			ret = m_rsslMap.Decode(m_decodeIterator);
            switch (ret)
            {
                case CodecReturnCode.NO_DATA:
                    m_errorCode = OmmError.ErrorCodes.NO_ERROR;
                    m_rsslMap.Flags = 0;
                    break;
                case CodecReturnCode.SUCCESS:
                    m_errorCode = OmmError.ErrorCodes.NO_ERROR;
                    break;
                case CodecReturnCode.ITERATOR_OVERRUN:
                    m_errorCode = OmmError.ErrorCodes.ITERATOR_OVERRUN;
                    return ret;
                case CodecReturnCode.INCOMPLETE_DATA:
                    m_errorCode = OmmError.ErrorCodes.INCOMPLETE_DATA;
                    return ret;
                default:
                    m_errorCode = OmmError.ErrorCodes.UNKNOWN_ERROR;
                    return ret;
            }

			if (m_rsslMap.CheckHasSetDefs())
			{
				switch (m_rsslMap.ContainerType)
				{
					case DataTypes.FIELD_LIST:
                        if (m_localFieldSetDefDb == null)
                        {
                            m_localFieldSetDefDb = new LocalFieldSetDefDb();
                        }
						m_localFieldSetDefDb.Clear();
						m_localFieldSetDefDb.Decode(m_decodeIterator);
						m_localDb = m_localFieldSetDefDb;
                        break;
					case DataTypes.ELEMENT_LIST:
                        if (m_localElementSetDefDb == null)
                        {
                            m_localElementSetDefDb = new LocalElementSetDefDb();
                        }
                        m_localElementSetDefDb.Clear();
                        m_localElementSetDefDb.Decode(m_decodeIterator);
                        m_localDb = m_localElementSetDefDb;
                        break;
					default:
						m_localDb = null;
                        m_errorCode = OmmError.ErrorCodes.UNSUPPORTED_DATA_TYPE;
						return CodecReturnCode.FAILURE;
				}
			}
			else
			{
				m_localDb = null;
            }

            if (m_rsslMap.CheckHasSummaryData()) 
			{
				return m_summaryData.Decode(m_rsslMap.EncodedSummaryData, 
					m_rsslMap.ContainerType, 
					m_MajorVersion, m_MinorVersion, 
					m_dataDictionary,
					m_localDb);
            }	
			return ret;			
        }

        /// <summary>
        /// Adds complex OMM data identified by Int of OMM data.
        /// In case a message type is added, the container expects that
        /// the message is either pre-encoded or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="key">key containing long key information</param>
        /// <param name="action">specifies action to be applied to the entry</param>
        /// <param name="value">specifies complex type associated with this entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyInt(long key, int action, ComplexType value, EmaBuffer? permissionData = null)
		{
            intObject.Clear();
            intObject.Value(key);
			m_mapEncoder.Add(Access.DataType.DataTypes.INT, intObject, action, value, permissionData);
            return this;
		}

        /// <summary>
        /// Adds no payload identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="key">key containing long key information</param>
        /// <param name="action">specifies action to be applied to the entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyInt(long key, int action, EmaBuffer? permissionData = null)
        {
            intObject.Clear();
            intObject.Value(key);
            m_mapEncoder.AddEntryWithNoPayload(Access.DataType.DataTypes.INT, intObject, action, permissionData);
            return this;
        }

        /// <summary>
        /// Adds complex OMM data identified by a specific simple type of OMM data.
        /// In case a message type is added, the container expects that 
        /// the message is either pre-encoded or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="key">key containing UInt key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="value">complex type contained in this entry</param>
        /// <param name="permissionData">permission data for this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyUInt(ulong key, int action, ComplexType value, EmaBuffer? permissionData = null)
        {
            uintObject.Clear();
            uintObject.Value(key);
            m_mapEncoder.Add(Access.DataType.DataTypes.UINT, uintObject, action, value, permissionData);
            return this;
        }

        /// <summary>
        /// Adds no payload identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="key">containing UInt key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyUInt(ulong key, int action, EmaBuffer? permissionData = null)
        {
            uintObject.Clear();
            uintObject.Value(key);
            m_mapEncoder.AddEntryWithNoPayload(Access.DataType.DataTypes.UINT, uintObject, action, permissionData);
            return this;
        }

        /// <summary>
        /// Adds complex OMM data identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="mantissa">specifies <see cref="OmmReal.Mantissa"/> part of key information</param>
        /// <param name="magnitudeType">specifies <see cref="OmmReal.MagnitudeType"/> part of key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="value">complex type contained in this entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyReal(long mantissa, int magnitudeType, int action, ComplexType value, EmaBuffer? permissionData = null)
        {
            realObject.Clear();
            if (realObject.Value(mantissa, magnitudeType) != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException("Attempt to set invalid Real value.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_mapEncoder.Add(Access.DataType.DataTypes.REAL, realObject, action, value, permissionData);
            return this;
        }

        /// <summary>
        /// Adds no payload identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="mantissa">specifies <see cref="OmmReal.Mantissa"/> part of key information</param>
        /// <param name="magnitudeType">specifies <see cref="OmmReal.MagnitudeType"/> part of key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyReal(long mantissa, int magnitudeType, int action, EmaBuffer? permissionData = null)
        {
            realObject.Clear();
            if (realObject.Value(mantissa, magnitudeType) != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException("Attempt to set invalid Real value.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_mapEncoder.AddEntryWithNoPayload(Access.DataType.DataTypes.REAL, realObject, action, permissionData);
            return this;
        }

        /// <summary>
        /// Adds no payload identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="key">specifies double to be converted to <see cref="OmmReal"/></param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="magnitudeType">OmmReal magnitudeType used for the conversion(Defaults to
        /// <see cref="OmmReal.MagnitudeTypes.EXPONENT_0"/></param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyRealFromDouble(double key, int action, int magnitudeType = OmmReal.MagnitudeTypes.EXPONENT_0,
            EmaBuffer? permissionData = null)
        {
            realObject.Clear();
            if (realObject.Value(key, magnitudeType) != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException("Attempt to set invalid Real value.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_mapEncoder.AddEntryWithNoPayload(Access.DataType.DataTypes.REAL, realObject, action, permissionData);
            return this;
        }

        /// <summary>
        /// Adds complex OMM data identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="key">specifies double to be converted to <see cref="OmmReal"/></param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="value">specifies complex type contained in this entry</param>
        /// <param name="magnitudeType">OmmReal magnitudeType used for the conversion(Defaults to
        /// <see cref="OmmReal.MagnitudeTypes.EXPONENT_0"/></param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyRealFromDouble(double key, int action, ComplexType value, int magnitudeType = OmmReal.MagnitudeTypes.EXPONENT_0,
            EmaBuffer? permissionData = null)
        {
            realObject.Clear();
            if (realObject.Value(key, magnitudeType) != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException("Attempt to set invalid Real value.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_mapEncoder.Add(Access.DataType.DataTypes.REAL, realObject, action, value, permissionData);
            return this;
        }

        /// <summary>
        /// Adds complex OMM data identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="key">containing float key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="value">specifies complex type contained in this entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyFloat(float key, int action, ComplexType value, EmaBuffer? permissionData = null)
        {
            floatObject.Clear();
            floatObject.Value(key);
            m_mapEncoder.Add(Access.DataType.DataTypes.FLOAT, floatObject, action, value, permissionData);
            return this;
        }

        /// <summary>
        /// Adds no payload identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="key">containing float key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyFloat(float key, int action, EmaBuffer? permissionData = null)
        {
            floatObject.Clear();
            floatObject.Value(key);
            m_mapEncoder.AddEntryWithNoPayload(Access.DataType.DataTypes.FLOAT, floatObject, action, permissionData);
            return this;
        }

        /// <summary>
        ///  Adds complex OMM data identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="key">containing double key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="value">complex type contained in this entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyDouble(double key, int action, ComplexType value, EmaBuffer? permissionData = null)
        {
            doubleObject.Clear();
            doubleObject.Value(key);
            m_mapEncoder.Add(Access.DataType.DataTypes.DOUBLE, doubleObject, action, value, permissionData);
            return this;
        }

        /// <summary>
        /// Adds no payload identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="key">containing double key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyDouble(double key, int action, EmaBuffer? permissionData = null)
        {
            doubleObject.Clear();
            doubleObject.Value(key);
            m_mapEncoder.AddEntryWithNoPayload(Access.DataType.DataTypes.DOUBLE, doubleObject, action, permissionData);
            return this;
        }

        /// <summary>
        /// Adds complex OMM data identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="year">specifies OmmDate year part of key information (0 - 4095)</param>
        /// <param name="month">specifies OmmDate month part of key information (0 - 12)</param>
        /// <param name="day">specifies OmmDate day part of key information (0 - 31)</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="value">complex type contained in this entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyDate(int year, int month, int day, int action, ComplexType value, EmaBuffer? permissionData = null)
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
            m_mapEncoder.Add(Access.DataType.DataTypes.DATE, dateObject, action, value, permissionData);
            return this;
        }

        /// <summary>
        /// Adds no payload identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="year">specifies OmmDate year part of key information (0 - 4095)</param>
        /// <param name="month">specifies OmmDate month part of key information (0 - 12</param>
        /// <param name="day">OmmDate day part of key information (0 - 31)</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyDate(int year, int month, int day, int action, EmaBuffer? permissionData = null)
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
            m_mapEncoder.AddEntryWithNoPayload(Access.DataType.DataTypes.DATE, dateObject, action, permissionData);
            return this;
        }

        /// <summary>
        /// Adds complex OMM data identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="hour">specifies OmmTime hour part of key information (0 - 23)</param>
        /// <param name="minute">specifies OmmTime minute part of key information (0 - 59)</param>
        /// <param name="second">specifies OmmTime second part of key information (0 - 60)</param>
        /// <param name="millisecond">specifies OmmTime millisecond part of key information (0 - 999)</param>
        /// <param name="microsecond">specifies OmmTime microsecond part of key information (0 - 999)</param>
        /// <param name="nanosecond">specifies OmmTime nanosecond part of key information (0 - 999)</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="value">complex type contained in this entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyTime(int hour, int minute, int second, int millisecond, int microsecond, int nanosecond, int action,
            ComplexType value, EmaBuffer? permissionData = null)
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
            m_mapEncoder.Add(Access.DataType.DataTypes.TIME, timeObject, action, value, permissionData);
            return this;
        }

        /// <summary>
        /// Adds no payload identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="hour">specifies OmmTime hour part of key information (0 - 23)</param>
        /// <param name="minute">specifies OmmTime minute part of key information (0 - 59)</param>
        /// <param name="second">specifies OmmTime second part of key information (0 - 60)</param>
        /// <param name="millisecond">specifies OmmTime millisecond part of key information (0 - 999)</param>
        /// <param name="microsecond">specifies OmmTime microsecond part of key information (0 - 999)</param>
        /// <param name="nanosecond">specifies OmmTime nanosecond part of key information (0 - 999)</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyTime(int hour, int minute, int second, int millisecond, int microsecond, int nanosecond, int action,
            EmaBuffer? permissionData = null)
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
            m_mapEncoder.AddEntryWithNoPayload(Access.DataType.DataTypes.TIME, timeObject, action, permissionData);
            return this;
        }

        /// <summary>
        /// Adds complex OMM data identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="year">specifies OmmDateTime year part of key information (0 - 4095)</param>
        /// <param name="month">specifies OmmDateTime month part of key information (0 - 12)</param>
        /// <param name="day">specifies OmmDateTime day part of key information (0 - 31)</param>
        /// <param name="hour">specifies OmmTime hour part of key information (0 - 23)</param>
        /// <param name="minute">specifies OmmTime minute part of key information (0 - 59)</param>
        /// <param name="second">specifies OmmTime second part of key information (0 - 60)</param>
        /// <param name="millisecond">specifies OmmTime millisecond part of key information (0 - 999)</param>
        /// <param name="microsecond">specifies OmmTime microsecond part of key information (0 - 999)</param>
        /// <param name="nanosecond">specifies OmmTime nanosecond part of key information (0 - 999)</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="value">complex type contained in this entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyDateTime(int year, int month, int day, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond,
            int action, ComplexType value, EmaBuffer? permissionData = null)
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
            m_mapEncoder.Add(Access.DataType.DataTypes.DATETIME, dateTimeObject, action, value, permissionData);
            return this;
        }

        /// <summary>
        /// Adds no payload identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="year">specifies OmmDateTime year part of key information (0 - 4095)</param>
        /// <param name="month">specifies OmmDateTime month part of key information (0 - 12)</param>
        /// <param name="day">specifies OmmDateTime day part of key information (0 - 31)</param>
        /// <param name="hour">specifies OmmTime hour part of key information (0 - 23)</param>
        /// <param name="minute">specifies OmmTime minute part of key information (0 - 59)</param>
        /// <param name="second">specifies OmmTime second part of key information (0 - 60)</param>
        /// <param name="millisecond">specifies OmmTime millisecond part of key information (0 - 999)</param>
        /// <param name="microsecond">specifies OmmTime microsecond part of key information (0 - 999)</param>
        /// <param name="nanosecond">specifies OmmTime nanosecond part of key information (0 - 999)</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyDateTime(int year, int month, int day, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond,
            int action, EmaBuffer? permissionData = null)
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
            m_mapEncoder.AddEntryWithNoPayload(Access.DataType.DataTypes.DATETIME, dateTimeObject, action, permissionData);
            return this;
        }

        /// <summary>
        /// Adds complex OMM data identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="timeliness">specifies <see cref="OmmQos.Timeliness"/> part of key information</param>
        /// <param name="rate">specifies <see cref="OmmQos.Rate"/> part of key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="value">complex type contained in this entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyQos(uint timeliness, uint rate, int action, ComplexType value, EmaBuffer? permissionData = null)
        {
            qosObject.Clear();
            SetEtaRate(rate, qosObject);
            SetEtaTimeliness(timeliness, qosObject);
            m_mapEncoder.Add(Access.DataType.DataTypes.QOS, qosObject, action, value, permissionData);
            return this;
        }

        /// <summary>
        /// Adds no payload identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="timeliness">specifies <see cref="OmmQos.Timeliness"/> part of key information</param>
        /// <param name="rate">specifies <see cref="OmmQos.Rate"/> part of key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyQos(uint timeliness, uint rate, int action, EmaBuffer? permissionData = null)
        {
            qosObject.Clear();
            SetEtaRate(rate, qosObject);
            SetEtaTimeliness(timeliness, qosObject);
            m_mapEncoder.AddEntryWithNoPayload(Access.DataType.DataTypes.QOS, qosObject, action, permissionData);
            return this;
        }

        /// <summary>
        /// Adds complex OMM data identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="streamState">specifies <see cref="OmmState.StreamState"/> part of key information</param>
        /// <param name="dataState">specifies <see cref="OmmState.DataState"/> part of key information</param>
        /// <param name="statusCode">specifies <see cref="OmmState.StatusCode"/> part of key information</param>
        /// <param name="statusText">specifies <see cref="OmmState.StatusText"/> part of key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="value">complex type contained in this entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyState(int streamState, int dataState, int statusCode, string statusText, int action, ComplexType value,
            EmaBuffer? permissionData = null)
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
            m_mapEncoder.Add(Access.DataType.DataTypes.STATE, stateObject, action, value, permissionData);
            return this;
        }

        /// <summary>
        /// Adds no payload identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="streamState">specifies <see cref="OmmState.StreamState"/> part of key information</param>
        /// <param name="dataState">specifies <see cref="OmmState.DataState"/> part of key information</param>
        /// <param name="statusCode">specifies <see cref="OmmState.StatusCode"/> part of key information</param>
        /// <param name="statusText">specifies <see cref="OmmState.StatusText"/> part of key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyState(int streamState, int dataState, int statusCode, string statusText, int action, EmaBuffer? permissionData = null)
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
            m_mapEncoder.AddEntryWithNoPayload(Access.DataType.DataTypes.STATE, stateObject, action, permissionData);
            return this;
        }

        /// <summary>
        /// Adds complex OMM data identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="key">int containing Enum key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="value">complex type contained in this entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyEnum(int key, int action, ComplexType value, EmaBuffer? permissionData = null)
        {
            enumObject.Clear();
            if (enumObject.Value(key) != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException("Attempt to set invalid Enum value.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_mapEncoder.Add(Access.DataType.DataTypes.ENUM, enumObject, action, value, permissionData);
            return this;
        }

        /// <summary>
        /// Adds no payload identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="key">int containing Enum key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyEnum(int key, int action, EmaBuffer? permissionData = null)
        {
            enumObject.Clear();
            if (enumObject.Value(key) != CodecReturnCode.SUCCESS)
                throw new OmmInvalidUsageException("Attempt to set invalid Enum value.", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            m_mapEncoder.AddEntryWithNoPayload(Access.DataType.DataTypes.ENUM, enumObject, action, permissionData);
            return this;
        }

        /// <summary>
        /// Adds complex OMM data identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="key">EmaBuffer containing Buffer key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="value">complex type contained in this entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyBuffer(EmaBuffer key, int action, ComplexType value, EmaBuffer? permissionData = null)
        {
            bufferObject.Clear();
            if (bufferObject.Data(new ByteBuffer(key.AsByteArray()).Flip()) != CodecReturnCode.SUCCESS)
                throw new OmmInvalidUsageException("Attempt to set invalid Buffer value.", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            m_mapEncoder.Add(Access.DataType.DataTypes.BUFFER, bufferObject, action, value, permissionData);
            return this;
        }

        /// <summary>
        /// Adds no payload identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="key">EmaBuffer containing Buffer key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyBuffer(EmaBuffer key, int action, EmaBuffer? permissionData = null)
        {
            bufferObject.Clear();
            if (bufferObject.Data(new ByteBuffer(key.AsByteArray()).Flip()) != CodecReturnCode.SUCCESS)
                throw new OmmInvalidUsageException("Attempt to set invalid Buffer value.", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            m_mapEncoder.AddEntryWithNoPayload(Access.DataType.DataTypes.BUFFER, bufferObject, action, permissionData);
            return this;
        }

        /// <summary>
        /// Adds complex OMM data identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="key">string containing Ascii key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="value">complex type contained in this entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyAscii(string key, int action, ComplexType value, EmaBuffer? permissionData = null)
        {
            bufferObject.Clear();
            if (bufferObject.Data(key) != CodecReturnCode.SUCCESS)
                throw new OmmInvalidUsageException("Attempt to set invalid Ascii value.", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            m_mapEncoder.Add(Access.DataType.DataTypes.ASCII, bufferObject, action, value, permissionData);
            return this;
        }

        /// <summary>
        /// Adds no payload identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="key">string containing Ascii key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyAscii(string key, int action, EmaBuffer? permissionData = null)
        {
            bufferObject.Clear();
            if (bufferObject.Data(key) != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException("Attempt to set invalid Ascii value.", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_mapEncoder.AddEntryWithNoPayload(Access.DataType.DataTypes.ASCII, bufferObject, action, permissionData);
            return this;
        }

        /// <summary>
        /// Adds complex OMM data identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="key">EmaBuffer containing Utf8 key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="value">complex type contained in this entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyUtf8(EmaBuffer key, int action, ComplexType value, EmaBuffer? permissionData = null)
        {
            bufferObject.Clear();
            if (bufferObject.Data(new ByteBuffer(key.AsByteArray()).Flip()) != CodecReturnCode.SUCCESS)
                throw new OmmInvalidUsageException("Attempt to set invalid Utf8 value.", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            m_mapEncoder.Add(Access.DataType.DataTypes.UTF8, bufferObject, action, value, permissionData);
            return this;
        }

        /// <summary>
        /// Adds no payload identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="key">EmaBuffer containing Utf8 key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyUtf8(EmaBuffer key, int action, EmaBuffer? permissionData = null)
        {
            bufferObject.Clear();
            if (bufferObject.Data(new ByteBuffer(key.AsByteArray()).Flip()) != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException("Attempt to set invalid Utf8 value.", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_mapEncoder.AddEntryWithNoPayload(Access.DataType.DataTypes.UTF8, bufferObject, action, permissionData);
            return this;
        }

        /// <summary>
        /// Adds complex OMM data identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="key">EmaBuffer containing Rmtes key information</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="value">complex type contained in this entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyRmtes(EmaBuffer key, int action, ComplexType value, EmaBuffer? permissionData = null)
        {
            bufferObject.Clear();
            if (bufferObject.Data(new ByteBuffer(key.AsByteArray()).Flip()) != CodecReturnCode.SUCCESS)
                throw new OmmInvalidUsageException("Attempt to set invalid Rmtes value.", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            m_mapEncoder.Add(Access.DataType.DataTypes.RMTES, bufferObject, action, value, permissionData);
            return this;
        }

        /// <summary>
        /// Adds no payload identified by a specific simple type of OMM data.
        /// </summary>
        /// <param name="key">EmaBuffer containing Rmtes key informatio</param>
        /// <param name="action">specifies action to apply to the entry</param>
        /// <param name="permissionData">EmaBuffer containing permission data related to  this entry</param>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if an error is detected</exception>
        public Map AddKeyRmtes(EmaBuffer key, int action, EmaBuffer? permissionData = null)
        {
            bufferObject.Clear();
            if (bufferObject.Data(new ByteBuffer(key.AsByteArray()).Flip()) != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException("Attempt to set invalid RMTES value.", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            m_mapEncoder.AddEntryWithNoPayload(Access.DataType.DataTypes.RMTES, bufferObject, action, permissionData);
            return this;
        }

        /// <summary>
        /// Completes encoding of the Map entries
        /// </summary>
        /// <returns>Reference to current <see cref="Map"/> object.</returns>
        public Map Complete() 
		{
			m_mapEncoder.Complete();
			return this;
		}

        internal override string ToString(int indent)
        {
            m_ToString.Length = 0;
            Utilities.AddIndent(m_ToString, indent).Append("Map");

            if (HasTotalCountHint)
                m_ToString.Append(" totalCountHint=\"").Append(m_rsslMap.TotalCountHint).Append("\"");

            if (m_rsslMap.CheckHasKeyFieldId())
                m_ToString.Append(" keyFieldId=\"").Append(m_rsslMap.KeyFieldId).Append("\"");

            if (m_rsslMap.CheckHasSummaryData())
            {
                ++indent;
                Utilities.AddIndent(m_ToString.AppendLine(), indent)
					.Append("SummaryData dataType=\"")
					.Append(Access.DataType.AsString(m_summaryData.DataType))
					.Append("\"")
					.AppendLine();

                ++indent;
                m_ToString.Append(m_summaryData.Data.ToString(indent));
                --indent;

                Utilities.AddIndent(m_ToString, indent).Append("SummaryDataEnd");
                --indent;
            }

            ++indent;

            foreach (MapEntry mapEntry in this)
            {
                var load = mapEntry.Load;
                if (load == null)
                    return "\nDecoding of just encoded object in the same application is not supported\n";

                var key = mapEntry.Key.Data;
                Utilities.AddIndent(m_ToString.AppendLine(), indent).Append("MapEntry action=\"")
                      .Append(mapEntry.MapActionAsString()).Append("\" key dataType=\"")
                      .Append(Access.DataType.AsString(key.DataType));

                if (key.m_dataType == DataTypes.BUFFER)
                {
                    m_ToString.Append("\" value=\"").Append(key.ToString());
                    Utilities.AddIndent(m_ToString.Append("\"").AppendLine(), indent);
                }
                else
                    m_ToString.Append("\" value=\"").Append(key.ToString()).Append("\"");

                if (mapEntry.HasPermissionData)
                {
                    m_ToString.Append(" permissionData=\"");
                    Utilities.AsHexString(m_ToString, mapEntry.PermissionData).Append("\"");
                }

                m_ToString.Append(" dataType=\"").Append(Access.DataType.AsString(load.DataType))
                         .Append("\"").AppendLine();

                ++indent;
                m_ToString.Append(load.ToString(indent));
                --indent;

                Utilities.AddIndent(m_ToString, indent).Append("MapEntryEnd");
            }

            --indent;

            Utilities.AddIndent(m_ToString.AppendLine(), indent).Append("MapEnd").AppendLine();

            return m_ToString.ToString();
        }

        /// <summary>
        /// Provides string representation of the current instance
        /// </summary>
        /// <returns>string representing current <see cref="Map"/> object.</returns>
		public override string ToString()
		{
			return ToString(0);
		}
    }

    internal class MapErrorEnumerator : Decoder, IEnumerator<MapEntry>
    {
        private MapEntry m_mapEntry;
        
		public MapEntry Current => m_decodingStarted ? m_mapEntry : null!;

        object? IEnumerator.Current => m_decodingStarted ? m_mapEntry : null;

		public MapErrorEnumerator()
        {
            m_dataType = DataType.DataTypes.MAP;
            m_isErrorDecoder = true;
			m_mapEntry = new MapEntry(m_objectManager!);
        }

        public MapErrorEnumerator(OmmError.ErrorCodes errorCode) : this()
        {
            base.m_decodingError.ErrorCode = errorCode;
            m_mapEntry!.Load = m_decodingError;
        }

        internal void SetErrorCode(OmmError.ErrorCodes errorCode)
        {
            m_decodingError.ErrorCode = errorCode;
            m_mapEntry!.Load = m_decodingError;
        }

        public void Dispose()
        {
			Reset();
            ReturnMapErrorEnumeratorToPool();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ReturnMapErrorEnumeratorToPool()
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
            m_decodingStarted = false;
            m_atEnd = false;
			m_mapEntry.Clear();
        }
    }
}
