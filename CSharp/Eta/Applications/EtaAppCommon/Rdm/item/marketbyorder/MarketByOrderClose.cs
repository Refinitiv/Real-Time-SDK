/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;
using System.Text;

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// The market by order close message. Used by an OMM Consumer, 
    /// OMM Non-Interactive Provider and OMM Interactive provider 
    /// to encode/decode a market by order close message.
    /// </summary>
    public class MarketByOrderClose : MsgBase
    {
        private ICloseMsg m_CloseMsg = new Msg();

        public override int StreamId { get => m_CloseMsg.StreamId; set { m_CloseMsg.StreamId = value; } }
        public override int DomainType { get => m_CloseMsg.DomainType; }
        public override int MsgClass { get => m_CloseMsg.MsgClass; }
        public override CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            return m_CloseMsg.Encode(encodeIter);
        }

        public override CodecReturnCode Decode(DecodeIterator dIter, Msg msg)
        {
            Clear();
            if (msg.MsgClass != MsgClasses.CLOSE)
                return CodecReturnCode.FAILURE;

            m_CloseMsg.StreamId = msg.StreamId;

            return CodecReturnCode.SUCCESS;
        }

        public override string ToString()
        {
            StringBuilder toStringBuilder = PrepareStringBuilder();
            toStringBuilder.Insert(0, "MarketByOrderClose: \n");

            toStringBuilder.Append(tab);
            toStringBuilder.Append("domain: ");
            toStringBuilder.Append(DomainTypes.ToString((int)Rdm.DomainType.MARKET_BY_ORDER));
            toStringBuilder.Append(eol);
            return toStringBuilder.ToString();
        }

        public override void Clear()
        {
            m_CloseMsg.Clear();
            m_CloseMsg.MsgClass = MsgClasses.CLOSE;
            m_CloseMsg.DomainType = (int)Rdm.DomainType.MARKET_BY_ORDER;
            m_CloseMsg.ContainerType = DataTypes.NO_DATA;
        }
    }
}
