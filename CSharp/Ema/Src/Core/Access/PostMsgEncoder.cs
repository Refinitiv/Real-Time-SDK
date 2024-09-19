/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;

namespace LSEG.Ema.Access
{
    internal class PostMsgEncoder : MsgEncoder
    {
        internal PostMsgEncoder(PostMsg postMsg) 
        {
            m_rsslMsg = postMsg.m_rsslMsg;
            m_encoderOwner = postMsg;
            EndEncodingEntry = EndEncodingAttributesOrPayload;
        }

        internal void SolicitAck(bool ack)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set SolicitAck when message is already initialized.");

            m_encoded = true;
            if (ack)
            {
                m_rsslMsg.Flags |= PostMsgFlags.ACK;
            }
            else
            {
                m_rsslMsg.Flags &= ~PostMsgFlags.ACK;
            }
        }

        internal void PostUserRights(int postUserRights)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set PostUserRights when message is already initialized.");
            if (postUserRights < 0 || postUserRights > 32767)
                throw new OmmInvalidUsageException("Passed in postUserRights is out of range. [0 - 32767]");

            m_encoded = true;
            m_rsslMsg.ApplyHasPostUserRights();
            m_rsslMsg.PostUserRights = postUserRights;
        }

        internal void PostId(long postId)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set PostId when message is already initialized.");
            if (postId < 0 || postId > 4294967295L)
                throw new OmmInvalidUsageException("Passed in postId is out of range. [0 - 4294967295]");

            m_encoded = true;
            m_rsslMsg.ApplyHasPostId();
            m_rsslMsg.PostId = postId;
        }

        internal void Complete(bool complete)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Complete when message is already initialized.");

            m_encoded = true;
            if (complete)
            {
                m_rsslMsg.Flags |= PostMsgFlags.POST_COMPLETE;
            }
            else
            {
                m_rsslMsg.Flags &= ~PostMsgFlags.POST_COMPLETE;
            }
        }
    }
}
