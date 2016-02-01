/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_filtertype_h
#define __thomsonreuters_eoa_foundation_filtertype_h

#include "Foundation/Include/Common.h"

namespace thomsonreuters {
	
namespace eoa {

namespace foundation {

class EoaString;

/**
	\class thomsonreuters::eoa::foundation::FilterType FilterType.h "Foundation/Include/FilterType.h"
	\brief FilterType class provides enumeration representing filter types.

	\remark All methods in this class are \ref SingleThreaded.

	@see FilterTypeName,
		Node,
		Leaf
*/
enum class EOA_FOUNDATION_API FilterType
{
	BlankableEnum,			/*!< Allow both, blank and non-blank entries while searching containers */

	NonBlankEnum			/*!< Allow non-blank entries only while searching containers */
};

/**
	\class thomsonreuters::eoa::foundation::FilterTypeName FilterType.h "Foundation/Include/FilterType.h"
	\brief FilterTypeName class converts FilterType enumeration into a string with its name.

	\remark All methods in this class are \ref SingleThreaded.

	@see FilterType
*/
class EOA_FOUNDATION_API FilterTypeName
{
public :

	///@name Constructor
	//@{
	/** Constructs FilterTypeName.
		\remark used for string representation only.
		@param[in] filterType specifies filter type to be converted to string
	*/
	FilterTypeName( FilterType filterType );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~FilterTypeName();
	//@}

	///@name Accessors
	//@{
	/** Returns the enum value as a string format.
		@return EoaString containing name of the filter type
	*/
	const EoaString& toString() const throw();

	/** Operator const char* overload.
	*/
	operator const char* () const throw();
	//@}

private :

	FilterType _value;

	FilterTypeName();
	FilterTypeName( const FilterTypeName& );
	FilterTypeName& operator=( const FilterTypeName& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_filtertype_h
