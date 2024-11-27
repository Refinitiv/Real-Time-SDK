/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;
using System.Text;

namespace LSEG.Eta.Example.Common
{
    public class MarketPriceStatus : MsgBase
    {
        private IStatusMsg m_StatusMsg = new Msg();

        public override int StreamId { get => m_StatusMsg.StreamId; set { m_StatusMsg.StreamId = value; } }
        public override int MsgClass { get => m_StatusMsg.MsgClass; }
        public override int DomainType
        {
            get => m_StatusMsg.DomainType; set => m_StatusMsg.DomainType = value;
        }
        public MarketPriceStatusFlags Flags { get; set; }

        public bool PrivateStream
        {
            get => (Flags & MarketPriceStatusFlags.PRIVATE_STREAM) != 0;
            set
            {
                if (value) { Flags |= MarketPriceStatusFlags.PRIVATE_STREAM; }
                else Flags &= ~MarketPriceStatusFlags.PRIVATE_STREAM;
            }
        }
        public bool HasState
        {
            get => (Flags & MarketPriceStatusFlags.HAS_STATE) != 0;
            set
            {
                if (value) { Flags |= MarketPriceStatusFlags.HAS_STATE; }
                else Flags &= ~MarketPriceStatusFlags.HAS_STATE;
            }
        }
        public State State { get; set; }

        public MarketPriceStatus()
        {
            State = new State();
            StreamId = 1;
            Clear();
        }

        public CodecReturnCode Copy(MarketPriceStatus destStatusMsg)
        {
            destStatusMsg.StreamId = StreamId;
            if (HasState)
            {
                destStatusMsg.HasState = true;
                State.Copy(destStatusMsg.State);
            }
            return CodecReturnCode.SUCCESS;
        }

        public override void Clear()
        {
            m_StatusMsg.Clear();
            m_StatusMsg.ContainerType = DataTypes.NO_DATA;
            m_StatusMsg.MsgClass = MsgClasses.STATUS;
            m_StatusMsg.DomainType = (int)Rdm.DomainType.MARKET_PRICE;
            Flags = 0;
            State.Clear();
            StreamId = 1;
        }

        public override CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            if (PrivateStream)
                m_StatusMsg.ApplyPrivateStream();

            if (HasState)
            {
                m_StatusMsg.ApplyHasState();
                State.Copy(m_StatusMsg.State);
            }

            CodecReturnCode ret = m_StatusMsg.Encode(encodeIter);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            return CodecReturnCode.SUCCESS;
        }

        public override  CodecReturnCode Decode(DecodeIterator dIter, Msg msg)
        {
            Clear();
            if (msg.MsgClass != MsgClasses.STATUS)
                return CodecReturnCode.FAILURE;


            IStatusMsg statusMsg = msg;
            StreamId = msg.StreamId;

            if (msg.CheckPrivateStream())
            {
               PrivateStream = true;
            }

            if (statusMsg.CheckHasState())
            {
                HasState = true;
                statusMsg.State.Copy(State);
            }


            return CodecReturnCode.SUCCESS;
        }     
        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, "MarketPriceStatus: \n");

            if (HasState)
            {
                stringBuf.Append(tab);
                stringBuf.Append("state: ");
                stringBuf.Append(State);
                stringBuf.AppendLine();
            }

            return stringBuf.ToString();
        }
    }
}
