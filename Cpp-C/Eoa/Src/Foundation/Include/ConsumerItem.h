/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_consumeritem_h
#define __thomsonreuters_eoa_foundation_consumeritem_h

/**
	@class thomsonreuters::eoa::foundation::ConsumerItem ConsumerItem.h "Foundation/Include/ConsumerItem.h"
	@brief ConsumerItem provides access to item's key, state, attribute and payload.

	\remark All methods in this class are \ref SingleThreaded.

	@see Node,
		EoaString,
		Node,
		OmmMemoryExhaustionException
*/

#include "Foundation/Include/Common.h"

namespace thomsonreuters {

namespace eoa {

namespace foundation {

class Node;
class EoaString;

class ConsumerItemImpl;
class ConsumerServiceImpl;

class EOA_FOUNDATION_API ConsumerItem
{
public:

	///@name Constructor
	//@{
	/** Constructs ConsumerItem.
		@throw OmmMemoryExhaustionException if app runs out of memory
	*/
	ConsumerItem();

	/** Copy constructor.
		param[in] other copied ConsumerItem object
		@throw OmmMemoryExhaustionException if app runs out of memory
	*/
	ConsumerItem( const ConsumerItem& other );

	/** Move constructor.
		param[in] other moved ConsumerItem object
	*/
	ConsumerItem( ConsumerItem&& other );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~ConsumerItem();
	//@}

	///@name Operations
	//@{
	/** Assignment operator.
		param[in] other copied ConsumerItem object
		@return reference to this object
		@throw OmmMemoryExhaustionException if app runs out of memory
	*/
	ConsumerItem& operator=( const ConsumerItem& other );

	/** Assignment move operator.
		param[in] other moved ConsumerItem object
		@return reference to this object
	*/
	ConsumerItem& operator=( ConsumerItem&& other ) throw();
	//@}

	///@name Accessors
	//@{
	/** Returns the symbol name.
		@return EoaString containing symbol name
	*/
	const EoaString& getSymbol() const throw();

	/** Returns the service name.
		@return EoaString containing service name
	*/
	const EoaString& getServiceName() const throw();

	/** Returns the DomainType, which is the unique identifier of a domain.
		@return domain type value
	*/
	UInt16 getDomainType() const throw();

	/** Indicates if item is active.
		@return true if item is active; false otherwise
	*/
	bool isActive() const throw();

	/** Indicates if item is healthy.
		@return true if item is healthy; false otherwise
	*/
	bool isOk() const throw();

	/** Indicates if item received an unsolicited image.
		@return true if item received an unsolicited image; false otherwise
	*/
	bool isResync() const throw();

	/** Indicates if item received complete image (e.g. last image part).
		@return true if item is complete; false otherwise
	*/
	bool isComplete() const throw();

	/** Returns a EoaString that represents status text of this item
		@return EoaString containing item's status text
	*/
	const EoaString& getStatusText() const throw();

	/** Returns the payload of this item
		@return reference to Node object
	*/
	const Node& getPayload() const throw();

	/** Returns a string representation of the class instance.
		@throw OmmMemoryExhaustionException if app runs out of memory
		@return EoaString containing string representation of the class instance
	*/
	const EoaString& toString() const;

	/** Operator const char* overload.
		@throw OmmMemoryExhaustionException if app runs out of memory
	*/
	operator const char* () const;
	//@}

private:

	ConsumerItemImpl*		_pConsumerItemImpl;
	
	friend class			ConsumerServiceImpl;
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_consumeritem_h
