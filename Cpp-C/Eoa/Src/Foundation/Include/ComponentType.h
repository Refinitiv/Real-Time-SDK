/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_componenttype_h
#define __thomsonreuters_eoa_foundation_componenttype_h

#include "Foundation/Include/Common.h"

namespace thomsonreuters {
	
namespace eoa {

namespace foundation {

class EoaString;

/**
	\class thomsonreuters::eoa::foundation::ComponentType ComponentType.h "Foundation/Include/ComponentType.h"
	\brief ComponentType class provides enumeration representing component types.

	\remark All methods in this class are \ref SingleThreaded.

	@see ComponentTypeName,
		Node,
		Leaf
*/
enum class EOA_FOUNDATION_API ComponentType
{
	NodeEnum,		/*!< Node type represents OMM container types
						such as Map, FieldList, Vector, etc. but OMM messages. */

	LeafEnum		/*!< Leaf type represents OMM simple data types
						such as UInt, double, Real, DateTime, etc. */
};

/**
	\class thomsonreuters::eoa::foundation::ComponentTypeName ComponentType.h "Foundation/Include/ComponentType.h"
	\brief ComponentTypeName class converts ComponentType enumeration into a string with its name.

	\remark All methods in this class are \ref SingleThreaded.

	@see ComponentType
*/
class EOA_FOUNDATION_API ComponentTypeName
{
public :

	///@name Constructor
	//@{
	/** Constructs ComponentTypeName.
		\remark used for string representation only.
		@param[in] componentType specifies component type to be converted to string
	*/
	ComponentTypeName( ComponentType componentType );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~ComponentTypeName();
	//@}

	///@name Accessors
	//@{
	/** Returns the enum value as a string format.
		@return EoaString containing name of the component type
	*/
	const EoaString& toString() const throw();

	/** Operator const char* overload.
	*/
	operator const char* () const throw();
	//@}

private :

	ComponentType _value;

	ComponentTypeName();
	ComponentTypeName( const ComponentTypeName& );
	ComponentTypeName& operator=( const ComponentTypeName& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_componenttype_h
