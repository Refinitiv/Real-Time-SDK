/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
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
		EoaBuffer,
		OmmInvalidUsageException,
		OmmMemoryExhaustionException
*/

#include "Foundation/Include/Component.h"
#include "Foundation/Include/Action.h"
#include "Foundation/Include/FilterType.h"
#include "Foundation/Include/EoaBuffer.h"

namespace thomsonreuters {

namespace eoa {

namespace foundation {

class EoaString;
class Leaf;
class ConsumerNodeStoreClient;

class NodeExternalDecoder;

class EOA_FOUNDATION_API Node : public Component
{
public:

	/**
		@class thomsonreuters::eoa::foundation::Node::const_iterator Node.h "Foundation/Include/Node.h"
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
	class EOA_FOUNDATION_API const_iterator
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

		friend class NodeImpl;
		friend class CacheNode;

		const_iterator& clear();

		NodeExternalDecoder*		_pDecoder;
	};

	///@name External Iterators
	//@{
	/** Iterator to beginning.
		@return Node::const_iterator object pointing to the first Component
		\remark Undefined behavior may occur while dereferencing returned object pointing to Node::end()
	*/
	virtual const_iterator begin() const = 0;

	/** Iterator to end.
		@return Node::const_iterator object pointing beyond the last Component in this Node
		\remark Undefined behavior may occur while dereferencing returned object pointing to Node::end()
	*/
	virtual const_iterator end() const = 0;
	//@}

	///@name Accessors
	//@{
	/** Returns the action associated with this node
		@throw OmmInvalidUsageException if isPresent() = false
		@return Action
	*/
	virtual Action getAction() const = 0;

	/** Returns parent of this Node instance.
		@throw OmmInvalidUsageException if this object has no parent node
		@return parent node
	*/
	virtual const Node& getParentNode() const = 0;

	/** Iterates through the components contained in this object.
		Typical usage is to extract component during each iteration via getCompoent().
		@param[in] filterType specifies a filter type to filter out data.
		@return false at the end of Node; true otherwise
	*/
	virtual bool forth( FilterType filterType = FilterType::BlankableEnum ) const throw() = 0;

	/** Resets iteration to start of this node.
		@return void
	*/
	virtual void reset() const throw() = 0;

	/** Returns a Component at the current position set by forth()
		@throw OmmInvalidUsageException if forth() was not called first
		@return Component
	*/
	virtual const Component& getComponent() const = 0;

	/** Checks if this object has summary data.
		@return true if this object contains summary; false otherwise
	*/
	virtual bool hasSummary() const throw() = 0;

	/** Returns summary associated with this object
		@throw OmmInvalidUsageException if this object contains no summary
		@return summary
	*/
	virtual const Node& getSummary() const = 0;

	/** Returns a Leaf having the same name as passed in.
		@param[in] name looked up Leaf's Tag name
		@param[in] filterType specifies a filter type to filter out data.
		@return matching leaf
	*/
	virtual const Leaf& fetchLeaf( const EoaString& name, FilterType filterType = FilterType::BlankableEnum ) const throw () = 0;

	/** Fetches a Node having the same name as passed in.
		@param[in] name looked up Node's Tag name
		@param[in] filterType specifies a filter type to filter out data.
		@return matching node
	*/
	virtual const Node& fetchNode( const EoaString& name, FilterType filterType = FilterType::BlankableEnum ) const throw () = 0;

	/** Returns a Component having the same name as passed in.
		@param[in] name looked up Component's Tag name
		@param[in] filterType specifies a filter type to filter out data.
		@return matching component
	*/
	virtual const Component& fetchComponent( const EoaString& name, FilterType filterType = FilterType::BlankableEnum ) const throw () = 0;
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~Node();
	//@}

protected :

	Node();

private :

	Node( const Node& );
	Node& operator=( const Node& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_node_h
