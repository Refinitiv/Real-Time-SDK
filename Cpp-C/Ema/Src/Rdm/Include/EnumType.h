/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/*|-----------------------------------------------------------------------------
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_rdm_EnumType_h
#define __refinitiv_ema_rdm_EnumType_h

/**
* A single defined enumerated value.
*/

#include "Access/Include/Common.h"
#include "Access/Include/EmaString.h"
#include "Access/Include/EmaVector.h"

namespace refinitiv {

namespace ema {

namespace rdm {

class DictionaryEntryImpl;
class EnumTypeImpl;

class EMA_ACCESS_API EnumType
{

public:

	///@name Accessors
	/**
	* The actual value representing the type.
	*
	* @return the value
	*/
	refinitiv::ema::access::UInt16 getValue() const;

	/**
	* A brief string representation describing what the type means (For example,
	* this may be an abbreviation of a currency to be displayed to a user).
	*
	* @return the display
	*/
	const refinitiv::ema::access::EmaString& getDisplay() const;

	/**
	* A more elaborate description of what the value means. This information is
	* typically optional and not displayed.
	*
	* @return the meaning
	*/
	const refinitiv::ema::access::EmaString& getMeaning() const;
	//@}

	///@name Operations
	/** Returns a string representation of the class instance.
	@throw OmmMemoryExhaustionException if app runs out of memory
	@return string representation of the class instance
	*/
	const refinitiv::ema::access::EmaString& toString() const;

	/** Operator const char* overload.
	@throw OmmMemoryExhaustionException if app runs out of memory
	*/
	operator const char* () const;
	//@}

private:

	friend class DataDictionaryImpl;
	friend class DictionaryEntryImpl;
	friend class EnumTypeImpl;
	friend class EnumTypeTableImpl;

	template<class T>
	friend class refinitiv::ema::access::EmaVector;

	EnumType();

	EnumType(const EnumType&);

	virtual ~EnumType();

	EnumType& operator=(const EnumType&);

	EnumTypeImpl*    _pImpl;
};

}

}

}

#endif // __refinitiv_ema_rdm_EnumType_h

