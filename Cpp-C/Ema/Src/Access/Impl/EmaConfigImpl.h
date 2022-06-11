/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2022 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_EmaConfigImpl_h
#define __refinitiv_ema_access_EmaConfigImpl_h

#ifdef WIN32
#include "direct.h"
#endif

#include "ConfigErrorHandling.h"
#include "OmmLoggerClient.h"
#include "HashTable.h"
#include "ElementList.h"
#include "EmaString.h"
#include "EmaList.h"
#include "OmmConsumerConfig.h"
#include "OmmNiProviderConfig.h"
#include "ProgrammaticConfigure.h"
#include "EmaRdm.h"
#include "OmmOAuth2ConsumerClient.h"
#include "OmmLoginCredentialConsumerClient.h"
#include "OAuth2Credential.h"
#include "OmmOAuth2CredentialImpl.h"
#include "LoginRdmReqMsgImpl.h"


#include "rtr/rsslRDMLoginMsg.h"
#include "rtr/rsslReactor.h"
#include "libxml/parser.h"


#define DEFAULT_CONS_NAME							  EmaString("EmaConsumer")
#define DEFAULT_IPROV_NAME							  EmaString("EmaIProvider")
#define DEFAULT_NIPROV_NAME							  EmaString("EmaNiProvider")

using namespace refinitiv::ema::rdm;


namespace refinitiv {

namespace ema {

namespace access {

class XMLnode;
class ProgrammaticConfigure;
class EmaConfigBaseImpl;
class WarmStandbyChannelConfig;
class LoginRdmReqMsgImpl;

static struct MsgTypeConverter
{
	const char* configInput;
	UInt16 convertedValue;
} msgTypeConverter[] =
{
	{ "MMT_LOGIN", MMT_LOGIN },
	{ "MMT_DIRECTORY", MMT_DIRECTORY },
	{ "MMT_DICTIONARY", MMT_DICTIONARY },
	{ "MMT_MARKET_PRICE", MMT_MARKET_PRICE },
	{ "MMT_MARKET_BY_ORDER", MMT_MARKET_BY_ORDER },
	{ "MMT_MARKET_BY_PRICE", MMT_MARKET_BY_PRICE },
	{ "MMT_MARKET_MAKER", MMT_MARKET_MAKER },
	{ "MMT_SYMBOL_LIST", MMT_SYMBOL_LIST },
	{ "MMT_SERVICE_PROVIDER_STATUS", MMT_SERVICE_PROVIDER_STATUS },
	{ "MMT_HISTORY", MMT_HISTORY },
	{ "MMT_HEADLINE", MMT_HEADLINE },
	{ "MMT_REPLAYHEADLINE", MMT_REPLAYHEADLINE },
	{ "MMT_REPLAYSTORY", MMT_REPLAYSTORY },
	{ "MMT_TRANSACTION", MMT_TRANSACTION },
	{ "MMT_YIELD_CURVE", MMT_YIELD_CURVE },
	{ "MMT_CONTRIBUTION", MMT_CONTRIBUTION },
	{ "MMT_PROVIDER_ADMIN", MMT_PROVIDER_ADMIN },
	{ "MMT_ANALYTICS", MMT_ANALYTICS },
	{ "MMT_REFERENCE", MMT_REFERENCE },
	{ "MMT_NEWS_TEXT_ANALYTICS", MMT_NEWS_TEXT_ANALYTICS },
	{ "MMT_SYSTEM", MMT_SYSTEM }
};

static struct QosRateConverter
{
	const char* configInput;
	UInt32 convertedValue;
} qosRateConverter[] =
{
	{ "Rate::TickByTick", 0 },
	{ "Rate::JustInTimeConflated", 0xFFFFFF00 }
};

static struct QosTimelinessConverter
{
	const char* configInput;
	UInt32 convertedValue;
} qosTimelinessConverter[] =
{
	{ "Timeliness::RealTime", 0 },
	{ "Timeliness::InexactDelayed", 0xFFFFFFFF }
};

static struct StreamStateConverter
{
	const char* configInput;
	OmmState::StreamState convertedValue;
} streamStateConverter[] =
{
	{ "Open", OmmState::OpenEnum },
	{ "NonStreaming", OmmState::NonStreamingEnum },
	{ "CloseRecover", OmmState::ClosedRecoverEnum },
	{ "Close", OmmState::ClosedEnum },
	{ "CloseRedirected", OmmState::ClosedRedirectedEnum }
};

static struct DataStateConverter
{
	const char* configInput;
	OmmState::DataState convertedValue;
} dataStateConverter[] =
{
	{ "NoChange", OmmState::NoChangeEnum },
	{ "Ok", OmmState::OkEnum },
	{ "Suspect", OmmState::SuspectEnum }
};

static struct StatusCodeConverter
{
	const char* configInput;
	OmmState::StatusCode convertedValue;
} statusCodeConverter[] = {
	{ "None", OmmState::NoneEnum },
	{ "NotFound", OmmState::NotFoundEnum },
	{ "Timeout", OmmState::TimeoutEnum },
	{ "NotAuthorized", OmmState::NotAuthorizedEnum },
	{ "InvalidArgument", OmmState::InvalidArgumentEnum },
	{ "UsageError", OmmState::UsageErrorEnum },
	{ "Preempted", OmmState::PreemptedEnum },
	{ "JustInTimeConflationStarted", OmmState::JustInTimeConflationStartedEnum },
	{ "TickByTickResumed", OmmState::TickByTickResumedEnum },
	{ "FailoverStarted", OmmState::FailoverStartedEnum },
	{ "FailoverCompleted", OmmState::FailoverCompletedEnum },
	{ "GapDetected", OmmState::GapDetectedEnum },
	{ "NoResources", OmmState::NoResourcesEnum },
	{ "TooManyItems", OmmState::TooManyItemsEnum },
	{ "AlreadyOpen", OmmState::AlreadyOpenEnum },
	{ "SourceUnknown", OmmState::SourceUnknownEnum },
	{ "NotOpen", OmmState::NotOpenEnum },
	{ "NonUpdatingItem", OmmState::NonUpdatingItemEnum },
	{ "UnsupportedViewType", OmmState::UnsupportedViewTypeEnum },
	{ "InvalidView", OmmState::InvalidViewEnum },
	{ "FullViewProvided", OmmState::FullViewProvidedEnum },
	{ "UnableToRequestAsBatch", OmmState::UnableToRequestAsBatchEnum },
	{ "NoBatchViewSupportInReq", OmmState::NoBatchViewSupportInReqEnum },
	{ "ExceededMaxMountsPerUser", OmmState::ExceededMaxMountsPerUserEnum },
	{ "Error", OmmState::ErrorEnum },
	{ "DacsDown", OmmState::DacsDownEnum },
	{ "UserUnknownToPermSys", OmmState::UserUnknownToPermSysEnum },
	{ "DacsMaxLoginsReached", OmmState::DacsMaxLoginsReachedEnum },
	{ "DacsUserAccessToAppDenied", OmmState::DacsUserAccessToAppDeniedEnum }
};

static struct WarmStandbyModeConverter
{
	const char* configInput;
	RsslReactorWarmStandbyMode convertedValue;
} warmStandbyModeConverter[] =
{
	{ "LOGIN_BASED", RSSL_RWSB_MODE_LOGIN_BASED },
	{ "SERVICE_BASED", RSSL_RWSB_MODE_SERVICE_BASED }
};

class ConfigElement
{
public:

	enum ConfigElementType
	{
		ConfigElementTypeInt64 = 0,
		ConfigElementTypeUInt64 = 1,
		ConfigElementTypeAscii = 2,
		ConfigElementTypeEnum = 3,
		ConfigElementTypeBool = 4,
		ConfigElementTypeDouble = 5
	};

	ConfigElement( const EmaString& name, XMLnode* parent ) : _name( name ), _parent( parent ) {}

	virtual ~ConfigElement() {}

	virtual void print() const = 0;

	const EmaString& name() const
	{
		return _name;
	}

	XMLnode* parent() const
	{
		return _parent;
	}

	void reParent( XMLnode* p )
	{
		_parent = p;
	}

	bool operator==( const ConfigElement& ) const;

	void appendErrorMessage( EmaString&, OmmLoggerClient::Severity );

	EmaString changeMessage( const EmaString&, const ConfigElement& ) const;

	virtual void addValue( const ConfigElement* ) = 0;

	virtual ConfigElementType type() const = 0;

protected:

	EmaString	_name;
	XMLnode*	_parent;
};

template<typename T>
class XMLConfigElement : public ConfigElement
{
public:

	XMLConfigElement( const EmaString& name, XMLnode* parent, ConfigElementType type, const T& value ) :
		ConfigElement( name, parent ),
		_type( type )
	{
		_values.push_back( value );
	}

	EmaVector<T>* values()
	{
		return &_values;
	}
	T* value()
	{
		return &_values[_values.size() - 1];
	}

	virtual ConfigElementType type() const
	{
		return _type;
	}

	void addValue( const ConfigElement* p )
	{
		const XMLConfigElement< T >* tmp( dynamic_cast<const XMLConfigElement<T>* >( p ) );
		if ( tmp )
			_values.push_back( tmp->_values[0] );
	}

	void print() const;

	bool operator==( const XMLConfigElement& ) const;

	EmaString changeMessage( const EmaString&, const XMLConfigElement< T >& ) const;

private:

	ConfigElementType	_type;
	EmaVector<T>		_values;
};

template < class T >
struct NamePair
{
	NamePair( T first, T second ) : first( first ), second( second ) {}
	NamePair( T& first, T& second ) : first( first ), second( second ) {}
	T first;
	T second;
};

static NamePair<EmaString> configReplacementPairs[] = { NamePair<EmaString>( EmaString( "Channel" ), EmaString( "ChannelSet" ) ) };

class ConfigElementList
{
public:

	ConfigElementList() : _theList( 0 ) {}

	ConfigElement* first() const
	{
		return _theList ? _theList->e : 0;
	}

	ConfigElement* next( const ConfigElement* ce ) const
	{
		Node* q;
		for ( Node* p = _theList; p; p = q )
		{
			if ( p->e == ce )
				return p->next ? p->next->e : 0;
			q = p->next;
		}

		return 0;
	}

	void add( ConfigElement* element )
	{
		Node* n( new Node( element ) );
		if ( _theList )
		{
			Node* last( 0 );
			Node* q;
			for ( q = _theList; q; q = q->next )
			{
				if ( element->name() == q->e->name() )
				{
					q->e = element;
					break;
				}
				last = q;
			}

			if ( ! q )
			{
				last->next = n;
				n->next = 0;
			}
		}
		else
			_theList = n;
	}

	NamePair<bool> replaceElement( ConfigElement* existingElement, ConfigElement* newElement )
	{
		if ( existingElement->name() == newElement->name() )
		{
			if ( existingElement->name() != "Name" &&
			     !( *existingElement == *newElement ) )
				return NamePair<bool>( true, true );
			return NamePair<bool>( true, false );
		}

		for ( int i = 0; i < sizeof configReplacementPairs / sizeof configReplacementPairs[0]; ++i )
		{
			if ( ( configReplacementPairs[i].first == existingElement->name() && configReplacementPairs[i].second == newElement->name() ) ||
			     ( configReplacementPairs[i].first == newElement->name() && configReplacementPairs[i].second == existingElement->name() ) )
				return NamePair<bool>( true, true );
		}

		return NamePair<bool>( false, false );
	}

	void appendAttributes( ConfigElementList* attributes, bool emptyList = false )
	{
		if ( _theList )
		{
			Node* nextAttribute( 0 );
			for ( Node* p = attributes->_theList; p; p = nextAttribute )
			{
				nextAttribute = p->next;

				Node* last( 0 );
				Node* q;
				for ( q = _theList; q; q = q->next )
				{
					NamePair<bool> replace( replaceElement( q->e, p->e ) );

					// attribute is name/value pair; this checks if two attributes have the same name
					if ( replace.first )
					{
						if ( replace.second )
						{
							EmaString actualName;
							if ( _theList->e->name() == "Name" )
							{
								XMLConfigElement<EmaString>* tmp( dynamic_cast<XMLConfigElement<EmaString> *>( _theList->e ) );
								if ( tmp )
									actualName = *( tmp->value() );
							}
							if ( p->e->name() == "Channel" || p->e->name() == "ChannelSet" )
							{
									ConfigElement * toFree( q->e );
									q->e = p->e;
									p->e->reParent(toFree->parent());
									EmaString errorMsg( q->e->changeMessage( actualName, *( p->e ) ) );
									p->e->appendErrorMessage(errorMsg, OmmLoggerClient::VerboseEnum);
									delete( toFree );
							}
							else
							{
								q->e->addValue( p->e );
								delete( p->e );
							}
						}

						else
							delete p->e;

						if ( attributes->_theList == p )
							attributes->_theList = nextAttribute;
						delete p;
						break;
					}
					last = q;
				}

				if ( ! q )
				{
					p->e->reParent( last->e->parent() );
					last->next = p;
					p->next = 0;
				}
			}

			attributes->_theList = 0;
			delete attributes->_theList;
		}
		else
			_theList = attributes->_theList;

		if ( emptyList )
			attributes->_theList = 0;
	}

	template<typename T>
	T* find( const EmaString& itemToRetrieve ) const
	{
		for ( Node* p = _theList; p; p = p->next )
		{
			if ( p->e->name() == itemToRetrieve )
			{
				XMLConfigElement<T>* e = dynamic_cast<XMLConfigElement<T> *>( p->e );
				if ( e )
					return e->value();
				if ( p->e->parent() )
				{
					EmaString errorMsg( "dynamic cast failed for [" );
					errorMsg.append( itemToRetrieve.c_str() ).append( "]" );
					p->e->appendErrorMessage( errorMsg, OmmLoggerClient::ErrorEnum );
				}
				return 0;
			}
		}
		return 0;
	}

	void print( int tabs = 0 ) const
	{
		for ( Node* p = _theList; p; p = p->next )
		{
			for ( int i = 0; i < tabs; ++i )
				printf( "\t" );
			if ( p->e )
				p->e->print();
			printf( "\n" );
			fflush( stdout );
		}
	}

	virtual ~ConfigElementList()
	{
		Node* q;
		for ( Node* p = _theList; p; p = q )
		{
			q = p->next;
			delete( p->e );
			delete( p );
		}
	}

private:

	struct Node
	{
		Node( ConfigElement* e ) : e( e ), next( 0 ) {}
		ConfigElement* e;
		Node* next;
	};

	Node* _theList;
};

template<typename T>
class xmlList
{
public:

	xmlList() : _theList( 0 ) {}

	bool insert( T* element, bool merge )
	{
		if ( _theList )
		{
			if ( merge )
			{

				for ( Node* p = _theList; p; p = p->next )
					if ( p->e->name() == element->name() )
					{
						if ( p->e->merge( element ) )
							return true;
					}
			}

			Node* p = _theList;
			while ( p->next )
				p = p->next;
			p->next = new Node( element );
		}
		else
		{
			_theList = new Node( element );
		}
		return false;
	}

	void merge( xmlList<T>* other )
	{
		bool merged;
		xmlList<T>* unmerged( new xmlList<T> );
		Node* nextNodeToMerge( other->_theList );
		while ( nextNodeToMerge )
		{
			Node* p = nextNodeToMerge;
			nextNodeToMerge = nextNodeToMerge->next;
			merged = false;
			for ( Node* q = _theList; q; q = q->next )
			{
				if ( p->e->name() == q->e->name() )
				{
					if ( q->e->merge( p->e ) )
					{
						merged = true;
						delete p;
						break;
					}
				}
			}

			if ( ! merged )
			{
				p->next = 0;
				if ( unmerged->size() )
				{
					Node* u = unmerged->_theList;
					while ( u->next )
						u = u->next;
					u->next = p;
				}
				else
					unmerged->_theList = p;
			}
		}

		if ( unmerged->size() )
		{
			Node* ptr = _theList;
			while ( ptr->next )
				ptr = ptr->next;
			ptr->next = unmerged->_theList;
			for ( ptr = unmerged->_theList; ptr; ptr = ptr->next )
			{
				ptr->e->reparent( _theList->e->parent() );
			}
			unmerged->_theList = 0;
		}

		delete unmerged;
		other->_theList = 0;
	}

	void print( int tabs = 0 ) const
	{
		for ( Node* p = _theList; p; p = p->next )
		{
			for ( int i = 0; i < tabs; ++i )
				printf( "\t" );
			p->e->print( tabs );
			fflush( stdout );
		}
	}

	const T* find( const EmaString& nodeName, EmaConfigErrorList** errors ) const
	{
		static const char* dot( "." );
		int dotPosition( const_cast<EmaString&>( nodeName ).find( dot, 0 ) );
		if ( dotPosition == -1 )
		{
			for ( Node* p = _theList; p; p = p->next )
				if ( p->e->name() == nodeName )
					return p->e;
		}
		else
		{
			EmaString actualNodeName = nodeName.substr( 0, dotPosition );
			EmaString name = nodeName.substr( dotPosition + 1, nodeName.length() - dotPosition - 1 );
			T* retVal( 0 );
			int arrayLoc( 0 );
			for ( Node* p = _theList; p; p = p->next )
			{
				if ( p->e->name() == actualNodeName )
				{
					EmaString target( "Name" );
					EmaString targetValue;
					if ( p->e->get( target, targetValue ) && targetValue == name )
						return p->e;
					else if ( p->e->errors().count() )
					{
						if ( ! *errors )
							*errors = new EmaConfigErrorList();
						( *errors )->add( p->e->errors() );
						p->e->errors().clear();
					}
				}
			}
			return retVal;
		}

		return 0;
	}

	unsigned int size()
	{
		unsigned int retVal( 0 );
		for ( const Node* p = _theList; p; p = p->next )
			++retVal;
		return retVal;
	}

	bool empty() const
	{
		return _theList == 0 ? true : false;
	}

	T* getFirst() const
	{
		if ( _theList )
			return _theList->e;
		return 0;
	}

	T* getNext( const T* current ) const
	{
		for ( Node* p = _theList; p; p = p->next )
		{
			if ( p->e == current )
				return p->next ? p->next->e : 0;
		}
		return 0;
	}

	virtual ~xmlList()
	{
		Node* q;
		for ( Node* p = _theList; p; p = q )
		{
			q = p->next;
			delete( p->e );
			delete( p );
		}
	}

private:

	struct Node
	{
		Node( T* e ) : e( e ), next( 0 ) {}
		T* e;
		Node* next;
	};

	Node* _theList;
};

class XMLnode
{
public:

	XMLnode( const EmaString& name, int level, XMLnode* parent ) : _name( name ), _parent( parent ), _attributes( new ConfigElementList() ), _children( new xmlList<XMLnode> ), _level( level ) {}

	virtual ~XMLnode()
	{
		delete( _attributes );
		delete( _children );
		_errors.clear();
	}

	bool addChild( XMLnode* child );

	void addAttribute( ConfigElement* element )
	{
		_attributes->add( element );
	}

	void appendAttributes( ConfigElementList* attributes, bool emptyList = false )
	{
		_attributes->appendAttributes( attributes, emptyList );
	}

	ConfigElementList* attributes() const
	{
		return _attributes;
	}

	template<typename T>
	bool get( const EmaString& itemToRetrieve, T& retrievedItem )
	{
		T* found( find<T>( itemToRetrieve ) );
		if ( found )
		{
			retrievedItem = *found;
			return true;
		}
		return false;
	}

	template<typename T>
	T* find( const EmaString& itemToRetrieve )
	{
		// format: zero or more children followed by an attribute
		// each element separated by an '|' symbol
		// for duplicate children names, which are common: childname.value of attribute name (i.e., each child must have a name)
		// value must have Ascii type
		// e.g., Sessions|session.Session_1|logger
		Int32 substrBeginningPosition( 0 );
		Int32 substrEndingPosition( 0 );
		const char* nodeSeparator( "|" );
		const char* nameSeparator( "." );
		EmaString attributeName;

		substrEndingPosition = static_cast<EmaString>( itemToRetrieve ).find( nodeSeparator, substrBeginningPosition );
		if ( substrEndingPosition == -1 )
		{
			return _attributes->find<T>( itemToRetrieve );
		}
		else
		{
			EmaString nodeName;
			nodeName = static_cast<EmaString>( itemToRetrieve ).substr( substrBeginningPosition, substrEndingPosition - substrBeginningPosition );
			EmaConfigErrorList*   errors( 0 );
			const XMLnode* tmp = _children->find( nodeName, &errors );
			if ( tmp )
			{
				T* retVal( const_cast<XMLnode*>( tmp )->find<T>( itemToRetrieve.substr( substrEndingPosition + 1, itemToRetrieve.length() - substrEndingPosition - 1 ) ) );
				if ( ! retVal )
				{
					_errors.add( const_cast<XMLnode*>( tmp )->errors() );
					( const_cast<XMLnode*>( tmp )->errors().clear() );
				}
				return retVal;
			}
			else if ( errors )
				_errors.add( *errors );
			return 0;
		}
	}

	void print( int tabs = 0 );

	const EmaString& name() const
	{
		return _name;
	}

	void name( const EmaString& n )
	{
		_name = n;
	}

	int level() const
	{
		return _level;
	}

	bool merge( XMLnode* newNode )
	{
		// we already know that node names match; check if we just need to merge subnodes.
		EmaString errorMsg;
		EmaString* name = _attributes->find<EmaString>( "Name" );
		EmaString* newNodeName = newNode->_attributes->find<EmaString>( "Name" );
		if ( name && newNodeName )
		{
			if ( *name == *newNodeName )
			{
				_attributes->appendAttributes( newNode->attributes() );
				return true;
			}
			else
				return false;
		}

		// at this point, we have two nodes with the same name, but they are not subnodes.
		// Merge attributes and then children
		_attributes->appendAttributes( newNode->attributes() );
		_children->merge( newNode->_children );
		return true;
	}

	unsigned int size() const
	{
		return _children->size();
	}

	XMLnode* parent()
	{
		return _parent;
	}

	void reparent( XMLnode* p )
	{
		_parent = p;
	}

	void appendErrorMessage( const EmaString& errorMsg, OmmLoggerClient::Severity severity );

	int errorCount()
	{
		return _errors.count();
	}

	EmaConfigErrorList& errors()
	{
		return _errors;
	}

	class NameString : public EmaString, public ListLinks< NameString >
	{
	public:

		NameString() : EmaString() {}
		NameString( EmaString& n ) : EmaString( n ) {}

		virtual ~NameString() {}
	};

	void verifyDefaultConsumer();

	void verifyDefaultNiProvider();

	void verifyDefaultDirectory();

	void getServiceNameList( const EmaString&, EmaVector< EmaString >& );

	void getAsciiAttributeValueList( const EmaString& nodeName, const EmaString& attributeName, EmaVector< EmaString >& valueList );

	void getEntryNodeList( const EmaString& nodeName, const EmaString& entryName, EmaVector< XMLnode* >& entryNodeList );

	void getNames( EmaList< NameString* >& theNames )
	{
		if ( _children && ! _children->empty() )
		{
			EmaString searchString( "Name" );
			const XMLnode* child( _children->getFirst() );
			while ( child )
			{
				EmaString name;
				if ( const_cast< XMLnode* >( child )->get< EmaString >( searchString, name ) )
					theNames.insert( new NameString( name ) );
				child = _children->getNext( child );
			}
		}
	}

	template<typename T>
	void getValues( const EmaString& attributeName, EmaVector< T >& values ) const
	{
		values.clear();
		if ( _attributes )
		{
			ConfigElement* p = _attributes->first();
			while ( p )
			{
				if ( p->name() == attributeName )
				{
					XMLConfigElement<T>* element = dynamic_cast<XMLConfigElement<T>* >( p );
					if ( element )
					{
						values = *( element->values() );
						return;
					}
				}
				p = _attributes->next( p );
			}
		}
	}

	xmlList<XMLnode>* getChildren() const
	{
		return _children;
	}

private:

	EmaString			_name;
	XMLnode*			_parent;
	ConfigElementList*	_attributes;
	xmlList<XMLnode>*	_children;
	int					_level;
	EmaConfigErrorList	_errors;
};

template< >
inline XMLnode* XMLnode::find< XMLnode >( const EmaString& itemToRetrieve )
{
	Int32 begin( 0 ), end;
	const char* nodeSeparator( "|" );
	Int32 length( itemToRetrieve.length() );
	EmaString nodeName;
	XMLnode* tmp( this );
	EmaConfigErrorList* errorList;
	EmaString foundPath;
	while ( length )
	{
		if ( tmp->_children->empty() )
			return 0;

		end = itemToRetrieve.find( nodeSeparator, begin );
		if ( end == -1 )
		{
			nodeName = itemToRetrieve.substr( begin, length );
			length = 0;
		}
		else
		{
			nodeName = itemToRetrieve.substr( begin, end - begin );
			length -= ( end - begin + 1 );
			begin = end + 1;
		}

		errorList = new EmaConfigErrorList();
		tmp = const_cast< XMLnode*>( tmp->_children->find( nodeName, &errorList ) );

		delete errorList;

		if ( tmp )
		{
			if ( foundPath.length() )
				foundPath += nodeSeparator;
			foundPath += nodeName;
			if ( itemToRetrieve == foundPath )
				return tmp;
		}
		else
			return 0;
	}
	return 0;
}

class EmaConfigImpl;

class AdminReqMsg
{
public :

	AdminReqMsg( EmaConfigImpl& );

	virtual ~AdminReqMsg();

	AdminReqMsg& set( RsslRequestMsg* );

	AdminReqMsg& clear();

	RsslRequestMsg* get();

	bool hasServiceName();

	void setServiceName( const EmaString& serviceName );

	const EmaString& getServiceName();

private :

	EmaConfigImpl&		_emaConfigImpl;
	RsslRequestMsg		_rsslMsg;
	RsslBuffer			_name;
	RsslBuffer			_header;
	RsslBuffer			_attrib;
	RsslBuffer			_payload;
	bool				_hasServiceName;
	EmaString			_serviceName;
};

class AdminRefreshMsg
{
public:

	AdminRefreshMsg(EmaConfigBaseImpl*);
	AdminRefreshMsg(const AdminRefreshMsg&);
	AdminRefreshMsg& operator=(const AdminRefreshMsg&);

	virtual ~AdminRefreshMsg();

	AdminRefreshMsg& clear();

	AdminRefreshMsg& set(RsslRefreshMsg*);

	RsslRefreshMsg* get();

private:

	EmaConfigBaseImpl* _pEmaConfigImpl;
	RsslRefreshMsg		_rsslMsg;
	RsslBuffer			_name;
	RsslBuffer			_header;
	RsslBuffer			_attrib;
	RsslBuffer			_payload;
	RsslBuffer			_statusText;
};

struct PortSetViaFunctionCall
{
	bool userSet;
	EmaString userSpecifiedValue;

	PortSetViaFunctionCall()
	{
		userSet = false;
		userSpecifiedValue = EmaString();
	}
};

class EmaConfigBaseImpl
{
public:

	EmaConfigBaseImpl( const EmaString & );
	~EmaConfigBaseImpl();

	void clear();

	const XMLnode* getNode(const EmaString& itemToRetrieve) const;

	template<typename T>
	bool get(const EmaString& itemToRetrieve, T& retrievedItem) const
	{
		int pos;
		for (pos = itemToRetrieve.length() - 1; pos >= 0; --pos)
			if ('|' == itemToRetrieve[pos])
				break;
		if (pos >= 0)
		{
			const XMLnode* node(getNode(itemToRetrieve.substr(0, pos)));
			if (node)
			{
				EmaVector< T > tmp;
				node->getValues(itemToRetrieve.substr(pos + 1, EmaString::npos), tmp);
				if (!tmp.empty())
				{
					retrievedItem = tmp[tmp.size() - 1];
					return true;
				}
			}
		}

		EmaString errorMsg("could not get value for item [");
		errorMsg.append(itemToRetrieve).append("]; will use available default value if not config programmatically");
		_pEmaConfig->appendErrorMessage(errorMsg, OmmLoggerClient::VerboseEnum);
		return false;
	}

	template<typename S>
	bool get(const EmaString& itemToRetrieve, EmaVector< S >& retrievedItem) const
	{
		int pos;
		for (pos = itemToRetrieve.length() - 1; pos >= 0; --pos)
			if ('|' == itemToRetrieve[pos])
				break;
		if (pos >= 0)
		{
			const XMLnode* node(getNode(itemToRetrieve.substr(0, pos)));
			if (node)
			{
				node->getValues(itemToRetrieve.substr(pos + 1, EmaString::npos), retrievedItem);
				if (!retrievedItem.empty())
					return true;
			}
		}

		EmaString errorMsg("could not get values for item [");
		errorMsg.append(itemToRetrieve).append("]; will use available default value if not config programmatically");
		_pEmaConfig->appendErrorMessage(errorMsg, OmmLoggerClient::VerboseEnum);
		return false;
	}

	template<typename T>
	bool set(const EmaString& itemToSet, const T& newValue) const
	{
		T* item(_pEmaConfig->find<T>(itemToSet));
		if (item)
		{
			*item = newValue;
			return true;
		}
		EmaString errorMsg("could not set value for item [");
		errorMsg.append(itemToSet).append("] - item not found in configuration");
		_pEmaConfig->appendErrorMessage(errorMsg, OmmLoggerClient::VerboseEnum);
		return false;
	}

	EmaConfigErrorList& configErrors()
	{
		return _pEmaConfig->errors();
	}

	void print()
	{
		_pEmaConfig->print();
		fflush(stdout);
	}

	void appendConfigError(const EmaString& text, OmmLoggerClient::Severity severityLevel)
	{
		_pEmaConfig->appendErrorMessage(text, severityLevel);
	}

	virtual void config(const Data&);

	void getLoggerName(const EmaString&, EmaString&) const;

	OmmLoggerClient::Severity readXMLconfiguration(const EmaString&);

	bool extractXMLdataFromCharBuffer(const EmaString&, const char*, int);

	void processXMLnodePtr(XMLnode*, const xmlNodePtr&);

	ConfigElement* convertEnum(const char* name, XMLnode*, const char* value, EmaString&);

	ConfigElement* createConfigElement(const char* name, XMLnode*, const char* value, EmaString&);

	bool validateConfigElement(const char*, ConfigElement::ConfigElementType) const;

	void createNameToValueHashTable();

	void getAsciiAttributeValueList(const EmaString&, const EmaString&, EmaVector< EmaString >&);

	void getEntryNodeList(const EmaString&, const EmaString&, EmaVector< XMLnode* >&);

	ProgrammaticConfigure* getProgrammaticConfigure()
	{
		return _pProgrammaticConfigure;
	}

	const EmaString& getInstanceNodeName() const
	{
		return _instanceNodeName;
	}

	void getServiceNames(const EmaString&, EmaVector< EmaString >&);

	virtual EmaString getConfiguredName() = 0;

	static void setDefaultConfigFileName(const EmaString&);

protected:

	XMLnode*				_pEmaConfig;
	ProgrammaticConfigure*	_pProgrammaticConfigure;

	EmaString				_instanceNodeName;
	EmaString				_configSessionName;

private:

	HashTable< EmaString, ConfigElement::ConfigElementType> nameToValueHashTable;
	static EmaString		defaultEmaConfigXMLFileName;
};

class EmaConfigImpl : public EmaConfigBaseImpl
{
public:

	EmaConfigImpl( const EmaString & );
	virtual ~EmaConfigImpl();

	void clear();

	void username( const EmaString& );
	void password( const EmaString& );
	void position( const EmaString& );
	void applicationId( const EmaString& );
	void applicationName( const EmaString& );
	void instanceId( const EmaString& );

	void clientId( const EmaString& );
	void clientSecret(const EmaString& );
	void tokenScope( const EmaString& );
	void takeExclusiveSignOnControl( bool );
	void tokenServiceUrl( const EmaString& );
	void tokenServiceUrlV1(const EmaString&);
	void tokenServiceUrlV2(const EmaString&);
	void serviceDiscoveryUrl( const EmaString& );

	void addAdminMsg( const ReqMsg& );

	void addAdminMsg( const RefreshMsg& );

	void getChannelName( const EmaString&, EmaString& ) const;

	void getWarmStandbyChannelName( const EmaString&, EmaString&, bool& foundProgrammaticCfg ) const;

	void getServerName(const EmaString&, EmaString&) const;

	bool getDictionaryName( const EmaString&, EmaString& ) const;

	void host( const EmaString& );

	void proxyHostName(const EmaString&);
	void proxyPort(const EmaString&);
	void securityProtocol(int);
	void proxyUserName(const EmaString&);
	void proxyPasswd(const EmaString&);
	void proxyDomain(const EmaString&);
	void objectName(const EmaString&);
	void libsslName(const EmaString&);
	void libcryptoName(const EmaString&);

	void addOAuth2Credential(const OAuth2Credential&);

	void addOAuth2Credential(const OAuth2Credential&, const OmmOAuth2ConsumerClient&);

	void addOAuth2Credential(const OAuth2Credential&, const OmmOAuth2ConsumerClient&, void*);

	void addLoginMsgCredential(const ReqMsg&, const EmaString&);

	void addLoginMsgCredential(const ReqMsg&, const EmaString&, const OmmLoginCredentialConsumerClient&);

	void addLoginMsgCredential(const ReqMsg&, const EmaString&, const OmmLoginCredentialConsumerClient&, void*);


	void libcurlName(const EmaString&);

	void sslCAStore(const EmaString&);

	void connectionType(const RsslConnectionTypes&);
	void encryptedConnectionType(const RsslConnectionTypes&);

	void protocolList(const EmaString& protocolList);

	RsslRDMLoginRequest* getLoginReq();

	RsslRequestMsg* getDirectoryReq();

	AdminReqMsg* getRdmFldDictionaryReq();

	AdminReqMsg* getEnumDefDictionaryReq();

	AdminRefreshMsg* getDirectoryRefreshMsg();

	OAuth2Credential& getOAuthCredential();


	const EmaString& getUserSpecifiedHostname() const
	{
		return _hostnameSetViaFunctionCall;
	}

	const PortSetViaFunctionCall& getUserSpecifiedPort() const
	{
		return _portSetViaFunctionCall;
	}

	const EmaString& getUserSpecifiedProxyHostname() const
	{
		return _proxyHostnameSetViaFunctionCall;
	}

	const EmaString& getUserSpecifiedProxyPort() const
	{
		return _proxyPortSetViaFunctionCall;
	}

	const int getUserSpecifiedSecurityProtocol() const
	{
		return _securityProtocolSetViaFunctionCall;
	}

	const EmaString& getUserSpecifiedObjectName()
	{
		return _objectName;
	}

	const EmaString& getUserSpecifiedLibSslName()
	{
		return _libSslName;
	}

	const EmaString& getUserSpecifiedLibCryptoName()
	{
		return _libCryptoName;
	}

	const EmaString& getUserSpecifiedLibcurlName()
	{
		return _libcurlName;
	}

	const EmaString& getUserSpecifiedProxyUserName()
	{
		return _proxyUserNameSetViaFunctionCall;
	}

	const EmaString& getUserSpecifiedProxyPasswd()
	{
		return _proxyPasswdSetViaFunctionCall;
	}

	const EmaString& getUserSpecifiedProxyDomain()
	{
		return _proxyDomainSetViaFunctionCall;
	}

	const EmaString& getUserSpecifiedSslCAStore()
	{
		return _sslCAStoreSetViaFunctionCall;
	}

	const EmaString& getUserSpecifiedTokenServiceUrlV1()
	{
		return _tokenServiceUrlV1;
	}

	const EmaString& getUserSpecifiedTokenServiceUrlV2()
	{
		return _tokenServiceUrlV2;
	}

	const EmaString& getUserSpecifiedServiceDiscoveryUrl()
	{
		return _serviceDiscoveryUrl;
	}

	EmaVector < OmmOAuth2CredentialImpl* > getOAuth2CredentialVector()
	{
		return _oAuth2Credentials;
	}

	EmaVector < LoginRdmReqMsgImpl* > getLoginCredentialVector()
	{
		return _LoginRequestMsgs;
	}

	LoginRdmReqMsgImpl& getLoginRdmReqMsg();


	virtual OmmRestLoggingClient* getOmmRestLoggingClient() const {
		return ((OmmRestLoggingClient*)NULL);
	}

	virtual void* getRestLoggingClosure() const {
		return ((void*)NULL);
	}

protected:

	LoginRdmReqMsgImpl			_loginRdmReqMsg;
	OAuth2Credential			_oAuthCredential;

	AdminReqMsg*			_pDirectoryRsslRequestMsg;
	AdminReqMsg*			_pRdmFldRsslRequestMsg;
	AdminReqMsg*			_pEnumDefRsslRequestMsg;

	AdminRefreshMsg*		_pDirectoryRsslRefreshMsg;

	EmaString				_hostnameSetViaFunctionCall;

	EmaString				_proxyHostnameSetViaFunctionCall;
	EmaString				_proxyPortSetViaFunctionCall;
	EmaString				_proxyUserNameSetViaFunctionCall;
	EmaString				_proxyPasswdSetViaFunctionCall;
	EmaString				_proxyDomainSetViaFunctionCall;
	EmaString				_sslCAStoreSetViaFunctionCall;
	int						_securityProtocolSetViaFunctionCall;

	void addLoginReqMsg( RsslRequestMsg* );

	void addDirectoryReqMsg( RsslRequestMsg* );

	void addDictionaryReqMsg( RsslRequestMsg*, const EmaString* );

	void addDirectoryRefreshMsg( RsslRefreshMsg* );

	PortSetViaFunctionCall		_portSetViaFunctionCall;

	const EmaString configFilePath;

	EmaString		_objectName;
	EmaString		_libSslName;
	EmaString		_libCryptoName;
	EmaString		_tokenServiceUrlV1;
	EmaString		_tokenServiceUrlV2;
	EmaString		_serviceDiscoveryUrl;
	EmaString		_libcurlName;

	EmaVector < OmmOAuth2CredentialImpl* > _oAuth2Credentials;

	EmaVector < LoginRdmReqMsgImpl* > _LoginRequestMsgs;

};

class EmaConfigServerImpl : public EmaConfigBaseImpl
{
public:

	EmaConfigServerImpl( const EmaString & path );
	virtual ~EmaConfigServerImpl();

	void clear();

	void addAdminMsg(const RefreshMsg&);

	void port(const EmaString&);

	void getServerName(const EmaString&, EmaString&) const;

	const PortSetViaFunctionCall& getUserSpecifiedPort() const;

	AdminRefreshMsg* getDirectoryRefreshMsg();

	bool getDirectoryName(const EmaString&, EmaString&) const;

	void libsslName(const EmaString&);
	void libcryptoName(const EmaString&);
	void libcurlName(const EmaString&);
	void serverCert(const EmaString&);
	void serverPrivateKey(const EmaString&);
	void cipherSuite(const EmaString&);
	void dhParams(const EmaString&);

	void connectionType(const RsslConnectionTypes& connectionType);

	const EmaString& getUserSpecifiedLibSslName()
	{
		return _libSslName;
	}

	const EmaString& getUserSpecifiedLibCryptoName()
	{
		return _libCryptoName;
	}

	const EmaString& getUserSpecifiedLibCurlName()
	{
		return _libCurlName;
	}

	const EmaString& getUserSpecifiedServerCert()
	{
		return _serverCert;
	}

	const EmaString& getUserSpecifiedServerPrivateKey()
	{
		return _serverPrivateKey;
	}

	const EmaString& getUserSpecifiedCipherSuite()
	{
		return _cipherSuite;
	}

	const EmaString& getUserSpecifiedDhParams()
	{
		return _dhParams;
	}

protected:

	void addDirectoryRefreshMsg(RsslRefreshMsg*);

	PortSetViaFunctionCall		_portSetViaFunctionCall;
	AdminRefreshMsg*			_pDirectoryRsslRefreshMsg;

	EmaString		_libSslName;
	EmaString		_libCryptoName;
	EmaString		_libCurlName;
	EmaString		_serverCert;
	EmaString		_serverPrivateKey;
	EmaString		_cipherSuite;
	EmaString		_dhParams;

};

}

}

}

#endif // __refinitiv_ema_access_EmaConfigImpl_h
