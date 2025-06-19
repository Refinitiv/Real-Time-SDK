/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.valueadd.common.VaNode;

/* Watchlist specific integer object used for hash table lookup. */
class WlInteger extends VaNode
{
    int _value;
    
    int value()
    {
        return _value;
    }
    
    void value(int value)
    {
        _value = value;
    }
    
    public int hashCode()
    {
        return _value;
    }

    public boolean equals(Object obj)
    {
        if (obj == this)
        {
            return true;
        }
        
        WlInteger intObj;
        
        try
        {
            intObj = (WlInteger)obj;
        }
        catch (ClassCastException e)
        {
            return false;
        }
        
        return (intObj._value == _value);
    }

    void clear()
    {
        _value = 0;
    }
}
