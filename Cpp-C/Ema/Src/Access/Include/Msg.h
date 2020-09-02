/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright (C) 2019 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_Msg_h
#define __thomsonreuters_ema_access_Msg_h

/**
	@class rtsdk::ema::access::Msg Msg.h "Access/Include/Msg.h"
	@brief Msg class is a parent class for all message representing classes.

	\remark All methods in this class are \ref SingleThreaded.

	@see ComplexType,
		Attrib,
		Payload,
		AckMsg,
		GenericMsg,
		PostMsg,
		RefreshMsg,
		RequestMsg,
		StatusMsg,
		UpdateMsg,
		EmaBuffer,
		EmaString
*/

#include "Access/Include/Attrib.h"
#include "Access/Include/Payload.h"
#include "Rdm/Include/EmaRdm.h"

namespace rtsdk {

namespace ema {

namespace access {

class MsgDecoder;
class MsgEncoder;

class EMA_ACCESS_API Msg : public ComplexType
{
public :

	///@name Accessors
	//@{
	/** Indicates presence of the MsgKey.
		@return true if name, name type, service id, service name, id, filter, or attribute is set; false otherwise
	*/
	bool hasMsgKey() const;

	/** Indicates presence of the Name within the MsgKey.
		@return true if name is set; false otherwise
	*/
	bool hasName() const;

	/** Indicates presence of the NameType within the MsgKey.
		@return true if name type is set; false otherwise
	*/
	bool hasNameType() const;

	/** Indicates presence of the ServiceId within the MsgKey.
		@return true if service id is set; false otherwise
	*/
	bool hasServiceId() const;

	/** Indicates presence of the Identifier within the MsgKey.
		@return true if Id is set; false otherwise
	*/
	bool hasId() const;

	/** Indicates presence of the Filter within the MsgKey.
		@return true if filter is set; false otherwise
	*/
	bool hasFilter() const;

	/** Indicates presence of the ExtendedHeader.
		@return true if extendedHeader is set; false otherwise
	*/
	bool hasExtendedHeader() const;

	/** Returns the StreamId, which is the unique open message stream identifier on the wire.
		@return stream id value
	*/
	Int32 getStreamId() const;

	/** Returns the DomainType, which is the unique identifier of a domain.
		@return domain type value
	*/
	UInt16 getDomainType() const;

	/** Returns the Name within the MsgKey.
		@throw OmmInvalidUsageException if hasName() returns false
		@return EmaString containing name
	*/
	const EmaString& getName() const;

	/** Returns the NameType within the MsgKey.
		@throw OmmInvalidUsageException if hasNameType() returns false
		@return name type value
	*/
	UInt8 getNameType() const;

	/** Returns the ServiceId within the MsgKey.
		@throw OmmInvalidUsageException if hasServiceId() returns false
		@return service id value
	*/
	UInt32 getServiceId() const;

	/** Returns the Identifier within the MsgKey.
		@throw OmmInvalidUsageException if hasId() returns false
		@return id value
	*/
	Int32 getId() const;

	/** Returns the Filter within the MsgKey.
		@throw OmmInvalidUsageException if hasFilter() returns false
		@return filter value
	*/
	UInt32 getFilter() const;

	/** Returns the ExtendedHeader.
		@throw OmmInvalidUsageException if hasExtendedHeader() returns false
		@return EmaBuffer containing extendedHeader info value
	*/
	const EmaBuffer& getExtendedHeader() const;

	/** Returns the contained attributes Data based on the attributes DataType.
		\remark Attrib contains no data if Attrib::getDataType() returns DataType::NoDataEnum
		@return reference to Attrib object
	*/
	const Attrib& getAttrib() const;

	/** Returns the contained payload Data based on the payload DataType.
		\remark Payload contains no data if Payload::getDataType() returns DataType::NoDataEnum
		@return reference to Payload object
	*/
	const Payload& getPayload() const;
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~Msg();
	//@}

protected :

	friend class OmmConsumerConfigImpl;
	friend class OmmNiProviderConfigImpl;
	friend class EmaConfigImpl;
	friend class EmaConfigServerImpl;
	friend class OmmNiProviderImpl;
	friend class OmmIProviderImpl;
	friend class ItemCallbackClient;
	friend class DirectoryItem;
	friend class LoginItem;
	friend class NiProviderLoginItem;
	friend class SingleItem;
	friend class NiProviderSingleItem;
	friend class IProviderSingleItem;
	friend class DictionaryItem;
	friend class NiProviderDictionaryItem;
	friend class IProviderDictionaryItem;
	friend class ProviderItem;
	friend class TunnelItem;
	friend class TunnelStreamLoginReqMsgImpl;
	friend class SubItem;
	friend class MsgDecoder;

	Msg();

	const Encoder& getEncoder() const;
	bool hasEncoder() const;
	void setDecoder( MsgDecoder* );
	bool hasDecoder() const;

	MsgDecoder*		_pDecoder;
	MsgEncoder*		_pEncoder;

	Attrib			_attrib;
	Payload			_payload;

private :

	Msg( const Msg& );
	Msg& operator=( const Msg& );
};

}

}

}

#endif // __thomsonreuters_ema_access_Msg_h
