/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// ComplexTypeData class can be used to represent SummaryData, Attribute data of messages and Payload. 
    /// 
    /// SummaryData is used to convey Omm SummaryData information optionally present on Map, Series and Vector.
    /// SummaryData contains objects of complex type.
    /// Payload conveys the data part of item image.
    /// 
    /// Objects of this class are intended to be short lived or rather transitional.
    /// This class is designed to efficiently perform extracting of SummaryData, Attribute, Payload and their content.
    /// Objects of this class are not cache-able.
    /// </summary>
    public sealed class ComplexTypeData
    {
        private NoData m_defaultData = new NoData();
        private OmmError m_error = new OmmError();
        private bool m_hasError;
        internal EmaObjectManager? m_objectManager;
        internal ComplexType m_data;
        private bool m_isDataExternal = false;

        internal ComplexTypeData() 
        {
            m_data = m_defaultData;
        } 

        internal ComplexTypeData(EmaObjectManager objectManager)
        {
            m_data = m_defaultData;
            m_objectManager = objectManager;
        }

        /// <summary>
        /// Constructor for ComplexTypeData class
        /// </summary>
        /// <param name="dataType">the intended data type</param>
        /// <param name="objectManager">the object pool</param>
        internal ComplexTypeData(int dataType, EmaObjectManager? objectManager)
        {
            m_objectManager = objectManager;
            var data = m_objectManager != null 
                ? m_objectManager.GetComplexTypeFromPool(dataType) 
                : EmaObjectManager.GetComplexTypeObject(dataType);
            m_isDataExternal = m_objectManager == null;
            m_data = data != null ? data : m_defaultData;
        }

        internal void SetExternalData(ComplexType data)
        {
            m_isDataExternal = true;
            m_data = data;
        } 

        /// <summary>
        /// Clears current ComplexTypeData instance
        /// </summary>
        public void Clear()
        {
            if (m_data != m_defaultData && !m_isDataExternal)
            {
                if (m_data.Encoder != null) m_data.Encoder!.Clear();
                m_data.ReturnToPool();
            }
            m_data = m_defaultData;
            m_hasError = false;
            m_isDataExternal = false;
        }

        /// <summary>
        /// The DataType of the contained data.
        /// Return of <see cref="DataType.DataTypes.NO_DATA"/> signifies no data present in ComplexDataType.
        /// Return of <see cref="DataType.DataTypes.ERROR"/> signifies error while extracting content of ComplexDataType.
        /// </summary>
        public int DataType { get => m_hasError ? Access.DataType.DataTypes.ERROR : m_data.DataType; }
        /// <summary>
        /// Returns the complex type based on the DataType.
        /// </summary>
        public ComplexType Data { get => m_data; internal set { m_data = value; } }

        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.RequestMsg"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.RequestMsg"/>
        /// </summary>
        public RequestMsg RequestMsg()
        {
            if (m_hasError || m_data.DataType != Access.DataType.DataTypes.REQ_MSG)
            {
                throw new OmmInvalidUsageException($"Attempt to use RequestMsg while actual entry data type is {Access.DataType.AsString(m_data.DataType)}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            return (RequestMsg)m_data;
        }
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.RefreshMsg"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.RefreshMsg"/>
        /// </summary>
        public RefreshMsg RefreshMsg()
        {
            if (m_hasError || m_data.DataType != Access.DataType.DataTypes.REFRESH_MSG)
            {
                throw new OmmInvalidUsageException($"Attempt to use RefreshMsg while actual entry data type is {Access.DataType.AsString(m_data.DataType)}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            return (RefreshMsg)m_data;
        }
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.UpdateMsg"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.UpdateMsg"/>
        /// </summary>
        public UpdateMsg UpdateMsg()
        {
            if (m_hasError || m_data.DataType != Access.DataType.DataTypes.UPDATE_MSG)
            {
                throw new OmmInvalidUsageException($"Attempt to use UpdateMsg while actual entry data type is {Access.DataType.AsString(m_data.DataType)}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            return (UpdateMsg)m_data;
        }
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.StatusMsg"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.StatusMsg"/>
        /// </summary>
        public StatusMsg StatusMsg()
        {
            if (m_hasError || m_data.DataType != Access.DataType.DataTypes.STATUS_MSG)
            {
                throw new OmmInvalidUsageException($"Attempt to use StatusMsg while actual entry data type is {Access.DataType.AsString(m_data.DataType)}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            return (StatusMsg)m_data;
        }
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.PostMsg"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.PostMsg"/>
        /// </summary>
        public PostMsg PostMsg()
        {
            if (m_hasError || m_data.DataType != Access.DataType.DataTypes.POST_MSG)
            {
                throw new OmmInvalidUsageException($"Attempt to use PostMsg while actual entry data type is {Access.DataType.AsString(m_data.DataType)}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            return (PostMsg)m_data;
        }
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.AckMsg"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.AckMsg"/>
        /// </summary>
        public AckMsg AckMsg()
        {
            if (m_hasError || m_data.DataType != Access.DataType.DataTypes.ACK_MSG)
            {
                throw new OmmInvalidUsageException($"Attempt to use AckMsg while actual entry data type is {Access.DataType.AsString(m_data.DataType)}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            return (AckMsg)m_data;
        }
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.GenericMsg"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.GenericMsg"/>
        /// </summary>
        public GenericMsg GenericMsg()
        {
            if (m_hasError || m_data.DataType != Access.DataType.DataTypes.GENERIC_MSG)
            {
                throw new OmmInvalidUsageException($"Attempt to use GenericMsg while actual entry data type is {Access.DataType.AsString(m_data.DataType)}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            return (GenericMsg)m_data;
        }
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.FieldList"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.FieldList"/>
        /// </summary>
        public FieldList FieldList()
        {
            if (m_hasError || m_data.DataType != Access.DataType.DataTypes.FIELD_LIST)
            {
                throw new OmmInvalidUsageException($"Attempt to use FiledList while actual entry data type is {Access.DataType.AsString(m_data.DataType)}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            return (FieldList)m_data;
        }
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.ElementList"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.ElementList"/>
        /// </summary>
        public ElementList ElementList()
        {
            if (m_hasError || m_data.DataType != Access.DataType.DataTypes.ELEMENT_LIST)
            {
                throw new OmmInvalidUsageException($"Attempt to use ElementList while actual entry data type is {Access.DataType.AsString(m_data.DataType)}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            return (ElementList)m_data;
        }
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.Vector"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.Vector"/>
        /// </summary>
        public Vector Vector()
        {
            if (m_hasError || m_data.DataType != Access.DataType.DataTypes.VECTOR)
            {
                throw new OmmInvalidUsageException($"Attempt to use Vector while actual entry data type is {Access.DataType.AsString(m_data.DataType)}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            return (Vector)m_data;
        }
        /// <summary>
        /// Returns the current OMM data represented as a specific complex type.
        /// </summary>
        /// <returns><see cref="Access.Map"/> class reference to contained object</returns>
        /// <exception cref="OmmInvalidUsageException">thrown if contained object is not <see cref="Access.Map"/></exception>
        public Map Map()
        {
            if (m_hasError || m_data.DataType != Access.DataType.DataTypes.MAP)
            {
                throw new OmmInvalidUsageException($"Attempt to use Map while actual entry data type is {Access.DataType.AsString(m_data.DataType)}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            return (Map)m_data;
        }
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.Series"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.Series"/>
        /// </summary>
        public Series Series()
        {
            if (m_hasError || m_data.DataType != Access.DataType.DataTypes.SERIES)
            {
                throw new OmmInvalidUsageException($"Attempt to use Series while actual entry data type is {Access.DataType.AsString(m_data.DataType)}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            return (Series)m_data;
        }
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.FilterList"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.FilterList"/>
        /// </summary>
        public FilterList FilterList()
        {
            if (m_hasError || m_data.DataType != Access.DataType.DataTypes.FILTER_LIST)
            {
                throw new OmmInvalidUsageException($"Attempt to use FilterList while actual entry data type is {Access.DataType.AsString(m_data.DataType)}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            return (FilterList)m_data;
        }
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.OmmOpaque"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.OmmOpaque"/>
        /// </summary>
        public OmmOpaque OmmOpaque()
        {
            if (m_hasError || m_data.DataType != Access.DataType.DataTypes.OPAQUE)
            {
                throw new OmmInvalidUsageException($"Attempt to use Opaque while actual entry data type is {Access.DataType.AsString(m_data.DataType)}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            return (OmmOpaque)m_data;
        }
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.OmmXml"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.OmmXml"/>
        /// </summary>
        public OmmXml OmmXml()
        {
            if (m_hasError || m_data.DataType != Access.DataType.DataTypes.XML)
            {
                throw new OmmInvalidUsageException($"Attempt to use Xml while actual entry data type is {Access.DataType.AsString(m_data.DataType)}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            return (OmmXml)m_data;
        }
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.OmmAnsiPage"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.OmmAnsiPage"/>
        /// </summary>
        public OmmAnsiPage OmmAnsiPage()
        {
            if (m_hasError || m_data.DataType != Access.DataType.DataTypes.ANSI_PAGE)
            {
                throw new OmmInvalidUsageException($"Attempt to use AnsiPage while actual entry data type is {Access.DataType.AsString(m_data.DataType)}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            return (OmmAnsiPage)m_data;
        }
        /// <summary>
        /// Returns Error.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="OmmError"/>
        /// </summary>
        public OmmError OmmError()
        {
            if (!m_hasError)
            {
                string error = $"Attempt to OmmError() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            return m_error;
        }

        internal CodecReturnCode Decode(Buffer body, 
            int dataType, 
            int majorVersion, int minorVersion, 
            DataDictionary? dictionary,
            object? localDb = null)
        {
            if (m_data != m_defaultData && !m_isDataExternal) 
                m_data.ReturnToPool();

            var load = m_objectManager != null 
                ? m_objectManager.GetComplexTypeFromPool(dataType) 
                : EmaObjectManager.GetComplexTypeObject(dataType);
            if (load == null)
            {
                SetError(Access.OmmError.ErrorCodes.UNSUPPORTED_DATA_TYPE);
                return CodecReturnCode.UNSUPPORTED_DATA_TYPE;
            }
            else
            {
                CodecReturnCode ret;
                if ((ret = load!.Decode(majorVersion, minorVersion, body, dictionary, localDb)) < CodecReturnCode.SUCCESS)
                {
                    load.ReturnToPool();
                    SetError(Access.OmmError.ErrorCodes.UNKNOWN_ERROR);
                    return ret;
                }
                m_isDataExternal = m_objectManager == null;
                m_data = load;
            }

            return CodecReturnCode.SUCCESS;
        }

        internal void SetError(OmmError.ErrorCodes errorCode)
        {
            Clear();
            m_hasError = true;
            m_error.ClearInt();
            m_error.ErrorCode = errorCode;
        }

        internal string ToString(int indent)
        {
            if (m_hasError)
            {
                return m_error.ToString(indent);
            }
            else
            {
                return m_data.ToString(indent);
            }
        }
    }
}
