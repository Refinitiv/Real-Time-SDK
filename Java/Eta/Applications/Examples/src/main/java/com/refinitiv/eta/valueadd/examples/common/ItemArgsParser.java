/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.common;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.eta.rdm.DomainTypes;

class ItemArgsParser
{
	final int ERROR_RETURN_CODE = -1;
	List<ItemArg> itemList = new ArrayList<ItemArg>();

	boolean isStart(String[] args, int argOffset)
	{
		if (args[argOffset].startsWith("mp:"))
		{
			return true;
		}
		else if (args[argOffset].startsWith("mpps:"))
		{
			return true;
		}
		else if (args[argOffset].startsWith("mbo:"))
		{
			return true;
		}
		else if (args[argOffset].startsWith("mbops:"))
		{
			return true;
		}
		else if (args[argOffset].startsWith("mbp:"))
		{
			return true;
		}
		else if (args[argOffset].startsWith("mbpps:"))
		{
			return true;
		}
		else if (args[argOffset].startsWith("yc:"))
		{
			return true;
		}
		else if (args[argOffset].startsWith("ycps:"))
		{
			return true;
		}
		else if (args[argOffset].startsWith("sl:"))
		{
			return true;
		}
		else if (args[argOffset].startsWith("sl"))
		{
			return true;
		}
		
		return false;
	}
	
	// returns offset past item arguments or -1 if error
	int parse(String[] args, int argOffset)
	{
		int retCode = ERROR_RETURN_CODE;
		
		String[] commaTokens = args[argOffset].split(",");
		for (int i = 0; i < commaTokens.length; i++)
		{
			if (parseItem(commaTokens[i]) < 0)
			{
				return ERROR_RETURN_CODE;
			}
		}
		retCode = argOffset + 1;
				
		return retCode;
	}
	
	private int parseItem(String itemStr)
	{
		int retCode = ERROR_RETURN_CODE;
		
		String[] tokens = itemStr.split(":");
		if (tokens.length == 2)
		{
			ItemArg itemArg = new ItemArg();
			itemArg.domain = stringToDomain(tokens[0]);
			if (tokens[0].contains("ps"))
			{
				itemArg.enablePrivateStream = true;
			}
			if (tokens[0].contains("sld"))
			{
				itemArg.symbolListData(true);
			}
			itemArg.itemName = tokens[1];
			itemList.add(itemArg);
			retCode = 1;
		}
		else if (tokens.length == 1 && itemStr.equals("sl"))
		{
			ItemArg itemArg = new ItemArg();
			itemArg.domain = DomainTypes.SYMBOL_LIST;
			itemList.add(itemArg);
			retCode = 1;
		}
		else if (tokens.length == 1 && itemStr.equals("sld"))
		{
			ItemArg itemArg = new ItemArg();
			itemArg.domain = DomainTypes.SYMBOL_LIST;
			itemArg.symbolListData(true);
			itemList.add(itemArg);
			retCode = 1;
		}

		return retCode;
	}
	
	private int stringToDomain(String domainStr)
	{
		int retCode = 0;
		
		if (domainStr.startsWith("mp"))
		{
			retCode = DomainTypes.MARKET_PRICE;
		}
		else if (domainStr.startsWith("mbo"))
		{
			retCode = DomainTypes.MARKET_BY_ORDER;
		}
		else if (domainStr.startsWith("mbp"))
		{
			retCode = DomainTypes.MARKET_BY_PRICE;
		}
		else if (domainStr.startsWith("yc"))
		{
			retCode = DomainTypes.YIELD_CURVE;
		}
		else if (domainStr.startsWith("sl"))
		{
			retCode = DomainTypes.SYMBOL_LIST;
		}
		
		return retCode;
	}

	List<ItemArg> itemList()
	{
		return itemList;
	}
}
