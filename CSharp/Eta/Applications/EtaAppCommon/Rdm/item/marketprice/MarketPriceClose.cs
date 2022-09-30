/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Rdm;
using Refinitiv.Eta.ValueAdd.Rdm;
using System.Text;

namespace Refinitiv.Eta.Example.Common
{
    public class MarketPriceClose : MsgBase
    {
        ICloseMsg m_CloseMsg = new Msg();
        public override int DomainType { get => m_CloseMsg.DomainType; set { m_CloseMsg.DomainType = value; } }
        public override int StreamId { get => m_CloseMsg.StreamId; set { m_CloseMsg.StreamId = value; } }
        public override int MsgClass { get => m_CloseMsg.MsgClass; }
        public MarketPriceClose()
        {
            Clear();
        }

        public override CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            return m_CloseMsg.Encode(encodeIter);
        }

        public override CodecReturnCode Decode(DecodeIterator dIter, Msg msg)
        {
            Clear();
            if (msg.MsgClass != MsgClasses.CLOSE)
                return CodecReturnCode.FAILURE;

            StreamId = msg.StreamId;

            return CodecReturnCode.SUCCESS;
        }

        public override string ToString()
        {
            StringBuilder toStringBuilder = PrepareStringBuilder();
            toStringBuilder.Insert(0, "MarketPriceClose: \n");

            toStringBuilder.Append(tab);
            toStringBuilder.Append("domain: ");
            toStringBuilder.Append(DomainType);
            toStringBuilder.Append(eol);
            return toStringBuilder.ToString();
        }
  
        public override void Clear()
        {
            m_CloseMsg.Clear();
            m_CloseMsg.DomainType = (int)Rdm.DomainType.MARKET_PRICE;
            m_CloseMsg.MsgClass = MsgClasses.CLOSE;
            m_CloseMsg.ContainerType = DataTypes.NO_DATA;
        }
    }
}
