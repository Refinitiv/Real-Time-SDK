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
    internal class StatusMsgEncoder : MsgEncoder
    {
        internal StatusMsgEncoder(StatusMsg statusMsg)
        {
            m_rsslMsg = statusMsg.m_rsslMsg;
            m_encoderOwner = statusMsg;
            EndEncodingEntry = EndEncodingAttributesOrPayload;
        }

        internal void State(int streamState, int dataState, int statusCode, string statusText)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set State when message is already initialized.");

            m_encoded = true;
            m_rsslMsg.ApplyHasState();
            if (CodecReturnCode.SUCCESS != m_rsslMsg.State.StreamState(streamState) ||
                CodecReturnCode.SUCCESS != m_rsslMsg.State.DataState(dataState) ||
                CodecReturnCode.SUCCESS != m_rsslMsg.State.Code(statusCode) ||
                CodecReturnCode.SUCCESS != m_rsslMsg.State.Text().Data(statusText))
            {
                throw new OmmInvalidUsageException($"Attempt to specify invalid state. Passed in value is = {streamState}/{dataState}/{statusCode}/{statusText}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
        }

        internal void ItemGroup(EmaBuffer itemGroup)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set ItemGroup when message is already initialized.");

            m_encoded = true;
            m_rsslMsg.ApplyHasGroupId();
            if (m_groupId != null && m_groupId.Capacity > itemGroup.Length)
            {
                m_groupId.Clear();
            }
            else
            {
                if (m_returnGroupIdBuffer)
                {
                    m_etaPool.ReturnByteBuffer(m_groupId);
                }
                m_groupId = m_etaPool.GetByteBuffer(itemGroup.Length);
                m_returnGroupIdBuffer = true;
                GC.ReRegisterForFinalize(this);
            }

            m_groupId.Put(itemGroup.m_Buffer, 0, itemGroup.Length);
            m_groupId.Flip();

            m_rsslMsg.GroupId.Clear();
            m_rsslMsg.GroupId.Data(m_groupId);
        }

        internal void ClearCache(bool clearCache)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set ClearCache when message is already initialized.");

            m_encoded = true;
            if (clearCache)
            {
                m_rsslMsg.Flags |= StatusMsgFlags.CLEAR_CACHE;
            }
            else
            {
                m_rsslMsg.Flags &= ~StatusMsgFlags.CLEAR_CACHE;
            }
        }

        internal void PrivateStream(bool privateStream)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set PrivateStream when message is already initialized.");

            m_encoded = true;
            if (privateStream)
            {
                m_rsslMsg.Flags |= StatusMsgFlags.PRIVATE_STREAM;
            }
            else
            {
                m_rsslMsg.Flags &= ~StatusMsgFlags.PRIVATE_STREAM;
            }
        }
    }
}
