/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Ema.Access
{
    internal sealed class AckMsgEncoder : MsgEncoder
    {        
        public AckMsgEncoder(AckMsg ackMsg)
        {
            m_rsslMsg = ackMsg.m_rsslMsg;
            m_encoderOwner = ackMsg;
            EndEncodingEntry = EndEncodingAttributesOrPayload;
        }

        internal void AckId(long ackId)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set ackId when message is already initialized.");

            m_encoded = true;
            m_rsslMsg.AckId = ackId;
        }

        internal void NackCode(int nackCode)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set NackCode when message is already initialized.");

            m_encoded = true;
            m_rsslMsg.ApplyHasNakCode();
            m_rsslMsg.NakCode = nackCode;
        }

        internal void Text(string text)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Text when message is already initialized.");

            m_encoded = true;
            m_rsslMsg.ApplyHasText();
            m_rsslMsg.Text.Data(text);
        }

        internal void PrivateStream(bool privateStream)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set PrivateStream when message is already initialized.");

            m_encoded = true;
            if (privateStream)
                m_rsslMsg.Flags |= AckMsgFlags.PRIVATE_STREAM;
            else
                m_rsslMsg.Flags &= ~AckMsgFlags.PRIVATE_STREAM;
        }

    }
}
