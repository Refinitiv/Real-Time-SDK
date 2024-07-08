/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020 LSEG. All rights reserved.                   --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtr_enumTableDefinition
#define __rtr_enumTableDefinition
#include "rtr/rsslHashTable.h"
#include "rtr/jsonToRwfSimple.h"

 /* This type contains Enum table definition to map Display value(utf8) to a enum value.*/
typedef struct {

	RsslHashLink	displayValueLink;
	RsslUInt16		enumValue;
	RsslBuffer		enumDisplay; /* This is in the utf8 format */

}EnumDefinition;

class  EnumTableDefinition 
{
public:

	EnumTableDefinition(jsonToRwfSimple* jonToRwfSimple, RsslUInt16 maxCount);

	void decreaseRefCount(); /* This is used to decrease reference count to delete this object */

	RsslRet addEnumDefinition(RsslEnumTypeTable* pEnumTypeTable, RsslBuffer* displayValue, RsslUInt32 hashSum, int* foundEnumValue);

	RsslBool findEnumDefinition(RsslBuffer* displayValue, RsslUInt32 hashSum, int* foundEnumValue);

private:
	RsslHashTable		_enumByDispalyValue;
	RsslInt32			_referenceCount;
	jsonToRwfSimple*	_pJsonToRwfSimple;
	bool				_initializedHashTable;
	RsslUInt16			_maxCount;

	virtual ~EnumTableDefinition(); /* Call by this class only after the reference count is zero. */

};



#endif // __rtr_enumTableDefinition
