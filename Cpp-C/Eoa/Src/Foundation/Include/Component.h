/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_component_h
#define __thomsonreuters_eoa_foundation_component_h

/**
	@class thomsonreuters::eoa::foundation::Component Component.h "Foundation/Include/Component.h"
	@brief Component class is a parent abstract class defining common interfaces for Node and Leaf classes.

	\remark All methods in this class are \ref SingleThreaded.

	@see ComponentType,
		DataType,
		Node,
		Leaf,
		EoaString,
		OmmMemoryExhaustionException,
		Tag
*/

#include "Foundation/Include/ComponentType.h"
#include "Foundation/Include/DataType.h"

namespace thomsonreuters {

namespace eoa {

namespace foundation {

class Tag;
class Decoder;

class EOA_FOUNDATION_API Component
{
public:

	///@name Accessors
	//@{
	/** Returns the type of this component
		@return ComponentType
	*/
	virtual ComponentType getComponentType() const throw() = 0;

	/** Returns the DataType of data contained in this component
		@return DataType
	*/
	virtual DataType getDataType() const throw() = 0;

	/** Returns the Tag object, which identifies this component's key type.
		@return the Tag of this object
	*/
	virtual const Tag& getTag() const throw() = 0;

	/** Returns the level of depth at which this component is in the current nesting hierarchy
		@return the depth level
		\remark the depth level is a zero based index with the top level component assigned the 0 value
	*/
	virtual UInt64 getDepth() const throw() = 0;

	/** Returns a string representation of this component's value.
		@throw OmmMemoryExhaustionException if app runs out of memory
		@return string representation of this component's value
	*/
	virtual const EoaString& getValue() const = 0;

	/** checks if this Component is locally stored.
		@return true if this component is locally stored; false otherwise
	*/
	virtual bool isLocal() const throw() = 0;

	/** checks if this Component is non blank.
		@return true if this component is present and non-blank; false otherwise
	*/
	virtual bool isNonBlank() const throw() = 0;

	/** checks if this Component is present.
		@return true if this component is present; false otherwise
	*/
	virtual bool isPresent() const throw() = 0;

	/** Returns a string representation of this component (e.g., component type, component's data type, and value).
		@throw OmmMemoryExhaustionException if app runs out of memory
		@return string representation of this component
	*/
	virtual const EoaString& toString() const = 0;

	/** Operator const char* overload.
		@throw OmmMemoryExhaustionException if app runs out of memory
	*/
	operator const char* () const;
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~Component();
	//@}

protected:

	Component();

private :

	friend class NodeImpl;
	friend class CacheNode;

	friend class Decoder;
	friend class NoComponentDecoder;
	friend class NoLeafDecoder;
	friend class NoNodeDecoder;
	friend class LeafImplDecoder;
	friend class NodeImplDecoder;

	friend class UpdateInfoImpl;
	friend class RefreshInfoImpl;
	friend class StatusInfoImpl;

	virtual Decoder& getDecoder() = 0;

	virtual const EoaString& toString( UInt64 indent, bool ) const = 0;

	Component( const Component& );
	Component& operator=( const Component& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_component_h
