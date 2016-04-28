package com.thomsonreuters.upa.valueadd.examples.common;

import com.thomsonreuters.upa.valueadd.examples.common.MarketPriceRequest;
import com.thomsonreuters.upa.rdm.DomainTypes;

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
