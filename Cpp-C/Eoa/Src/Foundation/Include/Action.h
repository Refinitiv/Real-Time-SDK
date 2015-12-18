/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_action_h
#define __thomsonreuters_eoa_foundation_action_h

#include "Foundation/Include/Common.h"

namespace thomsonreuters {
	
namespace eoa {

namespace foundation {

class EoaString;

/**
	\class thomsonreuters::eoa::foundation::Action Action.h "Foundation/Include/Action.h"
	\brief Action class provides enumeration representing container entry action.

	\remark All methods in this class are \ref SingleThreaded.

	@see ActionName,
		Node,
		Leaf
*/
enum class EOA_FOUNDATION_API Action
{
	AppendEnum,			/*!< Indicates to append entry */
	ChangeEnum,			/*!< Indicates to change entry */
	RemoveEnum,			/*!< Indicates to remove entry */
	ReplaceEnum			/*!< Indicates to replace entry */
};

/**
	\class thomsonreuters::eoa::foundation::ActionName Action.h "Foundation/Include/Action.h"
	\brief ActionName class converts Action enumeration into a string with its name.

	\remark All methods in this class are \ref SingleThreaded.

	@see Action
*/
class EOA_FOUNDATION_API ActionName
{
public :

	///@name Constructor
	//@{
	/** Constructs ActionName.
		\remark used for string representation only.
		@param[in] action specifies action to be converted to string
	*/
	ActionName( Action action );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~ActionName();
	//@}

	///@name Accessors
	//@{
	/** Returns the enum value as a string format.
		@return EoaString containing name of the action
	*/
	const EoaString& toString() const throw();

	/** Operator const char* overload.
	*/
	operator const char* () const throw();
	//@}

private :

	Action _value;

	ActionName();
	ActionName( const ActionName& );
	ActionName& operator=( const ActionName& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_action_h
