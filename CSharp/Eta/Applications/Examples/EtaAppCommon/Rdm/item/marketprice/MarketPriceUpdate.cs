/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using System.Text;

namespace LSEG.Eta.Example.Common
{
    public class MarketPriceUpdate : MarketPriceBase
    {
        private IUpdateMsg m_UpdateMsg = new Msg();

        public override int DomainType { get => (int)Rdm.DomainType.MARKET_PRICE; }
        public override int StreamId { get => m_UpdateMsg.StreamId; set { m_UpdateMsg.StreamId = value; } }
        public override int MsgClass { get => m_UpdateMsg.MsgClass; }

        public MarketPriceUpdate()
        {
            Clear();
        }
        
        public override Msg EncodeMsg()
        {   
            if (HasServiceId)
            {
                m_UpdateMsg.ApplyHasMsgKey();
                m_UpdateMsg.MsgKey.ApplyHasServiceId();
                m_UpdateMsg.MsgKey.ServiceId = ServiceId;

                m_UpdateMsg.MsgKey.Name = ItemName;
                m_UpdateMsg.MsgKey.ApplyHasName();
                m_UpdateMsg.MsgKey.NameType = InstrumentNameTypes.RIC;
                m_UpdateMsg.MsgKey.ApplyHasNameType();
            }

            return (Msg)m_UpdateMsg;
        }

        public override void Clear()
        {
            base.Clear();
            m_UpdateMsg.Clear();
            m_UpdateMsg.MsgClass = MsgClasses.UPDATE;
            m_UpdateMsg.DomainType = (int)Rdm.DomainType.MARKET_PRICE;
            m_UpdateMsg.ContainerType = DataTypes.FIELD_LIST;
        }

        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, "MarketPriceUpdate: \n");
            return stringBuf.ToString();
        }
    }
}
