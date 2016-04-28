package com.thomsonreuters.upa.valueadd.examples.common;

import com.thomsonreuters.upa.valueadd.examples.common.MarketPriceRequest;
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
