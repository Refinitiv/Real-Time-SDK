///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

import com.rtsdk.eta.valueadd.common.VaNode;

class LongObject extends VaNode
{
    long _value;
    
    long value()
    {
        return _value;
    }
    
    LongObject value(long value)
    {
        _value = value;
        
        return this;
    }
   
    public int hashCode()
    {
        return (int)(_value ^ (_value >>> 32));
    }
    
    public boolean equals(Object obj)
    {
        if (obj == this)
        {
            return true;
        }
        
        LongObject longObj;
        
        try
        {
        	longObj = (LongObject)obj;
        }
        catch (ClassCastException e)
        {
            return false;
        }
        
        return (longObj._value == _value);
    }

    LongObject clear()
    {
        _value = 0;
        
        return this;
    }
}
