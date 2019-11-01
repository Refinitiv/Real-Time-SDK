/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2017. All rights reserved.            --
 *|-----------------------------------------------------------------------------
*/

#include "DictionaryUtility.h"
#include "FieldListDecoder.h"
#include "ExceptionTranslator.h"
#include "OmmInvalidUsageException.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;

DictionaryUtility::DictionaryUtility()
{
}


DictionaryUtility::~DictionaryUtility()
{
}

const DataDictionary& DictionaryUtility::dataDictionary(const thomsonreuters::ema::access::FieldList& fieldList)
{
	if (!fieldList.hasDecoder())
	{
		throwIueException( "Failed to extract DataDictionary from the passed in FieldList", OmmInvalidUsageException::InvalidArgumentEnum );
	}

	Decoder& decoder = const_cast<thomsonreuters::ema::access::FieldList&>(fieldList).getDecoder();

	return static_cast<thomsonreuters::ema::access::FieldListDecoder&>(decoder).getDataDictionary();
}

