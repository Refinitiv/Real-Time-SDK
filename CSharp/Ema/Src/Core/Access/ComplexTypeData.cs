/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System.Runtime.CompilerServices;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// ComplexTypeData class can be used to represent SummaryData, Attribute data of messages and Payload.<br/>
    /// </summary>
    /// <remarks>
    /// SummaryData is used to convey Omm SummaryData information optionally present on Map, Series and Vector.<br/>
    /// SummaryData contains objects of complex type.<br/>
    /// Payload conveys the data part of item image.<br/>
    /// Objects of this class are intended to be short lived or rather transitional.<br/>
    /// This class is designed to efficiently perform extracting of SummaryData, Attribute, Payload and their content.<br/>
    /// Objects of this class are not cache-able.
    /// </remarks>
    public sealed class ComplexTypeData
    {
        private NoData m_defaultData = new NoData();
        private OmmError m_error = new OmmError();
        private bool m_hasError;
        internal EmaObjectManager m_objectManager = EmaGlobalObjectPool.Instance;
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
        internal ComplexTypeData(int dataType, EmaObjectManager objectManager)
        {
            m_objectManager = objectManager;
            var data = m_objectManager.GetComplexTypeFromPool(dataType);
            m_isDataExternal = false;
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
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Clear()
        {
            if (m_data != m_defaultData && !m_isDataExternal)
            {
                m_data.ClearAndReturnToPool_All();
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
        public int DataType { get => m_hasError ? Access.DataType.DataTypes.ERROR : m_data.m_dataType; }
        /// <summary>
        /// Returns the complex type based on the DataType.
        /// </summary>
        public ComplexType Data { get => m_data; internal set { m_data = value; } }
        
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.RequestMsg"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.RequestMsg"/>
        /// </summary>
        /// <returns>The <see cref="Access.RequestMsg"/> structure representing the current OMM data</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="RequestMsg"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public RequestMsg RequestMsg() => GetComplexData<RequestMsg>(Access.DataType.DataTypes.REQ_MSG);
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.RefreshMsg"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.RefreshMsg"/>
        /// </summary>
        /// <returns>The <see cref="Access.RefreshMsg"/> structure representing the current OMM data</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="RefreshMsg"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public RefreshMsg RefreshMsg() => GetComplexData<RefreshMsg>(Access.DataType.DataTypes.REFRESH_MSG);
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.UpdateMsg"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.UpdateMsg"/>
        /// </summary>
        /// <returns>The <see cref="Access.UpdateMsg"/> structure representing the current OMM data</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.UpdateMsg"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg UpdateMsg() => GetComplexData<UpdateMsg>(Access.DataType.DataTypes.UPDATE_MSG);
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.StatusMsg"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.StatusMsg"/>
        /// </summary>
        /// <returns>The <see cref="Access.StatusMsg"/> structure representing the current OMM data</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.StatusMsg"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public StatusMsg StatusMsg() => GetComplexData<StatusMsg>(Access.DataType.DataTypes.STATUS_MSG);
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.PostMsg"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.PostMsg"/>
        /// </summary>
        /// <returns>The <see cref="Access.PostMsg"/> structure representing the current OMM data</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.PostMsg"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public PostMsg PostMsg() => GetComplexData<PostMsg>(Access.DataType.DataTypes.POST_MSG);
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.AckMsg"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.AckMsg"/>
        /// </summary>
        /// <returns>The <see cref="Access.AckMsg"/> structure representing the current OMM data</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.AckMsg"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public AckMsg AckMsg() => GetComplexData<AckMsg>(Access.DataType.DataTypes.ACK_MSG);
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.GenericMsg"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.GenericMsg"/>
        /// </summary>
        /// <returns>The <see cref="Access.GenericMsg"/> structure representing the current OMM data</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.GenericMsg"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public GenericMsg GenericMsg() => GetComplexData<GenericMsg>(Access.DataType.DataTypes.GENERIC_MSG);
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.FieldList"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.FieldList"/>
        /// </summary>
        /// <returns>The <see cref="Access.FieldList"/> structure representing the current OMM data</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.FieldList"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public FieldList FieldList() => GetComplexData<FieldList>(Access.DataType.DataTypes.FIELD_LIST);
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.ElementList"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.ElementList"/>
        /// </summary>
        /// <returns>The <see cref="Access.ElementList"/> structure representing the current OMM data</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.ElementList"/></exception>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ElementList ElementList() => GetComplexData<ElementList>(Access.DataType.DataTypes.ELEMENT_LIST);
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.Vector"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.Vector"/>
        /// </summary>
        /// <returns><see cref="Access.Vector"/> structure representing the current OMM data</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.Vector"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public Vector Vector() => GetComplexData<Vector>(Access.DataType.DataTypes.VECTOR);
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.Map"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.Map"/>
        /// </summary>
        /// <returns><see cref="Access.Map"/> structure representing the current OMM data</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.Map"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public Map Map() => GetComplexData<Map>(Access.DataType.DataTypes.MAP);
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.Series"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.Series"/>
        /// </summary>
        /// <returns><see cref="Access.Series"/> structure representing the current OMM data</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.Series"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public Series Series() => GetComplexData<Series>(Access.DataType.DataTypes.SERIES);
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.FilterList"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.FilterList"/>
        /// </summary>
        /// <returns><see cref="Access.FilterList"/> structure representing the current OMM data</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.FilterList"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public FilterList FilterList() => GetComplexData<FilterList>(Access.DataType.DataTypes.FILTER_LIST);
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.OmmOpaque"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.OmmOpaque"/>
        /// </summary>
        /// <returns><see cref="Access.OmmOpaque"/> structure representing the current OMM data</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.OmmOpaque"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmOpaque OmmOpaque() => GetComplexData<OmmOpaque>(Access.DataType.DataTypes.OPAQUE);
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.OmmXml"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.OmmXml"/>
        /// </summary>
        /// <returns><see cref="Access.OmmXml"/> structure representing the current OMM data</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.OmmXml"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmXml OmmXml() => GetComplexData<OmmXml>(Access.DataType.DataTypes.XML);
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.OmmJson"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.OmmJson"/>
        /// </summary>
        /// <returns><see cref="Access.OmmJson"/> structure representing the current OMM data</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.OmmJson"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmJson OmmJson() => GetComplexData<OmmJson>(Access.DataType.DataTypes.JSON);
        /// <summary>
        /// Returns the current OMM data represented as a <see cref="Access.OmmAnsiPage"/> type.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="Access.OmmAnsiPage"/>
        /// </summary>
        /// <returns><see cref="Access.OmmAnsiPage"/> structure representing the current OMM data</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.OmmAnsiPage"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmAnsiPage OmmAnsiPage() => GetComplexData<OmmAnsiPage>(Access.DataType.DataTypes.ANSI_PAGE);
        /// <summary>
        /// Returns Error.
        /// Throws OmmInvalidUsageException if contained object is not <see cref="OmmError"/>
        /// </summary>
        /// <returns><see cref="Access.OmmError"/> structure</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="Access.OmmError"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmError OmmError()
        {
            if (!m_hasError)
            {
                string error = $"Attempt to OmmError() while actual entry data type is {Access.DataType.AsString(m_data.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            return m_error;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private T GetComplexData<T>(int dataType) where T : ComplexType
        {
            if (m_hasError || m_data.m_dataType != dataType)
            {
                throw new OmmInvalidUsageException($"Attempt to use {Access.DataType.AsString(dataType)} while actual entry data type is {Access.DataType.AsString(m_data.DataType)}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            return (T)m_data;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal CodecReturnCode Decode(Buffer body,
            int dataType, 
            int majorVersion, int minorVersion, 
            DataDictionary? dictionary,
            object? localDb = null)
        {
            if (dataType != DataTypes.NO_DATA)
            {

                if (dataType == m_data.DataType)
                {
                    m_data.Clear_Decode();
                    CodecReturnCode ret;
                    if ((ret = m_data!.Decode(majorVersion, minorVersion, body, dictionary, localDb)) < CodecReturnCode.SUCCESS)
                    {
                        m_data.ClearAndReturnToPool_All();
                        SetError(Access.OmmError.ErrorCodes.UNKNOWN_ERROR);
                        return ret;
                    }
                }
                else
                {
                    m_data.ClearAndReturnToPool_Decode();
                    var load = m_objectManager.GetComplexTypeFromPool(dataType);
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
                            load.ClearAndReturnToPool_All();
                            SetError(Access.OmmError.ErrorCodes.UNKNOWN_ERROR);
                            return ret;
                        }
                        m_isDataExternal = false;
                        m_data = load;
                    }
                }

                return CodecReturnCode.SUCCESS;
            }
            else
            {
                m_data = m_defaultData;
                return CodecReturnCode.SUCCESS;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void SetError(OmmError.ErrorCodes errorCode)
        {
            Clear();
            m_hasError = true;
            m_error.Clear_All();
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
