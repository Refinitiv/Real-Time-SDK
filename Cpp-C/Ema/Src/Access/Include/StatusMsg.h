/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright (C) 2019 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_StatusMsg_h
#define __thomsonreuters_ema_access_StatusMsg_h

/**
	@class thomsonreuters::ema::access::StatusMsg StatusMsg.h "Access/Include/StatusMsg.h"
	@brief StatusMsg conveys item state information.

	StatusMsg is used to convey item state information, permission change or item group id change.

	The following code snippet shows receiving and processing of StatusMsg.

	\code

	class AppClient ; public OmmConsumerClient
	{
		...

		void onStatusMsg( const StatusMsg& , const OmmConsumerEvent& );
	};

	void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& event )
	{
		cout << "Item's handle = " << event.getHandle() << "\n";

		if ( statusMsg.hasName() )
			cout << "Item's name = " << statusMsg.getName() << "\n";

		if ( statusMsg.hasServiceName() )
			cout << "Item's name = " << statusMsg.getServiceName() << "\n";

		if ( statusMsg.hasPermissionData() )
		{
			const EmaBuffer& permData = statusMsg.getPermissionData();

			...
		}

		if ( statusMsg.hasState() )
		{
			const OmmState& itemState = statusMsg.getState();

			...
		}
	}

	\endcode
			
	\remark Calling get***() method on an optional member of StatusMsg must be preceded by a
			call to respective has***() method.
	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform setting and getting of information from StatusMsg.
	\remark Objects of this class are not cache-able.
	\remark	Decoding of just encoded StatusMsg in the same application is not supported.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		Msg,
		EmaString,
		EmaBuffer
*/

#include "Access/Include/Msg.h"
#include "Access/Include/OmmState.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class EMA_ACCESS_API StatusMsg : public Msg
{
public :

	///@name Constructor
	//@{
	/** Constructs StatusMsg.
	*/
	StatusMsg();

	/** Copy constructor.
		\remark this is used to copy and process StatusMsg outside of EMA's callback methods.
		\remark this method does not support passing in just encoded StatusMsg in the application space.
	*/
	StatusMsg( const StatusMsg& other );
	//@}

	///@name Constructor
	//@{
	/** Destructor.
	*/
	virtual ~StatusMsg();
	//@}

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::StatusMsgEnum
	*/
	DataType::DataTypeEnum getDataType() const;

	/** Returns the Code, which indicates a special state of a DataType.
		@return Data::NoCodeEnum
	*/
	Data::DataCode getCode() const;

	/** Returns a buffer that in turn provides an alphanumeric null-terminated hexadecimal string representation.
		@return a buffer with the message hex information
	*/
	const EmaBuffer& getAsHex() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EmaString& toString() const;

	/** Indicates presence of ItemGroup.
		\remark Item Group is an optional member of StatusMsg
		@return true if Item Group information is set; false otherwise
	*/
	bool hasItemGroup() const;

	/** Indicates presence of State.
		\remark Item State is an optional member of StatusMsg
		@return true if state information is set; false otherwise
	*/
	bool hasState() const;

	/** Indicates presence of PermissionData.
		\remark permission data is optional member of StatusMsg
		@return true if permission data is set; false otherwise
	*/
	bool hasPermissionData() const;

	/** Indicates presence of PublisherId.
		\remark publisher id is an optional member of StatusMsg
		@return true if publisher id is set; false otherwise
	*/
	bool hasPublisherId() const;

	/** Indicates presence of the ServiceName within the MsgKey.
		\remark service name is an optional member of StatusMsg
		@return true if service name is set; false otherwise
	*/
	bool hasServiceName() const;

	/** Returns State.
		@throw OmmInvalidUsageException if hasState() returns false
		@return state of item (e.g. Open / Ok)
	*/
	const OmmState& getState() const;

	/** Returns ItemGroup.
		@throw OmmInvalidUsageException if hasItemGroup() returns false
		@return EmaBuffer containing item group information
	*/
	const EmaBuffer& getItemGroup() const;

	/** Returns PermissionData.
		@throw OmmInvalidUsageException if hasPermissionData() returns false
		@return EmaBuffer containing permission data information
	*/
	const EmaBuffer& getPermissionData() const;

	/** Returns PublisherIdUserId.
		@throw OmmInvalidUsageException if hasPublisherId() returns false
		@return publisher's user Id
	*/
	UInt32 getPublisherIdUserId() const;

	/** Returns PublisherIdUserAddress.
		@throw OmmInvalidUsageException if hasPublisherId() returns false
		@return publisher's user address
	*/
	UInt32 getPublisherIdUserAddress() const;

	/** Returns ClearCache.
		@return true if cache needs to be cleared on receipt of this status message; false otherwise
	*/
	bool getClearCache() const;

	/** Returns PrivateStream.
		@retturn true if this is private stream item; false otherwise
	*/
	bool getPrivateStream() const;

	/** Returns the ServiceName within the MsgKey.
		@return EmaString containing service name
	 */
	const EmaString& getServiceName() const;
	//@}

	///@name Operations
	//@{
	/** Clears the StatusMsg.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	StatusMsg& clear();

	/** Specifies StreamId.
		@param[in] streamId specifies stream id
		@return reference to this object
	*/
	StatusMsg& streamId( Int32 streamId );

	/** Specifies DomainType.
		@throw OmmUnsupportedDomainTypeException if domainType is greater than 255
		@param[in] domainType specifies RDM Message Model Type (default value is rdm::MMT_MARKET_PRICE)
		@return reference to this object
	*/
	StatusMsg& domainType( UInt16 domainType = thomsonreuters::ema::rdm::MMT_MARKET_PRICE );

	/** Specifies Name.
		@param[in] name specifies item name
		@return reference to this object
	*/
	StatusMsg& name( const EmaString& name );

	/** Specifies NameType.
		@param[in] nameType specifies RDM Instrument NameType (default value is rdm::INSTRUMENT_NAME_RIC)
		@return reference to this object
	*/
	StatusMsg& nameType( UInt8 nameType = rdm::INSTRUMENT_NAME_RIC );

	/** Specifies ServiceName.
		@throw OmmInvalidUsageException if service id is already set
		@param[in] serviceName specifies service name
		@return reference to this object
	*/
	StatusMsg& serviceName( const EmaString& serviceName );

	/** Specifies ServiceId.
		@throw OmmInvalidUsageException if service name is already set
		@param[in] serviceId specifies service id
		@return reference to this object
	*/
	StatusMsg& serviceId( UInt32 serviceId );

	/** Specifies Id.
		@param[in] id specifies Id
		@return reference to this object
	*/
	StatusMsg& id( Int32 id );

	/** Specifies Filter.
		@param[in] filter specifies filter
		@return reference to this object
	*/
	StatusMsg& filter( UInt32 filter );

	/** Specifies State.
		@param[in] streamState conveys item stream state value (default value is OmmState::OpenEnum)
		@param[in] dataState conveys item data state value (default value is OmmState::SuspectEnum)
		@param[in] statusCode conveys specific item state code (default value is OmmState::NoneEnum)
		@param[in] statusText conveys item status explanation (default value is 'empty string')
		@return reference to this object
	*/
	StatusMsg& state( OmmState::StreamState streamState = OmmState::OpenEnum,
		OmmState::DataState dataState = OmmState::OkEnum,
		UInt8 statusCode = OmmState::NoneEnum,
		const EmaString& statusText = EmaString() );

	/** Specifies ItemGroup.
		@param[in] itemGroup an EmaBuffer object with item group information
		@return reference to this object
	*/
	StatusMsg& itemGroup( const EmaBuffer& itemGroup );

	/** Specifies PermissionData.
		@param[in] permissionData an EmaBuffer object with permission data information
		@return reference to this object
	*/
	StatusMsg& permissionData( const EmaBuffer& permissionData );

	/** Specifies PublisherId.
		@param[in] UserId specifies publisher's user id
		@param[in] UserAddress specifies publisher's user address
		@return reference to this object
	*/
	StatusMsg& publisherId( UInt32 userId, UInt32 userAddress );

	/** Specifies Attrib.
		@param[in] attrib an object of ComplexType
		@return reference to this object
	*/
	StatusMsg& attrib( const ComplexType& data );

	/** Specifies Payload.
		@param[in] payload an object of ComplexType
		@return reference to this object
	*/
	StatusMsg& payload( const ComplexType& data );

	/** Specifies ExtendedHeader.
		@param[in] buffer an EmaBuffer containing extendedHeader information
		@return reference to this object
	*/
	StatusMsg& extendedHeader( const EmaBuffer& buffer );

	/** Specifies ClearCache.
		@param[in] clearCache true if cache needs to be cleared; false otherwise (default value is false)
		@return reference to this object
	*/
	StatusMsg& clearCache( bool clearCache = false );

	/** Specifies PrivateStream.
		@param[in] privateStream true if private stream; false otherwise (default value is false)
		@return reference to this object
	*/
	StatusMsg& privateStream( bool privateStream = false );
	//@}

private :

	friend class ItemCallbackClient;
	friend class DictionaryCallbackClient;

	const EmaString& toString( UInt64 ) const;

	Decoder& getDecoder();

	StatusMsg& operator=( const StatusMsg& );

	mutable EmaString		_toString;
};

}

}

}

#endif // __thomsonreuters_ema_access_StatusMsg_h
