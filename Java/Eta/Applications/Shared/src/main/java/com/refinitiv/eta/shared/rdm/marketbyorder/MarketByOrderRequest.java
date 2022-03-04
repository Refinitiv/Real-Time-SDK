/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

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
