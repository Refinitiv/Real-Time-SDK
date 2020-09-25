///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import com.refinitiv.ema.rdm.EnumType;
import com.refinitiv.eta.valueadd.common.VaNode;

class EnumTypeImpl extends VaNode implements EnumType
{
	private com.refinitiv.eta.codec.EnumType 	rsslEnumType;
	private StringBuilder 							toStringValue;
	
	EnumTypeImpl() {
		
	}
	
	EnumTypeImpl enumType(com.refinitiv.eta.codec.EnumType enumType)
	{	
		rsslEnumType = enumType;
		return this;
	}
	
	com.refinitiv.eta.codec.EnumType enumType()
	{
		return rsslEnumType;
	}

	@Override
	public int value() {
		return rsslEnumType.value();
	}

	@Override
	public String display() {
		return rsslEnumType.display().data() != null ? rsslEnumType.display().toString() : "";
	}

	@Override
	public String meaning() {
		return rsslEnumType.meaning().data() != null ? rsslEnumType.meaning().toString() : "";
	}
	
	@Override
	public String toString() {
		
		if ( toStringValue == null )
		{
			toStringValue = new StringBuilder(64);
		}
		else
		{
			toStringValue.setLength(0);
		}
		
		toStringValue.append("value=").append(value()).
		append(" display=\"").append(display()).
		append("\" meaning=\"").append(meaning()).append("\"");
		
		return toStringValue.toString();
	}
}
