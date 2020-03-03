/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
*/

#ifndef __thomsonreuters_ema_access_DictionaryHandler_h
#define __thomsonreuters_ema_access_DictionaryHandler_h

#include "rtr/rsslReactor.h"
#include "DictionaryCallbackClient.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmServerBaseImpl;
class ItemInfo;

class DictionaryPayload : public ListLinks< DictionaryPayload >
{
public:

	enum DictionaryType
	{
		RDM_FIELD_DICTIONARY,
		ENUM_TYPE
	};

	DictionaryPayload(Dictionary* dictionary, DictionaryType dictionaryType, bool ownDictionary);

	virtual ~DictionaryPayload();

	DictionaryType getDictionaryType() const;

	Dictionary* getDictionary() const;

private:

	Dictionary*			_pDictionary;
	DictionaryType		_dictionaryType;
	bool				_ownDictionary;
};

class DictionaryHandler
{
public:

	enum DictionaryRejectEnum
	{
		DICTIONARY_INVALID_MESSAGE,
		DICTIONARY_NOT_LOADED,
		DICTIONARY_ENCODING_FAILED,
		USER_IS_NOT_LOGGED_IN,
		SERVICE_ID_NOT_FOUND,
		DICTIONARY_NAME_NOT_FOUND
	};

	static const int INITIAL_DICTIONARY_STATUS_MSG_SIZE;

	static RsslReactorCallbackRet dictionaryCallback(RsslReactor*, RsslReactorChannel*, RsslRDMDictionaryMsgEvent*);

	static DictionaryHandler* create( OmmServerBaseImpl*);

	void loadDictionaryFromFile();

	static bool sendDictionaryResponse(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDictionaryMsgEvent* pRDMDictionaryMsgEvent);

	static RsslReturnCodes sendFieldDictionaryResponse(RsslReactor* reactor, RsslReactorChannel* reactorChannel, RsslRDMDictionaryMsg* dictionaryRequest, Dictionary*, RsslErrorInfo* error);

	static RsslReturnCodes sendEnumTypeDictionaryResponse(RsslReactor* reactor, RsslReactorChannel* reactorChannel, RsslRDMDictionaryMsg* dictionaryRequest, Dictionary*, RsslErrorInfo* error);

	static RsslReturnCodes sendRequestReject(RsslReactor* reactor, RsslReactorChannel* reactorChannel, RsslRDMDictionaryMsgEvent*, DictionaryRejectEnum reason, RsslErrorInfo* error, bool traceMessage = true);

	static void destroy(DictionaryHandler*&);

	DictionaryPayload* getDictionaryPayload(const EmaString&) const;

	void initialize();

	const EmaVector< ItemInfo* >&		getDictionaryItemList();

	void addItemInfo(ItemInfo*);

	void removeItemInfo(ItemInfo*);

	Dictionary* getDictionaryByServiceId(UInt64 serviceId);

	/* This is used only when the application uses the user control for the source directory */
	Dictionary* getDefaultDictionary();

private:

	static const EmaString			_clientName;

	class EmaStringPtrHasher
	{
	public:
		size_t operator()(const EmaString&) const;
	};

	class EmaStringPtrEqual_To
	{
	public:
		bool operator()(const EmaString&, const EmaString&) const;
	};

	class UInt64rHasher
	{
	public:
		UInt64 operator()(const UInt64&) const;
	};

	class UInt64Equal_To
	{
	public:
		bool operator()(const UInt64&, const UInt64&) const;
	};

	bool addDictionary(const EmaString&, DictionaryPayload*);

	void removeDictionary(const EmaString&);

	typedef HashTable< EmaString, DictionaryPayload*, EmaStringPtrHasher, EmaStringPtrEqual_To > DictionaryInfoHash;
	typedef HashTable< UInt64, Dictionary*, UInt64rHasher, UInt64Equal_To> ServiceDictionaryByIdHash;

	EmaList<DictionaryPayload*>	_dictionaryInfoList;
	DictionaryInfoHash          _dictionaryInfoHash;

	ServiceDictionaryByIdHash   _serviceDictionaryByIdHash;

	OmmServerBaseImpl*          _pOmmServerBaseImpl;
	RsslErrorInfo               _errorInfo;
	RsslRDMDictionaryMsg        _rdmDictionaryRefresh;
	RsslRDMDictionaryMsg        _rdmDictionaryStatus;
	UInt32                      _maxFieldDictFragmentSize;
	UInt32                      _maxEnumTypeFragmentSize;
	bool                        _apiAdminControl;

	EmaVector< ItemInfo* >		_itemInfoList;

	LocalDictionary*			_pDefaultLocalDictionary; // This is default LocalDictionary for Provider
	
	DictionaryHandler(OmmServerBaseImpl*);
	virtual ~DictionaryHandler();

	DictionaryHandler();
	DictionaryHandler(const DictionaryHandler*);
	DictionaryHandler& operator=(const DictionaryHandler*);

};

}

}

}

#endif // __thomsonreuters_ema_access_DictionaryHandler_h

