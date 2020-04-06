package com.thomsonreuters.ema.access;

class DictionaryConfig
{
	String		dictionaryName;
	String		rdmfieldDictionaryFileName;
	String		enumtypeDefFileName;
	String		rdmFieldDictionaryItemName;
	String		enumTypeDefItemName;
	boolean     isLocalDictionary;

	DictionaryConfig(boolean localDictionary)
	{
		isLocalDictionary = localDictionary;
	}

	void clear()
	{
		dictionaryName = null;
		rdmfieldDictionaryFileName = null;
		enumtypeDefFileName = null;
		rdmFieldDictionaryItemName = null;
		enumTypeDefItemName = null;
	}
}
