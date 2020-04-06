package com.thomsonreuters.ema.access;

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
