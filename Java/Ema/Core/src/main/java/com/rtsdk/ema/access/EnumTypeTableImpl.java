///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

import java.util.ArrayList;
import java.util.List;

import com.rtsdk.ema.rdm.EnumType;
import com.rtsdk.ema.rdm.EnumTypeTable;
import com.rtsdk.eta.valueadd.common.VaNode;
import com.rtsdk.eta.valueadd.common.VaPool;

class EnumTypeTableImpl extends VaNode implements EnumTypeTable {
	
	private com.rtsdk.eta.codec.EnumTypeTable 		rsslEnumTypeTable;
	private List<EnumType> 									enumTypeList;
	private List<Integer> 									fidsList;
	private StringBuilder									toStringValue;
	private boolean 										refreshEnumTypeList;
	
	private  VaPool											enumTypePool = new VaPool(false);
	
	EnumTypeTableImpl() {
		
	}

	EnumTypeTableImpl enumTypeTable(com.rtsdk.eta.codec.EnumTypeTable enumTypeTable) {
		
		refreshEnumTypeList = true;
		
		rsslEnumTypeTable = enumTypeTable;
		
		return this;
	}
	
	EnumTypeTableImpl clear()
	{
		refreshEnumTypeList = true;
		
		clearEnumTypeList();
		
		if ( fidsList != null )
		{
			fidsList.clear();
		}
		
		return this;
	}
	
	private void clearEnumTypeList()
	{
		if ( enumTypeList != null && enumTypeList.size() != 0 )
		{
			for(int index = 0; index < enumTypeList.size(); index++ )
			{
				((EnumTypeImpl)enumTypeList.get(index)).returnToPool();
			}
			
			enumTypeList.clear();
		}
	}

	@Override
	public List<EnumType> enumTypes() {
		
		if ( enumTypeList == null )
		{
			enumTypeList = new ArrayList<>(rsslEnumTypeTable.maxValue());
		}
		else if ( refreshEnumTypeList )
		{
			clearEnumTypeList();
		}
		else
		{
			return enumTypeList;
		}
		
		com.rtsdk.eta.codec.EnumType rsslEnumType;
		
		for(int index = 0; index <= rsslEnumTypeTable.maxValue(); index++ )
		{
			rsslEnumType = rsslEnumTypeTable.enumTypes()[index];
			
			if ( rsslEnumType != null )
			{
				enumTypeList.add(getEnumType(rsslEnumType));
			}
		}
		
		refreshEnumTypeList = false;
		
		return enumTypeList;
	}

	@Override
	public List<Integer> FidReferences() {
		
		if ( fidsList == null )
		{
			fidsList = new ArrayList<>(rsslEnumTypeTable.fidReferenceCount());
		}
		else
		{
			fidsList.clear();
		}
		
		for(int index = 0; index < rsslEnumTypeTable.fidReferenceCount(); index++)
		{
			fidsList.add(Integer.valueOf(rsslEnumTypeTable.fidReferences()[index]));
		}
		
		return fidsList;
	}
	
	@Override
	public String toString() {
	
		if ( toStringValue == null )
		{
			toStringValue = new StringBuilder(256);
		}
		else
		{
			toStringValue.setLength(0);
		}
		
		for(int index = 0; index < rsslEnumTypeTable.fidReferenceCount(); index++)
		{
			toStringValue.append("(Referenced by Fid ").append(rsslEnumTypeTable.fidReferences()[index]).append(")\n");
		}
		
		com.rtsdk.eta.codec.EnumType rsslEnumType;
		EnumTypeImpl enumTypImpl = new EnumTypeImpl();
		
		for(int index = 0; index <= rsslEnumTypeTable.maxValue(); index++ )
		{
			rsslEnumType = rsslEnumTypeTable.enumTypes()[index];
			
			if ( rsslEnumType != null )
			{
				toStringValue.append(enumTypImpl.enumType(rsslEnumType)).append("\n");
			}
		}
		
		return toStringValue.toString();
	}
	
	private EnumTypeImpl getEnumType(com.rtsdk.eta.codec.EnumType enumType)
	{
		EnumTypeImpl enumTypeImpl = (EnumTypeImpl)enumTypePool.poll();
		
		if ( enumTypeImpl == null )
		{
			enumTypeImpl = new EnumTypeImpl();
			enumTypePool.updatePool(enumTypeImpl);
		}
	
		enumTypeImpl.enumType(enumType);
	
		return enumTypeImpl;
	}
}
