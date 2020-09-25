/*|-----------------------------------------------------------------------------
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
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

namespace rtsdk {

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
	rtsdk::ema::access::UInt16 getValue() const;

	/**
	* A brief string representation describing what the type means (For example,
	* this may be an abbreviation of a currency to be displayed to a user).
	*
	* @return the display
	*/
	const rtsdk::ema::access::EmaString& getDisplay() const;

	/**
	* A more elaborate description of what the value means. This information is
	* typically optional and not displayed.
	*
	* @return the meaning
	*/
	const rtsdk::ema::access::EmaString& getMeaning() const;
	//@}

	///@name Operations
	/** Returns a string representation of the class instance.
	@throw OmmMemoryExhaustionException if app runs out of memory
	@return string representation of the class instance
	*/
	const rtsdk::ema::access::EmaString& toString() const;

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
	friend class rtsdk::ema::access::EmaVector;

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

