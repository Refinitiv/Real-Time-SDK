///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.rdm;

import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.OmmInvalidUsageException;

/**
 * 
 * Utilities that can be used by EMA application.
 *
 * @see DataDictionary
 */
public interface DictionaryUtility
{
	 /**
     * Extract an DataDictionary object used by FieldList when decoding it.
     * The DataDictionary is valid only in the context of a callback method.
     * 
     * @param fieldList the FieldList to extract {@link DataDictionary}
     * 
     * @throws OmmInvalidUsageException if it fails to extract. 
     * 
     * @return the DataDictionary if it exists
     */
	
	public DataDictionary dataDictionary(FieldList fieldList);
}
