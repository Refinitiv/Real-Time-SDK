/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Ema.Access
{
    internal class UpdateMsgEncoder : MsgEncoder
    {
        internal UpdateMsgEncoder(UpdateMsg updateMsg)
        {
            m_rsslMsg = updateMsg.m_rsslMsg;
            m_encoderOwner = updateMsg;
            EndEncodingEntry = EndEncodingAttributesOrPayload;
        }

        internal void UpdateTypeNum(int updateTypeNum)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set UpdateTypeNum value when message is already initialized.");
            if (updateTypeNum < 0 || updateTypeNum > 255)
                throw new OmmInvalidUsageException("Passed in updateTypeNum is out of range.  [0 - 255]");

            m_encoded = true;
            m_rsslMsg.UpdateType = updateTypeNum;
        }

        internal void DoNotCache(bool doNotCache)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set DoNotCache when message is already initialized.");

            m_encoded = true;
            if (doNotCache)
            {
                m_rsslMsg.Flags |= UpdateMsgFlags.DO_NOT_CACHE;
            }
            else
            {
                m_rsslMsg.Flags &= ~UpdateMsgFlags.DO_NOT_CACHE;
            }
        }

        internal void DoNotConflate(bool doNotConflate)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set DoNotConflate when message is already initialized.");

            m_encoded = true;
            if (doNotConflate)
            {
                m_rsslMsg.Flags |= UpdateMsgFlags.DO_NOT_CONFLATE;
            }
            else
            {
                m_rsslMsg.Flags &= ~UpdateMsgFlags.DO_NOT_CONFLATE;
            }
        }

        internal void DoNotRipple(bool doNotRipple)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set DoNotRipple when message is already initialized.");

            m_encoded = true;
            if (doNotRipple)
            {
                m_rsslMsg.Flags |= UpdateMsgFlags.DO_NOT_RIPPLE;
            }
            else
            {
                m_rsslMsg.Flags &= ~UpdateMsgFlags.DO_NOT_RIPPLE;
            }
        }

        internal void Conflated(int count, int time)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set ConflatedS when message is already initialized.");
            if (count < 0 || count > 32767 || time < 0 || time > 65535)
                throw new OmmInvalidUsageException("Passed in count or time is out of range. [0 - 32767] [0 - 65535]");

            m_encoded = true;
            m_rsslMsg.ApplyHasConfInfo();
            m_rsslMsg.ConflationCount = count;
            m_rsslMsg.ConflationTime = time;
        }
    }
}
