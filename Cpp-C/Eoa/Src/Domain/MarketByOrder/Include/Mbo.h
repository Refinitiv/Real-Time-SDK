/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_domain_marketbyorder_mbo_h
#define __thomsonreuters_eoa_domain_marketbyorder_mbo_h

#include "Foundation/Include/EoaBuffer.h"
#include "Foundation/Include/EoaString.h"
#include "Foundation/Include/Leaf.h"
#include "Foundation/Include/Node.h"
#include "Foundation/Include/OmmDate.h"
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
	@namespace domain::marketbyorder
	@brief The domain::marketbyorder namespace defines all the interfaces used by
	the EOA Market By Order package as well as applications using these package.
	
	The EOA Market By Order package provides RDM MarketByOrder specific services and
	functionalities, which are encapsulated by the Mbo class.
*/
namespace marketbyorder {

class MboConsumerItemImpl;
class MboReqSpecImpl;
class MboConsumerServiceImpl;
class MboRefreshInfoImpl;
class MboUpdateInfoImpl;
class MboStatusInfoImpl;
class MboQosImpl;
class MboStateImpl;

/**
	@class thomsonreuters::eoa::domain::marketbyorder::Mbo Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
	@brief Mbo class contains all classes used to request and process MarketByOrder item.

	@see Mbo::ConsumerItem,
		Mbo::RefreshInfo,
		Mbo::StatusInfo,
		Mbo::UpdateInfo,
		Mbo::ConsumerItemClient,
		Mbo::ConsumerService,
		Mbo::ReqSpec,
		Mbo::OrderBook,
		Mbo::Order,
		Mbo::Summary
*/
class EOA_DOMAIN_API Mbo
{
public:

class Qos;
class State;
class OrderBook;
class Order;
class Summary;
class ConsumerItemClient;
class ReqSpec;
class ConsumerService;


	/**
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::ConsumerItem Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::ConsumerItem provides access to MarketByOrder item's key, state and payload.

		\remark All methods in this class are \ref SingleThreaded.

		@see Mbo::OrderBook
	*/
	class ConsumerItem
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

		/** Returns the orderbook of this item
			@return reference to OrderBook object
		*/
		const OrderBook& getOrderBook() const;

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

		friend class MboConsumerServiceImpl;
		friend class ConsumerService;

		MboConsumerItemImpl*							_pImpl;
		void*											_closure;
		ConsumerItemClient*								_pClient;
		std::function<void( const ConsumerItem& )>		_callBackFunc;
	};


	/**
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::RefreshInfo Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::RefreshInfo provides access to the information brought in by a MarketByOrder refresh message.

		\remark All methods in this class are \ref SingleThreaded.

		@see Mbo::State,
			Mbo::Qos,
			Mbo::OrderBook
	*/
	class RefreshInfo
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

		/** Indicates presence of PartNum.
			@return true if part number is set; false otherwise
		*/
		bool hasPartNum() const;

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

		/** Returns the contained orderbook.
			@return reference to OrderBook object
		*/
		const OrderBook& getOrderBook() const;

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

		/** Returns PartNum.
			@throw OmmInvalidUsageException if hasPartNum() returns false
			@return part number
		*/
		foundation::UInt16 getPartNum() const;

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

		friend class MboConsumerServiceImpl;

		RefreshInfo();
		virtual ~RefreshInfo();

		MboRefreshInfoImpl*	_pImpl;

		RefreshInfo( const RefreshInfo& );
		RefreshInfo& operator=( const RefreshInfo& );
	};


	/**
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::StatusInfo Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::StatusInfo provides access to the information brought in by a MarketByOrder status message.

		\remark All methods in this class are \ref SingleThreaded.

		@see Mbo::State
	*/
	class StatusInfo
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

		friend class MboConsumerServiceImpl;

		StatusInfo();
		virtual ~StatusInfo();

		MboStatusInfoImpl*		_pImpl;

		StatusInfo( const StatusInfo& );
		StatusInfo& operator=( const StatusInfo& );
	};


	/**
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::UpdateInfo Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::UpdateInfo provides access to the information brought in by an MarketByOrder update message.

		\remark All methods in this class are \ref SingleThreaded.

		@see Mbo::OrderBook
	*/
	class UpdateInfo
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

		/** Returns the contained orderbook.
			@return reference to OrderBook object
		*/
		const OrderBook& getOrderBook() const;

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

		friend class MboConsumerServiceImpl;

		UpdateInfo();
		virtual ~UpdateInfo();

		MboUpdateInfoImpl*		_pImpl;

		UpdateInfo( const UpdateInfo& );
		UpdateInfo& operator=( const UpdateInfo& );
	};


	/**
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::ConsumerItemClient Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::ConsumerItemClient class provides callback interfaces notifying application about changes to registered items.

		Application needs to implement an application client class inheriting from Mbo::ConsumerItemClient.
		In its own class, application needs to override callback methods it desires to use for item processing.
		Default empty callback methods are implemented by Mbo::ConsumerItemClient class.

		\remark Thread safety of all the methods in this class depends on the user's implementation.

		@see Mbo::ConsumerItem,
			Mbo::RefreshInfo,
			Mbo::UpdateInfo,
			Mbo::StatusInfo
	*/
	class ConsumerItemClient
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

		/** Invoked upon receiving item partial refresh. 
			@param[out] current - accrued item information (contains the received change)
			@param[out] change - received item partial refresh
			@param[out] closure - item closure specified on subscribe( ... , ConsumerItemClient& , void* closure ) or snap( ... , ConsumerItemClient& , void* closure )
			@return void
		*/
		virtual void onConsumerItemPartial( const ConsumerItem& current, const RefreshInfo& change, void* closure ) {}

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
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::ConsumerService Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::ConsumerService represents a MarketByOrder service provided by the platform.
	
		Application creates Mbo::ConsumerService to register interest in items from a service.

		\remark All methods in this class are \ref ObjectLevelSafe

		@see Mbo::ConsumerItem,
			Mbo::ReqSpec,
			Mbo::ConsumerItemClient,
			EoaString
	*/
	class ConsumerService
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
			@param[in] itemSpec identifies item.
			@param[in] consumerItemClient specifies a client callback class
			@param[in] closure specifies application assigned value for the item. The default value is NULL.
			@return ConsumerItem object
		*/
		ConsumerItem subscribe( const ReqSpec& itemSpec, ConsumerItemClient& consumerItemClient, void* closure = 0 );

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
			@param[in] itemSpec identifies item.
			@return ConsumerItem object
		*/
		ConsumerItem subscribe( const ReqSpec& itemSpec );

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
			@param[in] itemSpec identifies item.
			@param[in] callBackFunc specifies a lambda expression, a function, a function pointer or any kind of function object.
			@return ConsumerItem object
		*/
		ConsumerItem snap( const ReqSpec& itemSpec, std::function<void( const ConsumerItem& )> callBackFunc );

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

		MboConsumerServiceImpl*		_pImpl;
	};


	/**
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::ReqSpec Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::ReqSpec is used to specify item's attributes.
	
		Mbo::ReqSpec allows application to register its interest in a specified item.
		Among other attributes, Mbo::ReqSpec conveys item's name, domain type, and
		desired quality of service.
	
		\remark All methods in this class are \ref SingleThreaded.
	*/
	class ReqSpec
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

		MboReqSpecImpl*		_pImpl;
	};


	/**
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::Side Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::Side is an enumeration specifying order's side.
	
		@see Mbo::Order
	*/
	enum class Side
	{
		UndefinedEnum = 0,	/*!< Indicates undefined side */
		BidEnum = 1,		/*!< Indicates order on the Bid side */
		AskEnum = 2			/*!< Indicates order on the Ask side */
	};


	/**
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::Currency Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::Currency is an enumeration specifying order's currency.
	
		@see Mbo::Summary
	*/
	enum class Currency
	{
		NotAllocatedEnum = 0,	/*!< Indicates not allocated value */
		CnyEnum = 156,			/*!< Indicates Chinese yuan renminbi */
		JpyEnum = 392,			/*!< Indicates Japanese yen */
		GbpEnum = 826,			/*!< Indicates UK pound sterling */
		UsdEnum = 840,			/*!< Indicates US dollar */
		EurEnum = 978			/*!< Indicates Euro */
	};


	/**
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::MarketState Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::MarketState is an enumeration specifying state of market.

		@see Mbo::Summary
	*/
	enum class MarketState
	{
		FasEnum = 1,		/*!< Indicates fast market */
		ThaEnum = 3,		/*!< Indicates trading halted */
		TreEnum = 4,		/*!< Indicates trading resumed */
		ClsEnum = 8,		/*!< Indicates market closed */
		OclEnum = 9			/*!< Indicates official close */
	};


	/**
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::ExchangeId Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::ExchangeId is an enumeration specifying id of exchange.

		@see Mbo::Summary
	*/
	enum class ExchangeId
	{
		UndefinedEnum = 0,	/*!< Indicates undefined exchange */
		AseEnum = 1,		/*!< Indicates NYSE AMEX */
		NysEnum = 2,		/*!< Indicates New York Stock Exchange */
		BosEnum = 3,		/*!< Indicates Boston Stock Exchange */
		CinEnum = 4,		/*!< Indicates National Stock Exchange */
		MidEnum = 8,		/*!< Indicates Chicago Stock Exchange */
		TorEnum = 10,		/*!< Indicates Toronto Stock Exchange */
		MonEnum = 11,		/*!< Indicates Montreal Stock Exchange */
		CmeEnum = 20,		/*!< Indicates CME:Chicago Mercantile Commodities */
		CbtEnum = 978		/*!< Indicates Chicago Board of Trade */
	};


	/**
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::PriceRankRule Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::PriceRankRule is an enumeration specifying price ranking rule.

		@see Mbo::Summary
	*/
	enum class PriceRankRule
	{
		NorEnum = 1,			/*!< Indicates Normal Market */
		InvEnum = 2,			/*!< Indicates Inverted Market */
		SqxEnum = 3,			/*!< Indicates Firm Quote, Limit Order (LSE SETSqx) */
		PonEnum = 4,			/*!< Indicates Price, Order Type, Time Priority Sequence Number */
		FiveEnum = 5			/*!< Indicates Not allocated (was Quote Type, Market Maker ID, Price, Quote Access Payment, Size, Time) */
	};


	/**
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::OrderRankRule Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::OrderRankRule is an enumeration specifying order and price ranking rule.

		@see Mbo::Summary
	*/
	enum class OrderRankRule
	{
		PstEnum = 1,		/*!< Indicates Price, Size, Time */
		PtsEnum = 2,		/*!< Indicates Price, Time, Size */
		PrsEnum = 3,		/*!< Indicates Price, Rank, Size */
		OptSEnum = 4,		/*!< Indicates Order Type, Price, Time, Size */
		PnEnum = 5,			/*!< Indicates Price, Time Priority Sequence Number */
		QmpEnum = 6,		/*!< Indicates Quote Type, Market Maker ID, Price, Quote Access Payment, Size, Time */
		PenEnum = 7			/*!< Indicates Price, Exchange Sequence Number */
	};


	/**
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::TradeUnit Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::TradeUnit is an enumeration specifying Trade Unit or precision at which prices are set.

		@see Mbo::Summary
	*/
	enum class TradeUnit
	{
		IntEnum = 0,			/*!< Indicates whole number */
		OneDpEnum = 1,			/*!< Indicates one decimal place */
		TwoDpEnum = 2,			/*!< Indicates two decimal places */
		ThreeDpEnum = 3,		/*!< Indicates three decimal places */
		FourDpEnum = 4,			/*!< Indicates four decimal places */
		FiveDpEnum = 5,			/*!< Indicates five decimal places */
		SixDpEnum = 6,			/*!< Indicates six decimal places */
		SevenDpEnum = 7,		/*!< Indicates seven decimal places */
		UndefinedEnum = 8		/*!< Indicates undefined */
	};


	/**
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::Order Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::Order specifies an order of MarketByOrder.
	
		\remark All methods in this class are \ref SingleThreaded.

		@see Mbo::OrderBook
	*/
	class Order
	{
	public:

		///@name Accessors
		//@{
		/** Returns ID of this order
			@return EoaBuffer containing order id
		*/
		const foundation::EoaBuffer& getId() const;

		/** Returns ID of this order
			@return EoaString containing order id
		*/
		const foundation::EoaString& getIdAsString() const;

		/** Returns action associated with this order
			@return Action
		*/
		foundation::Action getAction() const;

		/** Returns the action associated with this order as a string
			@return EoaString containing action name
		*/
		const foundation::EoaString& getActionAsString() const;

		/** Indicates presence of order's price
			@return true if order's price is set; false otherwise
		*/
		bool hasPrice() const;

		/** Returns order's price
			@throw OmmInvalidUsageException if hasPrice() returns false
			@return double representing order's price
		*/
		double getPrice() const;

		/** Returns order's price
			@return EoaString containing order's price if it is present
		*/
		const foundation::EoaString& getPriceAsString() const;

		/** Indicates presence of order's size
			@return true if order's size is set; false otherwise
		*/
		bool hasSize() const;

		/** Returns order's size
			@throw OmmInvalidUsageException if hasSize() returns false
			@return double representing order's size
		*/
		double getSize() const;

		/** Returns order's size
			@return EoaString containing order's size if it is present
		*/
		const foundation::EoaString& getSizeAsString() const;

		/** Indicates presence of order's side
			@return true if order's side is set; false otherwise
		*/
		bool hasSide() const;

		/** Returns order's side
			@throw OmmInvalidUsageException if hasSide() returns false
			@return Side enumeration
		*/
		Side getSide() const;

		/** Returns order's side
			@return EoaString containing order's side if it is present
		*/
		const foundation::EoaString& getSideAsString() const;

		/** Indicates presence of order's bid.
			@return true if order's bid is set; false otherwise
		*/
		bool hasBid() const;

		/** returns double representation of order's bid
			@throw OmmInvalidUsageException if hasBid() returns false
			@return double value of order's bid
		*/
		double getBid() const;

		/** returns string representation of oirder's bid
			@return EoaString containing value of order's bid if it is present
		*/
		const foundation::EoaString& getBidAsString() const;

		/** Indicates presence of order's ask
			@return true if order's ask is set; false otherwise
		*/
		bool hasAsk() const;

		/** returns double representation of order's ask
			@throw OmmInvalidUsageException if hasAsk() returns false
			@return double value of order's ask
		*/
		double getAsk() const;

		/** returns string representation of order's ask
			@return EoaString containing value of order's ask if it is present
		*/
		const foundation::EoaString& getAskAsString() const;

		/** Indicates presence of order's ask size
			@return true if order's ask size is set; false otherwise
		*/
		bool hasAskSize() const;

		/** Returns order's ask size
			@throw OmmInvalidUsageException if hasAskSize() returns false
			@return double representing order's ask size
		*/
		double getAskSize() const;

		/** Returns order's ask size
			@return EoaString containing order's ask size if it is present
		*/
		const foundation::EoaString& getAskSizeAsString() const;

		/** Indicates presence of order's bid size
			@return true if order's bid size is set; false otherwise
		*/
		bool hasBidSize() const;

		/** Returns order's bid size
			@throw OmmInvalidUsageException if hasBidSize() returns false
			@return double representing order's bid size
		*/
		double getBidSize() const;

		/** Returns order's bid size
			@return EoaString containing order's bid size if it is present
		*/
		const foundation::EoaString& getBidSizeAsString() const;

		/** Indicates presence of order's quote time
			@return true if order's quote time is set; false otherwise
		*/
		bool hasQuoteTime() const;

		/** Returns order's quote time
			@throw OmmInvalidUsageException if hasQuoteTime() returns false
			@return UInt64 representing order's quote time
		*/
		foundation::UInt64 getQuoteTime() const;

		/** Returns order's quote time
			@return EoaString containing order's quote time if it is present
		*/
		const foundation::EoaString& getQuoteTimeAsString() const;

		/** returns Order as Node
			\remark allows retrieval of any field from the Order
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

		friend class MboRefreshInfoImpl;
		friend class MboStatusInfoImpl;
		friend class MboUpdateInfoImpl;
		friend class MboConsumerItemImpl;
		friend class OrderBook;

		const foundation::EoaString& toString( foundation::UInt64 indent ) const;

		Order();
		virtual ~Order();

		const foundation::Node*			_pNode;
		mutable foundation::EoaString	_toString;
		mutable foundation::EoaString	_idString;

		Order( const Order& );
		Order& operator=( const Order& );
	};


	/**
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::Summary Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::Summary specifies summary of MarketByOrder.
	
		\remark All methods in this class are \ref SingleThreaded.

		@see Mbo::OrderBook
	*/
	class Summary
	{
	public:

		///@name Accessors
		//@{
		/** Indicates presence of the equity RIC name
			@return true if the RIC name is set; false otherwise
		*/
		bool hasStockRic() const;

		/** Returns the equity RIC name
			@throw OmmInvalidUsageException if hasStockRic() returns false
			@return EoaString containing the equity RIC name
		*/
		const foundation::EoaString& getStockRic() const;

		/** Returns the equity RIC name
			@return EoaString containing the equity RIC name if it is present
		*/
		const foundation::EoaString& getStockRicAsString() const;

		/** Indicates presence of the QuoteDate
			@return true if the Quotedate is set; false otherwise
		*/
		bool hasQuoteDate() const;

		/** Returns the QuoteDate
			@throw OmmInvalidUsageException if hasQuoteDate() returns false
			@return QuoteDate
		*/
		const foundation::OmmDate& getQuoteDate() const;

		/** Returns the QuoteDate name
			@return EoaString containing QuoteDate if it is present
		*/
		const foundation::EoaString& getQuoteDateAsString() const;

		/** Indicates presence of the TradeUnits
			@return true if the TradeUnits is set; false otherwise
		*/
		bool hasTradeUnits() const;

		/** Returns the TradeUnits
			@throw OmmInvalidUsageException if hasTradeUnits() returns false
			@return TradeUnits enumeration
		*/
		TradeUnit getTradeUnits() const;

		/** Returns the TradeUnits name
			@return EoaString containing TradeUnits name if it is present
		*/
		const foundation::EoaString& getTradeUnitsAsString() const;

		/** Indicates presence of the Exchange ID
			@return true if the Exchange ID is set; false otherwise
		*/
		bool hasExchangeId() const;

		/** Returns the Exchange ID
			@throw OmmInvalidUsageException if hasExchangeId() returns false
			@return Exchange ID enumeration
		*/
		ExchangeId getExchangeId() const;

		/** Returns the Exchange ID name
			@return EoaString containing Exchange ID name if it is present
		*/
		const foundation::EoaString& getExchangeIdAsString() const;

		/** Indicates presence of the Maket State
			@return true if the Market State is set; false otherwise
		*/
		bool hasMarketState() const;

		/** Returns the Market State
			@throw OmmInvalidUsageException if hasMarketState() returns false
			@return Market State enumeration
		*/
		MarketState getMarketState() const;

		/** Returns the Market State name
			@return EoaString containing Market State name if it is present
		*/
		const foundation::EoaString& getMarketStateAsString() const;

		/** Indicates presence of the currency
			@return true if the currency is set; false otherwise
		*/
		bool hasCurrency() const;

		/** Returns the Currency
			@throw OmmInvalidUsageException if hasCurrency() returns false
			@return Currency enumeration
		*/
		Currency getCurrency() const;

		/** Returns the Currency name
			@return EoaString containing currency name if it is present
		*/
		const foundation::EoaString& getCurrencyAsString() const;

		/** Indicates presence of the price ranking rule
			@return true if the price ranking rule is set; false otherwise
		*/
		bool hasPriceRankRule() const;

		/** Returns the Price Ranking Rule
			@throw OmmInvalidUsageException if hasPriceRankRule() returns false
			@return Price Ranking Rule enumeration
		*/
		PriceRankRule getPriceRankRule() const;

		/** Returns the Price Ranking Rule name
			@return EoaString containing Price Ranking Rule name if it is present
		*/
		const foundation::EoaString& getPriceRankRuleAsString() const;

		/** Indicates presence of the Order Ranking Rule
			@return true if the Order Ranking Rule is set; false otherwise
		*/
		bool hasOrderRankRule() const;

		/** Returns the Order Ranking Rule
			@throw OmmInvalidUsageException if hasOrderRankRule() returns false
			@return Order Ranking Rule enumeration
		*/
		OrderRankRule getOrderRankRule() const;

		/** Returns the Order Ranking Rule name
			@return EoaString containing Order Ranking Rule name if it is present
		*/
		const foundation::EoaString& getOrderRankRuleAsString() const;

		/** returns Summary as Node
			\remark allows retrieval of any field from the Summary
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

		friend class MboRefreshInfoImpl;
		friend class MboStatusInfoImpl;
		friend class MboUpdateInfoImpl;
		friend class MboConsumerItemImpl;
		friend class OrderBook;

		const foundation::EoaString& toString( foundation::UInt64 indent ) const;

		Summary();
		virtual ~Summary();

		const foundation::Node*			_pNode;
		mutable foundation::EoaString	_toString;

		Summary( const Summary& );
		Summary& operator=( const Summary& );
	};


	/**
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::OrderBook Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::OrderBook specifies MarketByOrder payload.
	
		\remark All methods in this class are \ref SingleThreaded.

		@see Mbo::Order,
			Mbo::Summary,
			Mbo::RefreshInfo,
			Mbo::UpdateInfo,
			Mbo::ConsumerItem
	*/
	class OrderBook
	{
	public:

		/**
			@class thomsonreuters::eoa::domain::marketbyorder::Mbo::OrderBook::const_iterator Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
			@brief Mbo::OrderBook::const_iterator is an external and explicit Mbo::OrderBook read iterator.

			Mbo::OrderBook::const_iterator allows for concurent and independent iteration over the same Mbo::OrderBook instance.

			The following code snippet shows getting data from Mbo::OrderBook using the "for-each" loop.

			\code

			for ( const auto& order : orderBook )
				cout << order << endl;

			\endcode
	
			The following code snippet shows extracting data from Mbo::OrderBook using the const_iterator.

			\code

			for ( auto iter = orderBook.begin(); iter != orderBook.end(); ++iter )
				cout << *iter << endl;

			\endcode

			\remark Undefined behavior may occur while attempting to dereference Mbo::OrderBook::const_iterator objects
					equal to the Mbo::OrderBook::end().
			\remark Undefined behavior may occur while using objects of Mbo::OrderBook::const_iterator associated with 
					the Mbo::OrderBook object who is out of scope.
			\remark All methods in this class are \ref SingleThreaded.

			@see Mbo::Order,
				Mbo::OrderBook
		*/
		class const_iterator
		{
		public :

			///@name Constructor
			//@{
			/** Default constructor.
				\remark equivalent to Mbo::OrderBook::end()
			*/
			const_iterator();
		
			/** Constructor set to the begining of Mbo::OrderBook
				@param[in] orderBook Mbo::OrderBook object to whose begining's this iterator will point to
			*/
			const_iterator( const Mbo::OrderBook& orderBook );

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
				@return const reference to Mbo::Order
			*/
			const Mbo::Order& operator*() const;

			/** Pointer operator.
				@return const pointer to Mbo::Order
			*/
			const Mbo::Order* operator->() const;

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

			friend class OrderBook;

			foundation::Node::const_iterator	_nodeIterator;
			mutable Order						_order;
		};
	
		///@name External Iterators
		//@{
		/** Iterator to beginning.
			@return Mbo::OrderBook::const_iterator object pointing to the first Mbo::Order
			\remark Undefined behavior may occur while dereferencing returned object pointing to Mbo::OrderBook::end()
		*/
		const_iterator begin() const;

		/** Iterator to end.
			@return Mbo::OrderBook::const_iterator object pointing beyond the last Mbo::Order in this Mbo::OrderBook
			\remark Undefined behavior may occur while dereferencing returned object pointing to Mbo::OrderBook::end()
		*/
		const_iterator end() const;
		//@}
	
		///@name Accessors
		//@{
		/** Checks if this object has summary data.
			@return true if this object contains summary; false otherwise
		*/
		bool hasSummary() const;

		/** Returns summary associated with this object
			@throw OmmInvalidUsageException if this object contains no summary
			@return Summary
		*/
		const Summary& getSummary() const;

		/** returns OrderBook as Node object
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

		friend class MboRefreshInfoImpl;
		friend class MboStatusInfoImpl;
		friend class MboUpdateInfoImpl;
		friend class MboConsumerItemImpl;

		const foundation::EoaString& toString( foundation::UInt64 indent ) const;

		OrderBook();
		virtual ~OrderBook();

		const foundation::Node*			_pNode;
		mutable Summary					_summary;
		mutable foundation::EoaString	_toString;

		OrderBook( const OrderBook& );
		OrderBook& operator=( const OrderBook& );
	};


	/**
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::Qos Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::Qos represents Quality Of Service information in MarketByOrder.

		\remark All methods in this class are \ref SingleThreaded.

		@see Mbo::RefreshInfo
	*/
	class Qos
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

		friend class MboRefreshInfoImpl;

		Qos();
		virtual ~Qos();
		Qos( const Qos& );
		Qos& operator=( const Qos& );

		MboQosImpl*	_pImpl;
	};


	/**
		@class thomsonreuters::eoa::domain::marketbyorder::Mbo::State Mbo.h "Domain/MarketByOrder/Include/Mbo.h"
		@brief Mbo::State represents State information in MarketByOrder. 

		\remark All methods in this class are \ref SingleThreaded.

		@see Mbo::RefreshInfo,
			Mbo::StatusInfo
	*/
	class State 
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

		friend class MboRefreshInfoImpl;
		friend class MboStatusInfoImpl;

		State();
		virtual ~State();
		State( const State& );
		State& operator=( const State& );

		MboStateImpl*		_pImpl;
	};


	/** Returns a string with the version of the Mbo class.
		@return string with version
	*/
	static const foundation::EoaString& getVersion();

private:

	static const foundation::EoaString		_version;

	Mbo();
	virtual ~Mbo();

	Mbo( const Mbo& );
	Mbo& operator=( const Mbo& );
};

}

}

}

}

#endif // __thomsonreuters_eoa_domain_marketbyorder_mbo_h
