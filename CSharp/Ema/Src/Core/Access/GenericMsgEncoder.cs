/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;

namespace LSEG.Ema.Access
{
    internal sealed class GenericMsgEncoder : MsgEncoder
    {
        public GenericMsgEncoder(GenericMsg genericMsg)
        {
            m_rsslMsg = genericMsg.m_rsslMsg;
            m_encoderOwner = genericMsg;
            EndEncodingEntry = EndEncodingAttributesOrPayload;
        }

        internal void SecndarySeqNum(long secSeqNum)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set SecondarySeqNum when message is already initialized.");

            m_encoded = true;
            m_rsslMsg.ApplyHasSecondarySeqNum();
            m_rsslMsg.SecondarySeqNum = secSeqNum;
        }

        internal void ProviderDriven(bool providerDriven)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set SeqNum when message is already initialized.");

            m_encoded = true;
            if (providerDriven)
            {
                m_rsslMsg.Flags |= GenericMsgFlags.PROVIDER_DRIVEN;
            } 
            else
            {
                m_rsslMsg.Flags &= ~GenericMsgFlags.PROVIDER_DRIVEN;
            }
        }

        internal void MsgComplete(bool complete)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Complete when message is already initialized.");

            m_encoded = true;
            if (complete)
            {
                m_rsslMsg.Flags |= GenericMsgFlags.MESSAGE_COMPLETE;
            }
            else
            {
                m_rsslMsg.Flags &= ~GenericMsgFlags.MESSAGE_COMPLETE;
            }
        }
    }
}
