/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_UpdateMsg_h
#define __thomsonreuters_ema_access_UpdateMsg_h

/**
	@class thomsonreuters::ema::access::UpdateMsg UpdateMsg.h "Access/Include/UpdateMsg.h"
	@brief UpdateMsg conveys changes to item data.

	The following code snippet shows receiving and processing of UpdateMsg.

	\code

	class AppClient ; public OmmConsumerClient
	{
		...

		void onUpdateMsg( const UpdateMsg& , const OmmConsumerEvent& );
	};

	void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& event )
	{
		cout << "Item's handle = " << event.getHandle() << "\n";

		if ( updateMsg.hasName() )
			cout << "Item's name = " << updateMsg.getName() << "\n";

		if ( updateMsg.hasServiceName() )
			cout << "Item's name = " << updateMsg.getServiceName() << "\n";

		switch ( updateMsg.getPayload().getDataTyoe() )
		{
			case DataType::FieldListEnum :
				decode( updateMsg.getPayload().getFieldList() );
				break;
			case DataType::NoDataEnum :
				break;
		}
	}

	\endcode
			
	\remark Calling get***() method on an optional member of UpdateMsg must be preceded by a
			call to respective has***() method.
	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform setting and getting of information from UpdateMsg.
	\remark Objects of this class are not cache-able.
	\remark	Decoding of just encoded UpdateMsg in the same application is not supported.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		Msg,
		EmaString,
		EmaBuffer
*/

#include "Access/Include/Msg.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class EMA_ACCESS_API UpdateMsg : public Msg
{
public :

	///@name Constructor
	//@{
	/** Constructs UpdateMsg.
	*/
	UpdateMsg();
	//@}

	///@name Constructor
	//@{
	/** Destructor.
	*/
	virtual ~UpdateMsg();
	//@}

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::UpdateMsgEnum
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

	/** Indicates presence of SeqNum.
		\remark sequence number is an optional member of UpdateMsg
		@return true if sequence number is set; false otherwise
	*/
	bool hasSeqNum() const;

	/** Indicates presence of PermissionData.
		\remark permission data is an optional member of UpdateMsg
		@return true if permission data is set; false otherwise
	*/
	bool hasPermissionData() const;

	/** Indicates presence of Conflated.
		@return true if update contains conflated data; false otherwise
	*/
	bool hasConflated() const;

	/** Indicates presence of PublisherId.
		\remark publisher id is an optional member of UpdateMsg
		@return true if publisher id is set; false otherwise
	*/
	bool hasPublisherId() const;

	/** Indicates presence of the ServiceName within the MsgKey.
		\remark service name is an optional member of UpdateMsg
		@return true if service name is set; false otherwise
	*/
	bool hasServiceName() const;

	/** Returns UpdateTypeNum.
		@return update type number (e.g., rdm::INSTRUMENT_UPDATE_QUOTE)
	*/
	UInt8 getUpdateTypeNum() const;

	/** Returns SeqNum.
		@throw OmmInvalidUsageException if hasSeqNum() returns false
		@return sequence number 
	*/
	UInt32 getSeqNum() const;

	/** Returns PermissionData.
		@throw OmmInvalidUsageException if hasPermissionData() returns false
		@return EmaBuffer containing permission data
	*/
	const EmaBuffer& getPermissionData() const;

	/** Returns ConflatedTime.
		@throw OmmInvalidUsageException if hasConflated() returns false
		@return time conflation was on
	*/
	UInt16 getConflatedTime() const;

	/** Returns ConflatedCount.
		@throw OmmInvalidUsageException if hasConflated() returns false
		@return number of conflated updates
	*/
	UInt16 getConflatedCount() const;

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

	/** Returns DoNotCache.
		@return true if this update must not be cached; false otherwise
	*/
	bool getDoNotCache() const;

	/** Returns DoNotConflate.
		@return true if this update must not be conflated; false otherwise
	*/
	bool getDoNotConflate() const;

	/** Returns DoNotRipple.
		@return true if this update does not ripple; false otherwise
	*/
	bool getDoNotRipple() const;

	/** Returns the ServiceName within the MsgKey.
		@throw OmmInvalidUsageException if hasServiceName() returns false
		@return EmaString containing service name
	*/
	const EmaString& getServiceName() const;
	//@}

	///@name Operations
	//@{
	/** Clears the UpdateMsg.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	UpdateMsg& clear();

	/** Specifies StreamId.
		@param[in] streamId specifies stream id
		@return reference to this object
	*/
	UpdateMsg& streamId( Int32 streamId );

	/** Specifies DomainType.
		@throw OmmUnsupportedDomainType if domainType is greater than 255
		@param[in] domainType specifies RDM Message Model Type (default value is rdm::MMT_MARKET_PRICE)
		@return reference to this object
	*/
	UpdateMsg& domainType( UInt16 domainType = rdm::MMT_MARKET_PRICE );

	/** Specifies Name.
		@param[in] name specifies item name
		@return reference to this object
	*/
	UpdateMsg& name( const EmaString& name );

	/** Specifies NameType.
		@param[in] nameType specifies RDM Instrument NameType (default value is rdm::INSTRUMENT_NAME_RIC)
		@return reference to this object
	*/
	UpdateMsg& nameType( UInt8 nameType = rdm::INSTRUMENT_NAME_RIC );

	/** Specifies ServiceName.
		@throw OmmInvalidUsageException if service id is already set
		@param[in] serviceName specifies service name
		@return reference to this object
	*/
	UpdateMsg& serviceName( const EmaString& serviceName );

	/** Specifies ServiceId.
		@throw OmmInvalidUsageException if service name is already set
		@param[in] serviceId specifies service id
		@return reference to this object
	*/
	UpdateMsg& serviceId( UInt32 serviceId );

	/** Specifies Id.
		@param[in] id specifies Id
		@return reference to this object
	*/
	UpdateMsg& id( Int32 id );

	/** Specifies Filter.
		@param[in] filter specifies filter
		@return reference to this object
	*/
	UpdateMsg& filter( UInt32 filter );

	/** Specifies UpdateTypeNum.
		@param[in] updateTypeNum specifies update type number (e.g., rdm::INSTRUMENT_UPDATE_QUOTE)
	*/
	UpdateMsg& updateTypeNum( UInt8 updateTypeNum );

	/** Specifies SeqNum.
		@param[in] seqNum specifies sequence number
		@return reference to this object
	*/
	UpdateMsg& seqNum( UInt32 seqNum );

	/** Specifies PermissionData.
		@param[in] permissionData an EmaBuffer object with permission data information
		@return reference to this object
	*/
	UpdateMsg& permissionData( const EmaBuffer& permissionData );

	/** Specifies Conflated.
		@param[in] count specifies how many updates were conflated
		@param[in] time specifies how long the conflation was on
		@return reference to this object
	*/
	UpdateMsg& conflated( UInt16 count = 0, UInt16 time = 0 );

	/** Specifies PublisherId.
		@param[in] UserId specifies publisher's user id
		@param[in] UserAddress specifies publisher's user address
		@return reference to this object
	*/
	UpdateMsg& publisherId( UInt32 userId, UInt32 userAddress );

	/** Specifies Attrib.
		@param[in] attrib an object of ComplexType
		@return reference to this object
	*/
	UpdateMsg& attrib( const ComplexType& data );

	/** Specifies Payload.
		@param[in] payload an object of ComplexType
		@return reference to this object
	*/
	UpdateMsg& payload( const ComplexType& data );

	/** Specifies ExtendedHeader.
		@param[in] buffer an EmaBuffer containing extendedHeader information
		@return reference to this object
	*/
	UpdateMsg& extendedHeader( const EmaBuffer& buffer );

	/** Specifies DoNotCache.
		@param[in] doNotCache true if this update must not be cached; false otherwise (default value is false)
		@return reference to this object
	*/
	UpdateMsg& doNotCache( bool doNotCache = false );

	/** Specifies DoNotConflate.
		@param[in] doNotConflate true if this update must not be conflated; false otherwise (default value is false)
		@return reference to this object
	*/
	UpdateMsg& doNotConflate( bool doNotConflate = false );

	/** Specifies DoNotRipple.
		@param[in] doNotRipple true if this update does not ripple; false otherwise (default value is false)
		@return reference to this object
	*/
	UpdateMsg& doNotRipple( bool doNotRipple = false );
	//@}

private :

	friend class ItemCallbackClient;

	const EmaString& toString( UInt64 ) const;

	Decoder& getDecoder();

	UpdateMsg( const UpdateMsg& );
	UpdateMsg& operator=( const UpdateMsg& );

	mutable EmaString		_toString;
};

}

}

}

#endif // __thomsonreuters_ema_access_UpdateMsg_h
