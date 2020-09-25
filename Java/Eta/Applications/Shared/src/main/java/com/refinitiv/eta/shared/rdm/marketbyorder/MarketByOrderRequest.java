package com.refinitiv.eta.shared.rdm.marketbyorder;

import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceRequest;
import com.refinitiv.eta.rdm.DomainTypes;

/**
 * The market by order request message. Used by an OMM Consumer application to
 * encode/decode a market by order request message.
 */
public class MarketByOrderRequest extends MarketPriceRequest
{
    public MarketByOrderRequest()
    {
        super(DomainTypes.MARKET_BY_ORDER);
    }
}
