/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

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
