///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2017. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import com.thomsonreuters.ema.rdm.DataDictionary;
import com.thomsonreuters.ema.rdm.DictionaryUtility;

class DictionaryUtilityImpl implements DictionaryUtility {

	@Override
	public DataDictionary dataDictionary(FieldList fieldList) {
		
		FieldListImpl fieldListImpl = (FieldListImpl)fieldList;
		
		if( fieldListImpl._objManager == null )
		{
			throw new OmmInvalidUsageExceptionImpl().message("Failed to extract DataDictionary from the passed in FieldList",
					OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		}
		
		return fieldListImpl._dataDictionaryImpl;
	}
}
