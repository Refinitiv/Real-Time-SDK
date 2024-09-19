/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Example.Common
{

    /// <summary>
    /// The market by price request message. Used by an OMM Consumer application to
    /// encode/decode a market by order request message.
    /// </summary>
    public class MarketByPriceRequest : MarketPriceRequest
    {
        public MarketByPriceRequest() : base((int)Rdm.DomainType.MARKET_BY_PRICE) { }

        public override void Clear()
        {
            base.Clear();
            m_RequestMsg.DomainType = (int)Rdm.DomainType.MARKET_BY_PRICE;
        }
    }
}
