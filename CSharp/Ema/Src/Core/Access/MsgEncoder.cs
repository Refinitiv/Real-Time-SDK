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
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal class MsgEncoder : Encoder
    {
        private const int MSG_FLAGS_POS = 8;

#pragma warning disable CS8618
        internal Eta.Codec.Msg m_rsslMsg;
#pragma warning restore CS8618
        internal bool m_containerTypeSet = false;

        internal bool m_msgEncodeInitialized = false;
        internal bool m_msgKeyEncodeStarted = false;
        internal bool m_msgKeyEncodeCompleted = false;
        internal bool m_msgEncodeCompleted = false;
        internal bool m_msgPayloadEncodeStarted = false;

        internal bool m_copyByteBuffer = true;
        internal bool m_returnExtendedHeader = false;
        internal bool m_returnPermData = false;
        internal bool m_returnEncodedBody = false;
        internal bool m_returnMsgKeyAttrib = false;
        internal bool m_returnGroupIdBuffer = false;

        internal ByteBuffer? m_encodedBody;
        internal ByteBuffer? m_encodedAttrib;
        internal ByteBuffer? m_permData;
        internal ByteBuffer? m_extendedHeader;
        internal ByteBuffer? m_groupId;

        internal bool m_serviceNameSet = false;

        internal bool m_preencoded = false;
        internal int m_msgStartPosition = 0;
        internal bool m_encoded = false;

        internal void DomainType(int domainType)
        {
            if (m_msgEncodeInitialized)
            {
                throw new OmmInvalidUsageException("Invalid attempt to set DomainType type when message encoding already initialized.");
            }
            if (domainType < 0 || domainType > 255)
            {
                throw new OmmInvalidUsageException("Passed in domainType is out of range. [0 - 255]");
            }

            m_encoded = true;
            m_rsslMsg.DomainType = domainType;
        }

        internal void StreamId(int streamId)
        {
            if (m_msgEncodeInitialized)
            {
                throw new OmmInvalidUsageException("Invalid attempt to set StreamId type when message encoding already initialized.");
            }

            m_encoded = true;
            m_rsslMsg.StreamId = streamId;
        }

        internal void ServiceId(int serviceId)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set ServiceId when message encoding already initialized.");
            if (serviceId < 0 || serviceId > 65535)
                throw new OmmInvalidUsageException($"Passed in ServiceId {serviceId} is out of range [0 - 65535].");

            m_encoded = true;
            m_rsslMsg.ApplyHasMsgKey();
            m_rsslMsg.MsgKey.ApplyHasServiceId();
            m_rsslMsg.MsgKey.ServiceId = serviceId;
        }

        internal void Name(string name)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Name when message is already initialized.");
            if (name == null)
                throw new OmmInvalidUsageException("Passed in name is null");

            m_encoded = true;
            m_rsslMsg.ApplyHasMsgKey();
            m_rsslMsg.MsgKey.ApplyHasName();
            m_rsslMsg.MsgKey.Name.Data(name);
        }

        internal void NameType(int nameType)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set NameType when message is already initialized."); 
            if (nameType < 0 || nameType > 255)
                throw new OmmInvalidUsageException("Passed in nameType is out of range. [0 - 255]");

            m_encoded = true;
            m_rsslMsg.ApplyHasMsgKey();
            m_rsslMsg.MsgKey.ApplyHasNameType();
            m_rsslMsg.MsgKey.NameType = nameType;
        }

        internal void Filter(long filter)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Filter when message is already initialized.");
            if (filter < 0 || filter > 4294967295L)
                throw new OmmInvalidUsageException("Passed in filter is out of range. [0 - 4294967295]");

            m_encoded = true;
            m_rsslMsg.ApplyHasMsgKey();
            m_rsslMsg.MsgKey.ApplyHasFilter();
            m_rsslMsg.MsgKey.Filter = filter;
        }

        internal void Identifier(int id)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Identifier when message is already initialized.");

            m_encoded = true;
            m_rsslMsg.ApplyHasMsgKey();
            m_rsslMsg.MsgKey.ApplyHasIdentifier();
            m_rsslMsg.MsgKey.Identifier = id;
        }

        internal void PermissionData(EmaBuffer permissionData)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Identifier when message is already initialized.");

            m_encoded = true;
            m_rsslMsg.ApplyHasPermData();
            if (m_permData != null && m_permData.Capacity > permissionData.Length)
            {
                m_permData.Clear();
            }
            else
            {
                if (m_returnPermData)
                {
                    m_etaPool.ReturnByteBuffer(m_permData);
                }
                m_permData = m_etaPool.GetByteBuffer(permissionData.Length);
                m_returnPermData = true;
                GC.ReRegisterForFinalize(this);
            }

            m_permData.Put(permissionData.m_Buffer, 0, permissionData.Length);
            m_permData.Flip();

            m_rsslMsg.PermData.Clear();
            m_rsslMsg.PermData.Data(m_permData);
        }

        internal void ExtendedHeader(EmaBuffer extendedHeader)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Identifier when message is already initialized.");

            m_encoded = true;
            m_rsslMsg.ApplyHasExtendedHdr();

            if (m_extendedHeader != null && m_extendedHeader.Capacity > extendedHeader.Length)
            {
                m_extendedHeader.Clear();
            }
            else
            {
                if (m_returnExtendedHeader)
                {
                    m_etaPool.ReturnByteBuffer(m_extendedHeader);
                }
                m_extendedHeader = m_etaPool.GetByteBuffer(extendedHeader.Length);
                m_returnExtendedHeader = true;
                GC.ReRegisterForFinalize(this);
            }

            m_extendedHeader.Put(extendedHeader.m_Buffer, 0, extendedHeader.Length);
            m_extendedHeader.Flip();

            m_rsslMsg.ExtendedHeader.Clear();
            m_rsslMsg.ExtendedHeader.Data(m_extendedHeader);
        }

        internal void SeqNum(long seqNum)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set SeqNum when message is already initialized.");
            if (seqNum < 0 || seqNum > 4294967295L)
                throw new OmmInvalidUsageException("Passed in seqNum is out of range. [0 - 4294967295]");

            m_encoded = true;
            m_rsslMsg.ApplyHasSeqNum();
            m_rsslMsg.SeqNum = seqNum;
        }

        internal void PartNum(int partNum)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Payload type when message encoding already initialized.");
            if (partNum < 0 || partNum > 32767)
                throw new OmmInvalidUsageException("Passed in partNum is out of range. [0 - 32767]");

            m_encoded = true;
            m_rsslMsg.ApplyHasPartNum();
            m_rsslMsg.PartNum = partNum;
        }

        internal void PublisherId(long userId, long userAddress)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Payload type when message encoding already initialized.");
            if (userId < 0 || userId > 4294967295L || userAddress < 0 || userAddress > 4294967295L)
                throw new OmmInvalidUsageException("Passed in userId or userAddress is out of range. [0 - 4294967295]");

            m_encoded = true;
            m_rsslMsg.ApplyHasPostUserInfo();
            m_rsslMsg.PostUserInfo.UserId = userId;
            m_rsslMsg.PostUserInfo.UserAddr = userAddress;
        }

        internal void SetPreEncodedPayload(Encoder sourceEncoder)
        {
            var sourceByteBuffer = sourceEncoder.m_iteratorByteBuffer!;
            if (m_copyByteBuffer)
            {
                if (m_encodedBody != null && m_encodedBody.Capacity > sourceEncoder.m_encodeIterator!.Buffer().Length)
                {
                    m_encodedBody!.Clear();
                }
                else
                {
                    if (m_returnEncodedBody)
                    {
                        m_etaPool.ReturnByteBuffer(m_encodedBody);
                    }
                    m_encodedBody = m_etaPool.GetByteBuffer(sourceEncoder.m_encodeIterator!.Buffer().Length);
                    m_returnEncodedBody = true;
                }

                m_encodedBody.Put(sourceByteBuffer);

                m_rsslMsg.EncodedDataBody.Clear();
                m_rsslMsg.EncodedDataBody.Data(m_encodedBody.Flip());
            }
            else
            {
                if (m_returnEncodedBody)
                {
                    m_etaPool.ReturnByteBuffer(m_encodedBody);
                    m_returnEncodedBody = false;
                }
                CaptureEncodedBodyByteBuffer(sourceEncoder);
                m_rsslMsg.EncodedDataBody.Data(m_encodedBody!.Flip());
            }
        }

        internal void SetPreEncodedAttributes(Encoder sourceEncoder)
        {
            var sourceByteBuffer = sourceEncoder.m_iteratorByteBuffer!;
            if (m_copyByteBuffer)
            {
                if (m_encodedAttrib != null && m_encodedAttrib.Capacity > sourceEncoder.m_encodeIterator!.Buffer().Length)
                {
                    m_encodedAttrib!.Clear();
                }
                else
                {
                    if (m_returnMsgKeyAttrib)
                    {
                        m_etaPool.ReturnByteBuffer(m_encodedAttrib);
                    }
                    m_encodedAttrib = m_etaPool.GetByteBuffer(sourceEncoder.m_encodeIterator!.Buffer().Length);
                    m_returnMsgKeyAttrib = true;
                }

                m_encodedAttrib.Put(sourceByteBuffer);

                m_rsslMsg.MsgKey.EncodedAttrib.Clear();
                m_rsslMsg.MsgKey.EncodedAttrib.Data(m_encodedAttrib.Flip());
            }
            else
            {
                if (m_returnMsgKeyAttrib)
                {
                    m_etaPool.ReturnByteBuffer(m_encodedAttrib);
                    m_returnMsgKeyAttrib = false;
                }
                CaptureEncodedAttributesByteBuffer(sourceEncoder);
                m_rsslMsg.MsgKey.EncodedAttrib.Data(m_encodedAttrib!.Flip());
            }
        }

        internal void CaptureEncodedBodyByteBuffer(Encoder sourceEncoder)
        {
            m_encodedBody = sourceEncoder.m_iteratorByteBuffer;
            sourceEncoder.m_releaseIteratorByteBuffer = false;
            m_returnEncodedBody = true;
            GC.ReRegisterForFinalize(this);
        }

        internal void CaptureEncodedAttributesByteBuffer(Encoder sourceEncoder)
        {
            m_encodedAttrib = sourceEncoder.m_iteratorByteBuffer;
            sourceEncoder.m_releaseIteratorByteBuffer = false;
            m_returnMsgKeyAttrib = true;
            GC.ReRegisterForFinalize(this);
        }

        internal void Payload(ComplexType payload)
        {
            int dataType = ConvertDataTypeToEta(payload.m_dataType);
            m_rsslMsg.ContainerType = dataType;

            if (dataType != DataTypes.MSG)
            {
                if (payload.Encoder != null && payload.Encoder.OwnsIterator())
                {
                    if (m_msgEncodeInitialized) // post-encoded attributes include this case as well
                        throw new OmmInvalidUsageException("Invalid attempt to add preencoded Payload when message encoding already initialized.");

                    SetPreEncodedPayload(payload.Encoder);
                    m_preencoded = true;
                }
                else if (payload.m_hasDecodedDataSet)
                {
                    if (m_msgEncodeInitialized) // post-encoded attributes include this case as well
                        throw new OmmInvalidUsageException("Invalid attempt to add decoded Payload when message encoding already initialized.");

                    m_rsslMsg.EncodedDataBody = payload.m_bodyBuffer;
                    m_preencoded = true;
                }
                else
                {
                    if (m_preencoded)
                        throw new OmmInvalidUsageException("Setting payload that is not preencoded is not allowed when preencoded Attributes are already set.");
                    if (m_serviceNameSet)
                        throw new OmmInvalidUsageException("Setting payload that is not preencoded is not allowed when ServiceName is already set.");
                    if (m_msgKeyEncodeStarted && !m_msgKeyEncodeCompleted)
                        throw new OmmInvalidUsageException("Invalid attempt to start adding Payload when MsgKey encoding has not been finished.");            

                    InitPayloadEncoding();
                    PassEncIterator(payload.Encoder);
                }
            }
            else
            {
                AddMessagePayload((Msg)payload);
            } 

            m_encoded = true;
        }

        internal void Attrib(ComplexType attrib)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Attributes when message encoding is already initialized.");
            if (m_msgEncodeCompleted)
                throw new OmmInvalidUsageException("Invalid attempt to add Attributes when Msg encoding has been completed.");

            m_rsslMsg.ApplyHasMsgKey();
            m_rsslMsg.MsgKey.ApplyHasAttrib();
            m_rsslMsg.MsgKey.AttribContainerType = ConvertDataTypeToEta(attrib.m_dataType);

            if (m_rsslMsg.MsgKey.AttribContainerType != DataTypes.MSG)
            {
                if (attrib.Encoder != null && attrib.Encoder.OwnsIterator())
                {
                    if (m_msgEncodeInitialized) // post-encoded payload is included in this case as well
                        throw new OmmInvalidUsageException("Invalid attempt to add preencoded Attributes when message encoding already initialized.");
                    
                    m_preencoded = true;
                    SetPreEncodedAttributes(attrib.Encoder);
                }
                else if (attrib.m_hasDecodedDataSet)
                {
                    if (m_msgEncodeInitialized) // post-encoded payload is included in this case as well
                        throw new OmmInvalidUsageException("Invalid attempt to add preencoded Attributes when message encoding already initialized.");

                    m_preencoded = true;
                    m_rsslMsg.MsgKey.EncodedAttrib = attrib.m_bodyBuffer;
                }
                else
                {
                    if (m_preencoded)
                        throw new OmmInvalidUsageException("Setting Attributes that are not preencoded is not allowed when preencoded Payload is already set.");
                    if (m_serviceNameSet)
                        throw new OmmInvalidUsageException("Setting Attributes that are not preencoded is not allowed when ServiceName is already set.");
                    if (m_msgPayloadEncodeStarted && !m_msgEncodeCompleted)
                        throw new OmmInvalidUsageException("Invalid attempt to start adding Attributes when Msg Payload encoding has not been finished.");

                    InitAttribEncoding();
                    PassEncIterator(attrib.Encoder);
                }
            }
            else
            {
                AddMessageAttributes((Msg)attrib);
            }

            m_encoded = true;
        }

        private void InitPayloadEncoding()
        {
            AcquireEncodeIterator();

            if (!m_msgEncodeInitialized)
            {
                m_rsslMsg.EncodeInit(m_encodeIterator, 0);
            }
            else
            {
                int pos = MSG_FLAGS_POS;
                var buffer = m_encodeIterator!.Buffer().Data();
                if ((buffer.ReadByteAt(m_msgStartPosition + pos) & 0x80) != 0) pos += 2; else pos += 1;
                buffer.WriteAt(m_msgStartPosition + pos, (byte)(m_rsslMsg.ContainerType - DataTypes.CONTAINER_TYPE_MIN));
            }

            m_msgEncodeInitialized = true;
            m_msgPayloadEncodeStarted = true;
        }

        private void InitAttribEncoding()
        {
            AcquireEncodeIterator();

            m_msgEncodeInitialized = true;
            m_msgKeyEncodeStarted = true;
            m_msgStartPosition = m_encodeIterator!.Buffer().Data().WritePosition;

            m_rsslMsg.EncodeInit(m_encodeIterator, 0);
        }

        private void AddMessagePayload(Msg msg)
        {
            MsgEncoder encoder = (MsgEncoder)msg.Encoder!;
            if (encoder.m_encoded)
            {
                if (encoder.m_encodeIterator != null || encoder.m_preencoded || !encoder.m_msgEncodeInitialized || m_serviceNameSet )
                {
                    if (m_msgEncodeInitialized)
                        throw new OmmInvalidUsageException("Invalid attempt to add preencoded Payload when message encoding already initialized.");

                    encoder.EncodeComplete();
                    SetPreEncodedPayload(encoder);
                    m_preencoded = true;
                }
                else
                    throw new OmmInvalidUsageException("Invalid attempt to message payload in unsupported state.");
            }
            else if (msg.m_hasDecodedDataSet)
            {
                if (m_msgEncodeInitialized)
                    throw new OmmInvalidUsageException("Invalid attempt to add preencoded Payload when message encoding already initialized.");

                m_rsslMsg.EncodedDataBody = msg.m_bodyBuffer;
                m_preencoded = true;
            }
            else
            {
                throw new OmmInvalidUsageException("Adding empty message as container payload not supported.");
            }
        }

        private void AddMessageAttributes(Msg msg)
        {
            MsgEncoder encoder = (MsgEncoder)msg.Encoder!;
            if (encoder.m_encoded)
            {
                if (encoder.m_encodeIterator != null || m_preencoded || m_serviceNameSet)
                {
                    if (m_msgEncodeInitialized)
                        throw new OmmInvalidUsageException("Invalid attempt to add preencoded Attributes when message encoding already initialized.");

                    encoder.EncodeComplete();
                    SetPreEncodedAttributes(encoder); 
                    m_preencoded = true;
                }
                else
                {
                    InitAttribEncoding();
                    if (!encoder.EncodeComplete(m_encodeIterator!, out var error))
                        throw new OmmInvalidUsageException($"{error!}. Adding message that is not pre-encoded as Attributes is not supported.");
                    EndEncodingAttributes();
                }
            }
            else if (msg.m_hasDecodedDataSet)
            {
                if (m_msgEncodeInitialized)
                    throw new OmmInvalidUsageException("Invalid attempt to add preencoded Attributes when message encoding already initialized.");

                m_rsslMsg.MsgKey.EncodedAttrib = msg.m_bodyBuffer;
                m_preencoded = true;
            }
            else
            {
                throw new OmmInvalidUsageException("Adding empty message as message attributes not supported.");
            }
        }    

        /// <summary>
        /// This method is called by the Attributes / Payload container when its encoding is finished.
        /// </summary>
        /// <exception cref="OmmInvalidUsageException"> is thrown in case of MsgEncoder invalid state or encoding errors.</exception>
        internal void EndEncodingAttributesOrPayload()
        {
            if (!m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to finish encoding attributes or payload containers while encoding has not started.");

            if (m_msgKeyEncodeStarted && !m_msgKeyEncodeCompleted)
            {
                EndEncodingAttributes();
                return;
            }

            if (m_msgPayloadEncodeStarted && !m_msgEncodeCompleted)
            {
                EndEncodingPayload();
                return;
            }
        }

        private void EndEncodingPayload()
        {
            var ret = m_rsslMsg.EncodeComplete(m_encodeIterator, true);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                m_rsslMsg.EncodeComplete(m_encodeIterator, false);
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslMsg.EncodeComplete(m_encodeIterator, true);
            }

            if (ret < CodecReturnCode.SUCCESS)
                throw new OmmInvalidUsageException($"Failed to finish encoding Message, return code: {ret.GetAsString()}");

            m_msgEncodeCompleted = true;
            m_containerComplete = true;
        }

        private void EndEncodingAttributes()
        {
            var ret = m_rsslMsg.EncodeKeyAttribComplete(m_encodeIterator, true);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                m_rsslMsg.EncodeKeyAttribComplete(m_encodeIterator, false);
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslMsg.EncodeKeyAttribComplete(m_encodeIterator, true);
            }

            if (ret < CodecReturnCode.SUCCESS)
                throw new OmmInvalidUsageException($"Failed to finish encoding Attributes, return code: {ret.GetAsString()}");

            m_msgKeyEncodeCompleted = true;
        }

        /// <summary>
        /// Complete encoding into internal buffer that this MsgEncoder owns and can later be fetched
        /// </summary>
        /// <exception cref="OmmInvalidUsageException"> is thrown in case any encoding errors are encountered.</exception>
        internal void EncodeComplete()
        {
            if (m_containerComplete) // message payload was completed
                return;

            if (!m_msgEncodeInitialized) // message has no payload and attributes or they are pre-encoded
            {
                AcquireEncodeIterator();
                var retCode = m_rsslMsg.Encode(m_encodeIterator);
                if (retCode < CodecReturnCode.SUCCESS)
                {
                    throw new OmmInvalidUsageException($"Failed to encode Msg, return code: {retCode.GetAsString()}");
                }
            }
            else if (m_msgKeyEncodeCompleted) // the container is not complete so it has no payload, otherwise we'd have already returned
            {
                var ret = m_rsslMsg.EncodeComplete(m_encodeIterator, true);
                while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
                {
                    m_rsslMsg.EncodeComplete(m_encodeIterator, false);
                    ReallocateEncodeIteratorBuffer();
                    ret = m_rsslMsg.EncodeComplete(m_encodeIterator, true);
                }

                if (ret < CodecReturnCode.SUCCESS)
                    throw new OmmInvalidUsageException($"Failed to finish encoding Message, return code: {ret.GetAsString()}");

                m_containerComplete = true;
            }
            else
                throw new OmmInvalidUsageException("Cannot add the provided message: its Attributes encoding is not complete.");

            m_containerComplete = true; 
        }

        /// <summary>
        /// Either provides a <see cref="Buffer"/> with encoded message data 
        /// or encodes the message into the external EncodeIterator if suitable.
        /// </summary>
        /// <param name="externalIterator">the <see cref="EncodeIterator"/> that is currently handled by the calling container</param>
        /// <param name="dropBufferOwnership">true if the current Msg should not release the internal ByteBuffer with encoded msg data 
        /// (this will be done by the calling container), false otherwise.</param>
        /// <returns><see cref="Buffer"/> with pre-encoded message in case it was encoded into its internal buffer 
        /// or null in case it was encoded into the external EncodeIterator provided by the calling container.</returns>
        /// <exception cref="OmmInvalidUsageException"> thrown in case of invalid MsgEncoder state or encoding errors.</exception>
        internal Buffer? CompleteEncode(EncodeIterator externalIterator, bool dropBufferOwnership)
        {
            if (m_encodeIterator != null) // message encoding has already started into its internal buffer so complete the encoding if needed and get it
            {
                EncodeComplete();
                return GetEncodedBuffer(dropBufferOwnership);
            }
            else // either message has no payload or attributes or they are set as pre-encoded
            {
                if (!EncodeComplete(externalIterator, out var error))
                    throw new OmmInvalidUsageException($"Failed to complete internal message encoding: {error}. Adding message that is not pre-encoded to container is not supported.");
                return null;
            }
        }

        internal bool EncodeComplete(EncodeIterator encodeIterator, out string? error)
        {
            error = null;
            if (m_preencoded || !m_msgEncodeInitialized)
            {
                var retCode = m_rsslMsg.Encode(encodeIterator);
                if (retCode < CodecReturnCode.SUCCESS)
                {
                   error = $"Failed to encode Msg, return code: {retCode.GetAsString()}";
                   return false;
                }

                m_containerComplete = true;
                return true;
            }
            else
            {
                error = "Encoding using external EncodeIterator is possible only for empty or preencoded Attributes and Payload.";
                return false;
            }
        }

        public override void Clear()
        {
            base.Clear();
            
            m_msgEncodeInitialized = false;
            m_msgKeyEncodeStarted = false;
            m_msgKeyEncodeCompleted = false;
            m_msgEncodeCompleted = false;
            m_msgPayloadEncodeStarted = false;
            m_containerTypeSet = false;
            m_preencoded = false;

            if (m_returnEncodedBody) m_etaPool.ReturnByteBuffer(m_encodedBody);
            if (m_returnMsgKeyAttrib) m_etaPool.ReturnByteBuffer(m_encodedAttrib);
            if (m_returnExtendedHeader) m_etaPool.ReturnByteBuffer(m_extendedHeader);
            if (m_returnPermData) m_etaPool.ReturnByteBuffer(m_permData);
            if (m_returnGroupIdBuffer) m_etaPool.ReturnByteBuffer(m_groupId);

            m_encodedBody = null;
            m_encodedAttrib = null;
            m_extendedHeader = null;
            m_permData = null;
            m_groupId = null;

            m_returnEncodedBody = false;
            m_returnMsgKeyAttrib = false;
            m_returnExtendedHeader = false;
            m_returnPermData = false;
            m_returnGroupIdBuffer = false;

            m_serviceNameSet = false;
            m_msgStartPosition = 0;
            m_encoded = false;
        }

        ~MsgEncoder()
        {
            if (m_returnEncodedBody) m_etaPool.ReturnByteBuffer(m_encodedBody);
            if (m_returnMsgKeyAttrib) m_etaPool.ReturnByteBuffer(m_encodedAttrib);
            if (m_returnExtendedHeader) m_etaPool.ReturnByteBuffer(m_extendedHeader);
            if (m_returnPermData) m_etaPool.ReturnByteBuffer(m_permData);
            if (m_returnGroupIdBuffer) m_etaPool.ReturnByteBuffer(m_groupId);
        }
    }
}
