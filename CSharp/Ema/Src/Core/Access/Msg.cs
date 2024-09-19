/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023, 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// Base message interface for all message representing classes.
    /// </summary>
    public abstract class Msg : ComplexType
    {
        internal Eta.Codec.Msg m_rsslMsg = new Eta.Codec.Msg();
        internal Eta.Codec.Msg m_internalRsslMsg;
        internal ComplexTypeData m_payload = new ComplexTypeData();
        internal ComplexTypeData m_attrib = new ComplexTypeData();
        internal ComplexType? m_defaultData = new NoData();
        internal MsgEncoder m_msgEncoder;
        private DecodeIterator m_helperDecodeIterator = new DecodeIterator();

        private EmaBuffer m_extendedHeader = new EmaBuffer();
        private bool m_extendedHeaderSet = false;
        private EmaBuffer m_permData = new EmaBuffer();
        private bool m_permDataSet = false;
        private EmaBuffer m_itemGroupData = new EmaBuffer();
        private bool m_itemGroupDataSet = false;

        internal int m_msgClass;

        /// <summary>
        /// This is used to indicate whether this Msg is cloned from another message
        /// in order to return its payload back to the global pool.
        /// </summary>
        protected bool m_isClonedMsg = false;

        private string? m_serviceName = null;

#pragma warning disable CS8618

        /// <summary>
        /// Constructor for Msg instance
        /// </summary>
        internal Msg()
        {
            ClearTypeSpecific_All = ClearMsg_All;
            ClearTypeSpecific_Decode = ClearMsg_Decode;

            DecodeComplexType = DecodeMsg;

            m_dataType = Access.DataType.DataTypes.MSG;
            m_internalRsslMsg = m_rsslMsg;
        }

        internal Msg(EmaObjectManager manager)
        {
            m_objectManager = manager;
            m_payload.m_objectManager = manager;
            m_attrib.m_objectManager = manager;

            m_dataType = Access.DataType.DataTypes.MSG;

            ClearTypeSpecific_All = ClearMsg_All;
            ClearTypeSpecific_Decode = ClearMsg_Decode;
            DecodeComplexType = DecodeMsg;

            m_internalRsslMsg = m_rsslMsg;
        }

#pragma warning restore CS8618

        internal void SetObjectManager(EmaObjectManager manager)
        {
            m_objectManager = manager;
            m_payload.m_objectManager = manager;
            m_attrib.m_objectManager = manager;

            m_internalRsslMsg = m_rsslMsg;
        }

        /// <summary>
        /// Determines whether MsgKey of the message holds the name
        /// </summary>
        /// <returns>true if the message key contains a name, false otherwise.</returns>
        public virtual bool HasName { get => m_rsslMsg.CheckHasMsgKey() && m_rsslMsg.MsgKey != null && m_rsslMsg.MsgKey.CheckHasName(); }

        /// <summary>
        /// Determines whether the message has message key
        /// </summary>
        /// <returns>true if the message contains a message key, false otherwise.</returns>
        public virtual bool HasMsgKey { get => m_rsslMsg.CheckHasMsgKey() && m_rsslMsg.MsgKey != null; }

        /// <summary>
        /// Determines whether MsgKey has NameType property
        /// </summary>
        /// <returns>true if the message key has the name type property, false otherwise.</returns>
        public virtual bool HasNameType { get => m_rsslMsg.CheckHasMsgKey() && m_rsslMsg.MsgKey != null && m_rsslMsg.MsgKey.CheckHasNameType(); }

        /// <summary>
        /// Determines whether the message key has serviceId present
        /// </summary>
        /// <returns>true if the message key contains a service ID, false otherwise.</returns>
        public virtual bool HasServiceId { get => m_rsslMsg.CheckHasMsgKey() && m_rsslMsg.MsgKey != null && m_rsslMsg.MsgKey.CheckHasServiceId(); }

        /// <summary>
        /// Determines whether the message has identifier
        /// </summary>
        /// <returns>true if the message key contains an identifier, false otherwise.</returns>
        public virtual bool HasId { get => m_rsslMsg.CheckHasMsgKey() && m_rsslMsg.MsgKey != null && m_rsslMsg.MsgKey.CheckHasIdentifier(); }

        /// <summary>
        /// Determines whether the message has filter
        /// </summary>
        public virtual bool HasFilter { get => m_rsslMsg.CheckHasMsgKey() && m_rsslMsg.MsgKey != null && m_rsslMsg.MsgKey.CheckHasFilter(); }

        /// <summary>
        /// Determines whether the message has extended header
        /// </summary>
        /// <returns>true if the message contains an Extended Header, false otherwise.</returns>
        public virtual bool HasExtendedHeader { get => m_rsslMsg.CheckHasExtendedHdr(); }

        /// <summary>
        /// Checks whether the message contains MsgKey attributes
        /// </summary>
        /// <returns>true if the message key contains additional attributes, false otherwise.</returns>
        public virtual bool HasAttrib { get => m_rsslMsg.CheckHasMsgKey() && m_rsslMsg.MsgKey != null && m_rsslMsg.MsgKey.CheckHasAttrib(); }

        /// <summary>
        /// Determines whether current Msg instance has Service name set
        /// </summary>
        /// <returns>true if the message contains a service name, false otherwise.</returns>
        public bool HasServiceName { get => m_msgEncoder.m_serviceNameSet; }

        /// <summary>
        /// The stream id associated with the message
        /// </summary>
        /// <returns>The stream ID.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int StreamId()
        {
            return m_rsslMsg.StreamId;
        }

        /// <summary>
        /// The domain type of the message
        /// </summary>
        /// <returns>The domain type.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int DomainType()
        {
            return m_rsslMsg.DomainType;
        }

        /// <summary>
        /// The name within the message key
        /// </summary>
        /// <returns>The name string</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if the message does not have a set name, which can be verified with <see cref="Msg.HasName"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public string Name()
        {
            if (!HasName)
            {
                throw new OmmInvalidUsageException("Invalid attempt to call Name() while it is not set.");
            }

            Buffer nameBuffer = m_rsslMsg.MsgKey.Name;
            if (nameBuffer.Length == 0)
                return string.Empty;
            else
                return nameBuffer.ToString();
        }

        /// <summary>
        /// ServiceId within the message key
        /// </summary>
        /// <returns>The service ID.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if the message does not have a set service ID, which can be verified with <see cref="Msg.HasServiceId"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int ServiceId()
        {
            if (!HasServiceId)
            {
                throw new OmmInvalidUsageException("Invalid attempt to call ServiceId() while it is not set.");
            }

            return m_rsslMsg.MsgKey.ServiceId;
        }

        /// <summary>
        /// The identifier within the message key
        /// </summary>
        /// <returns>The identifier.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if the message does not have an ID, which can be verified with <see cref="Msg.HasId"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int Id()
        {
            if (!HasId)
            {
                throw new OmmInvalidUsageException("Invalid attempt to call Id() while it is not set.");
            }

            return m_rsslMsg.MsgKey.Identifier;
        }

        /// <summary>
        /// The filter within the message key
        /// </summary>
        /// <returns>The message key filter.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if the message does not have a set filter, which can be verified with <see cref="Msg.HasFilter"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public long Filter()
        {
            if (!HasFilter)
            {
                throw new OmmInvalidUsageException("Invalid attempt to call Filter() while it is not set.");
            }

            return m_rsslMsg.MsgKey.Filter;
        }

        /// <summary>
        /// The name type within the message key
        /// </summary>
        /// <returns>The name type.</returns>
        ///  <exception cref="OmmInvalidUsageException">Thrown if the message does not have a set name type, which can be verified with <see cref="Msg.HasNameType"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int NameType()
        {
            if (!HasNameType)
            {
                throw new OmmInvalidUsageException("Invalid attempt to call NameType() while it is not set.");
            }

            return m_rsslMsg.MsgKey.NameType;
        }

        /// <summary>
        /// The extended header
        /// </summary>
        /// <returns>The extended header.</returns>
        ///  <exception cref="OmmInvalidUsageException">Thrown if the message does not have a set extended header, which can be verified with <see cref="Msg.HasExtendedHeader"/></exception>
        public EmaBuffer ExtendedHeader()
        {
            if (!m_rsslMsg.CheckHasExtendedHdr())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call ExtendedHeader() while it is not set.");
            }
            return GetExtendedHeaderEmaBuffer();
        }

        /// <summary>
        /// Returns the contained attributes Data based on the attribute's DataType.
        /// Attrib contains no data if <see cref="ComplexTypeData.DataType"/> returns
        /// <see cref="DataType.DataTypes.NO_DATA"/>.
        /// </summary>
        /// <returns>The message key attrib data.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ComplexTypeData Attrib()
        {
            return m_attrib;
        }

        /// <summary>
        /// Returns the contained payload Data based on the payload DataType.
        /// Payload contains no data if <see cref="ComplexTypeData.DataType"/> returns
        /// <see cref="DataType.DataTypes.NO_DATA"/>.
        /// </summary>
        /// <returns>Reference to Payload object.</returns>
        /// <exception cref="NotImplementedException"></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ComplexTypeData Payload()
        {
            return m_payload;
        }

        /// <summary>
        /// Returns the ServiceName within the MsgKey.
        /// </summary>
        /// <returns>String containing service name</returns>
        ///  <exception cref="OmmInvalidUsageException">Thrown if the message does not have a set service name, which can be verified with <see cref="Msg.HasServiceName"/></exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public string ServiceName()
        {
            if (!m_msgEncoder.m_serviceNameSet)
            {
                throw new OmmInvalidUsageException("Invalid attempt to call ServiceName() while Service name is not set.");
            }

            return m_serviceName!;
        }

        /// <summary>
        /// Clear current Msg instance.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ClearMsg_All()
        {
            m_payload.Clear();
            m_attrib.Clear();

            m_extendedHeaderSet = false;
            m_permDataSet = false;
            m_permData.Clear();
            m_extendedHeader.Clear();
            m_itemGroupDataSet = false;

            m_rsslMsg.Clear();
            m_rsslMsg.MsgClass = m_msgClass;

            m_rsslMsg = m_internalRsslMsg;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ClearMsg_Decode()
        {
            m_payload.Clear();
            m_attrib.Clear();

            m_extendedHeaderSet = false;
            m_permDataSet = false;
            m_permData.Clear();
            m_extendedHeader.Clear();
            m_itemGroupDataSet = false;

            m_rsslMsg = m_internalRsslMsg;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ClearMsg_Internal()
        {
            m_extendedHeaderSet = false;
            m_permDataSet = false;
            m_permData.Clear();
            m_extendedHeader.Clear();
            m_itemGroupDataSet = false;

            m_rsslMsg = m_internalRsslMsg;
        }

        /// <summary>
        /// Performs a deep copy of the current decoded message into the proivded <see cref="Msg"/> instance.
        /// </summary>
        /// <param name="destMsg">the destination <see cref="Msg"/> instance to copy data to</param>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void CopyMsg(Msg destMsg)
        {
            destMsg.Clear_All();
            destMsg.m_dataDictionary = m_dataDictionary;

            CopyFromInternalRsslMsg(m_rsslMsg, destMsg);

            destMsg.m_hasDecodedDataSet = true;

            destMsg.m_isClonedMsg = true;

            // Sets the service name if it is set
            if (m_msgEncoder!.m_serviceNameSet && m_serviceName != null)
            {
                destMsg.SetServiceName(m_serviceName);
            }
        }

        #region Internal methods

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void CopyFromInternalRsslMsg(IMsg sourceRsslMsg, Msg destMsg)
        {
            sourceRsslMsg.Copy(destMsg.m_rsslMsg, CopyMsgFlags.ALL_FLAGS);

            if (sourceRsslMsg.MsgKey != null && m_rsslMsg.MsgKey.CheckHasAttrib())
            {
                DecodeComplexTypeInternal(sourceRsslMsg.MsgKey!.AttribContainerType, destMsg.m_rsslMsg.MsgKey.EncodedAttrib, m_dataDictionary, null, destMsg.m_attrib);
            }

            if (destMsg.m_rsslMsg.EncodedDataBody != null && destMsg.m_rsslMsg.EncodedDataBody.Length > 0)
            {
                DecodeComplexTypeInternal(sourceRsslMsg.ContainerType, destMsg.m_rsslMsg.EncodedDataBody, m_dataDictionary, null, destMsg.m_payload);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal CodecReturnCode Decode(IMsg msg, int majorVersion, int minorVersion, DataDictionary? dataDictionary)
        {
            ClearMsg_Internal();
            ClearTypeSpecific_Decode?.Invoke();
            m_hasDecodedDataSet = true;

            m_MajorVersion = majorVersion;
            m_MinorVersion = minorVersion;

            m_dataDictionary = dataDictionary;
            m_rsslMsg = (Eta.Codec.Msg)msg;
            m_bodyBuffer = msg.EncodedMsgBuffer;

            MsgKey msgKey = m_rsslMsg.MsgKey;
            bool hasAttrib = msgKey != null && msgKey.CheckHasAttrib();
            int attribType = hasAttrib ? msgKey!.AttribContainerType : Eta.Codec.DataTypes.NO_DATA;

            CodecReturnCode ret;
            if (hasAttrib)
            {
                ret = DecodeComplexTypeInternal(attribType,
                    msgKey!.EncodedAttrib!,
                    m_dataDictionary,
                    null,
                    m_attrib);

                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            return DecodeComplexTypeInternal(m_rsslMsg.ContainerType,
                m_rsslMsg.EncodedDataBody,
                m_dataDictionary,
                null,
                m_payload);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal CodecReturnCode Decode(Buffer body, int majorVersion, int minorVersion, DataDictionary? dataDictionary, object? localDb)
        {
            ClearMsg_Internal();
            m_dataDictionary = dataDictionary;
            m_bodyBuffer = body;
            return Decode(majorVersion, minorVersion, dataDictionary, localDb);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal CodecReturnCode DecodeMsg(int majorVersion, int minorVersion, Buffer body, DataDictionary? dictionary, object? localDb)
        {
            ClearMsg_Internal();
            m_dataDictionary = dictionary;
            m_bodyBuffer = body;
            return Decode(majorVersion, minorVersion, dictionary, localDb);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal CodecReturnCode Decode(int majorVersion, int minorVersion, DataDictionary? dataDictionary, object? localDb)
        {
            m_hasDecodedDataSet = true;
            m_MajorVersion = majorVersion;
            m_MinorVersion = minorVersion;

            CodecReturnCode ret;
            m_decodeIterator.Clear();

            if ((ret = m_decodeIterator.SetBufferAndRWFVersion(m_bodyBuffer, m_MajorVersion, m_MinorVersion)) < CodecReturnCode.SUCCESS)
            {
                m_errorCode = OmmError.ErrorCodes.ITERATOR_SET_FAILURE;
                m_attrib.SetError(m_errorCode);
                m_payload.SetError(m_errorCode);
                return ret;
            }

            m_rsslMsg.Clear();
            ret = m_rsslMsg.Decode(m_decodeIterator);

            Debug.Assert(m_msgClass == m_rsslMsg.MsgClass, $"Decoding message: expected message class: {m_msgClass}, actual message class: {m_rsslMsg.MsgClass}");

            switch (ret)
            {
                case CodecReturnCode.SUCCESS:
                    ret = DecodeAttribAndPayload(dataDictionary!, null);
                    return ret;

                case CodecReturnCode.ITERATOR_OVERRUN:
                    m_errorCode = OmmError.ErrorCodes.ITERATOR_OVERRUN;
                    m_attrib.SetError(m_errorCode);
                    m_payload.SetError(m_errorCode);
                    return ret;

                case CodecReturnCode.INCOMPLETE_DATA:
                    m_errorCode = OmmError.ErrorCodes.INCOMPLETE_DATA;
                    m_attrib.SetError(m_errorCode);
                    m_payload.SetError(m_errorCode);
                    return ret;

                default:
                    m_errorCode = OmmError.ErrorCodes.UNKNOWN_ERROR;
                    m_attrib.SetError(m_errorCode);
                    m_payload.SetError(m_errorCode);
                    return ret;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal CodecReturnCode DecodeAttribAndPayload(DataDictionary dictionary, object? localDb)
        {
            bool hasAttrib = m_rsslMsg.MsgKey != null && m_rsslMsg.MsgKey.CheckHasAttrib();
            int attribType = hasAttrib ? m_rsslMsg.MsgKey!.AttribContainerType : Eta.Codec.DataTypes.NO_DATA;
            if (hasAttrib)
            {
                var ret = DecodeComplexTypeInternal(attribType,
                m_rsslMsg.MsgKey!.EncodedAttrib,
                dictionary,
                localDb,
                m_attrib);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            return DecodeComplexTypeInternal(m_rsslMsg.ContainerType,
                m_rsslMsg.EncodedDataBody,
                dictionary,
                localDb,
                m_payload);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private CodecReturnCode DecodeComplexTypeInternal(int dataType,
            Buffer body,
            DataDictionary? dictionary,
            object? localDb,
            ComplexTypeData complexType)
        {
            int ommDataType = dataType;
            if (ommDataType == DataTypes.MSG)
            {
                m_helperDecodeIterator.Clear();
                m_helperDecodeIterator.SetBufferAndRWFVersion(m_rsslMsg.EncodedDataBody, m_MajorVersion, m_MinorVersion);
                ommDataType = Decoder.GetMsgDataType(m_helperDecodeIterator.ExtractMsgClass());
            }

            return complexType.Decode(body, ommDataType, m_MajorVersion, m_MinorVersion, dictionary, localDb);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void SetMsgServiceName(string serviceName)
        {
            if (serviceName == null)
                throw new OmmInvalidUsageException("Passed in ServiceName is null.");
            if (m_msgEncoder.m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set ServiceName while Message encoding already started.");

            m_rsslMsg.ApplyHasMsgKey();
            m_serviceName = serviceName;
            m_msgEncoder.m_serviceNameSet = true;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void SetServiceName(string serviceName)
        {
            m_serviceName = serviceName;
            m_msgEncoder.m_serviceNameSet = true;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal EmaBuffer GetExtendedHeaderEmaBuffer()
        {
            if (!m_extendedHeaderSet)
            {
                m_extendedHeader.CopyFrom(m_rsslMsg.ExtendedHeader.Data().Contents,
                    m_rsslMsg.ExtendedHeader.Position,
                    m_rsslMsg.ExtendedHeader.Length);
                m_extendedHeaderSet = true;
            }
            return m_extendedHeader;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal EmaBuffer GetPermDataEmaBuffer()
        {
            if (!m_permDataSet)
            {
                m_permData.CopyFrom(m_rsslMsg.PermData.Data().Contents,
                    m_rsslMsg.PermData.Position,
                    m_rsslMsg.PermData.Length);
                m_permDataSet = true;
            }
            return m_permData;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal EmaBuffer GetItemGroupBuffer()
        {
            if (!m_itemGroupDataSet)
            {
                m_itemGroupData.CopyFrom(m_rsslMsg.GroupId.Data().Contents,
                    m_rsslMsg.GroupId.Position,
                    m_rsslMsg.GroupId.Length);
                m_itemGroupDataSet = true;
            }
            return m_itemGroupData;
        }

        internal bool EncodeComplete(EncodeIterator encodeIterator, out string? error)
        {
            return m_msgEncoder.EncodeComplete(encodeIterator, out error);
        }

        internal virtual void EncodeComplete()
        { }

        /// <summary>
        /// Obtains string representation of the current Message.
        /// </summary>
        /// <returns>String representation of the current Message.</returns>
        public override string ToString()
        {
            return ToString(0);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal override string ToString(int indent)
        {
            if (m_hasDecodedDataSet)
            {
                return FillString(indent);
            }

            return $"\n{GetType().Name}.ToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.";
        }

        /// <summary>
        /// Obtains string representation of the current Message
        /// </summary>
        /// <param name="dataDictionary"><see cref="Ema.Rdm.DataDictionary"/> object used to obtain string representaiton
        /// of the Message object</param>
        /// <returns><see cref="string"/> object representing current ComplexType instance</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override string ToString(Rdm.DataDictionary dataDictionary)
        {
            if (!dataDictionary.IsEnumTypeDefLoaded || !dataDictionary.IsFieldDictionaryLoaded)
            {
                return "\nThe provided DataDictionary is not properly loaded.";
            }
            EncodeComplete();
            if (Encoder != null && Encoder.m_encodeIterator != null && Encoder.m_containerComplete)
            {
                var encodedBuffer = Encoder.m_encodeIterator!.Buffer();

                var tmpObject = (Msg?)m_objectManager.GetComplexTypeFromPool(this.DataType);
                if (tmpObject == null)
                {
                    return $"\nToString(DataDictionary) is called on an invalid DataType {DataType}.";
                }

                string result = string.Empty;
                try
                {
                    CodecReturnCode ret;
                    if ((ret = tmpObject.Decode(encodedBuffer, Codec.MajorVersion(), Codec.MinorVersion(), dataDictionary.rsslDataDictionary(), null)) != CodecReturnCode.SUCCESS)
                    {
                        return $"\nFailed to decode Msg of type {DataType}: {ret.GetAsString()}";
                    }
                    result = tmpObject!.FillString(0)!;
                }
                finally
                {
                    tmpObject!.ClearAndReturnToPool_All();
                }
                return result;
            }
            else
            {
                return $"\nMsg instance of type {DataType} contains no valid encoded data.";
            }
        }

        #endregion Internal methods

        /// <summary>
        /// Finalizer
        /// </summary>
        ~Msg()
        {
            if (m_isClonedMsg)
            {
                m_payload.m_data.ClearAndReturnToPool_All();
            }
        }
    }
}