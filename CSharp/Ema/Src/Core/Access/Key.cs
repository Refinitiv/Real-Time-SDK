/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// Key conveys MapEntry key information.
    /// </summary>
    /// <remarks>
    /// Key contains objects of primitive type (e.g. they are not complex type).<br/>
    /// Objects of this type are intended to be short-lived and are not to be cached.
    /// </remarks>
    sealed public class Key
    {
        private NoData m_defaultData = new NoData();
        private OmmError m_decodingError = new OmmError();
        internal Data m_data;
        
        private DecodeIterator m_decodeIterator = new DecodeIterator();
        private EmaObjectManager m_objectManager = EmaGlobalObjectPool.Instance;

        internal Key(EmaObjectManager objectManager) 
        {
            m_objectManager = objectManager;
            m_data = m_defaultData;
        }

        /// <summary>
        /// Clears current Key instance
        /// </summary>
        public void Clear()
        {
            m_data.ClearAndReturnToPool_All();
            m_data = m_defaultData;
        }

        /// <summary>
        /// Returns the DataType of the contained data.<br/>
        /// Return of <see cref="DataType.DataTypes.ERROR"/> signifies error while extracting content of Key.
        /// </summary>
        public int DataType { get => m_data.m_dataType; }
        
        /// <summary>
        /// Returns the simple type based on the DataType.
        /// </summary>
        public Data Data { get => m_data; }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.<br/>
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.INT"/>
        /// </summary>
        /// <returns>long value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.INT"/></exception>
        public long Int()
        {
            if (Access.DataType.DataTypes.INT != m_data.m_dataType)
            {
                string error = $"Attempt to call Int() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to int() while entry data is blank.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return ((OmmInt)m_data).Value;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. <br/>
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.INT"/>
        /// </summary>
        /// <returns><see cref="OmmInt"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.INT"/></exception>
        public OmmInt OmmInt()
        {
            if (Access.DataType.DataTypes.INT != m_data.m_dataType)
            {
                string error = $"Attempt to call OmmInt() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to call ommInt() while entry data is blank.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmInt)m_data;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. <br/>
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.UINT"/>
        /// </summary>
        /// <returns>uint value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.UINT"/></exception>
        public ulong UInt()
        {
            if (Access.DataType.DataTypes.UINT != m_data.m_dataType)
            {
                string error = $"Attempt to call UInt() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to uint() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return ((OmmUInt)m_data).Value;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. 
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.UINT"/>
        /// </summary>
        /// <returns><see cref="OmmUInt"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.UINT"/></exception>
        public OmmUInt OmmUInt()
        {
            if (m_data.m_dataType != Access.DataType.DataTypes.UINT)
            {
                string error = $"Attempt to ommUInt() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to ommUInt() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmUInt)m_data;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. 
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.REAL"/>
        /// </summary>
        /// <returns><see cref="OmmReal"/> value</returns>
        public OmmReal Real()
        {
            if (m_data.m_dataType != Access.DataType.DataTypes.REAL)
            {
                string error = $"Attempt to real() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to real() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmReal)m_data;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. 
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.FLOAT"/>
        /// </summary>
        /// <returns>float value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.FLOAT"/></exception>
        public float Float()
        {
            if (m_data.m_dataType != Access.DataType.DataTypes.FLOAT)
            {
                string error = $"Attempt to float() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to float() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return ((OmmFloat)m_data).Value;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. 
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.FLOAT"/>
        /// </summary>
        /// <returns><see cref="OmmFloat"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.FLOAT"/></exception>
        public OmmFloat OmmFloat()
        {
            if (m_data.m_dataType != Access.DataType.DataTypes.FLOAT)
            {
                string error = $"Attempt to ommFloat() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to call OmmFloat() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmFloat)m_data;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. 
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.DOUBLE"/>
        /// </summary>
        /// <returns>double value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.DOUBLE"/></exception>
        public double Double()
        {
            if (m_data.m_dataType != Access.DataType.DataTypes.DOUBLE)
            {
                string error = $"Attempt to double() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to double() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return ((OmmDouble)m_data).Value;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. 
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.DOUBLE"/>
        /// </summary>
        /// <returns><see cref="OmmDouble"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.DOUBLE"/></exception>
        public OmmDouble OmmDouble()
        {
            if (m_data.m_dataType != Access.DataType.DataTypes.DOUBLE)
            {
                string error = $"Attempt to OmmDouble() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to OmmDouble() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmDouble)m_data;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. 
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.DATE"/>
        /// </summary>
        /// <returns><see cref="OmmDate"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.DATE"/></exception>
        public OmmDate Date()
        {
            if (m_data.m_dataType != Access.DataType.DataTypes.DATE)
            {
                string error = $"Attempt to date() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to date() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmDate)m_data;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. 
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.TIME"/>
        /// </summary>
        /// <returns><see cref="OmmTime"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.TIME"/></exception>
        public OmmTime Time()
        {
            if (m_data.m_dataType != Access.DataType.DataTypes.TIME)
            {
                string error = $"Attempt to time() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to time() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmTime)m_data;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. 
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.DATETIME"/>
        /// </summary>
        /// <returns><see cref="OmmDateTime"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.DATETIME"/></exception>
        public OmmDateTime DateTime()
        {
            if (m_data.m_dataType != Access.DataType.DataTypes.DATETIME)
            {
                string error = $"Attempt to dateTime() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to dateTime() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmDateTime)m_data;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. 
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.QOS"/>
        /// </summary>
        /// <returns><see cref="OmmQos"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.QOS"/></exception>
        public OmmQos Qos()
        {
            if (m_data.m_dataType != Access.DataType.DataTypes.QOS)
            {
                string error = $"Attempt to call OmmQos() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to qos() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmQos)m_data;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. 
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.STATE"/>
        /// </summary>
        /// <returns><see cref="OmmState"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.STATE"/></exception>
        public OmmState State()
        {
            if (m_data.m_dataType != Access.DataType.DataTypes.STATE)
            {
                string error = $"Attempt to call OmmState() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to state() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmState)m_data;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. 
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.ENUM"/>
        /// </summary>
        /// <returns>integer value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.ENUM"/></exception>
        public int Enum()
        {
            if (m_data.m_dataType != Access.DataType.DataTypes.ENUM)
            {
                string error = $"Attempt to enum() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to enum() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return ((OmmEnum)m_data).Value;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. 
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.ENUM"/>
        /// </summary>
        /// <returns><see cref="OmmEnum"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.ENUM"/></exception>
        public OmmEnum OmmEnum()
        {
            if (m_data.m_dataType != Access.DataType.DataTypes.ENUM)
            {
                string error = $"Attempt to ommEnum() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to ommEnum() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmEnum)m_data;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. 
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.BUFFER"/>
        /// </summary>
        /// <returns><see cref="OmmBuffer"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.BUFFER"/></exception>
        public OmmBuffer Buffer()
        {
            if (m_data.m_dataType != Access.DataType.DataTypes.BUFFER)
            {
                string error = $"Attempt to buffer() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to buffer() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmBuffer)m_data;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. 
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.ASCII"/>
        /// </summary>
        /// <returns><see cref="OmmAscii"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.ASCII"/></exception>
        public OmmAscii Ascii()
        {
            if (m_data.m_dataType != Access.DataType.DataTypes.ASCII)
            {
                string error = $"Attempt to ascii() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to ascii() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmAscii)m_data;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. 
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.UTF8"/>
        /// </summary>
        /// <returns><see cref="OmmUtf8"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.UTF8"/></exception>
        public OmmUtf8 Utf8()
        {
            if (m_data.m_dataType != Access.DataType.DataTypes.UTF8)
            {
                string error = $"Attempt to utf8() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to utf8() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmUtf8)m_data;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. 
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.RMTES"/>
        /// </summary>
        /// <returns><see cref="OmmRmtes"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.RMTES"/></exception>
        public OmmRmtes Rmtes()
        {
            if (m_data.m_dataType != Access.DataType.DataTypes.RMTES)
            {
                string error = $"Attempt to rmtes() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Access.Data.DataCode.BLANK == m_data.Code)
                throw new OmmInvalidUsageException("Attempt to rmtes() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmRmtes)m_data;
        }

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type. 
        /// Throws OmmInvalidUsageException if contained object is not <see cref="DataType.DataTypes.ERROR"/>
        /// </summary>
        /// <returns><see cref="OmmError"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="DataType.DataTypes.ERROR"/></exception>
        public OmmError Error()
        {
            if (m_data.m_dataType != Access.DataType.DataTypes.ERROR)
            {
                string error = $"Attempt to error() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            return (OmmError)m_data;
        }

        internal CodecReturnCode DecodeKey(int majorVersion, int minorVersion, Buffer body, int keyDataType) 
        {
            m_decodeIterator.Clear();
            CodecReturnCode ret = m_decodeIterator.SetBufferAndRWFVersion(body, majorVersion, minorVersion);
            if (ret < CodecReturnCode.SUCCESS)
            {
                m_decodingError.ErrorCode = OmmError.ErrorCodes.ITERATOR_SET_FAILURE;
                m_data = m_decodingError;
                return ret;
            }
            var load = m_objectManager.GetDataObjectFromPool(keyDataType);

            if (load == null)
            {
                m_decodingError.ErrorCode = OmmError.ErrorCodes.UNSUPPORTED_DATA_TYPE;
                m_data = m_decodingError;
                return CodecReturnCode.UNSUPPORTED_DATA_TYPE;
            }
            if ((ret = load.Decode(m_decodeIterator)) < CodecReturnCode.SUCCESS)
            {
                load.ClearAndReturnToPool_All();
                m_decodingError.ErrorCode = load.m_errorCode;
                m_data = m_decodingError;
                return ret;
            }
            m_data = load;

            return CodecReturnCode.SUCCESS;
        }

        internal string ToString(int indent)
        {
            return m_data != null ? m_data.ToString(indent) : string.Empty;
        }
    }
}
