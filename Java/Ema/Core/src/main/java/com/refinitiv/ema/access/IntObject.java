/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.eta.valueadd.common.VaNode;

public class IntObject  extends VaNode
{
	int _value;
	    
	    int value()
	    {
	        return _value;
	    }
	    
	    IntObject value(int value)
	    {
	        _value = value;
	        
	        return this;
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
	         
	    	 IntObject intObj;
	         
	         try
	         {
	             intObj = (IntObject)obj;
	         }
	         catch (ClassCastException e)
	         {
	             return false;
	         }
	         
	         return (intObj._value == _value);
	    }
	
	    IntObject clear()
	    {
	        _value = 0;
	        
	        return this;
	    }
}
