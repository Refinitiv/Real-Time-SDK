/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmConsumerConfigImpl_h
#define __thomsonreuters_ema_access_OmmConsumerConfigImpl_h

#ifdef WIN32
#include "direct.h"
#endif

#include "OmmConsumerConfig.h"
#include "OmmLoggerClient.h"
#include "DictionaryCallbackClient.h"
#include "HashTable.h"
#include "ElementList.h"

#include "libxml/parser.h"
#include "ConfigErrorHandling.h"

#include "rtr/rsslRDMLoginMsg.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class XMLnode;
class OmmConsumerActiveConfig;

class ConfigElement {
public:
	typedef enum {
		ConfigElementTypeInt64 = 0,
		ConfigElementTypeUInt64 = 1,
		ConfigElementTypeAscii = 2,
		ConfigElementTypeEnum = 3,
		ConfigElementTypeBool = 4,
	} ConfigElementType;
	ConfigElement(const EmaString & name, XMLnode * parent) : _name(name), _parent(parent) {}
	virtual ~ConfigElement() {}
	virtual void print() = 0;
	const EmaString & name() const
	{
		return _name;
	}
	XMLnode * parent() const
	{
		return _parent;
	}
	void reParent( XMLnode * p )
	{
		_parent = p;
	}
	bool operator== ( const ConfigElement & ) const;
	void appendErrorMessage( EmaString & , OmmLoggerClient::Severity );
	EmaString changeMessage( const EmaString &, const ConfigElement &) const;
protected:
	virtual ConfigElementType type() const = 0;
	EmaString _name;
	XMLnode * _parent;
};

template<typename T>
class XMLConfigElement : public ConfigElement {
public:
	XMLConfigElement(const EmaString & name, XMLnode * parent, ConfigElementType type, const T & value) : ConfigElement(name, parent), _type(type), _value(value) {
		XMLnode * p(parent);
	}
	T * value() { return &_value; }
	virtual ConfigElementType type() const
	{
		return _type;
	}
	void print()
	{
		printf("%s: %d (parent %p)", _name.c_str(), _value, _parent);
		fflush(stdout);
	}
	bool operator== ( const XMLConfigElement & ) const;
	EmaString changeMessage( const EmaString &, const XMLConfigElement< T > & ) const;
private:
	ConfigElementType _type;
	T _value;
};

class ConfigElementList {
public:
	ConfigElementList() : theList(0) {}
	void add(ConfigElement * element)
	{
		Node * n(new Node(element));
		if (theList) {
			Node *last(0);
			Node *q;
			for (q = theList; q; q = q->next)
			{
				if (element->name() == q->e->name())
				{
					q->e = element;
					break;
				}
				last = q;
			}

			if (! q) {
				last->next = n;
				n->next = 0;
			}
		}
		else
			theList = n;
	}
	void appendAttributes(ConfigElementList * attributes, bool emptyList = false)
	{
		if (theList) {
			Node *nextAttribute(0);
			for (Node *p = attributes->theList; p; p = nextAttribute) 
			{
				nextAttribute = p->next;

				Node *last(0);
				Node *q;
				for (q = theList; q; q = q->next)
				{
					if ( p->e->name() == q->e->name() )
					{
						if ( p->e->name() == "Name")
							delete p->e;
						else
						{
							if (! (*(p->e) == *(q->e) ) )
							{
								EmaString actualName;
								if (theList->e->name() == "Name")
								{
									XMLConfigElement<EmaString> * tmp(dynamic_cast<XMLConfigElement<EmaString> *>( theList->e ) );
									if (tmp)
										actualName = *(tmp->value());
								}
								EmaString errorMsg(q->e->changeMessage(actualName, *(p->e)));
								ConfigElement * toFree( q->e );
								q->e = p->e;
								p->e->reParent(toFree->parent());
								p->e->appendErrorMessage(errorMsg, OmmLoggerClient::VerboseEnum);
								delete( toFree );
							}
							else
								delete p->e;
						}
						if (attributes->theList == p)
							attributes->theList = nextAttribute;
						delete p;
						break;
					}
					last = q;
				}

				if (! q) {
					p->e->reParent(last->e->parent());
					last->next = p;
					p->next = 0;
				}
			}
			// at this point, we've either assigned p->e to an existing item or appended p to theList
			attributes->theList = 0;
			delete attributes->theList;
		}
		else
			theList = attributes->theList;
		
		if (emptyList)
			attributes->theList = 0;
	}

	template<typename T>
	T *
	find( const EmaString & itemToRetrieve ) const
	{
		for ( Node * p = theList; p; p = p->next ) {
			if (p->e->name() == itemToRetrieve) {
				XMLConfigElement<T> * e = dynamic_cast<XMLConfigElement<T> *>(p->e);
				if ( e )
					return e->value();
				if ( p->e->parent() )
				{
					EmaString errorMsg( "dynamic cast failed for [" );
					errorMsg.append( itemToRetrieve.c_str() ).append( "]" );
					p->e->appendErrorMessage(errorMsg, OmmLoggerClient::ErrorEnum);
				}
				return 0;
			}
		}
		return 0;
	}
	void print(int tabs = 0) {
		for (Node *p = theList; p; p = p->next)
		{
			for (int i = 0; i < tabs; ++i)
				printf("\t");
			if ( p->e )
				p->e->print();
			printf("\n");
			fflush(stdout);
		}
	}
	~ConfigElementList()
	{
		Node * q;
		for (Node * p = theList; p; p = q) {
			q = p->next;
			delete(p->e);
			delete(p);
		}
	}
private:
	struct Node {
		Node(ConfigElement * e) : e(e), next(0) {}
		ConfigElement * e;
		Node * next;
	};
	Node * theList;
};

template<typename T>
class xmlList {
public:
	xmlList() : theList(0) {}
	bool insert(T * element)
	{
		if (theList)
		{
			 {
			 	for (Node * p = theList; p; p = p->next)
			 		if (p->e->name() == element->name())
					{
			 			if ( p->e->merge(element) )
							return true;
					}
			 }

			Node *p = theList;
			while (p->next)
				p = p->next;
			p->next = new Node(element);
		}
		else
		{
			theList = new Node(element);
		}
		return false;
	}

	void merge(xmlList<T> * other)
	{
		bool merged;
		xmlList<T> * unmerged(new xmlList<T>);
		Node * nextNodeToMerge(other->theList);
		while (nextNodeToMerge) {
			Node *p = nextNodeToMerge;
			nextNodeToMerge = nextNodeToMerge->next;
			merged = false;
			for (Node * q = theList; q; q = q->next)
			{
				if ( p->e->name() == q->e->name() )
				{
					if ( q->e->merge(p->e) ) {
						merged = true;
						delete p;
						break;
					}
				}
			}

			if ( ! merged )
			{
				p->next = 0;
				if ( unmerged->size() ) {
					Node * u = unmerged->theList;
					while ( u->next )
						u = u->next;
					u->next = p;
				}
				else
					unmerged->theList = p;
			}
		}

		if ( unmerged->size() ) {
			Node * ptr = theList;
			while ( ptr->next )
				ptr = ptr->next;
			ptr->next = unmerged->theList;
			for ( ptr = unmerged->theList; ptr; ptr = ptr->next )
			{
				ptr->e->reparent( theList->e->parent() );
			}
			unmerged->theList = 0;
		}

		delete unmerged;
		other->theList = 0;
	}

	void print(int tabs = 0)
	{
		for (Node *p = theList; p; p = p->next)
		{
			for (int i = 0; i < tabs; ++i)
				printf("\t");
			p->e->print(tabs);
			fflush(stdout);
		}
	}

	const T * find( const EmaString & nodeName, EmaConfigErrorList ** errors )
	{
		static const char * dot(".");
		int dotPosition(const_cast<EmaString &>(nodeName).find(dot, 0));
		if (dotPosition == -1)
		{
			for( Node * p = theList; p; p = p->next )
				if (p->e->name() == nodeName)
					return p->e;
		}
		else 
		{
			EmaString actualNodeName = nodeName.substr(0, dotPosition);
			EmaString name = nodeName.substr(dotPosition + 1, nodeName.length() - dotPosition - 1);
			T * retVal(0);
			int arrayLoc(0);
			for (Node * p = theList; p; p = p->next)
			{
					if (p->e->name() == actualNodeName) {
						EmaString target("Name");
						EmaString targetValue;
						if (p->e->get(target, targetValue) && targetValue == name)
							return p->e;
						else if ( p->e->errors().count() )
						{
                            if ( ! *errors )
                                *errors = new EmaConfigErrorList();
                            (*errors)->add( p->e->errors() );
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
		unsigned int retVal(0);
		for (const Node * p = theList; p; p = p->next)
			++retVal;
		return retVal;
	}
	bool empty()
	{
		return ( theList == 0 );
	}
	const T * getFirst() { return theList->e; }
	const T * getNext( const T * current )
	{
		for ( Node * p = theList; p; p = p->next )
		{
			if ( p->e == current )
				return p->next ? p->next->e : 0;
		}
		return 0;
	}
	~xmlList()
	{
		Node * q;
		for (Node * p = theList; p; p = q) {
			q = p->next;
			delete(p->e);
			delete(p);
		}
	}
private:
	struct Node {
		Node(T * e) : e(e), next(0) {}
		T * e;
		Node * next;
	};
	Node * theList;
};

class XMLnode {
public:
	XMLnode(const EmaString & name, int level, XMLnode * parent) : _name(name), _parent(parent), _attributes(new ConfigElementList()), _children(new xmlList<XMLnode>), _level(level) {}
	~XMLnode()
	{
		delete ( _attributes );
		delete ( _children );
		_errors.clear();
	}
	bool addChild(XMLnode * child)
	{
		return _children->insert(child);
	}
	void addAttribute(ConfigElement * element)
	{
		_attributes->add(element);
	}
	void appendAttributes(ConfigElementList * attributes, bool emptyList = false)
	{
		_attributes->appendAttributes(attributes, emptyList);
	}
	ConfigElementList * attributes()
	{
		return _attributes;
	}
	template<typename T>
	bool get(const EmaString & itemToRetrieve, T & retrievedItem)
	{
		T * found(find<T>(itemToRetrieve));
		if (found) {
			retrievedItem = *found;
			return true;
		}
		return false;
	}
	
    template<typename T>
    T *
    find(const EmaString & itemToRetrieve) {
       	// format: zero or more children followed by an attribute
		// each element separated by an '|' symbol
		// for duplicate children names, which are common: childname.value of attribute name (i.e., each child must have a name)
		// value must have Ascii type
		// e.g., Sessions|session.Session_1|logger
		Int32 substrBeginningPosition(0);
		Int32 substrEndingPosition(0);
		const char * nodeSeparator("|");
		const char * nameSeparator(".");
		EmaString attributeName;

		substrEndingPosition = static_cast<EmaString>(itemToRetrieve).find(nodeSeparator, substrBeginningPosition);
		if ( substrEndingPosition == -1 )
		{
			return _attributes->find<T>( itemToRetrieve );
		}
		else
		{
			EmaString nodeName;
			nodeName = static_cast<EmaString>(itemToRetrieve).substr(substrBeginningPosition, substrEndingPosition - substrBeginningPosition);
			EmaConfigErrorList  * errors(0);
			const XMLnode * tmp = _children->find(nodeName, &errors);
			if (tmp) {
				T * retVal(const_cast<XMLnode *>(tmp)->find<T>(itemToRetrieve.substr(substrEndingPosition + 1, itemToRetrieve.length() - substrEndingPosition - 1)));
				if (! retVal) {
					_errors.add(const_cast<XMLnode *>(tmp)->errors());
					(const_cast<XMLnode *>(tmp)->errors().clear());
				}
				return retVal;
			}
			else if ( errors )
				_errors.add(*errors);
			return 0;
		}
	}

	void print(int tabs = 0);
	const EmaString & name() { return _name; }
	int level() { return _level; }
	bool merge(XMLnode * newNode)
	{
		// we already know that node names match; check if we just need to merge subnodes.
		EmaString errorMsg;
		EmaString * name = _attributes->find<EmaString>( "Name" );
		EmaString * newNodeName = newNode->_attributes->find<EmaString>( "Name" );
		if ( name && newNodeName ) {
			if ( *name == *newNodeName ) {
				_attributes->appendAttributes(newNode->attributes());
				return true;
			}
			else
				return false;
		}

		// at this point, we have two nodes with the same name, but they are not subnodes. Merge
		// attributes and then children
		_attributes->appendAttributes(newNode->attributes());
		_children->merge(newNode->_children);
		return true;
	}
	unsigned int size()
	{
		return _children->size(); 
	}
	XMLnode * parent()
	{
		return _parent;
	}
	void reparent(XMLnode * p)
	{
		_parent = p;
	}
	void appendErrorMessage( const EmaString & errorMsg, OmmLoggerClient::Severity severity);
	int errorCount() { return _errors.count(); }
	EmaConfigErrorList & errors() { return _errors; }
	
	class NameString : public EmaString, public ListLinks< NameString >
	{
	public:
		NameString() : EmaString() {}
		NameString( EmaString & n ) : EmaString( n ) {}
	};

	void verifyDefaultConsumer();

	void getNames( EmaList< NameString > & theNames )
	{
		if ( _children && ! _children->empty() )
		{
			EmaString searchString( "Name" );
			const XMLnode * child( _children->getFirst() );
			while ( child )
			{
				EmaString name;
				bool found( const_cast< XMLnode * >(child)->get< EmaString >( searchString, name ) );
				if ( found )
				{
					NameString * nS( new NameString( name ) );
					theNames.insert( nS );
				}
				child = _children->getNext( child );
			}
		}
	}

private:
	EmaString _name;
	XMLnode * _parent;
	ConfigElementList * _attributes;
	xmlList<XMLnode> * _children;
	int _level;
	EmaConfigErrorList _errors;
};

template< >
inline XMLnode *
XMLnode::find< XMLnode >(const EmaString & itemToRetrieve )
{
	Int32 begin(0), end;
	const char * nodeSeparator( "|" );
	Int32 length( itemToRetrieve.length() );
	EmaString nodeName;
	XMLnode * tmp( this );
	EmaConfigErrorList * e( new EmaConfigErrorList );
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

		tmp = const_cast< XMLnode *>( tmp->_children->find( nodeName, &e ) );
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

class OmmConsumerConfigImpl;

class AdminReqMsg
{
public :

	AdminReqMsg( OmmConsumerConfigImpl& );
	virtual ~AdminReqMsg();

	AdminReqMsg& set( RsslRequestMsg* );

	AdminReqMsg& clear();

	RsslRequestMsg* get();

private :

	OmmConsumerConfigImpl&	_ommConsConfigImpl;
	RsslRequestMsg			_rsslMsg;
	RsslBuffer				_name;
	RsslBuffer				_header;
	RsslBuffer				_attrib;
	RsslBuffer				_payload;
};

class LoginRdmReqMsg
{
public :

	LoginRdmReqMsg( OmmConsumerConfigImpl& );

	virtual ~LoginRdmReqMsg();

	LoginRdmReqMsg& clear();

	LoginRdmReqMsg& set( RsslRequestMsg* );

	RsslRDMLoginRequest* get();

	LoginRdmReqMsg& username( const EmaString& );

	LoginRdmReqMsg& position( const EmaString& );

	LoginRdmReqMsg& password( const EmaString& );

	LoginRdmReqMsg& applicationId( const EmaString& );

	LoginRdmReqMsg& applicationName( const EmaString& );

private :

	OmmConsumerConfigImpl&	_ommConsConfigImpl;
	EmaString				_username;
	EmaString				_password;
	EmaString				_position;
	EmaString				_applicationId;
	EmaString				_applicationName;
	RsslRDMLoginRequest		_rsslRdmLoginRequest;
};

class ProgrammaticConfigure
{
public:
	ProgrammaticConfigure(const Map &, EmaConfigErrorList& );

	void addConfigure(const Map &);

	bool getDefaultConsumer(EmaString &);

	bool getActiveChannelName( const EmaString&, EmaString& );

	bool getActiveLoggerName( const EmaString&, EmaString& );

	bool getActiveDictionaryName( const EmaString&, EmaString& );

	void retrieveConsumerConfig( const EmaString&, OmmConsumerActiveConfig& );

	void retrieveChannelConfig( const EmaString&, OmmConsumerActiveConfig&, bool );

	void retrieveLoggerConfig( const EmaString&, OmmConsumerActiveConfig& );

	void retrieveDictionaryConfig( const EmaString&, OmmConsumerActiveConfig& );

private:

	static bool retrieveDefaultConsumer(const Map &, EmaString&);

	static void retrieveDependencyNames( const Map&, const EmaString&, UInt8& flags, EmaString&, EmaString&, EmaString& );

	static void retrieveConsumer( const Map&, const EmaString&, EmaConfigErrorList&, OmmConsumerActiveConfig& );

	static void retrieveChannel( const Map&, const EmaString&, EmaConfigErrorList&, OmmConsumerActiveConfig&, bool );

	static void retrieveLogger( const Map&, const EmaString&, EmaConfigErrorList&, OmmConsumerActiveConfig& );

	static void retrieveDictionary( const Map&, const EmaString&, EmaConfigErrorList&, OmmConsumerActiveConfig& );

	void clear();

	ProgrammaticConfigure(const ProgrammaticConfigure &);
	ProgrammaticConfigure & operator=(const ProgrammaticConfigure&);

	EmaString _consumerName;
	EmaString _channelName;
	EmaString _loggerName;
	EmaString _dictionaryName;

	bool _loadnames;
	UInt8 _nameflags;

	EmaVector<const Map*> _configList;

	EmaConfigErrorList& _emaConfigErrList;
};

class OmmConsumerConfigImpl
{
public:

	OmmConsumerConfigImpl();
	virtual ~OmmConsumerConfigImpl();

	void clear();

	void username( const EmaString& ); 
	void password( const EmaString& ); 
	void position( const EmaString& ); 
	void applicationId( const EmaString& );
	void applicationName( const EmaString& );

	void addAdminMsg( const ReqMsg& ); 

	void operationModel( OmmConsumerConfig::OperationModel ); 
	void config( const Data& ); 

	OmmConsumerConfig::OperationModel getOperationModel() const;

	void consumerName( const EmaString& );
	EmaString getConsumerName() const;
	EmaString getChannelName( const EmaString& ) const;
	EmaString getLoggerName( const EmaString& ) const;
	EmaString getDictionaryName( const EmaString& ) const;

	void host( const EmaString& ); 

	const EmaString& getHostname() const;
	const EmaString& getPort() const;

	template<typename T>
	bool get(const EmaString & itemToRetrieve, T & retrievedItem) const
	{
		bool retVal(_pEmaConfig->get<T>(itemToRetrieve, retrievedItem));
		if (! retVal) {
			EmaString errorMsg("could not get value for item [");
			errorMsg.append(itemToRetrieve).append("]; will use default value if available");
			_pEmaConfig->appendErrorMessage(errorMsg, OmmLoggerClient::VerboseEnum);
		}
		return retVal;
	}

	template<typename T>
	bool set(const EmaString & itemToSet, const T & newValue) const
	{
		T * item( _pEmaConfig->find<T>( itemToSet ) );
		if ( item ) {
			*item = newValue;
			return true;
		}
		EmaString errorMsg("could not set value for item [");
		errorMsg.append(itemToSet ).append( "] - item not found in configuration" );
		_pEmaConfig->appendErrorMessage(errorMsg, OmmLoggerClient::VerboseEnum);
		return false;
	}

	OmmLoggerClient::Severity readXMLconfiguration(const EmaString &);
	bool extractXMLdataFromCharBuffer(const EmaString &, const char *, int);
	void processXMLnodePtr(XMLnode *, const xmlNodePtr &);

	EmaConfigErrorList & configErrors()
	{
		return _pEmaConfig->errors();
	}
    void print()
	{
		_pEmaConfig->print();
		fflush(stdout);
	}

	void appendConfigError( const EmaString& text, OmmLoggerClient::Severity severityLevel )
	{
		_pEmaConfig->appendErrorMessage( text, severityLevel );
	}

	RsslRDMLoginRequest* getLoginReq();
	RsslRequestMsg* getDirectoryReq();
	RsslRequestMsg* getRdmFldDictionaryReq();
	RsslRequestMsg* getEnumDefDictionaryReq();

	EmaString & getUserSpecifiedHostname() { return _hostnameSetViaFunctionCall; }
	EmaString & getUserSpecifiedPort() { return _portSetViaFunctionCall; }
	ProgrammaticConfigure * pProgrammaticConfigure () { return _pProgrammaticConfigure; }

private:
	OmmConsumerConfig::OperationModel		_operationModel;

	UInt32					_handleHashTableSize;

	XMLnode*				_pEmaConfig;

	LoginRdmReqMsg			_loginRdmReqMsg;
	AdminReqMsg*			_pDirectoryReqRsslMsg;
	AdminReqMsg*			_pRdmFldReqRsslMsg;
	AdminReqMsg*			_pEnumDefReqRsslMsg;

	EmaString               _hostnameSetViaFunctionCall;
	EmaString               _portSetViaFunctionCall;

	ProgrammaticConfigure*       _pProgrammaticConfigure;

	void addLoginReqMsg( RsslRequestMsg* );
	void addDirectoryReqMsg( RsslRequestMsg* );
	void addDictionaryReqMsg( RsslRequestMsg* );

	ConfigElement* convertEnum( const char* name, XMLnode*, const char* value, EmaString& );
	ConfigElement* createConfigElement( const char * name, XMLnode*, const char* value, EmaString & );
	bool validateConfigElement( const char *, ConfigElement::ConfigElementType );
	void createNameToValueHashTable();
	void verifyXMLdefaultConsumer();
	HashTable< EmaString, ConfigElement::ConfigElementType> nameToValueHashTable;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmConsumerConfigImpl_h
