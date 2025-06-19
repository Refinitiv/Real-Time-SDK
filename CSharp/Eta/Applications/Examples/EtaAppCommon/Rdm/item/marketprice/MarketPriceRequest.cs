/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Rdm;

namespace LSEG.Eta.Example.Common
{
    public class MarketPriceRequest : MarketRequest
    {
        public (int, int) Priority { get; set; }

        public MarketPriceRequest(int domainType) : base(domainType) { }

        public MarketPriceRequest() : base((int)Rdm.DomainType.MARKET_PRICE)
        { }

        public override void Clear()
        {
            base.Clear();
            m_RequestMsg.DomainType = (int)Rdm.DomainType.MARKET_PRICE;
        }
    }
}
