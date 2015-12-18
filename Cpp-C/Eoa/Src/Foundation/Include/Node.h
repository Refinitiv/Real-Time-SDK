/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_node_h
#define __thomsonreuters_eoa_foundation_node_h

/**
	@class thomsonreuters::eoa::foundation::Node Node.h "Foundation/Include/Node.h"
	@brief Node class represents all OMM container types.

	Node class does not represent OMM messages.

	\remark All methods in this class are \ref SingleThreaded.

	@see Component,
		Tag,
		Leaf,
		ConsumerNodeStoreClient,
		EoaString,
		OmmInvalidUsageException,
		OmmMemoryExhaustionException
*/

#include "Foundation/Include/Component.h"
#include "Foundation/Include/Action.h"
#include "Foundation/Include/EoaBuffer.h"
#include "Foundation/Include/FilterType.h"
#include "Foundation/Include/Tag.h"

namespace thomsonreuters {

namespace eoa {

namespace foundation {

class Leaf;
class ConsumerNodeStoreClient;

class NodeDecoder;
class NodeExternalDecoder;

class EOA_FOUNDATION_API Node : public Component
{
public:

	/**
		@class thomsonreuters::eoa::foundation::Node::const_iterator Node.h "Foundation/Include/FieldList.h"
		@brief Node::const_iterator is an external and explicit Node read iterator.

		Node::const_iterator allows for concurent and independent iteration over the same Node instance.

		The following code snippet shows getting data from Node using the "for-each" loop.

		\code

		for ( const auto& component : node )
			cout << component << endl;

		\endcode
	
		The following code snippet shows extracting data from Node using the const_iterator.

		\code

		for ( const_iterator iter = node.begin(); iter != node.end(); ++iter )
			cout << *iter << endl;

		\endcode

		\remark Undefined behavior may occur while attempting to dereference Node::const_iterator objects
				equal to the Node::end().
		\remark Undefined behavior may occur while using objects of Node::const_iterator associated with 
				the Node object who is out of scope.
		\remark All methods in this class are \ref SingleThreaded.

		@see Component,
			Tag,
			Leaf
	*/
	class const_iterator
	{
	public :

		///@name Constructor
		//@{
		/** Default constructor.
			\remark equivalent to Node::end()
		*/
		const_iterator();
		
		/** Constructor set to the begining of Node
			@param[in] node Node object to whose begining's this iterator will point to
		*/
		const_iterator( const Node& node );

		/** Copy constructor.
			@param[in] other const_iterator to copy from
		*/
		const_iterator( const const_iterator& other );
		
		/** Move constructor.
			@param[in] other const_iterator to move from
		*/
		const_iterator( const_iterator&& other );
		//@}

		/** Destructor.
		*/
		virtual ~const_iterator();
		//@}

		///@name Operators
		//@{
		/** Assignment operator.
			@param[in] other const_iterator to copy from
			@return reference to this object.
		*/
		const_iterator& operator=( const const_iterator& other );

		/** Pointer dereference operator.
			@return const reference to Component
		*/
		const Component& operator*() const;

		/** Pointer operator.
			@return const pointer to Component
		*/
		const Component* operator->() const;

		/** Prefix increment operator.
			@return reference to this object.
		*/
		const_iterator& operator++();
		
		/** Postfix increment operator.
			@return previous value of this object.
		*/
		const_iterator operator++( int );
		
		/** Inequality compare operator.
			@param[in] other object to compare against this object
			@return true if passed in object is different than this object
		*/
		bool operator!=( const const_iterator& other ) const;

		/** Equality compare operator.
			@param[in] other object to compare against this object
			@return true if passed in object is equal to this object
		*/
		bool operator==( const const_iterator& other ) const;
		//@}

	private :

		friend class Node;
		friend class CacheNode;

		const_iterator& clear();

		NodeExternalDecoder*		_pDecoder;
	};

	///@name Constructor
	//@{
	/** Constructs Node.
		@throw OmmInvalidUsageException if invalid tag type is specified
		@param[in] tagType specifies tag type of this object
	*/
	Node( TagType tagType = TagType::FieldListEnum );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~Node();
	//@}

	///@name External Iterators
	//@{
	/** Iterator to beginning.
		@return Node::const_iterator object pointing to the first Component
		\remark Undefined behavior may occur while dereferencing returned object pointing to Node::end()
	*/
	virtual const_iterator begin() const;

	/** Iterator to end.
		@return Node::const_iterator object pointing beyond the last Component in this Node
		\remark Undefined behavior may occur while dereferencing returned object pointing to Node::end()
	*/
	virtual const_iterator end() const;
	//@}

	///@name Accessors
	//@{
	/** Returns the component type of Node
		@return ComponentType
		\remark return of other than ComponentType::NodeEnum indicates lack of presence of this component
	*/
	ComponentType getComponentType() const throw() override;

	/** Returns the DataType of data contained in this node
		@return DataType
	*/
	DataType getDataType() const throw() override;

	/** Returns the Tag object, which identifies this node's key type.
		@return the Tag of this object
	*/
	const Tag& getTag() const throw() override;

	/** Returns the level of depth at which this node is in the current container nesting hierarchy
		@return the depth level
		\remark the depth level is a zero based index with the top level component assigned the 0 value
	*/
	UInt64 getDepth() const throw() override;

	/** Returns the action associated with this node
		@return Action
	*/
	virtual Action getAction() const throw();

	/** Returns parent of this Node instance.
		@throw OmmInvalidUsageException if this object has no parent node
		@return parent node
	*/
	virtual const Node& getParentNode() const;

	/** Iterates through the components contained in this object.
		Typical usage is to extract component during each iteration via getCompoent().
		@param[in] filterType specifies a filter type to filter out data.
		@return false at the end of Node; true otherwise
	*/
	virtual bool forth( FilterType filterType = FilterType::BlankableEnum ) const throw();

	/** Resets iteration to start of this node.
		@return void
	*/
	virtual void reset() const throw();

	/** Returns a Component at the current position set by forth()
		@throw OmmInvalidUsageException if forth() was not called first
		@return Component
	*/
	virtual const Component& getComponent() const;

	/** Checks if this object has summary data.
		@return true if this object contains summary; false otherwise
	*/
	virtual bool hasSummary() const throw();

	/** Returns summary associated with this object
		@throw OmmInvalidUsageException if this object contains no summary
		@return summary
	*/
	virtual const Node& getSummary() const;

	/** Returns a Leaf having the same name as passed in.
		@param[in] name looked up Leaf's Tag name
		@param[in] filterType specifies a filter type to filter out data.
		@return matching leaf
	*/
	virtual const Leaf& fetchLeaf( const EoaString& name, FilterType filterType = FilterType::BlankableEnum ) const throw ();

	/** Fetches a Node having the same name as passed in.
		@param[in] name looked up Node's Tag name
		@param[in] filterType specifies a filter type to filter out data.
		@return matching node
	*/
	virtual const Node& fetchNode( const EoaString& name, FilterType filterType = FilterType::BlankableEnum ) const throw ();

	/** Returns a Component having the same name as passed in.
		@param[in] name looked up Component's Tag name
		@param[in] filterType specifies a filter type to filter out data.
		@return matching component
	*/
	virtual const Component& fetchComponent( const EoaString& name, FilterType filterType = FilterType::BlankableEnum ) const throw ();

	/** checks if this object is locally cached.
		@return true if this component is locally cached; false otherwise
	*/
	bool isLocal() const throw() override;

	/** checks if this object is non-blank.
		@return true if this component is non blank; false otherwise
	*/
	bool isNonBlank() const throw() override;

	/** checks if this object is present.
		@return true if this component is present; false otherwise
	*/
	bool isPresent() const throw() override;

	/** Returns a string representation of this object (e.g. component type, component's data type, and value).
		@throw OmmMemoryExhaustionException if app runs out of memory
		@return string representation of this object
	*/
	const EoaString& toString() const override;

	/** Returns a string representation of this object's value.
		@throw OmmMemoryExhaustionException if app runs out of memory
		@return string representation of this object's value
	*/
	const EoaString& getValue() const override;
	//@}

	///@name Operations
	//@{
	/** Intersects this object with passed in node.
		@param[in] node for intersecting with this object.
		@return reference to this object
	*/
	Node& intersectNode( const Node& node );
	
	/** Unions this object with passed in node.
		@param[in] node to union with this object.
		@return reference to this object
	*/
	Node& unionNode( const Node& node );

	/** Adds client to receive change notification of this object.
		@param[in] client specifies ConsumerNodeStoreClient instance receiving notifications about this Node.
		@return reference to this object
	*/
	Node& addNodeStoreClient( const ConsumerNodeStoreClient& client );

	/** Removes client receiving change notification of this object.
		@param[in] client specifies ConsumerNodeStoreClient instance receiving notifications about this Node.
		@return reference to this object
	*/
	Node& removeNodeStoreClient( const ConsumerNodeStoreClient& client );

	/** Adds a Component to this object.
		@param[in] tag specifies a tag for the Component as UInt64.
		@param[in] component specifies a Component for adding to this Node instance
		@param[in] action specifies actions for the Component. The default value is NodeAction::Append
		@param[in] permData specifies permission data for the Component. The default value is empty buffer.
		@return reference to this object
	*/
	Node& add( UInt64 tag, const Component& component, Action action = Action::AppendEnum, const EoaBuffer& permData = EoaBuffer() );

	/** Adds a Component to this object.
		@param[in] tag specifies a tag for the Component as UInt64.
		@param[in] component specifies a Component for adding to this Node instance
		@param[in] action specifies actions for the Component. The default value is NodeAction::Append
		@param[in] permData specifies permission data for the Component. The default value is empty buffer.
		@return reference to this object
	*/
	Node& add( Int64 tag, const Component& component, Action action = Action::AppendEnum, const EoaBuffer& permData = EoaBuffer() );

	/** Adds a Component to this object.
		@param[in] tag specifies a tag for the Component as double.
		@param[in] component specifies a Component for adding to this Node instance
		@param[in] action specifies actions for the Component. The default value is NodeAction::Append
		@param[in] permData specifies permission data for the Component. The default value is empty buffer.
		@return reference to this object
	*/
	Node& add( double tag, const Component& component, Action action = Action::AppendEnum, const EoaBuffer& permData = EoaBuffer() );

	/** Adds a Component to this object.
		@param[in] tag specifies a tag for the Component as EoaString.
		@param[in] component specifies a Component for adding to this Node instance
		@param[in] action specifies actions for the Component. The default value is NodeAction::Append
		@param[in] permData specifies permission data for the Component. The default value is empty buffer.
		@return reference to this object
	*/
	Node& add( const EoaString tag, const Component& component, Action action = Action::AppendEnum, const EoaBuffer& permData = EoaBuffer() );

	/** Adds a Component to this object.
		@param[in] tag specifies a tag for the Component as EoaBuffer.
		@param[in] component specifies a Component for adding to this Node instance
		@param[in] action specifies actions for the Component. The default value is NodeAction::Append
		@param[in] permData specifies permission data for the Component. The default value is empty buffer.
		@return reference to this object
	*/
	Node& add( const EoaBuffer& tag, const Component& component, Action action = Action::AppendEnum, const EoaBuffer& permData = EoaBuffer() );

	/** Adds a Component to this object.
		@param[in] tag specifies a tag for the Component.
		@param[in] component specifies a Component for adding to this Node instance
		@param[in] action specifies actions for the Component. The default value is NodeAction::Append
		@param[in] permData specifies permission data for the Component. The default value is empty buffer.
		@return reference to this object
	*/
	Node& add( const Tag& tag, const Component& component, Action action = Action::AppendEnum, const EoaBuffer& permData = EoaBuffer() );

	/** Clears this object for reuse.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	Node& clear();

	/** Specifies this object's summary
		@param[in] node passed in summary node
		@return reference to this object
	*/
	Node& summary( const Node& node );
	//@}

private :

	friend class NoNode;
	friend class RefreshInfoImpl;
	friend class UpdateInfoImpl;
	friend class StatusInfoImpl;

	mutable EoaString		_toString;
	mutable NodeDecoder*	_pDecoder;

	Decoder& getDecoder() override;

	const EoaString& toString( UInt64 indent, bool ) const override;

	Node( const Node& );
	Node& operator=( const Node& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_node_h
