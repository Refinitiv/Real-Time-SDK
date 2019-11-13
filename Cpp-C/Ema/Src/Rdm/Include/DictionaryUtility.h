/*|-----------------------------------------------------------------------------
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_rdm_DictionaryUtility_h
#define __thomsonreuters_ema_rdm_DictionaryUtility_h

#include "Access/Include/Common.h"
#include "Access/Include/FieldList.h"
/**
*
* Dicitonary Utilities that can be used by EMA application.
*
* @see DataDictionary
* @see FieldList
*/

namespace thomsonreuters {

namespace ema {

namespace rdm {

class DataDictionary;

class EMA_ACCESS_API DictionaryUtility
{
public:

	/**
	* Extract an DataDictionary object used by FieldList when decoding it.
	* The DataDictionary is valid only in the context of a callback method.
	*
	* @param[in] fieldList the FieldList to extract 
	*
	* @throw OmmInvalidUsageException if it fails to extract 
	*
	* @return the DataDictionary for the field list
	*/
	static const DataDictionary& dataDictionary(const thomsonreuters::ema::access::FieldList& fieldList);

private:

	DictionaryUtility();

	DictionaryUtility(const DictionaryUtility&);

	DictionaryUtility& operator=(const DictionaryUtility&);

	virtual ~DictionaryUtility();
};

}

}

}

#endif // __thomsonreuters_ema_rdm_DictionaryUtility_h