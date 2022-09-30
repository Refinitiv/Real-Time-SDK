/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Transports;
using Refinitiv.Eta.Transports.Interfaces;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Refinitiv.Eta.Example.Common
{
    public class UnSupportedMsgHandler
    {
        private const int MSG_SIZE = 1024;

        private ProviderSession m_ProviderSession;
        private IStatusMsg m_StatusMsg = new Msg();
        private EncodeIterator m_EncodeIter = new EncodeIterator();

		public UnSupportedMsgHandler(ProviderSession providerClientSession)
		{
			m_ProviderSession = providerClientSession;
		}

		public CodecReturnCode ProcessRequest(IChannel chnl, Msg msg, out Error? error)
		{
			error = null;
			switch (msg.MsgClass)
			{
				case MsgClasses.REQUEST:
					return SendStatus(chnl, msg, out error);

				case MsgClasses.CLOSE:
					Console.WriteLine($"Received close message with streamId={msg.StreamId} and unsupported Domain '{msg.DomainType}'");
					return CodecReturnCode.SUCCESS;

				default:
					Console.WriteLine($"Received unhandled Msg Class: {MsgClasses.ToString(msg.MsgClass)} with streamId={msg.StreamId} and unsupported Domain '{msg.DomainType}'");
					return CodecReturnCode.SUCCESS;
			}
		}

        private CodecReturnCode EncodeStatus(IChannel chnl, Msg requestMsg, ITransportBuffer msgBuf, out Error? error)
        {
            error = null;
            m_StatusMsg.Clear();

            /* set-up message */
            m_StatusMsg.MsgClass = MsgClasses.STATUS;
            m_StatusMsg.StreamId = requestMsg.StreamId;
            m_StatusMsg.DomainType = requestMsg.DomainType;
            m_StatusMsg.ContainerType = DataTypes.NO_DATA;
            m_StatusMsg.Flags = StatusMsgFlags.HAS_STATE;
            m_StatusMsg.State.StreamState(StreamStates.CLOSED);
            m_StatusMsg.State.DataState(DataStates.SUSPECT);
            m_StatusMsg.State.Code(StateCodes.USAGE_ERROR);
            m_StatusMsg.State.Text().Data($"Request rejected for stream id '{requestMsg.StreamId}' - domain type '{requestMsg.DomainType}' is not supported");

            /* clear encode iterator */
            m_EncodeIter.Clear();

            /* encode message */

            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }

            ret = m_StatusMsg.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"StatusMsg.Encode() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }

            return CodecReturnCode.SUCCESS;
        }
        public CodecReturnCode SendStatus(IChannel channel, Msg requestMsg, out Error? error)
        {
            /* get a buffer for the not supported status */
            ITransportBuffer msgBuf = channel.GetBuffer(MSG_SIZE, false, out error);
            if (msgBuf == null)
            {
                return CodecReturnCode.FAILURE;
            }

            /* encode not supported status */
            CodecReturnCode ret = EncodeStatus(channel, requestMsg, msgBuf, out error);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            Console.WriteLine($"Rejecting Item Request with streamId={requestMsg.StreamId} Reason: Domain '{requestMsg.DomainType}' Not Supported");

            /* send not supported status */
            return (CodecReturnCode)m_ProviderSession.Write(channel, msgBuf, out error);
        }
    }
}
