///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import com.refinitiv.ema.rdm.DataDictionary;

class DictionaryConfig
{
	String		dictionaryName;
	String		rdmfieldDictionaryFileName;
	String		enumtypeDefFileName;
	String		rdmFieldDictionaryItemName;
	String		enumTypeDefItemName;
	boolean     isLocalDictionary;
	DataDictionary dataDictionary;

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
		dataDictionary = null;
	}
}
