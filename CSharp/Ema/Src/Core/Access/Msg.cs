/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;
using System.Diagnostics;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    /// <summary>
    ///  Msg interface is for all message representing classes.
    /// </summary>
    public class Msg : ComplexType
    {
        internal Eta.Codec.Msg m_rsslMsg = new Eta.Codec.Msg();
        internal ComplexTypeData m_payload = new ComplexTypeData();
        internal ComplexTypeData m_attrib = new ComplexTypeData();
        internal ComplexType? m_payloadData;
        internal ComplexType? m_defaultData = new NoData();
        internal ComplexType? m_attribData;
        internal string? m_ServiceName;
        internal bool m_ServiceNameSet;
        internal Decoder? m_decoder;
        private DecodeIterator m_helperDecodeIterator = new DecodeIterator();
        internal OmmError.ErrorCodes m_errorCode;
        internal DataDictionary? m_dataDictionary;

        private EmaBuffer m_extendedHeader = new EmaBuffer();
        private bool m_extendedHeaderSet = false;
        private EmaBuffer m_permData = new EmaBuffer();
        private bool m_permDataSet = false;
        private EmaBuffer m_itemGroupData = new EmaBuffer();
        private bool m_itemGroupDataSet = false;

        internal int m_msgClass;

        string? m_serviceName = null;

        /// <summary>
        /// Constructor for Msg instance
        /// </summary>
        public Msg() {}

        internal Msg(EmaObjectManager manager)
        {
            m_objectManager = manager;
            m_payload.m_objectManager = manager;
            m_attrib.m_objectManager = manager;
        }

        internal void SetObjectManager(EmaObjectManager? manager)
        {
            m_objectManager = manager;
            m_payload.m_objectManager = manager;
            m_attrib.m_objectManager = manager;
        }

        /// <summary>
        /// Determines whether MsgKey of the message holds the name
        /// </summary>
        public virtual bool HasName { get => m_rsslMsg.CheckHasMsgKey() && m_rsslMsg.MsgKey != null && m_rsslMsg.MsgKey.CheckHasName(); }

        /// <summary>
        /// Determines whether the message has message key
        /// </summary>
        public virtual bool HasMsgKey { get => m_rsslMsg.CheckHasMsgKey() && m_rsslMsg.MsgKey != null; }

        /// <summary>
        /// Determines whether MsgKey has NameType property
        /// </summary>
        public virtual bool HasNameType { get => m_rsslMsg.CheckHasMsgKey() && m_rsslMsg.MsgKey != null && m_rsslMsg.MsgKey.CheckHasNameType(); }

        /// <summary>
        /// Determines whether the message key has serviceId present
        /// </summary>
        public virtual bool HasServiceId { get => m_rsslMsg.CheckHasMsgKey() && m_rsslMsg.MsgKey != null && m_rsslMsg.MsgKey.CheckHasServiceId(); }

        /// <summary>
        /// Determines whether the message has identifier
        /// </summary>
        public virtual bool HasId { get => m_rsslMsg.CheckHasMsgKey() && m_rsslMsg.MsgKey != null && m_rsslMsg.MsgKey.CheckHasIdentifier(); }

        /// <summary>
        /// Determines whether the message has filter
        /// </summary>
        public virtual bool HasFilter { get => m_rsslMsg.CheckHasMsgKey() && m_rsslMsg.MsgKey != null && m_rsslMsg.MsgKey.CheckHasFilter(); }

        /// <summary>
        /// Determines whether the message has extended header
        /// </summary>
        public virtual bool HasExtendedHeader { get => m_rsslMsg.CheckHasExtendedHdr(); }

        /// <summary>
        /// Checks whether the message contains MsgKey attributes
        /// </summary>
        public virtual bool HasAttrib { get => m_rsslMsg.CheckHasMsgKey() && m_rsslMsg.MsgKey != null && m_rsslMsg.MsgKey.CheckHasAttrib(); }

        /// <summary>
        /// Determines whether current Msg instance has Service name set
        /// </summary>
        public bool HasServiceName { get => ((MsgEncoder)Encoder!).m_serviceNameSet; }

        /// <summary>
        ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
        /// </summary>
        public override int DataType => Access.DataType.DataTypes.MSG;

        /// <summary>
        /// The stream id associated with the message
        /// </summary>
        public int StreamId() 
        {
            return m_rsslMsg.StreamId;
        }

        /// <summary>
        /// The domain type of the message
        /// </summary>
        public int DomainType()
        {
            return m_rsslMsg.DomainType;
        }

        /// <summary>
        /// The name within the message key
        /// </summary>
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
        public ComplexTypeData Attrib() 
        {
            return m_attrib;
        }
        
        /// <summary>
        /// Returns the contained payload Data based on the payload DataType.
        /// Payload contains no data if <see cref="ComplexTypeData.DataType"/> returns
        /// <see cref="DataType.DataTypes.NO_DATA"/>.
        /// </summary>
        /// <returns>reference to Payload object</returns>
        public ComplexTypeData Payload()
        {
            return m_payload;
        }

        /// <summary>
        /// Returns the ServiceName within the MsgKey.
        /// </summary>
        /// <returns>String containing service name</returns>
        public string ServiceName()
        {
            if (!((MsgEncoder)Encoder!).m_serviceNameSet)
            {
                throw new OmmInvalidUsageException("Invalid attempt to call ServiceName() while Service name is not set.");
            }

            return m_serviceName!;
        }

        /// <summary>
        /// Clear current Msg instance.
        /// </summary>
        internal override void ClearInt()
        {
            base.ClearInt();
            
            m_payload.Clear();
            m_attrib.Clear();

            m_extendedHeaderSet = false;
            m_permDataSet = false;
            m_permData.Clear();
            m_extendedHeader.Clear();

            Encoder!.Clear();

            m_rsslMsg.Clear();
            m_rsslMsg.MsgClass = m_msgClass;
        }

        /// <summary>
        /// Performs a deep copy of the current decoded message into the proivded <see cref="Msg"/> instance.
        /// </summary>
        /// <param name="destMsg">the destination <see cref="Msg"/> instance to copy data to</param>
        internal void CopyMsg(Msg destMsg)
        {
            destMsg.m_objectManager = m_objectManager;
            destMsg.m_payload.m_objectManager = m_objectManager;
            destMsg.m_attrib.m_objectManager = m_objectManager;

            destMsg.ClearInt();
            destMsg.m_dataDictionary = m_dataDictionary;

            CopyFromInternalRsslMsg(m_rsslMsg, destMsg);

            // Sets the service name if it is set
            if (((MsgEncoder)Encoder!).m_serviceNameSet && m_serviceName != null)
            {
                destMsg.SetServiceName(m_serviceName);
            }
        }
        
        #region Internal methods

        internal void CopyFromInternalRsslMsg(IMsg sourceRsslMsg, Msg destMsg)
        {
            sourceRsslMsg.Copy(destMsg.m_rsslMsg, CopyMsgFlags.ALL_FLAGS);

            if (sourceRsslMsg.MsgKey != null && m_rsslMsg.MsgKey.CheckHasAttrib())
            {
                m_decodeIterator.Clear();
                if (destMsg.m_decodeIterator.SetBufferAndRWFVersion(destMsg.m_rsslMsg.MsgKey.EncodedAttrib, m_MajorVersion, m_MinorVersion) < CodecReturnCode.SUCCESS)
                {
                    m_errorCode = OmmError.ErrorCodes.ITERATOR_SET_FAILURE;
                    m_attrib.SetError(m_errorCode);
                }
                DecodeComplexType(sourceRsslMsg.MsgKey!.AttribContainerType, destMsg.m_decodeIterator, destMsg.m_rsslMsg.MsgKey.EncodedAttrib, m_dataDictionary!, null, destMsg.m_attrib);
            }

            if (destMsg.m_rsslMsg.EncodedDataBody != null && destMsg.m_rsslMsg.EncodedDataBody.Length > 0)
            {
                m_decodeIterator.Clear();
                if (destMsg.m_decodeIterator.SetBufferAndRWFVersion(destMsg.m_rsslMsg.EncodedDataBody, m_MajorVersion, m_MinorVersion) < CodecReturnCode.SUCCESS)
                {
                    m_errorCode = OmmError.ErrorCodes.ITERATOR_SET_FAILURE;
                    m_payload.SetError(m_errorCode);
                }
                DecodeComplexType(sourceRsslMsg.ContainerType, destMsg.m_decodeIterator, destMsg.m_rsslMsg.EncodedDataBody, m_dataDictionary!, null, destMsg.m_payload);
            }
        }

        internal override CodecReturnCode Decode(DecodeIterator dIter)
        {
            return CodecReturnCode.FAILURE;
        }

        internal CodecReturnCode Decode(IMsg msg, int majorVersion, int minorVersion, DataDictionary? dataDictionary)
        {
            ClearInt();
            m_dataDictionary = dataDictionary;
            m_bodyBuffer = msg.EncodedMsgBuffer;
            if (m_bodyBuffer != null && m_bodyBuffer.Length > 0)
            {
                return Decode(majorVersion, minorVersion, dataDictionary, null);
            }
            else
            {
                CopyFromInternalRsslMsg(msg, this);
                return CodecReturnCode.SUCCESS;
            }       
        }

        internal CodecReturnCode Decode(Buffer body, int majorVersion, int minorVersion, DataDictionary? dataDictionary, object? localDb)
        {
            ClearInt();
            m_dataDictionary = dataDictionary;
            m_bodyBuffer = body;
            return Decode(majorVersion, minorVersion, dataDictionary, localDb);
        }

        internal override CodecReturnCode Decode(int majorVersion, int minorVersion, Buffer body, DataDictionary? dictionary, object? localDb)
        {
            ClearInt();
            m_dataDictionary = dictionary;
            m_bodyBuffer = body;
            return Decode(majorVersion, minorVersion, dictionary, localDb);
        }

        internal CodecReturnCode Decode(int majorVersion, int minorVersion, DataDictionary? dataDictionary, object? localDb)
        {
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
                    ret = DecodeAttribAndPayload(m_decodeIterator, dataDictionary!, null);
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

        internal CodecReturnCode DecodeAttribAndPayload(DecodeIterator decodeIterator, DataDictionary dictionary, object? localDb)
        {
            bool hasAttrib = m_rsslMsg.MsgKey != null && m_rsslMsg.MsgKey.CheckHasAttrib();
            int attribType = hasAttrib ? m_rsslMsg.MsgKey!.AttribContainerType : Eta.Codec.DataTypes.NO_DATA;
            var ret = DecodeComplexType(attribType,
                decodeIterator,
                hasAttrib ? m_rsslMsg.MsgKey!.EncodedAttrib : null,
                dictionary,
                localDb,
                m_attrib);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            return DecodeComplexType(m_rsslMsg.ContainerType,
                decodeIterator,
                m_rsslMsg.EncodedDataBody,
                dictionary,
                localDb,
                m_payload);
        }

        private CodecReturnCode DecodeComplexType(int dataType, 
            DecodeIterator decodeIterator, 
            Buffer? body, 
            DataDictionary dictionary, 
            object? localDb, 
            ComplexTypeData complexType)
        {
            int ommDataType = dataType;
            if (ommDataType == DataTypes.MSG)
            {
                m_helperDecodeIterator.Clear();
                m_helperDecodeIterator.SetBufferAndRWFVersion(m_rsslMsg.EncodedDataBody, decodeIterator.MajorVersion(), decodeIterator.MinorVersion());
                ommDataType = Decoder.GetMsgDataType(m_helperDecodeIterator.ExtractMsgClass());
            }

            return complexType.Decode(body!, ommDataType, decodeIterator.MajorVersion(), decodeIterator.MinorVersion(), dictionary, localDb);
        }

        internal override string ToString(int indent)
        {
            throw new NotImplementedException(); //This will be overrided by the actual message types.
        }

        internal void SetMsgServiceName(string serviceName)
        {
            if (serviceName == null)
                throw new OmmInvalidUsageException("Passed in ServiceName is null.");
            if (((MsgEncoder)Encoder!).m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set ServiceName while Message encoding already started.");

            m_rsslMsg.ApplyHasMsgKey();
            m_serviceName = serviceName;
            ((MsgEncoder)Encoder!).m_serviceNameSet = true;
        }

        internal void SetServiceName(string serviceName)
        {
            m_serviceName = serviceName;
            ((MsgEncoder)Encoder!).m_serviceNameSet = true;
        }

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
            return ((MsgEncoder)Encoder!).EncodeComplete(encodeIterator, out error);
        }

        internal virtual void EncodeComplete() { }

        #endregion
    }
}
