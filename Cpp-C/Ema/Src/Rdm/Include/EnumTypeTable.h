/*|-----------------------------------------------------------------------------
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.
*|                See the project's LICENSE.md for details.
*|           Copyright (C) 2019 LSEG. All rights reserved.                 --
*|-----------------------------------------------------------------------------
*/

#ifndef __refinitiv_ema_rdm_EnumTypeTable_h
#define __refinitiv_ema_rdm_EnumTypeTable_h

/**
* A table of enumerated types.  A field that uses this table will contain a value
* corresponding to an enumerated type in this table.
*/

#include "EnumType.h"
#include "Access/Include/EmaVector.h"

namespace refinitiv {

namespace ema {

namespace rdm {

class EnumTypeTableImpl;

class EMA_ACCESS_API EnumTypeTable
{
public:

	///@name Accessors
	/**
	* Returns the list of EnumType that is belonged to this EnumTypeTable.
	* @return the list of EnumType
	*/
	const refinitiv::ema::access::EmaVector<EnumType>& getEnumTypes() const;

	/**
	* Returns the list of Field ID that references to this EnumTypeTable.
	* @return the list of FID
	*/
	const refinitiv::ema::access::EmaVector<refinitiv::ema::access::Int16>& getFidReferences() const;
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

	friend class DictionaryEntryImpl;
	friend class DataDictionaryImpl;

	template<class T>
	friend class refinitiv::ema::access::EmaVector;

	EnumTypeTableImpl* _pImpl;

	EnumTypeTable();

	EnumTypeTable(const EnumTypeTable&);

	EnumTypeTable& operator=(const EnumTypeTable&);

	virtual ~EnumTypeTable();
};

}

}

}

#endif // __refinitiv_ema_rdm_EnumTypeTable_h
