/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import java.util.ArrayList;
import java.util.List;

class ServiceDictionaryConfig
{
	int						serviceId;
	List<DictionaryConfig> dictionaryUsedList;
	List<DictionaryConfig>	dictionaryProvidedList;
	
	ServiceDictionaryConfig()
	{
		dictionaryUsedList = new ArrayList<>();
		dictionaryProvidedList = new ArrayList<>();
	}
	
	void clear()
	{
		dictionaryUsedList.clear();
		dictionaryProvidedList.clear();
	}
	
	DictionaryConfig findDictionary(String dictionaryName, boolean isDictProvided)
	{
		if (isDictProvided)
		{
			for (DictionaryConfig config : dictionaryProvidedList)
			{
				if (config.dictionaryName.equals(dictionaryName))
					return config;
			}
			return null;
		}
		else
		{
			for (DictionaryConfig config : dictionaryUsedList)
			{
				if (config.dictionaryName.equals(dictionaryName))
					return config;
			}
			return null;
		}
	}
}
