/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;
using System.Runtime.CompilerServices;

namespace LSEG.Ema.Access
{
    internal class RefreshMsgEncoder : MsgEncoder
    {
        internal RefreshMsgEncoder(RefreshMsg refreshMsg)
        {
            m_rsslMsg = refreshMsg.m_rsslMsg;
            m_encoderOwner = refreshMsg;
            EndEncodingEntry = EndEncodingAttributesOrPayload;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void Complete(bool complete)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Complete when message is already initialized.");

            m_encoded = true;
            if (complete)
            {
                m_rsslMsg.Flags |= RefreshMsgFlags.REFRESH_COMPLETE;
            }
            else
            {
                m_rsslMsg.Flags &= ~RefreshMsgFlags.REFRESH_COMPLETE;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void Solicited(bool solicited)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Solicited when message is already initialized.");

            m_encoded = true;
            if (solicited)
            {
                m_rsslMsg.Flags |= RefreshMsgFlags.SOLICITED;
            }
            else
            {
                m_rsslMsg.Flags &= ~RefreshMsgFlags.SOLICITED;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void DoNotCache(bool doNotCache)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set DoNotCache when message is already initialized.");

            m_encoded = true;
            if (doNotCache)
            {
                m_rsslMsg.Flags |= RefreshMsgFlags.DO_NOT_CACHE;
            }
            else
            {
                m_rsslMsg.Flags &= ~RefreshMsgFlags.DO_NOT_CACHE;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ClearCache(bool clearCache)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set ClearCache when message is already initialized.");

            m_encoded = true;
            if (clearCache)
            {
                m_rsslMsg.Flags |= RefreshMsgFlags.CLEAR_CACHE;
            }
            else
            {
                m_rsslMsg.Flags &= ~RefreshMsgFlags.CLEAR_CACHE;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void PrivateStream(bool privateStream)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set PrivateStream when message is already initialized.");

            m_encoded = true;
            if (privateStream)
            {
                m_rsslMsg.Flags |= RefreshMsgFlags.PRIVATE_STREAM;
            }
            else
            {
                m_rsslMsg.Flags &= ~RefreshMsgFlags.PRIVATE_STREAM;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ItemGroup(EmaBuffer itemGroup)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set ItemGroup when message is already initialized.");

            m_encoded = true;
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

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void Qos(uint timeliness, uint rate)
        {
            if (m_msgEncodeInitialized)
            {
                throw new OmmInvalidUsageException("Invalid attempt to set Qos when message is already initialized.");
            }

            m_encoded = true;
            m_rsslMsg.ApplyHasQos();
            m_rsslMsg.Qos.Clear();
            Utilities.ToRsslQos(timeliness, rate, m_rsslMsg.Qos);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void State(int streamState, int dataState, int statusCode, string statusText)
        {
            if (m_msgEncodeInitialized) 
            {
                throw new OmmInvalidUsageException("Invalid attempt to set State when message is already initialized.");
            }

            m_encoded = true;
            if (CodecReturnCode.SUCCESS != m_rsslMsg.State.StreamState(streamState) ||
                CodecReturnCode.SUCCESS != m_rsslMsg.State.DataState(dataState) ||
                CodecReturnCode.SUCCESS != m_rsslMsg.State.Code(statusCode) ||
                CodecReturnCode.SUCCESS != m_rsslMsg.State.Text().Data(statusText))
            {
                throw new OmmInvalidUsageException($"Attempt to specify invalid state. Passed in value is = {streamState}/{dataState}/{statusCode}/{statusText}", 
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
        }
    }
}
