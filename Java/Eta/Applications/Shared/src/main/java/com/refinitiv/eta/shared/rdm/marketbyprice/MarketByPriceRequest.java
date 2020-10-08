package com.refinitiv.eta.shared.rdm.marketbyprice;

import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceRequest;
import com.refinitiv.eta.rdm.DomainTypes;

/**
 * The market by price request message. Used by an OMM Consumer application to
 * encode/decode a market by order request message.
 */
public class MarketByPriceRequest extends MarketPriceRequest
{
    public MarketByPriceRequest()
    {
       super(DomainTypes.MARKET_BY_PRICE);
    }
}
