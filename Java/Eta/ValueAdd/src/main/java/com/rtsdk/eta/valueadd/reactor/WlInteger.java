package com.rtsdk.eta.valueadd.reactor;

import com.rtsdk.eta.valueadd.common.VaNode;

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
