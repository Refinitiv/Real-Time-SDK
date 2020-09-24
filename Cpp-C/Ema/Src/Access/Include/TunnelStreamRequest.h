/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_TunnelStreamRequest_h
#define __rtsdk_ema_access_TunnelStreamRequest_h

#include "EmaString.h"

namespace rtsdk {

namespace ema {

namespace access {

class ReqMsg;
class TunnelStreamLoginReqMsgImpl;

/**
	@class rtsdk::ema::access::CosCommon TunnelStreamRequest.h "Access/Include/TunnelStreamRequest.h"
	@brief CosCommon encapsulates common member of the ClassOfService class

	CosCommon describes common options related to the exchange of messages.

	@see ClassOfService
*/
class EMA_ACCESS_API CosCommon
{
public :

	///@name Constructor
	//@{
	/** Default constructor
		\remark specifies default maximum message size as 6144
	*/
	CosCommon();

	/** Copy constructor
		@param[in] other specifies CosCommon object to copy from
	*/
	CosCommon( const CosCommon& other );
	//@}

	///@name Destructor
	//@{
	/** Destructor
	*/
	virtual ~CosCommon();
	//@}

	///@name Operations
	//@{
	/** Assignment operator
		@param[in] other specifies CosCommon object to copy from
		@return reference to this object
	*/
	CosCommon& operator=( const CosCommon& other);
	
	/** Clears object by setting defaults
		@return reference to this object
	*/
	CosCommon& clear();
	
	/** Specifies maximum size of messages exchanged on the tunnel stream.
		@param[in] maxSize specifies maximum message size (valid range 1 - 2,147,483,647)
		@return reference to this object
		\remark this value is set by providers while accepting a tunnel stream
		@throw OmmOutOfRangeException if passed in value is out of valid range
	*/
	CosCommon& maxMsgSize( UInt64 maxMsgSize = 6144 );
	//@}

	///@name Accessors
	//@{
	/** Returns maximum message size assigned by provider accepting the tunnel stream request
		@return maximum message size
	*/
	UInt64 getMaxMsgSize() const;
	//@}

private :

	UInt64 _maxMsgSize;
};

/**
	@class rtsdk::ema::access::CosAuthentication TunnelStreamRequest.h "Access/Include/TunnelStreamRequest.h"
	@brief CosAuthentication encapsulates authentication member of the ClassOfService class

	CosAuthentication contains options to authenticate a consumer to the corresponding provider.

	@see ClassOfService
*/
class EMA_ACCESS_API CosAuthentication
{
public :

	/** @enum CosAuthenticationType
		An enumeration representing authentication type.
	*/
	enum CosAuthenticationType {
		NotRequiredEnum = 0,		/*!< Indicates authentication is not required */
		OmmLoginEnum = 1			/*!< Indicates OmmLogin message is required for authentication */
	};

	///@name Constructor
	//@{
	/** Default constructor
	*/
	CosAuthentication();

	/** Copy constructor
		@param[in] other specifies CosAuthentication object to copy from
	*/
	CosAuthentication( const CosAuthentication& other );
	//@}

	///@name Destructor
	//@{
	/** Destructor
	*/
	virtual ~CosAuthentication();
	//@}
	
	///@name Operations
	//@{
	/** Assignment operator
		@param[in] other specifies CosAuthentication object to copy from
		@return reference to this object	
	*/	
	CosAuthentication& operator=( const CosAuthentication& other );
	
	/** Clears object by setting defaults
		@return reference to this object
	*/
	CosAuthentication& clear();

	/** Specifies authentication type
		@param[in] type authentication type
		@return reference to this object
		@throw OmmOutOfRangeException if passed in authentication type is not valid
	*/
	CosAuthentication& type( CosAuthenticationType type = NotRequiredEnum );
	//@}

	///@name Accessors
	//@{
	/** Returns Authentication type
		@return authentication type
	*/
	CosAuthenticationType getType() const;
	//@}

private :

	CosAuthenticationType _type;	
};

/**
	@class rtsdk::ema::access::CosFlowControl TunnelStreamRequest.h "Access/Include/TunnelStreamRequest.h"
	@brief CosFlowControl encapsulates flow control member of the ClassOfService class

	CosFlowControl contains options related to flow control, such as the type and the allowed window of outstanding data.

	@see ClassOfService
*/
class EMA_ACCESS_API CosFlowControl
{
public :

	/** @enum CosFlowControlType
		An enumeration representing flow control type.
	*/
	enum CosFlowControlType {
		NoneEnum = 0,				/*!< Indicates no flow control */
		BidirectionalEnum = 1		/*!< Indicates bidirectional flow control */
	};

	///@name Constructor
	//@{
	/** Default constructor
	*/
	CosFlowControl();

	/** Copy constructor
		@param[in] other specifies CosFlowControl object to copy from
	*/
	CosFlowControl( const CosFlowControl& other );
	//@}

	///@name Destructor
	//@{
	/** Destructor
	*/
	virtual ~CosFlowControl();
	//@}
	
	///@name Operations
	//@{
	/** Assignment operator
		@param[in] other specifies CosFlowControl object to copy from
		@return reference to this object	
	*/
	CosFlowControl& operator=( const CosFlowControl& other );
	
	/** Clears object by setting defaults
		@return reference to this object
	*/
	CosFlowControl& clear();

	/** Specifies flow control type
		@param[in] type flow control type
		@return reference to this object
		@throw OmmOutOfRangeException if passed in flow control type is not valid
	*/
	CosFlowControl& type( CosFlowControlType type = NoneEnum );
	
	/** Specifies the amount of data (in bytes) that the remote peer can send to
		the application over a reliable tunnel stream.
		@param[in] size specifies number of bytes; valid range is 0 - 2,147,483,647
		@return reference to this object
		@throw OmmOutOfRangeException if passed in size is out of range
		\remark If CosFlowControlType::NoneEnum is set, this parameter has no effect
		\remark if -1 is set and CosFlowControlType::BidirectionalEnum is used, then the default value of 12288 is used
	*/
	CosFlowControl& recvWindowSize( Int64 size = -1 );

	/** Specifies the amount of data (in bytes) that the application can send to
		the remote peer over a reliable tunnel stream.
		@param[in] size specifies number of bytes; valid range is 0 - 2,147,483,647
		@return reference to this object
		@throw OmmOutOfRangeException if passed in size is out of range
		\remark If CosFlowControlType::NoneEnum is set, this parameter has no effect
	*/
	CosFlowControl& sendWindowSize( Int64 size );
	//@}

	///@name Accessors
	//@{
	/** Returns flow control type
		@return flow control type
	*/	
	CosFlowControlType getType() const;

	/** Returns receive window size
		@return receive window size
	*/	
	Int64 getRecvWindowSize() const;

	/** Returns send window size
		@return send window size
	*/	
	Int64 getSendWindowSize() const;
	//@}

private :

	CosFlowControlType		_type;	
	Int64					_recvWindowSize;
	Int64					_sendWindowSize;
};

/**
	@class rtsdk::ema::access::CosDataIntegrity TunnelStreamRequest.h "Access/Include/TunnelStreamRequest.h"
	@brief CosDataIntegrity encapsulates data integrity member of the ClassOfService class

	CosDataIntegrity contains options related to the reliability of content exchanged over the tunnel stream.

	@see ClassOfService
*/
class EMA_ACCESS_API CosDataIntegrity
{
public :

	/** @enum CosDataIntegrityType
		An enumeration representing data integrity type.
	*/
	enum CosDataIntegrityType {
		BestEffortEnum = 0,			/*!< Indicates best efforts data integrity type */
		ReliableEnum = 1			/*!< Indicates reliable data integrity type */
	};

	///@name Constructor
	//@{
	/** Default constructor
	*/
	CosDataIntegrity();

	/** Copy constructor
		@param[in] other specifies CosDataIntegrity object to copy from
	*/
	CosDataIntegrity( const CosDataIntegrity& other );
	//@}

	///@name Destructor
	//@{
	/** Destructor
	*/
	virtual ~CosDataIntegrity();
	//@}
	
	///@name Operations
	//@{
	/** Assignment operator
		@param[in] other specifies CosDataIntegrity object to copy from
		@return reference to this object	
	*/		
	CosDataIntegrity& operator=( const CosDataIntegrity& other );

	/** Clears object by setting defaults
		@return reference to this object
	*/	
	CosDataIntegrity& clear();
	
	/** Specifies data integrity type
		@param[in] type data integrity type
		@return reference to this object
		@throw OmmOutOfRangeException if passed in data integrity type is not valid
	*/
	CosDataIntegrity& type( CosDataIntegrityType type = BestEffortEnum );
	//@}

	///@name Accessors
	//@{
	/** Returns data integrity type
		@return data integrity type
	*/	
	CosDataIntegrityType getType() const;
	//@}

private :

	CosDataIntegrityType _type;	
};

/**
	@class rtsdk::ema::access::CosGuarantee TunnelStreamRequest.h "Access/Include/TunnelStreamRequest.h"
	@brief CosGuarantee encapsulates guarantee member of the ClassOfService class

	CosGuarantee contains options related to the guarantee of content submitted over the tunnel stream.

	@see ClassOfService
*/
class EMA_ACCESS_API CosGuarantee
{
public :

	/** @enum CosGuaranteeType
		An enumeration representing guarantee type.
	*/
	enum CosGuaranteeType {
		NoneEnum = 0,					/*!< Indicates no guarantee */
		PersistentQueueEnum = 1			/*!< Indicates persistent queue */
	};

	///@name Constructor
	//@{
	/** Default constructor
	*/
	CosGuarantee();

	/** Copy constructor
		@param[in] other specifies CosGuarantee object to copy from
	*/
	CosGuarantee( const CosGuarantee& other);
	//@}

	///@name Destructor
	//@{
	/** Destructor
	*/
	virtual ~CosGuarantee();
	//@}
	
	///@name Operations
	//@{
	/** Assignment operator
		@param[in] other specifies CosGuarantee object to copy from
		@return reference to this object	
	*/		
	CosGuarantee& operator=( const CosGuarantee& other );
	
	/** Clears object by setting defaults
		@return reference to this object
	*/
	CosGuarantee& clear();

	/** Specifies guarantee type
		@param[in] type guarantee type
		@return reference to this object
		@throw OmmOutOfRangeException if passed in guarantee type is not valid
	*/
	CosGuarantee& type( CosGuaranteeType type = NoneEnum );

	/** Specifies if messages are persisted locally
		@param[in] persistLocally specifies if  messages need to be persisted locally
		@return reference to this object
		\remark If type is set to CosGuaranteeType::NoneEnum, then this parameter has no effect
	*/
	CosGuarantee& persistLocally( bool persistLocally = true );

	/** Specifies where files containing persistent messages are stored
		@param[in] filePath specifies path to store files containing persistent messages
		@return reference to this object
		\remark If an empty string is passed in, the current working directory is assumed
	*/
	CosGuarantee& persistenceFilePath( const EmaString& filePath = EmaString() );
	//@}

	///@name Accessors
	//@{
	/** Returns guarantee type
		@return guarantee type
	*/	
	CosGuaranteeType getType() const;

	/** Returns if messages should be persisted locally
		@return if messages should be persisted locally
	*/
	bool getPersistLocally() const;

	/** Returns file path where files containing persistent messages may be stored
		@return file path where files containing persistent messages are stored
	*/
	const EmaString& getPersistenceFilePath() const;
	//@}

private :

	CosGuaranteeType		_type;
	bool					_persistLocally;
	EmaString				_filePath;
};

/**
	@class rtsdk::ema::access::ClassOfService TunnelStreamRequest.h "Access/Include/TunnelStreamRequest.h"
	@brief ClassOfService encapsulates behaviours of tunnel stream.

	ClassOfService is used to negotiate behaviors of a tunnel stream. Negotiated behaviors are divided into
	five categories: common, authentication, flow control, data integrity, and guarantee.

	@see CosCommon,
		CosAuthentication,
		CosFlowControl,
		CosDataIntegrity
		CosGuarantee,
		TunnelStreamRequest
*/
class EMA_ACCESS_API ClassOfService
{
public :

	///@name Constructor
	//@{
	/** Default constructor
	*/
	ClassOfService();

	/** Copy constructor
		@param[in] other specifies ClassOfService object to copy from
	*/
	ClassOfService( const ClassOfService& other );
	//@}

	///@name Destructor
	//@{
	/** Destructor
	*/
	virtual ~ClassOfService();
	//@}

	///@name Operations
	//@{
	/** Assignment operator
		@param[in] other specifies ClassOfService object to copy from
		@return reference to this object	
	*/	
	ClassOfService& operator=( const ClassOfService& other );
	
	/** Clears object by setting defaults
		@return reference to this object
	*/
	ClassOfService& clear();

	/** Specifies CosCommon member
		@param[in] cosCommon specifies CosCommon member
		@return reference to this object
	*/
	ClassOfService& common( const CosCommon& cosCommon );

	/** Specifies CosAuthentication member
		@param[in] cosAuthentication specifies CosAuthentication member
		@return reference to this object
	*/
	ClassOfService& authentication( const CosAuthentication& cosAuthentication );

	/** Specifies CosFlowControl member
		@param[in] cosFlowControl specifies CosFlowControl member
		@return reference to this object
	*/
	ClassOfService& flowControl( const CosFlowControl& cosFlowControl );

	/** Specifies CosDataIntegrity member
		@param[in] cosDataIntegrity specifies CosDataIntegrity member
		@return reference to this object
	*/
	ClassOfService& dataIntegrity( const CosDataIntegrity& cosDataIntegrity );

	/** Specifies CosGuarantee member
		@param[in] cosGuarantee specifies CosGuarantee member
		@return reference to this object
	*/
	ClassOfService& guarantee( const CosGuarantee& cosGuarantee );
	//@}

	///@name Accessors
	//@{
	/** Returns CosCommon member
		@return CosCommon member
	*/
	const CosCommon& getCommon() const;

	/** Returns CosAuthentication member
		@return CosAuthentication member
	*/
	const CosAuthentication& getAuthentication() const;

	/** Returns CosFlowControl member
		@return CosFlowControl member
	*/
	const CosFlowControl& getFlowControl() const;

	/** Returns CosDataIntegrity member
		@return CosDataIntegrity member
	*/
	const CosDataIntegrity& getDataIntegrity() const;

	/** Returns CosGuarantee member
		@return CosGuarantee member
	*/
	const CosGuarantee& getGuarantee() const;
	//@}
	
private :

	CosCommon			_common;
	CosAuthentication	_authentication;
	CosFlowControl		_flowControl;
	CosDataIntegrity	_dataIntegrity;
	CosGuarantee		_guarantee;
};

/**
	@class rtsdk::ema::access::TunnelStreamRequest TunnelStreamRequest.h "Access/Include/TunnelStreamRequest.h"
	@brief TunnelStreamRequest encapsulates tunnel stream request parameters.

	TunnelStreamRequest contains options used for creation of a tunnel stream.

	\code

	// create and populate TunnelStreamRequest object
	ClassOfService cos;
	cos.authentication( CosAuthentication().type( CosAuthentication::OmmLoginEnum ) )
		.dataIntegrity( CosDataIntegrity().type( CosDataIntegrity::ReliableEnum ) )
		.flowControl( CosFlowControl().type( CosFlowControl::BidirectionalEnum )
									.recvWindowSize( 1200 )
									.sendWindowSize( 1200 ) )
		.guarantee( CosGuarantee().type( CosGuarantee::NoneEnum ) );

	TunnelStreamRequest tsr;
	tsr.classOfService( cos )
		.domainType( MMT_SYSTEM )
		.name( "TUNNEL_STREAM" )
		.serviceId( 1 )
		.responseTimeout( 45 );
		
	\endcode

	@see ClassOfService,
		OmmConsumer
*/
class EMA_ACCESS_API TunnelStreamRequest
{
public :

	///@name Constructor
	//@{
	/** Default constructor
	*/
	TunnelStreamRequest();

	/** Copy constructor
		@param[in] other specifies TunnelStreamRequest object to copy from
	*/
	TunnelStreamRequest( const TunnelStreamRequest& other );
	//@}

	///@name Destructor
	//@{
	/** Destructor
	*/
	virtual ~TunnelStreamRequest();
	//@}

	///@name Operations
	//@{
	/** Assignment operator
		@param[in] other specifies TunnelStreamRequest object to copy from
		@return reference to this object
	*/
	TunnelStreamRequest& operator=( const TunnelStreamRequest& other );

	/** Clears object by setting defaults
		@return reference to this object
	*/	
	TunnelStreamRequest& clear();

	/** Specifies DomainType.
		@param[in] domainType specifies RDM Message Model Type
		@return reference to this object
		@throw OmmUnsupportedDomainTypeException if domainType is greater than 255
	*/
	TunnelStreamRequest& domainType( UInt8 domainType );
	
	/** Specifies ServiceId.
		\remark One service identification must be set, either id or name.
		@param[in] serviceId specifies service id
		@return reference to this object
		@throw OmmInvalidUsageException if service name is already set
	*/
	TunnelStreamRequest& serviceId( UInt32 serviceId );
	
	/** Specifies ServiceName.
		\remark One service identification must be set, either id or name.
		@param[in] serviceName specifies service name
		@return reference to this object
		@throw OmmInvalidUsageException if service id is already set
	*/
	TunnelStreamRequest& serviceName( const EmaString& serviceName );

	/** Specifies the tunnel stream name, which is provided to the remote application.
		@param[in] name specifies tunnel stream name
		@return reference to this object
	*/
	TunnelStreamRequest& name( const EmaString& name );

	/** Sets the duration to wait for a provider to respond to a tunnel stream open request.
		@param[in] timeOut specifies time to wait for the response to the open request (in seconds)
		@return reference to this object
	*/
	TunnelStreamRequest& responseTimeout( UInt32 timeout = 60 );

	/** Specifies the number of guaranteed output buffers available for the tunnel stream.
		@param[in] value specifies number of output buffers
		@return reference to this object
	*/
	TunnelStreamRequest& guaranteedOutputBuffers( UInt32 value = 50 );

	/** Specifies the ClassOfService.
		@param[in] cos specifies ClassOfService
		@return reference to this object
	*/
	TunnelStreamRequest& classOfService( const ClassOfService& cos );
	
	/** Specifies login request message to be used if authentication type is set to CosAuthentication::OmmLoginEnum.
		@param[in] loginReq specifies the OMM login request message
		@return reference to this object
		\remark if not specified, OmmConsumer will use same login request message as one used for establihment of the connection
	*/
	TunnelStreamRequest& loginReqMsg( const ReqMsg& loginReq );
	//@}

	///@name Accessors
	//@{
	/** Indicates presence of service id
		@return true if service id is set
	*/
	bool hasServiceId() const;

	/** Indicates presence of service name
		@return true if service name is set
	*/
	bool hasServiceName() const;

	/** Indicates presence of name
		@return true if service name is set
	*/
	bool hasName() const;

	/** Indicates presence of login request message.
		@return true if login request message is set
	*/
	bool hasLoginReqMsg() const;

	/** Returns DomainType.
		@return domain type
	*/
	UInt16 getDomainType() const;

	/** Returns ServiceId.
		@return service id
		@throw OmmInvalidUsageException if service id is not set
	*/
	UInt32 getServiceId() const;

	/** Returns ServiceNme.
		@return service name
		@throw OmmInvalidUsageException if service name is not set
	*/
	const EmaString& getServiceName() const;

	/** Returns tunnel stream name.
		@return tunnel stream name
		@throw OmmInvalidUsageException if name is not set
	*/
	const EmaString& getName() const;

	/** Returns ResponseTimeOut.
		@return response timeOut
	*/
	UInt32 getResponseTimeOut() const;

	/** Returns number of GuaranteedOutputBuffers.
		@return number of guaranteed output buffers
	*/
	UInt32 getGuaranteedOutputBuffers() const;

	/** Retursn ClassOfService.
		@return class of service
	*/
	const ClassOfService& getClassOfService() const;

	/** Returns Login request message
		@return login request message
		@throw OmmInvalidUsageException if login request message is not set
	*/
	const ReqMsg& getLoginReqMsg() const;
	//@}

private :

	friend class TunnelItem;

	UInt8							_domainType;
	UInt32							_serviceId;
	UInt32							_responseTimeout;
	UInt32							_guaranteedOutputBuffers;
	bool							_serviceIdSet;
	bool							_serviceNameSet;
	bool							_nameSet;
	EmaString						_serviceName;
	EmaString						_name;
	ClassOfService					_cos;
	TunnelStreamLoginReqMsgImpl*	_pImpl;
};

};

};

};

#endif // __rtsdk_ema_access_TunnelStreamRequest_h
