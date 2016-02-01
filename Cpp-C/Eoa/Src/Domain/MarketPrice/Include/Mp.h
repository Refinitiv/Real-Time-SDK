/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_domain_marketprice_mp_h
#define __thomsonreuters_eoa_domain_marketprice_mp_h

#pragma warning( disable : 4251 )

#include "Foundation/Include/EoaBuffer.h"
#include "Foundation/Include/EoaString.h"
#include "Foundation/Include/Leaf.h"
#include "Foundation/Include/Node.h"
#include "Foundation/Include/OmmMemoryExhaustionException.h"
#include "Foundation/Include/OmmInvalidUsageException.h"
#include "Foundation/Include/OmmInaccessibleLogFileException.h"
#include "Foundation/Include/OmmOutOfRangeException.h"
#include "Foundation/Include/OmmUnsupportedDomainTypeException.h"

#include <functional>

namespace thomsonreuters {

namespace eoa {

namespace domain {
	
/**
	@namespace domain::marketprice
	@brief The domain::marketprice namespace defines all the interfaces used by
	the EOA Market Price package as well as applications using these package.
	
	The EOA Market Price package provides RDM MarketPrice specific services and
	functionalities, which are encapsulated by the Mp class.
*/
namespace marketprice {

class MpConsumerItemImpl;
class MpReqSpecImpl;
class MpConsumerServiceImpl;
class MpRefreshInfoImpl;
class MpUpdateInfoImpl;
class MpStatusInfoImpl;
class MpQosImpl;
class MpStateImpl;

/**
	@class thomsonreuters::eoa::domain::marketprice::Mp Mp.h "Domain/MarketPrice/Include/Mp.h"
	@brief Mp class contains all classes used to request and process MarketPrice item.
	
	@see Mp::ConsumerItem,
		Mp::RefreshInfo,
		Mp::StatusInfo,
		Mp::UpdateInfo,
		Mp::ConsumerItemClient,
		Mp::ConsumerService,
		Mp::ReqSpec,
		Mp::Quote,
		Mp::Qos,
		Mp::State
*/

class EOA_DOMAIN_API Mp
{
public:

class Quote;
class Qos;
class State;
class ConsumerItemClient;
class ReqSpec;
class ConsumerService;

	/**
		@class thomsonreuters::eoa::domain::marketprice::Mp::ConsumerItem Mp.h "Domain/MarketPrice/Include/Mp.h"
		@brief Mp::ConsumerItem provides access to MarketPrice item's key, state and quote.

		\remark All methods in this class are \ref SingleThreaded.

		@see Mp::Quote
	*/
	class EOA_DOMAIN_API ConsumerItem
	{
	public:

		///@name Constructor
		//@{
		/** Constructs ConsumerItem.
			@throw OmmMemoryExhaustionException if app runs out of memory
		*/
		ConsumerItem();

		/** Copy constructor.
			param[in] other copied ConsumerItem object
			@throw OmmMemoryExhaustionException if app runs out of memory
		*/
		ConsumerItem( const ConsumerItem& other );

		/** Move constructor.
			param[in] other moved ConsumerItem object
		*/
		ConsumerItem( ConsumerItem&& other );
		//@}

		///@name Destructor
		//@{
		/** Destructor.
		*/
		virtual ~ConsumerItem();
		//@}

		///@name Operations
		//@{
		/** Assignment operator.
			param[in] other copied ConsumerItem object
			@return reference to this object
			@throw OmmMemoryExhaustionException if app runs out of memory
		*/
		ConsumerItem& operator=( const ConsumerItem& other );

		/** Assignment move operator.
			param[in] other moved ConsumerItem object
			@return reference to this object
		*/
		ConsumerItem& operator=( ConsumerItem&& other );
		//@}

		///@name Accessors
		//@{
		/** Returns the symbol name.
			@return EoaString containing symbol name
		*/
		const foundation::EoaString& getSymbol() const;

		/** Returns the service name.
			@return EoaString containing service name
		*/
		const foundation::EoaString& getServiceName() const;

		/** Returns the DomainType, which is the unique identifier of a domain.
			@return domain type value
		*/
		foundation::UInt16 getDomainType() const;

		/** Indicates if item is active.
			@return true if item is active; false otherwise
		*/
		bool isActive() const;

		/** Indicates if item is healthy.
			@return true if item is healthy; false otherwise
		*/
		bool isOk() const;

		/** Indicates if item received an unsolicited image.
			@return true if item received an unsolicited image; false otherwise
		*/
		bool isResync() const;

		/** Indicates if item received complete image (e.g. last image part).
			@return true if item is complete; false otherwise
		*/
		bool isComplete() const;

		/** Returns a EoaString that represents status text of this item
			@return EoaString containing item's status text
		*/
		const foundation::EoaString& getStatusText() const;

		/** Returns the quote of this item
			@return reference to Quote object
		*/
		const Quote& getQuote() const;

		/** Returns a string representation of the class instance.
			@throw OmmMemoryExhaustionException if app runs out of memory
			@return EoaString containing string representation of the class instance
		*/
		const foundation::EoaString& toString() const;

		/** Operator const char* overload.
			@throw OmmMemoryExhaustionException if app runs out of memory
		*/
		operator const char* () const;
		//@}

	private:

		friend class MpConsumerServiceImpl;
		friend class Mp::ConsumerService;

		MpConsumerItemImpl*								_pImpl;
		void*											_closure;
		ConsumerItemClient*								_pClient;
		std::function< void( const ConsumerItem& ) >	_callBackFunc;		
	};

	/**
		@class thomsonreuters::eoa::domain::marketprice::Mp::RefreshInfo Mp.h "Domain/MarketPrice/Include/Mp.h"
		@brief Mp::RefreshInfo provides access to the information brought in by a MarketPrice refresh message.

		\remark All methods in this class are \ref SingleThreaded.

		@see Mp::Quote,
			Mp::Qos,
			Mp::State
	*/
	class EOA_DOMAIN_API RefreshInfo
	{
	public:

		///@name Accessors
		//@{
		/** Indicates presence of the Name.
			@return true if name is set; false otherwise
		*/
		bool hasName() const;

		/** Indicates presence of the NameType.
			@return true if name type is set; false otherwise
		*/
		bool hasNameType() const;

		/** Indicates presence of the ServiceId.
			@return true if service id is set; false otherwise
		*/
		bool hasServiceId() const;

		/** Indicates presence of the ServiceName.
			@return true if service name is set; false otherwise
		*/
		bool hasServiceName() const;

		/** Indicates presence of Qos.
			@return true if Qos is set; false otherwise
		*/
		bool hasQos() const;

		/** Indicates presence of SeqNum.
			@return true if sequence number is set; false otherwise
		*/
		bool hasSeqNum() const;

		/** Indicates presence of PermissionData.
			@return true if permission data is set; false otherwise
		*/
		bool hasPermissionData() const;

		/** Indicates presence of PublisherId.
			@return true if publisher id is set; false otherwise
		*/
		bool hasPublisherId() const;

		/** Returns the DomainType, which is the unique identifier of a domain.
			@return domain type value
		*/
		foundation::UInt16 getDomainType() const;

		/** Returns the Name.
			@throw OmmInvalidUsageException if hasName() returns false
			@return EoaString containing name
		*/
		const foundation::EoaString& getName() const;

		/** Returns the NameType.
			@throw OmmInvalidUsageException if hasNameType() returns false
			@return name type value
		*/
		foundation::UInt8 getNameType() const;

		/** Returns the ServiceId.
			@throw OmmInvalidUsageException if hasServiceId() returns false
			@return service id value
		*/
		foundation::UInt32 getServiceId() const;

		/** Returns the ServiceName.
			@throw OmmInvalidUsageException if hasServiceName() returns false
			@return EoaString containing service name
		*/
		const foundation::EoaString& getServiceName() const;

		/** Returns the contained quote.
			@return reference to Quote object
		*/
		const Quote& getQuote() const;

		/** Returns State.
			@return state of item
		*/
		const State& getState() const;

		/** Returns Qos.
			@throw OmmInvalidUsageException if hasQos() returns false
			@return Qos of item
		*/
		const Qos& getQos() const;

		/** Returns SeqNum.
			@throw OmmInvalidUsageException if hasSeqNum() returns false
			@return sequence number
		*/
		foundation::UInt32 getSeqNum() const;

		/** Returns ItemGroup.
			@return EoaBuffer containing item group information
		*/
		const foundation::EoaBuffer& getItemGroup() const;

		/** Returns PermissionData.
			@throw OmmInvalidUsageException if hasPermissionData() returns false
			@return EoaBuffer containing permission data
		*/
		const foundation::EoaBuffer& getPermissionData() const;

		/** Returns PublisherIdUserId.
			@throw OmmInvalidUsageException if hasPublisherId() returns false
			@return publisher's user Id
		*/
		foundation::UInt32 getPublisherIdUserId() const;

		/** Returns PublisherIdUserAddress.
			@throw OmmInvalidUsageException if hasPublisherId() returns false
			@return publisher's user address
		*/
		foundation::UInt32 getPublisherIdUserAddress() const;

		/** Returns Solicited.
			@return true if this is solicited refresh; false otherwise
		*/ 
		bool getSolicited() const;

		/** Returns DoNotCache.
			@return true if this refresh must not be cached; false otherwise
		*/
		bool getDoNotCache() const;

		/** Returns Complete.
			@return true if this is the last part of the multi part refresh message
		*/
		bool getComplete() const;

		/** Returns ClearCache.
			@return true if cache needs to be cleared before applying this refresh; false otherwise
		*/
		bool getClearCache() const;

		/** Returns PrivateStream.
			@return true if this is private stream item
		*/
		bool getPrivateStream() const;

		/** Returns a string representation of the class instance.
			@throw OmmMemoryExhaustionException if app runs out of memory
			@return string representation of the class instance
		*/
		const foundation::EoaString& toString() const;

		/** Operator const char* overload.
			@throw OmmMemoryExhaustionException if app runs out of memory
		*/
		operator const char* () const;
		//@}

	private:

		friend class MpConsumerServiceImpl;

		RefreshInfo();
		virtual ~RefreshInfo();

		MpRefreshInfoImpl*	_pImpl;

		RefreshInfo( const RefreshInfo& );
		RefreshInfo& operator=( const RefreshInfo& );
	};

	/**
		@class thomsonreuters::eoa::domain::marketprice::Mp::StatusInfo Mp.h "Domain/MarketPrice/Include/Mp.h"
		@brief Mp::StatusInfo provides access to the information brought in by a MarketPrice status message.

		\remark All methods in this class are \ref SingleThreaded.

		@see Mp::State
	*/
	class EOA_DOMAIN_API StatusInfo
	{
	public:

		///@name Accessors
		//@{
		/** Indicates presence of the Name.
			@return true if name is set; false otherwise
		*/
		bool hasName() const;

		/** Indicates presence of the NameType.
			@return true if name type is set; false otherwise
		*/
		bool hasNameType() const;

		/** Indicates presence of the ServiceId.
			@return true if service id is set; false otherwise
		*/
		bool hasServiceId() const;

		/** Indicates presence of the ServiceName.
			@return true if service name is set; false otherwise
		*/
		bool hasServiceName() const;

		/** Indicates presence of ItemGroup.
			@return true if Item Group information is set; false otherwise
		*/
		bool hasItemGroup() const;

		/** Indicates presence of State.
			@return true if state information is set; false otherwise
		*/
		bool hasState() const;

		/** Indicates presence of PermissionData.
			@return true if permission data is set; false otherwise
		*/
		bool hasPermissionData() const;

		/** Indicates presence of PublisherId.
			\remark publisher id is an optional member of StatusMsg
			@return true if publisher id is set; false otherwise
		*/
		bool hasPublisherId() const;

		/** Returns the DomainType, which is the unique identifier of a domain.
			@return domain type value
		*/
		foundation::UInt16 getDomainType() const;

		/** Returns the Name.
			@throw OmmInvalidUsageException if hasName() returns false
			@return EoaString containing name
		*/
		const foundation::EoaString& getName() const;

		/** Returns the NameType.
			@throw OmmInvalidUsageException if hasNameType() returns false
			@return name type value
		*/
		foundation::UInt8 getNameType() const;

		/** Returns the ServiceId.
			@throw OmmInvalidUsageException if hasServiceId() returns false
			@return service id value
		*/
		foundation::UInt32 getServiceId() const;

		/** Returns the ServiceName.
			@throw OmmInvalidUsageException if hasServiceName() returns false
			@return EoaString containing service name
		 */
		const foundation::EoaString& getServiceName() const;

		/** Returns State.
			@throw OmmInvalidUsageException if hasState() returns false
			@return state of item
		*/
		const State& getState() const;

		/** Returns ItemGroup.
			@throw OmmInvalidUsageException if hasItemGroup() returns false
			@return EoaBuffer containing item group information
		*/
		const foundation::EoaBuffer& getItemGroup() const;

		/** Returns PermissionData.
			@throw OmmInvalidUsageException if hasPermissionData() returns false
			@return EoaBuffer containing permission data
		*/
		const foundation::EoaBuffer& getPermissionData() const;

		/** Returns PublisherIdUserId.
			@throw OmmInvalidUsageException if hasPublisherId() returns false
			@return publisher's user Id
		*/
		foundation::UInt32 getPublisherIdUserId() const;

		/** Returns PublisherIdUserAddress.
			@throw OmmInvalidUsageException if hasPublisherId() returns false
			@return publisher's user address
		*/
		foundation::UInt32 getPublisherIdUserAddress() const;

		/** Returns ClearCache.
			@return true if cache needs to be cleared before applying this refresh; false otherwise
		*/
		bool getClearCache() const;

		/** Returns PrivateStream.
			@return true if this is private stream item
		*/
		bool getPrivateStream() const;

		/** Returns a string representation of the class instance.
			@throw OmmMemoryExhaustionException if app runs out of memory
			@return string representation of the class instance
		*/
		const foundation::EoaString& toString() const;

		/** Operator const char* overload.
			@throw OmmMemoryExhaustionException if app runs out of memory
		*/
		operator const char* () const;
		//@}

	private:

		friend class MpConsumerServiceImpl;

		StatusInfo();
		virtual ~StatusInfo();

		MpStatusInfoImpl*		_pImpl;

		StatusInfo( const StatusInfo& );
		StatusInfo& operator=( const StatusInfo& );
	};

	/**
		@class thomsonreuters::eoa::domain::marketprice::Mp::UpdateInfo Mp.h "Domain/MarketPrice/Include/Mp.h"
		@brief Mp::UpdateInfo provides access to the information brought in by an MarketPrice update message.

		\remark All methods in this class are \ref SingleThreaded.

		@see Mp::Quote
	*/
	class EOA_DOMAIN_API UpdateInfo
	{
	public:

		///@name Accessors
		//@{
		/** Indicates presence of the Name.
			@return true if name is set; false otherwise
		*/
		bool hasName() const;

		/** Indicates presence of the NameType.
			@return true if name type is set; false otherwise
		*/
		bool hasNameType() const;

		/** Indicates presence of the ServiceId.
			@return true if service id is set; false otherwise
		*/
		bool hasServiceId() const;

		/** Indicates presence of the ServiceName.
			@return true if service name is set; false otherwise
		*/
		bool hasServiceName() const;

		/** Indicates presence of SeqNum.
			@return true if sequence number is set; false otherwise
		*/
		bool hasSeqNum() const;

		/** Indicates presence of PermissionData.
			@return true if permission data is set; false otherwise
		*/
		bool hasPermissionData() const;

		/** Indicates presence of Conflated.
			@return true if update contains conflated data; false otherwise
		*/
		bool hasConflated() const;

		/** Indicates presence of PublisherId.
			@return true if publisher id is set; false otherwise
		*/
		bool hasPublisherId() const;

		/** Returns the DomainType, which is the unique identifier of a domain.
			@return domain type value
		*/
		foundation::UInt16 getDomainType() const;

		/** Returns the Name.
			@throw OmmInvalidUsageException if hasName() returns false
			@return EoaString containing name
		*/
		const foundation::EoaString& getName() const;

		/** Returns the NameType.
			@throw OmmInvalidUsageException if hasNameType() returns false
			@return name type value
		*/
		foundation::UInt8 getNameType() const;

		/** Returns the ServiceId.
			@throw OmmInvalidUsageException if hasServiceId() returns false
			@return service id value
		*/
		foundation::UInt32 getServiceId() const;

		/** Returns the ServiceName.
			@throw OmmInvalidUsageException if hasServiceName() returns false
			@return EoaString containing service name
		*/
		const foundation::EoaString& getServiceName() const;

		/** Returns the contained quote.
			@return reference to Quote object
		*/
		const Quote& getQuote() const;

		/** Returns UpdateTypeNum.
			@return update type number
		*/
		foundation::UInt8 getUpdateTypeNum() const;

		/** Returns SeqNum.
			@throw OmmInvalidUsageException if hasSeqNum() returns false
			@return sequence number 
		*/
		foundation::UInt32 getSeqNum() const;

		/** Returns PermissionData.
			@throw OmmInvalidUsageException if hasPermissionData() returns false
			@return EoaBuffer containing permission data
		*/
		const foundation::EoaBuffer& getPermissionData() const;

		/** Returns ConflatedTime.
			@throw OmmInvalidUsageException if hasConflated() returns false
			@return time conflation was on
		*/
		foundation::UInt16 getConflatedTime() const;

		/** Returns ConflatedCount.
			@throw OmmInvalidUsageException if hasConflated() returns false
			@return number of conflated updates
		*/
		foundation::UInt16 getConflatedCount() const;

		/** Returns PublisherIdUserId.
			@throw OmmInvalidUsageException if hasPublisherId() returns false
			@return publisher's user Id
		*/
		foundation::UInt32 getPublisherIdUserId() const;

		/** Returns PublisherIdUserAddress.
			@throw OmmInvalidUsageException if hasPublisherId() returns false
			@return publisher's user address
		*/
		foundation::UInt32 getPublisherIdUserAddress() const;

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

		/** Returns a string representation of the class instance.
			@throw OmmMemoryExhaustionException if app runs out of memory
			@return string representation of the class instance
		*/
		const foundation::EoaString& toString() const;

		/** Operator const char* overload.
			@throw OmmMemoryExhaustionException if app runs out of memory
		*/
		operator const char* () const;
		//@}

	private:

		friend class MpConsumerServiceImpl;

		UpdateInfo();
		virtual ~UpdateInfo();

		MpUpdateInfoImpl*		_pImpl;

		UpdateInfo( const UpdateInfo& );
		UpdateInfo& operator=( const UpdateInfo& );
	};

	/**
		@class thomsonreuters::eoa::domain::marketPrice::Mp::ConsumerItemClient Mp.h "Domain/MarketPrice/Include/Mp.h"
		@brief Mp::ConsumerItemClient class provides callback interfaces notifying application about changes to registered items.

		Application needs to implement an application client class inheriting from Mp::ConsumerItemClient.
		In its own class, application needs to override callback methods it desires to use for item processing.
		Default empty callback methods are implemented by Mp::ConsumerItemClient class.

		\remark Thread safety of all the methods in this class depends on the user's implementation.

		@see Mp::ConsumerItem,
			Mp::RefreshInfo,
			Mp::UpdateInfo,
			Mp::StatusInfo
	*/
	class EOA_DOMAIN_API ConsumerItemClient 
	{
	public:

		///@name Callbacks
		//@{
		/** Invoked upon receiving item sync. 
			@param[out] current - accrued item information (contains the received change)
			@param[out] change - received item refresh
			@param[out] closure - item closure specified on subscribe( ... , ConsumerItemClient& , void* closure ) or snap( ... , ConsumerItemClient& , void* closure )
			@return void
		*/
		virtual void onConsumerItemSync( const ConsumerItem& current, const RefreshInfo& change, void* closure ) {}

		/** Invoked upon receiving item sync. 
			@param[out] current - accrued item information (contains the received change)
			@param[out] change - received item update
			@param[out] closure - item closure specified on subscribe( ... , ConsumerItemClient& , void* closure ) or snap( ... , ConsumerItemClient& , void* closure )
			@return void
		*/
		virtual void onConsumerItemUpdate( const ConsumerItem& current, const UpdateInfo& change, void* closure ) {}

		/** Invoked upon receiving item status. 
			@param[out] current - accrued item information (contains the received change)
			@param[out] change - received item status
			@param[out] closure - item closure specified on subscribe( ... , ConsumerItemClient& , void* closure ) or snap( ... , ConsumerItemClient& , void* closure )
			@return void
		*/
		virtual void onConsumerItemStatus( const ConsumerItem& current, const StatusInfo& change, void* closure ) {}
		//@}

	protected:

		ConsumerItemClient();
		virtual ~ConsumerItemClient();

	private:

		ConsumerItemClient( const ConsumerItemClient& );
		ConsumerItemClient& operator=( const ConsumerItemClient& );
	};

	/**
		@class thomsonreuters::eoa::domain::marketprice::Mp::ConsumerService Mp.h "Domain/MarketPrice/Include/Mp.h"
		@brief Mp::ConsumerService represents a Market service provided by the platform.
	
		Application creates Mp::ConsumerService to register interest in items from a service.

		\remark All methods in this class are \ref ObjectLevelSafe

		@see Mp::ConsumerItem,
			Mp::ReqSpec,
			Mp::ConsumerItemClient,
			EoaString
	*/
	class EOA_DOMAIN_API ConsumerService
	{
	public:

		///@name Constructor
		//@{
		/** Constructs ConsumerService.
			@param[in] name specifies a known service name.
			@throw OmmMemoryExhaustionException if app runs out of memory
			@throw OmmInaccessibleLogFileException if app runs out of memory
		*/
		ConsumerService( const foundation::EoaString& serviceName );
		//@}

		///@name Destructor
		//@{
		/** Destructor.
		*/
		virtual ~ConsumerService();
		//@}

		///@name Operations
		//@{
		/** Request streaming subscription.
			\remark this is an asynchronous subscription request; e.g. this method does not wait for any response
			@throw OmmMemoryExhaustionException if app runs out of memory
			@param[in] itemName identifies item by name (assumes all other request attributes are defaulted).
			@param[in] consumerItemClient specifies a client callback class
			@param[in] closure specifies application assigned value for the item. The default value is NULL.
			@return ConsumerItem object
		*/
		ConsumerItem subscribe( const foundation::EoaString& itemName, ConsumerItemClient& consumerItemClient, void* closure = 0 );

		/** Request streaming subscription.
			\remark this is an asynchronous subscription request; e.g. this method does not wait for any response
			@throw OmmMemoryExhaustionException if app runs out of memory
			@param[in] itemSpec identifies item.
			@param[in] consumerItemClient specifies a client callback class
			@param[in] closure specifies application assigned value for the item. The default value is NULL.
			@return ConsumerItem object
		*/
		ConsumerItem subscribe( const ReqSpec& itemSpec, ConsumerItemClient& consumerItemClient, void* closure = 0 );

		/** Request streaming subscription.
			\remark this is an asynchronous subscription request; e.g. this method does not wait for any response
			@throw OmmMemoryExhaustionException if app runs out of memory
			@param[in] itemName identifies item by name (assumes all other request attributes are defaulted).
			@param[in] callBackFunc specifies a lambda expression, a function, a function pointer or any kind of function object.
			@return ConsumerItem object
		*/
		ConsumerItem subscribe( const foundation::EoaString& itemName, std::function<void( const ConsumerItem& )> callBackFunc );

		/** Request streaming subscription.
			\remark this is an asynchronous subscription request; e.g. this method does not wait for any response
			@throw OmmMemoryExhaustionException if app runs out of memory
			@param[in] itemSpec identifies item.
			@param[in] callBackFunc specifies a lambda expression, a function, a function pointer or any kind of function object.
			@return ConsumerItem object
		*/
		ConsumerItem subscribe( const ReqSpec& itemSpec, std::function<void( const ConsumerItem& )> callBackFunc );

		/** Request streaming subscription.
			this is an asynchronous subscription request; e.g. this method does not wait for any response
			@throw OmmMemoryExhaustionException if app runs out of memory
			@param[in] itemName identifies item by name (assumes all other request attributes are defaulted).
			@return ConsumerItem object
		*/
		ConsumerItem subscribe( const foundation::EoaString& itemName );

		/** Request streaming subscription.
			this is an asynchronous subscription request; e.g. this method does not wait for any response
			@throw OmmMemoryExhaustionException if app runs out of memory
			@param[in] itemSpec identifies item.
			@return ConsumerItem object
		*/
		ConsumerItem subscribe( const ReqSpec& itemSpec );

		/** Request non streaming subscription.
			this is an asynchronous subscription request; e.g. this method does not wait for any response
			@throw OmmMemoryExhaustionException if app runs out of memory
			@param[in] itemName identifies item by name (assumes all other request attributes are defaulted).
			@param[in] consumerItemClient specifies a client callback class
			@param[in] closure specifies application assigned value for the item. The default value is NULL.
			@return ConsumerItem object
		*/
		ConsumerItem snap( const foundation::EoaString& itemName, ConsumerItemClient& consumerItemClient, void* closure = 0 );

		/** Request non streaming subscription.
			this is an asynchronous subscription request; e.g. this method does not wait for any response
			@throw OmmMemoryExhaustionException if app runs out of memory
			@param[in] itemSpec identifies item.
			@param[in] consumerItemClient specifies a client callback class
			@param[in] closure specifies application assigned value for the item. The default value is NULL.
			@return ConsumerItem object
		*/
		ConsumerItem snap( const ReqSpec& itemSpec, ConsumerItemClient& consumerItemClient, void* closure = 0 );

		/** Request non streaming subscription.
			this is an asynchronous subscription request; e.g. this method does not wait for any response
			@throw OmmMemoryExhaustionException if app runs out of memory
			@param[in] itemName identifies item by name (assumes all other request attributes are defaulted).
			@param[in] callBackFunc specifies a lambda expression, a function, a function pointer or any kind of function object.
			@return ConsumerItem object
		*/
		ConsumerItem snap( const foundation::EoaString& itemName, std::function<void( const ConsumerItem& )> callBackFunc );

		/** Request non streaming subscription.
			this is an asynchronous subscription request; e.g. this method does not wait for any response
			@throw OmmMemoryExhaustionException if app runs out of memory
			@param[in] itemSpec identifies item.
			@param[in] callBackFunc specifies a lambda expression, a function, a function pointer or any kind of function object.
			@return ConsumerItem object
		*/
		ConsumerItem snap( const ReqSpec& itemSpec, std::function<void( const ConsumerItem& )> callBackFunc );

		/** Request non streaming subscription
			this is a synchronous subscription request; e.g. this method blocks till refresh complete is received
			@throw OmmMemoryExhaustionException if app runs out of memory
			@param[in] itemName identifies item by name (assumes all other request attributes are defaulted).
			@return ConsumerItem object
		*/
		ConsumerItem snap( const foundation::EoaString& itemName );

		/** Request non streaming subscription
			this is a synchronous subscription request; e.g. this method blocks till refresh complete is received
			@throw OmmMemoryExhaustionException if app runs out of memory
			@param[in] itemSpec identifies item.
			@return ConsumerItem object
		*/
		ConsumerItem snap( const ReqSpec& itemSpec );

		/** Request to "pull" item's data from EOA's internal cache to ConsumerItem owned by application
			@param[in] item ConsumerItem to be refreshed
			return void
		*/
		void refresh( ConsumerItem& item );
		//@}

	private:

		ConsumerService( const ConsumerService& );
		ConsumerService& operator=( const ConsumerService& );

		MpConsumerServiceImpl*	_pImpl;
	};

	/**
		@class thomsonreuters::eoa::domain::marketprice::Mp::ReqSpec Mp.h "Domain/MarketPrice/Include/Mp.h"
		@brief Mp::ReqSpec is used to specify item's attributes.
	
		Mp::ReqSpec allows application to register its interest in a specified item.
		Among other attributes, Mp::ReqSpec conveys item's name, and desired quality of service.
	
		\remark All methods in this class are \ref SingleThreaded.
	*/
	class EOA_DOMAIN_API ReqSpec
	{
	public:

		/** @enum Rate
			An enumeration representing Qos Rate.
		*/
		enum Rate
		{
			TickByTickEnum = 0,						/*!< Rate is Tick By Tick, indicates every change to
														information is conveyed */

			JustInTimeConflatedEnum = 0xFFFFFF00,	/*!< Rate is Just In Time Conflated, indicates extreme
														bursts of data may be conflated */

			BestConflatedRateEnum = 0xFFFFFFFF,		/*!< Request Rate with range from 1 millisecond conflation to maximum conflation. */

			BestRateEnum = 0xFFFFFFFE				/*!< Request Rate with range from tick-by-tick to maximum conflation. */
		};

		/** @enum Timeliness
			An enumeration representing Qos Timeliness.
		*/
		enum Timeliness
		{
			RealTimeEnum = 0,							/*!< Timeliness is RealTime, indicates information is updated
															as soon as new information becomes available */

			BestDelayedTimelinessEnum = 0xFFFFFFFF,		/*!< Request Timeliness with range from one second delay to maximum delay. */

			BestTimelinessEnum = 0xFFFFFFFE				/*!< Request Timeliness with range from real-time to maximum delay. */
		};

		///@name Constructor
		//@{
		/** Default Constructor.
			@throw OmmMemoryExhaustionException if app runs out of memory
		*/
		ReqSpec();

		/** Copy Constructor.
			@throw OmmMemoryExhaustionException if app runs out of memory
			@param[in] other object to copy from
		*/
		ReqSpec( const ReqSpec& other );

		/** Move Constructor.
			@param[in] other object to move from
		*/
		ReqSpec( ReqSpec&& other );

		/** Constructs ReqSpec.
			@throw OmmMemoryExhaustionException if app runs out of memory
			@param[in] name specifies an item name.
		*/
		ReqSpec( const foundation::EoaString& name );
		//@}

		///@name Destructor
		//@{
		/** Destructor.
		*/
		virtual ~ReqSpec();
		//@}

		///@name Operations
		//@{
		/** Assignment operator.
			@param[in] other object to copy from
			@return reference to this object
		*/
		ReqSpec& operator=( const ReqSpec& other );

		/** Specifies item name.
			@param[in] name represents item name
			@return reference to this object
		*/
		ReqSpec& name( const foundation::EoaString& name );

		/** Specifies name type.
			@param[in] nameType represents name type
			@return reference to this object
		*/
		ReqSpec& nameType( foundation::UInt8 nameType );

		/** Specifies item priority.
			@param[in] priorityClass represents items's priority class. 
			@param[in] priorityCount represents items's priority count.
			@return reference to this object
		*/
		ReqSpec& priority( foundation::UInt8 priorityClass = 1, foundation::UInt16 priorityCount = 1 );

		/** Specifies Qos.
			@param[in] timeliness represents Qos timeliness.
			@param[in] rate represents Qos rate.
			@return reference to this object
		*/
		ReqSpec& qos( foundation::Int32 timeliness = BestTimelinessEnum , foundation::Int32 rate = BestRateEnum );

		/** Specifies conflated in updates.
			@param[in] conflatedInUpdates represents conflated in updates.
			@return reference to this object
		*/
		ReqSpec& conflatedInUpdates( bool conflatedInUpdates = false );

		/** Specifies private stream.
			@param[in] privateStream represents private stream.
			@return reference to this object
		*/
		ReqSpec& privateStream( bool privateStream = false );

		/** Clears the ReqSpec.
			\remark Invoking clear() method clears all the values and resets all the defaults
			@return reference to this object
		*/
		ReqSpec& clear();
		//@}

	private:

		friend class ConsumerService;

		MpReqSpecImpl*		_pImpl;
	};


	/**
		@class thomsonreuters::eoa::domain::marketprice::Mp::Quote Mp.h "Domain/MarketPrice/Include/Mp.h"
		@brief Mp::Quote specifies MarketPrice payload.
	
		\remark All methods in this class are \ref SingleThreaded.

		@see Mp::ConsumerItem,
			Mp::RefreshInfo,
			Mp::UpdateInfo
	*/
	class EOA_DOMAIN_API Quote
	{
	public:

		///@name Accessors
		//@{
		/** Indicates presence of Bid in the Quote.
			@return true if Bid is set; false otherwise
		*/
		bool hasBid() const;

		/** returns double representation of Bid
			@throw OmmInvalidUsageException if hasBid() returns false
			@return double value of Bid
		*/
		double getBid() const;

		/** returns string representation of Bid
			@return EoaString containing value of Bid if it is present
		*/
		const foundation::EoaString& getBidAsString() const;

		/** Indicates presence of Ask in the Quote.
			@return true if Ask is set; false otherwise
		*/
		bool hasAsk() const;

		/** returns double representation of Ask
			@throw OmmInvalidUsageException if hasAsk() returns false
			@return double value of Ask
		*/
		double getAsk() const;

		/** returns string representation of Ask
			@return EoaString containing value of Ask if it is present
		*/
		const foundation::EoaString& getAskAsString() const;

		/** Indicates presence of TradePrice in the Quote.
			@return true if TradePrice is set; false otherwise
		*/
		bool hasTradePrice() const;

		/** returns double representation of TradePrice
			@throw OmmInvalidUsageException if hasTradePrice() returns false
			@return double value of TradePrice
		*/
		double getTradePrice() const;

		/** returns string representation of TradePice
			@return EoaString containing value of TradePrice if it is present
		*/
		const foundation::EoaString& getTradePriceAsString() const;

		/** returns Quote as Node object
			\remark allows retrieval of any field from the quote object
		*/
		const foundation::Node& getAsNode() const;

		/** Returns a string representation of the class instance.
			@throw OmmMemoryExhaustionException if app runs out of memory
			@return string representation of the class instance
		*/
		const foundation::EoaString& toString() const;

		/** Operator const char* overload.
			@throw OmmMemoryExhaustionException if app runs out of memory
		*/
		operator const char* () const;
		//@}

	private:

		const foundation::EoaString& toString( foundation::UInt64 indent ) const;

		friend class MpRefreshInfoImpl;
		friend class MpStatusInfoImpl;
		friend class MpUpdateInfoImpl;
		friend class MpConsumerItemImpl;

		Quote();
		virtual ~Quote();

		const foundation::Node*				_pNode;
		mutable foundation::EoaString		_toString;

		Quote( const Quote& );
		Quote& operator=( const Quote& );
	};
	

	/**
		@class thomsonreuters::eoa::domain::marketprice::Mp::Qos Mp.h "Domain/MarketPrice/Include/Mp.h"
		@brief Mp::Qos represents Quality Of Service information in MarkePrice.

		\remark All methods in this class are \ref SingleThreaded.

		@see Mp::RefreshInfo
	*/
	class EOA_DOMAIN_API Qos
	{
	public :

		/** @enum QosRate
			An enumeration representing qos rate.
		*/
		enum Rate
		{
			TickByTickEnum = 0,							/*!< Indicates tick by tick rate */

			JustInTimeConflatedEnum = 0xFFFFFF00		/*!< Indicates just in time conflated rate */
		};

		/** @enum QosTimeliness
			An enumeration representing Qos timeliness.
		*/
		enum Timeliness
		{
			RealTimeEnum = 0,					/*!< Indicates real time timeliness */

			InexactDelayedEnum = 0xFFFFFFFF		/*!< Indicates timeliness with an unknown delay value */
		};

		///@name Accessors
		//@{	
		/** Returns the QosRate value as a string format.
			@return string representation of this object Rate if it is present
		*/
		const foundation::EoaString& getRateAsString() const;
		
		/** Returns the QosTimeliness value as a string format.
			@return string representation of this object timeliness if it is present
		*/
		const foundation::EoaString& getTimelinessAsString() const;

		/** Returns Timeliness.
			@return value of OmmQos::Timeliness
		*/
		foundation::UInt32 getTimeliness() const;

		/** Returns Rate.
			@return value of OmmQos::Rate
		*/
		foundation::UInt32 getRate() const;

		/** Returns a string representation of the class instance.
			@throw OmmMemoryExhaustionException if app runs out of memory
			@return string representation of the class instance
		*/
		const foundation::EoaString& toString() const;

		/** Operator const char* overload.
			@throw OmmMemoryExhaustionException if app runs out of memory
		*/
		operator const char* () const;
		//@}

	private :

		friend class MpRefreshInfoImpl;

		Qos();
		virtual ~Qos();
		Qos( const Qos& );
		Qos& operator=( const Qos& );

		MpQosImpl*	_pImpl;
	};


	/**
		@class thomsonreuters::eoa::domain::marketprice::Mp::State Mp.h "Domain/MarketPrice/Include/Mp.h"
		@brief Mp::State represents State information in MarketPrice. 

		\remark All methods in this class are \ref SingleThreaded.

		@see Mp::RefreshInfo,
			Mp::StatusInfo
	*/
	class EOA_DOMAIN_API State 
	{
	public :

		/** @enum StreamState
			An enumeration representing item stream state.
		*/
		enum StreamState
		{
			OpenEnum			= 1,	/*!< Indicates the stream is opened and will incur interest
											after the final refresh. */

			NonStreamingEnum	= 2,	/*!< Indicates the item will not incur interest after
											the final refresh.  */

			ClosedRecoverEnum	= 3,	/*!< Indicates the stream is closed, typically unobtainable or
											identity indeterminable due to a comms outage. The item may be
											available in the future. */

			ClosedEnum			= 4,	/*!< Indicates the stream is closed. */

			ClosedRedirectedEnum = 5	/*!< Indicates the stream is closed and has been renamed.
											The stream is available with another name. This stream state is only
											valid for refresh messages. The new item name is in the Name get 
											accessor method. */
		};

		/** @enum DataState
			An enumeration representing item data state.
		*/
		enum DataState
		{
			NoChangeEnum	= 0,		/*!< Indicates the health of the data item did not change. */

			OkEnum			= 1,		/*!< Indicates the entire data item is healthy. */

			SuspectEnum		= 2			/*!< Indicates the health of some or all of the item's data is stale or unknown. */
		};

		/** #enum StatusCode
			An enumeration representing status code.
		*/
		enum StatusCode
		{
			NoneEnum						= 0,	/*!< None */
			NotFoundEnum					= 1,	/*!< Not Found */
			TimeoutEnum						= 2,	/*!< Timeout */
			NotAuthorizedEnum				= 3,	/*!< Not Authorized */
			InvalidArgumentEnum				= 4,	/*!< Invalid Argument */
			UsageErrorEnum					= 5,	/*!< Usage Error */
			PreemptedEnum					= 6,	/*!< Pre-empted */
			JustInTimeConflationStartedEnum	= 7,	/*!< Just In Time Filtering Started */
			TickByTickResumedEnum			= 8,	/*!< Tick By Tick Resumed */
			FailoverStartedEnum				= 9,	/*!< Fail-over Started */
			FailoverCompletedEnum			= 10,	/*!< Fail-over Completed */
			GapDetectedEnum					= 11,	/*!< Gap Detected */
			NoResourcesEnum					= 12,	/*!< No Resources */
			TooManyItemsEnum				= 13,	/*!< Too Many Items */
			AlreadyOpenEnum					= 14,	/*!< Already Open */
			SourceUnknownEnum				= 15,	/*!< Source Unknown */
			NotOpenEnum						= 16,	/*!< Not Open */
			NonUpdatingItemEnum				= 19,	/*!< Non Updating Item */
			UnsupportedViewTypeEnum			= 20,	/*!< Unsupported View Type */
			InvalidViewEnum					= 21,	/*!< Invalid View */
			FullViewProvidedEnum			= 22,	/*!< Full View Provided */
			UnableToRequestAsBatchEnum		= 23,	/*!< Unable To Request As Batch */
			NoBatchViewSupportInReqEnum		= 26,	/*!< Request does not support batch or view */
			ExceededMaxMountsPerUserEnum	= 27,	/*!< Exceeded maximum number of mounts per user */
			ErrorEnum						= 28,	/*!< Internal error from sender */
			DacsDownEnum					= 29,	/*!< Connection to DACS down, users are not allowed to connect */
			UserUnknownToPermSysEnum		= 30,	/*!< User unknown to permissioning system, it could be DACS, AAA or EED */
			DacsMaxLoginsReachedEnum		= 31,	/*!< Maximum logins reached */
			DacsUserFoundationToAppDeniedEnum	= 32,	/*!< User is not allowed to use application */
			InvalidFormedMsgEnum			= 256,
			ChannelUnavailableEnum			= 257,
			ServiceUnavailableEnum			= 258,
			ServiceDownEnum					= 259,
			ServiceNotAcceptingRequestsEnum = 260,
			LoginClosedEnum					= 261,
			DirectoryClosedEnum				= 262,
			ItemNotFoundEnum				= 263,
			DictionaryUnavailableEnum		= 264,
			FieldIdNotFoundDictionaryUnavailableEnum = 265,
			ItemRequestTimeoutEnum			= 266
		};

		///@name Accessors
		//@{	
		/** Returns the StreamState value as a string format.
			@return string representation of this object StreamState if it is present
		*/
		const foundation::EoaString& getStreamStateAsString() const;
		
		/** Returns the DataState value as a string format.
			@return string representation of this object DataState if it is present
		*/
		const foundation::EoaString& getDataStateAsString() const;
		
		/** Returns the StatusCode value as a string format.
			@return string representation of this object StatusCode if it is present
		*/
		const foundation::EoaString& getStatusCodeAsString() const;

		/** Returns StreamState.
			@return value of StreamState
		*/
		StreamState getStreamState() const;

		/** Returns DataState.
			@return value of DataState
		*/
		DataState getDataState() const;

		/** Returns StatusCode.
			@return value of StatusCode
		*/
		foundation::UInt16 getStatusCode() const;

		/** Returns StatusText.
			@return EoaString containing status text information
		*/
		const foundation::EoaString& getStatusText() const;

		/** Returns a string representation of the class instance.
			@throw OmmMemoryExhaustionException if app runs out of memory
			@return string representation of the class instance
		*/
		const foundation::EoaString& toString() const;

		/** Operator const char* overload.
			@throw OmmMemoryExhaustionException if app runs out of memory
		*/
		operator const char* () const;
		//@}

	private :

		friend class MpRefreshInfoImpl;
		friend class MpStatusInfoImpl;

		State();
		virtual ~State();
		State( const State& );
		State& operator=( const State& );

		MpStateImpl*		_pImpl;
	};

	/** Returns a string with the version of the Mp class.
		@return string
	*/
	static const foundation::EoaString& getVersion();

private:

	static const foundation::EoaString		_version;

	Mp();
	virtual ~Mp();

	Mp( const Mp& );
	Mp&	operator=( const Mp& );
};

}

}

}

}

#endif // __thomsonreuters_eoa_domain_marketprice_mp_h
