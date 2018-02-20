package com.thomsonreuters.upa.shared.rdm.symbollist;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;

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
