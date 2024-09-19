/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared.rdm.symbollist;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;

/**
 * Symbol list item data.
 */
public class SymbolListItem
{
    public boolean isInUse;
    public int interestCount;
    public Buffer itemName;
    
    public SymbolListItem()
    {
        isInUse = false;
        interestCount = 0;
        itemName = CodecFactory.createBuffer();
    }
    
    /**
     * clears symbol list item fields.
     */
    public void clear()
    {
        isInUse = false;
        interestCount = 0;
        itemName.clear();
    }
}
