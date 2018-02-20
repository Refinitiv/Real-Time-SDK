///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2017. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import com.thomsonreuters.ema.rdm.EnumType;
import com.thomsonreuters.upa.valueadd.common.VaNode;

class EnumTypeImpl extends VaNode implements EnumType
{
	private com.thomsonreuters.upa.codec.EnumType 	rsslEnumType;
	private StringBuilder 							toStringValue;
	
	EnumTypeImpl() {
		
	}
	
	EnumTypeImpl enumType(com.thomsonreuters.upa.codec.EnumType enumType)
	{	
		rsslEnumType = enumType;
		return this;
	}
	
	com.thomsonreuters.upa.codec.EnumType enumType()
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
