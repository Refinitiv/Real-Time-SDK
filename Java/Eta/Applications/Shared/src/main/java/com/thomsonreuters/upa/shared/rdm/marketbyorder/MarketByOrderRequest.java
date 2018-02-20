package com.thomsonreuters.upa.shared.rdm.marketbyorder;

import com.thomsonreuters.upa.shared.rdm.marketprice.MarketPriceRequest;
import com.thomsonreuters.upa.rdm.DomainTypes;

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
