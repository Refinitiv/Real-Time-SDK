/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_domain_marketmaker_mm_h
#define __thomsonreuters_eoa_domain_marketmaker_mm_h

#pragma warning( disable : 4251 )

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
	@namespace domain::marketmaker
	@brief The domain::marketmaker namespace defines all the interfaces used by
	the EOA Market Maker package as well as applications using this package.
	
	The EOA Market Maker package provides RDM MarketMaker domain specific services and
	functionalities, which are encapsulated by the Mm class.
*/

namespace marketmaker {

class MmConsumerItemImpl;
class MmReqSpecImpl;
class MmConsumerServiceImpl;
class MmRefreshInfoImpl;
class MmUpdateInfoImpl;
class MmStatusInfoImpl;
class MmQosImpl;
class MmStateImpl;

/**
	@class thomsonreuters::eoa::domain::marketmaker::Mm Mm.h "Domain/MarketMaker/Include/Mm.h"
	@brief Mm class contains all classes used to request and process an item from MarketMaker domain.

	@see Mm::ConsumerItem,
		Mm::RefreshInfo,
		Mm::StatusInfo,
		Mm::UpdateInfo,
		Mm::ConsumerItemClient,
		Mm::ConsumerService,
		Mm::ReqSpec,
		Mm::MarketMakerList,
		Mm::MarketMakerQuote,
		Mm::Summary
*/
class EOA_DOMAIN_API Mm
{
public:

class Qos;
class State;
class MarketMakerList;
class MarketMakerQuote;
class Summary;
class ConsumerItemClient;
class ReqSpec;
class ConsumerService;


	/**
		@class thomsonreuters::eoa::domain::marketmaker::Mm::ConsumerItem Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::ConsumerItem provides access to MarketMaker item's key, state and payload.

		Mm::ConsumerItem stores item's key, state and payload.

		\remark All methods in this class are \ref SingleThreaded.

		@see Mm::MarketMakerList
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

		/** Returns the MarketMakerList of this item
			@return reference to MarketMakerList object
		*/
		const MarketMakerList& getMarketMakerList() const;

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

		friend class MmConsumerServiceImpl;
		friend class ConsumerService;

		MmConsumerItemImpl*								_pImpl;
		void*											_closure;
		ConsumerItemClient*								_pClient;
		std::function< void( const ConsumerItem& )>		_callBackFunc;
	};


	/**
		@class thomsonreuters::eoa::domain::marketmaker::Mm::RefreshInfo Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::RefreshInfo provides access to the information brought in by a MarketMaker refresh message.

		Mm::RefreshInfo provides access to temporary item data valid only in the context of a callback method.

		\remark All methods in this class are \ref SingleThreaded.

		@see Mm::State,
			Mm::Qos,
			Mm::MarketMakerList
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

		/** Returns the contained MarketMakerList.
			@return reference to MarketMakerList object
		*/
		const MarketMakerList& getMarketMakerList() const;

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

		friend class MmConsumerServiceImpl;

		RefreshInfo();
		virtual ~RefreshInfo();

		MmRefreshInfoImpl*	_pImpl;

		RefreshInfo( const RefreshInfo& );
		RefreshInfo& operator=( const RefreshInfo& );
	};


	/**
		@class thomsonreuters::eoa::domain::marketmaker::Mm::StatusInfo Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::StatusInfo provides access to the information brought in by a MarketMaker status message.

		Mm::StatusInfo provides access to temporary item data valid only in the context of a callback method.

		\remark All methods in this class are \ref SingleThreaded.

		@see Mm::State
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

		friend class MmConsumerServiceImpl;

		StatusInfo();
		virtual ~StatusInfo();

		MmStatusInfoImpl*		_pImpl;

		StatusInfo( const StatusInfo& );
		StatusInfo& operator=( const StatusInfo& );
	};


	/**
		@class thomsonreuters::eoa::domain::marketmaker::Mm::UpdateInfo Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::UpdateInfo provides access to the information brought in by an MarketMaker update message.

		Mm::UpdateInfo provides access to temporary item data valid only in the context of a callback method.

		\remark All methods in this class are \ref SingleThreaded.

		@see Mm::MarketMakerList
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

		/** Returns the contained MarketMakerList.
			@return reference to MarketMakerList object
		*/
		const MarketMakerList& getMarketMakerList() const;

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

		friend class MmConsumerServiceImpl;

		UpdateInfo();
		virtual ~UpdateInfo();

		MmUpdateInfoImpl*		_pImpl;

		UpdateInfo( const UpdateInfo& );
		UpdateInfo& operator=( const UpdateInfo& );
	};


	/**
		@class thomsonreuters::eoa::domain::marketmaker::Mm::ConsumerItemClient Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::ConsumerItemClient class provides callback interfaces notifying application about changes to registered items.

		Application needs to implement an application client class inheriting from Mm::ConsumerItemClient.
		In its own class, application needs to override callback methods it desires to use for item processing.
		Default empty callback methods are implemented by Mm::ConsumerItemClient class.

		\remark Thread safety of all the methods in this class depends on the user's implementation.

		@see Mm::ConsumerItem,
			Mm::RefreshInfo,
			Mm::UpdateInfo,
			Mm::StatusInfo
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
		@class thomsonreuters::eoa::domain::marketmaker::Mm::ConsumerService Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::ConsumerService represents a MarketMaker service provided by the platform.
	
		Application creates Mm::ConsumerService to register interest in items from a service.

		\remark All methods in this class are \ref ObjectLevelSafe

		@see Mm::ConsumerItem,
			Mm::ReqSpec,
			Mm::ConsumerItemClient,
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

		MmConsumerServiceImpl*		_pImpl;
	};


	/**
		@class thomsonreuters::eoa::domain::marketmaker::Mm::ReqSpec Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::ReqSpec is used to specify item's attributes.
	
		Mm::ReqSpec allows application to register its interest in a specified item.
		Among other attributes, Mm::ReqSpec conveys item's name, domain type, and
		desired quality of service.
	
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

		MmReqSpecImpl*		_pImpl;
	};

	/**
		@class thomsonreuters::eoa::domain::marketmaker::Mm::Currency Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::Currency is an enumeration specifying currency.
	
		@see Mm::Summary
	*/
	enum class EOA_DOMAIN_API Currency
	{
		NotAllocatedEnum = 0,	/*!< Indicates not allocated value */
		AfaEnum = 4,            /*!< Indicates Afghanistan afghani */
		AllEnum = 8,            /*!< Indicates Albanian lek */
		DzdEnum = 12,           /*!< Indicates Algerian dinar */
		AdpEnum = 20,           /*!< Indicates Andorran peseta */
		AonEnum = 24,           /*!< Indicates Angolan new kwanza */
		ArsEnum = 32,           /*!< Indicates Argentine peso */
		AudEnum = 36,           /*!< Indicates Australian dollar */
		AucEnum = 37,           /*!< Indicates Australian cent */
		AtsEnum = 40,           /*!< Indicates Austrian schilling */
		BsdEnum = 44,           /*!< Indicates Bahamian dollar */
		BhdEnum = 48,           /*!< Indicates Bahraini dinar */
		BdtEnum = 50,           /*!< Indicates Bangladesh taka */
		AmdEnum = 51,           /*!< Indicates Armenian Dram */
		BbdEnum = 52,           /*!< Indicates Barbados dollar */
		BefEnum = 56,           /*!< Indicates Belgian franc */
		BmdEnum = 60,           /*!< Indicates Bermudian dollar */
		BtnEnum = 64,           /*!< Indicates Bhutan ngultrum */
		BobEnum = 68,           /*!< Indicates Bolivian boliviano */
		BadEnum = 70,           /*!< Indicates Bosnia and Herzegovina Dinar */
		BwpEnum = 72,           /*!< Indicates Botswanian pula */
		BrlEnum = 76,           /*!< Indicates Brazilian real */
		BzdEnum = 84,           /*!< Indicates Belize dollar */
		SbdEnum = 90,           /*!< Indicates Solomon Islands dollar */
		BndEnum = 96,           /*!< Indicates Brunei dollar */
		BglEnum = 100,          /*!< Indicates Bulgarian Lev (pre 5 July 1999) */
		MmkEnum = 104,          /*!< Indicates Myanmar kyat */
		BifEnum = 108,          /*!< Indicates Burundi franc */
		KhrEnum = 116,          /*!< Indicates Kampuchean riel */
		CadEnum = 124,          /*!< Indicates Canadian dollar */
		CveEnum = 132,          /*!< Indicates Cape Verde escudo */
		KydEnum = 136,          /*!< Indicates Cayman Islands dollar */
		LkrEnum = 144,          /*!< Indicates Sri Lanka rupee */
		ClpEnum = 152,          /*!< Indicates Chilean peso */
		CnyEnum = 156,          /*!< Indicates Chinese yuan renminbi */
		CopEnum = 170,          /*!< Indicates Colombian peso */
		KmfEnum = 174,          /*!< Indicates Comoro franc */
		ZrnEnum = 180,          /*!< Indicates Zaire new zaire */
		CrcEnum = 188,          /*!< Indicates Costa Rican colon */
		HrkEnum = 191,          /*!< Indicates Croatian Kuna */
		CupEnum = 192,          /*!< Indicates Cuban peso */
		CypEnum = 196,          /*!< Indicates Cyprus pound */
		CzkEnum = 203,          /*!< Indicates Czech koruna */
		DkkEnum = 208,          /*!< Indicates Danish krone */
		DopEnum = 214,          /*!< Indicates Dominican peso */
		EcsEnum = 218,          /*!< Indicates Ecuador sucre */
		SvcEnum = 222,          /*!< Indicates El Salvador colon */
		EekEnum = 226,          /*!< Indicates Estonian kroon */
		EtbEnum = 230,          /*!< Indicates Ethiopian birr */
		ErnEnum = 232,          /*!< Indicates Nakfa */
		FkpEnum = 238,          /*!< Indicates Falkland Islands pound */
		FjdEnum = 242,          /*!< Indicates Fiji dollar */
		FimEnum = 246,          /*!< Indicates Finnish markka */
		FrfEnum = 250,          /*!< Indicates French franc */
		DjfEnum = 262,          /*!< Indicates Djibouti franc */
		GmdEnum = 270,          /*!< Indicates Gambian dalasi */
		DemEnum = 280,          /*!< Indicates Deutsche mark */
		GhcEnum = 288,          /*!< Indicates Ghanaian cedi */
		GipEnum = 292,          /*!< Indicates Gibraltar pound */
		GrdEnum = 300,          /*!< Indicates Greek drachma */
		GtqEnum = 320,          /*!< Indicates Guatemalan quetzal */
		GnfEnum = 324,          /*!< Indicates Guinea franc */
		GydEnum = 328,          /*!< Indicates Guyana dollar */
		HtgEnum = 332,          /*!< Indicates Haitian gourde */
		HnlEnum = 340,          /*!< Indicates Honduran lempira */
		HkdEnum = 344,          /*!< Indicates Hong Kong dollar */
		HkcEnum = 345,          /*!< Indicates Hong Kong cent */
		HufEnum = 348,          /*!< Indicates Hungarian forint */
		IskEnum = 352,          /*!< Indicates Iceland krona */
		InrEnum = 356,          /*!< Indicates Indian rupee */
		IdrEnum = 360,          /*!< Indicates Indonesian rupiah */
		IrrEnum = 364,          /*!< Indicates Iranian rial */
		IqdEnum = 368,          /*!< Indicates Iraqi dinar */
		IepEnum = 372,          /*!< Indicates Irish pound */
		IeeEnum = 373,          /*!< Indicates Irish pence */
		IlsEnum = 376,          /*!< Indicates Israeli shekel */
		ItlEnum = 380,          /*!< Indicates Italian lira */
		JmdEnum = 388,          /*!< Indicates Jamaican dollar */
		JpyEnum = 392,          /*!< Indicates Japanese yen */
		KztEnum = 398,          /*!< Indicates Kazakhstan Tenge  */
		JodEnum = 400,          /*!< Indicates Jordanian dinar */
		KesEnum = 404,          /*!< Indicates Kenyan shilling */
		KpwEnum = 408,          /*!< Indicates North Korean won */
		KrwEnum = 410,          /*!< Indicates Republic of Korea won */
		KwdEnum = 414,          /*!< Indicates Kuwaiti dinar */
		KgsEnum = 417,          /*!< Indicates Kyrgyzstan Som */
		LakEnum = 418,          /*!< Indicates Lao kip */
		LbpEnum = 422,          /*!< Indicates Lebanese pound */
		LslEnum = 426,          /*!< Indicates Lesotho loti */
		LvlEnum = 428,          /*!< Indicates Latvian Lats */
		LrdEnum = 430,          /*!< Indicates Liberian dollar */
		LydEnum = 434,          /*!< Indicates Libyan dinar */
		LtlEnum = 440,          /*!< Indicates Lithuanian litas */
		LufEnum = 442,          /*!< Indicates Luxembourg franc */
		MopEnum = 446,          /*!< Indicates Macau pataca */
		MwkEnum = 454,          /*!< Indicates Malawi kwacha */
		MyrEnum = 458,          /*!< Indicates Malaysian ringgit */
		MycEnum = 459,          /*!< Indicates Malaysian cent */
		MvrEnum = 462,          /*!< Indicates Maldivian rufiyaa */
		MlfEnum = 466,          /*!< Indicates Mali franc */
		MtlEnum = 470,          /*!< Indicates Maltese lira */
		MroEnum = 478,          /*!< Indicates Mauritanian ouguiya */
		MurEnum = 480,          /*!< Indicates Mauritius rupee */
		MxnEnum = 484,          /*!< Indicates Mexican peso */
		MntEnum = 496,          /*!< Indicates Mongolian tugrik */
		MdlEnum = 498,          /*!< Indicates Moldovan leu */
		MadEnum = 504,          /*!< Indicates Moroccan dirham */
		MzmEnum = 508,          /*!< Indicates Mozambique metical */
		OmrEnum = 512,          /*!< Indicates Rial Omani */
		NadEnum = 516,          /*!< Indicates Namibia dollar */
		NprEnum = 524,          /*!< Indicates Nepalese rupee */
		NlgEnum = 528,          /*!< Indicates Netherlands guilder */
		AngEnum = 532,          /*!< Indicates Netherlands Antillean guilder */
		AwgEnum = 533,          /*!< Indicates Aruban florin */
		VuvEnum = 548,          /*!< Indicates Vanuatu vatu */
		NzdEnum = 554,          /*!< Indicates New Zealand dollar */
		NioEnum = 558,          /*!< Indicates Nicaraguan cordoba oro */
		NgnEnum = 566,          /*!< Indicates Nigerian naira */
		NokEnum = 578,          /*!< Indicates Norwegian krone */
		PkrEnum = 586,          /*!< Indicates Pakistan rupee */
		PabEnum = 590,          /*!< Indicates Panamanian balboa */
		PgkEnum = 598,          /*!< Indicates Papua New Guinea kina */
		PygEnum = 600,          /*!< Indicates Paraguay guarani */
		PenEnum = 604,          /*!< Indicates Peruvian nuevo sol */
		PhpEnum = 608,          /*!< Indicates Philippine peso */
		PlzEnum = 616,          /*!< Indicates Old Polish zloty */
		PteEnum = 620,          /*!< Indicates Portuguese escudo */
		GwpEnum = 624,          /*!< Indicates Guinea-Bissau peso */
		TpeEnum = 626,          /*!< Indicates Timor escudo */
		QarEnum = 634,          /*!< Indicates Qatari rial */
		RolEnum = 642,          /*!< Indicates Romanian leu */
		RwfEnum = 646,          /*!< Indicates Rwanda franc */
		ShpEnum = 654,          /*!< Indicates Saint Helena pound */
		StdEnum = 678,          /*!< Indicates Sao Tome and Principe dobra */
		SarEnum = 682,          /*!< Indicates Saudi Arabian riyal */
		ScrEnum = 690,          /*!< Indicates Seychelles rupee */
		SllEnum = 694,          /*!< Indicates Sierra Leone leone */
		SgdEnum = 702,          /*!< Indicates Singapore dollar */
		SkkEnum = 703,          /*!< Indicates Slovak Koruna */
		VndEnum = 704,          /*!< Indicates Vietnamese dong */
		SitEnum = 705,          /*!< Indicates Slovenian tolar */
		SosEnum = 706,          /*!< Indicates Somali shilling */
		ZarEnum = 710,          /*!< Indicates South African rand */
		ZwdEnum = 716,          /*!< Indicates Zimbabwe dollar */
		EspEnum = 724,          /*!< Indicates Spanish peseta */
		SspEnum = 728,          /*!< Indicates South Sudanese Pound */
		SddEnum = 736,          /*!< Indicates Sudanese dinar */
		SrgEnum = 740,          /*!< Indicates Suriname guilder */
		SzlEnum = 748,          /*!< Indicates Swaziland lilangeni */
		SekEnum = 752,          /*!< Indicates Swedish krona */
		ChfEnum = 756,          /*!< Indicates Swiss franc */
		SypEnum = 760,          /*!< Indicates Syrian pound */
		ThbEnum = 764,          /*!< Indicates Thai baht */
		TopEnum = 776,          /*!< Indicates Tongan pa'anga */
		TtdEnum = 780,          /*!< Indicates Trinidad and Tobago dollar */
		AedEnum = 784,          /*!< Indicates UAE dirham */
		TndEnum = 788,          /*!< Indicates Tunisian dinar */
		TrlEnum = 792,          /*!< Indicates Old Turkish lira */
		TmmEnum = 795,          /*!< Indicates Turkmenistan manat */
		UgxEnum = 800,          /*!< Indicates Uganda shilling */
		UakEnum = 804,          /*!< Indicates Ukrainian karbovanet */
		MkdEnum = 807,          /*!< Indicates Macedonian denar */
		RubEnum = 810,          /*!< Indicates Russian ruble */
		EgpEnum = 818,          /*!< Indicates Egyptian pound */
		GbpEnum = 826,          /*!< Indicates UK pound sterling */
		TzsEnum = 834,          /*!< Indicates Tanzanian shilling */
		UsdEnum = 840,          /*!< Indicates US dollar */
		UscEnum = 841,          /*!< Indicates US cents */
		UyuEnum = 858,          /*!< Indicates Peso Uruguayo */
		UzsEnum = 860,          /*!< Indicates Uzbekistan Sum */
		VebEnum = 862,          /*!< Indicates Old Venezuelan bolivar */
		WstEnum = 882,          /*!< Indicates Samoan tala */
		YerEnum = 886,          /*!< Indicates Yemeni rial */
		CsdEnum = 891,          /*!< Indicates Serbian Dinar */
		ZmkEnum = 894,          /*!< Indicates Zambian kwacha */
		RobEnum = 900,          /*!< Indicates Romanian bani */
		TwdEnum = 901,          /*!< Indicates New Taiwan dollar */
		BhfEnum = 902,          /*!< Indicates Bahraini Fils  */
		KwfEnum = 903,          /*!< Indicates Kuwaiti Fils */
		OmbEnum = 904,          /*!< Indicates Baiza Omani  */
		SahEnum = 905,          /*!< Indicates Saudi Arabian Halalah */
		QadEnum = 906,          /*!< Indicates Qatari Dirham */
		AefEnum = 907,          /*!< Indicates UAE Fils */
		CucEnum = 931,          /*!< Indicates Cuba Convertible Peso */
		ZwlEnum = 932,          /*!< Indicates Zimbabwe Dollar  */
		TmtEnum = 934,          /*!< Indicates Turkmenistan New Manat */
		GhsEnum = 936,          /*!< Indicates Ghana Cedi */
		VefEnum = 937,          /*!< Indicates Venezuelan Bolivar */
		SdgEnum = 938,          /*!< Indicates SUDANESE POUND */
		UyiEnum = 940,          /*!< Indicates Uruguay Peso en Unidades Indexadas */
		RsdEnum = 941,          /*!< Indicates Serbian Dinar */
		ZwnEnum = 942,          /*!< Indicates 'new' Zimbabwe Dollar */
		MznEnum = 943,          /*!< Indicates Mozambique Metical */
		AznEnum = 944,          /*!< Indicates Azerbaijanian Manat */
		RonEnum = 946,          /*!< Indicates New Romanian Leu */
		ChwEnum = 948,          /*!< Indicates Swiss WIR Franc */
		TryEnum = 949,          /*!< Indicates Turkish lira */
		XafEnum = 950,          /*!< Indicates CFA franc BEAC */
		XcdEnum = 951,          /*!< Indicates East Caribbean dollar */
		XofEnum = 952,          /*!< Indicates CFA franc BCEAO */
		XpfEnum = 953,          /*!< Indicates CFP franc */
		XeuEnum = 954,          /*!< Indicates European currency unit */
		XbaEnum = 955,          /*!< Indicates European composite unit */
		XbbEnum = 956,          /*!< Indicates European monetary unit */
		XbcEnum = 957,          /*!< Indicates European unit of Account 9 */
		XbdEnum = 958,          /*!< Indicates European unit of Account 17 */
		XauEnum = 959,          /*!< Indicates Gold */
		XdrEnum = 960,          /*!< Indicates Special drawing right */
		XagEnum = 961,          /*!< Indicates Silver */
		XptEnum = 962,          /*!< Indicates Platinum */
		XtsEnum = 963,          /*!< Indicates Reserved for testing */
		XpdEnum = 964,          /*!< Indicates Palladium */
		XuaEnum = 965,          /*!< Indicates ADB Unit of Account */
		ZmwEnum = 967,          /*!< Indicates Zambian kwacha */
		SrdEnum = 968,          /*!< Indicates Surinam Dollar */
		MgaEnum = 969,          /*!< Indicates Malagasy Ariary */
		AfnEnum = 971,          /*!< Indicates Afghanistan afghani */
		TjsEnum = 972,          /*!< Indicates Tajikistan Somoni */
		AoaEnum = 973,          /*!< Indicates Angolan kwanza */
		ByrEnum = 974,          /*!< Indicates Belarussian Ruble  */
		BgnEnum = 975,          /*!< Indicates Bulgarian Lev */
		CdfEnum = 976,          /*!< Indicates Congolese Franc  */
		BamEnum = 977,          /*!< Indicates Convertible Mark */
		EurEnum = 978,          /*!< Indicates Euro */
		MxvEnum = 979,          /*!< Indicates Mexico Unidad de Inversion */
		UahEnum = 980,          /*!< Indicates Ukrainian Hryvnia */
		GelEnum = 981,          /*!< Indicates Georgian Lari */
		BovEnum = 984,          /*!< Indicates Bolivian Mvdol */
		PlnEnum = 985,          /*!< Indicates New Polish zloty */
		ClfEnum = 990,          /*!< Indicates Chilean unidad de fomento */
		ZalEnum = 991,          /*!< Indicates South African financial rand */
		EsbEnum = 995,          /*!< Indicates Spanish peseta (accounts 'B') */
		EsaEnum = 996,          /*!< Indicates Spanish peseta (accounts 'A') */
		UsnEnum = 997,          /*!< Indicates US dollar (next day) */
		UssEnum = 998,          /*!< Indicates US dollar (same day) */
		XxxEnum = 999,          /*!< Indicates Code assigned for no currency transactions */
		TstEnum = 2000,         /*!< Indicates Test */
		CacEnum = 2002,         /*!< Indicates Canadian cents **** */
		NzcEnum = 2006,         /*!< Indicates New Zealand cents **** */
		SgcEnum = 2007,         /*!< Indicates Singapore cents **** */
		ZacEnum = 2010,         /*!< Indicates South African cents **** */
		ZwcEnum = 2011,         /*!< Indicates Zimbabwe cents */
		EucEnum = 2012,         /*!< Indicates The Euro minor currency, the cent */
		NacEnum = 2013,         /*!< Indicates Namibian cent */
		IlaEnum = 2014,         /*!< Indicates Israeli Agora */
		HrlEnum = 2016,         /*!< Indicates Croatian Lipa (was Croatian Kuna) */
		BwtEnum = 2017,         /*!< Indicates Botswana Thebe */
		ZzzEnum = 2018,         /*!< Indicates Multi-Currency */
		TrkEnum = 2019,         /*!< Indicates New Turkish Kurus */
		CouEnum = 2020,         /*!< Indicates Colombia Unidad Valor Real */
		MwtEnum = 2021,         /*!< Indicates Malawi Tambala */
		BrcEnum = 2022,         /*!< Indicates Brazilian Centavos */
		CnhEnum = 2023,         /*!< Indicates Chinese Yuan Offshore */
		UstEnum = 2024,         /*!< Indicates Thousands of US dollars */
		XvnEnum = 2025,         /*!< Indicates Ven */
		CruEnum = 2026,         /*!< Indicates Costa Rican Unidad de Desarrolla */
		BtcEnum = 2027,         /*!< Indicates bitcoin */
		UyrEnum = 2028          /*!< Indicates Unidades Reajustables */
	};


	/**
		@class thomsonreuters::eoa::domain::marketmaker::Mm::MarketState Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::MarketState is an enumeration specifying state of market.

		@see Mm::Summary
	*/
	enum class EOA_DOMAIN_API MarketState
	{
		NorEnum = 0,    /*!< Indicates normal market */
		FasEnum = 1,    /*!< Indicates fast market */
		FalEnum = 2,    /*!< Indicates fire alert */
		ThaEnum = 3,    /*!< Indicates trading halted */
		TreEnum = 4,    /*!< Indicates trading resumed */
		BscEnum = 5,    /*!< Indicates bomb scare */
		CcpEnum = 6,    /*!< Indicates call completed */
		CipEnum = 7,    /*!< Indicates call in progress */
		ClsEnum = 8,    /*!< Indicates market closed */
		OclEnum = 9,    /*!< Indicates official close */
		UclEnum = 10,   /*!< Indicates unofficial close */
		ScmEnum = 11,   /*!< Indicates session closed, required for futures session trading */
		IndEnum = 12,   /*!< Indicates indicative open for GLOBEX instruments */
		RfqEnum = 13,   /*!< Indicates request for quote from GLOBEX traders */
		PssEnum = 14,   /*!< Indicates post-settle session trading in progress (N.Amer.Fut.) */
		AptEnum = 15,   /*!< Indicates automated pit trading (LIFFOE) */
		BboEnum = 16,   /*!< Indicates best bid/offer required for CBOT */
		ImbEnum = 17,   /*!< Indicates implied bid - NYMEX */
		ImaEnum = 18,   /*!< Indicates implied ask - NYMEX */
		ImqEnum = 19,   /*!< Indicates implied quote - NYMEX */
		BasEnum = 20,   /*!< Indicates bid/ask represents current market */
		LtpEnum = 21,   /*!< Indicates late trade price */
		ErtEnum = 22,   /*!< Indicates erratic trade price */
		StlEnum = 23,   /*!< Indicates Settlement Received */
		GtoEnum = 24,   /*!< Indicates Globex Trigger Opening Received : variation recalculated - Matif */
		GorEnum = 25,   /*!< Indicates Globex Pre-Opening Reference received - Matif */
		GloEnum = 26,   /*!< Indicates Globex Session */
		GlpEnum = 27,   /*!< Indicates Globex Extended Session (Bank Holidays) */
		FlrEnum = 28,   /*!< Indicates Floor Session */
		DtaEnum = 29,   /*!< Indicates Delta Indicator */
		GldEnum = 30,   /*!< Indicates Globex Morning Session */
		GdpEnum = 31,   /*!< Indicates Globex Morning Extended Session */
		AueEnum = 32,   /*!< Indicates Auto Execution */
		SbbEnum = 33,   /*!< Indicates Specialist Book Bid */
		SbaEnum = 34,   /*!< Indicates Specialist Book Ask */
		BbaEnum = 35,   /*!< Indicates Specialist Book Bid and Ask */
		InaEnum = 36,   /*!< Indicates Inactive */
		RotEnum = 37,   /*!< Indicates Rotation */
		DgnEnum = 38,   /*!< Indicates Danger notice */
		PcsEnum = 39,   /*!< Indicates Post-Close Session */
		PreEnum = 40,   /*!< Indicates Pre-Open */
		NetEnum = 41,   /*!< Indicates Netting/Uncrossing */
		IraEnum = 42,   /*!< Indicates Combination of Implied and Regular ASK */
		IrbEnum = 43,   /*!< Indicates Combination of Implied and Regular BID */
		IrqEnum = 44,   /*!< Indicates Combination of Implied and Regular Quote */
		OpnEnum = 45,   /*!< Indicates Market Open */
		AucEnum = 46,   /*!< Indicates Opening Auction and Closing Auction */
		SusEnum = 47,   /*!< Indicates Suspension */
		MkhEnum = 48,   /*!< Indicates Market Halt */
		SmhEnum = 49,   /*!< Indicates Segment Halt */
		SmcEnum = 50,   /*!< Indicates Suspension Market Condition for FUTURES */
		PscEnum = 51,   /*!< Indicates Post-Close session for FUTURES */
		NegEnum = 52   /*!< Indicates Negdeals */
	};


	/**
		@class thomsonreuters::eoa::domain::marketmaker::Mm::ExchangeId Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::ExchangeId is an enumeration specifying id of exchange.

		@see Mm::Summary
	*/
	enum class EOA_DOMAIN_API ExchangeId
	{
		UndefinedEnum = 0,    /*!< Indicates undefined exchange */
		AseEnum = 1,   /*!< Indicates NYSE AMEX */
		NysEnum = 2,   /*!< Indicates New York Stock Exchange */
		BosEnum = 3,   /*!< Indicates Boston Stock Exchange */
		CinEnum = 4,   /*!< Indicates National Stock Exchange (formerly Cincinnati Stock Exchange) */
		PseEnum = 5,   /*!< Indicates NYSE Arca */
		XphEnum = 6,   /*!< Indicates NASDAQ OMX PSX when trading in SIAC (formerly Philadelphia Stock Exchange) */
		ThmEnum = 7,   /*!< Indicates NASDAQ InterMarket */
		MidEnum = 8,   /*!< Indicates Chicago Stock Exchange */
		NyqEnum = 9,   /*!< Indicates Consolidated Issue, listed by NYSE */
		TorEnum = 10,   /*!< Indicates Toronto Stock Exchange */
		MonEnum = 11,   /*!< Indicates Montreal Stock Exchange */
		TsxEnum = 12,   /*!< Indicates TSX Venture Exchange */
		DexEnum = 13,   /*!< Indicates Direct Edge Holdings - EDGX (CTA) */
		AoeEnum = 14,   /*!< Indicates American Options Exchange */
		NyoEnum = 15,   /*!< Indicates New York Options Exchange */
		PhoEnum = 16,   /*!< Indicates Philadelphia Options Exchange */
		PaoEnum = 17,   /*!< Indicates NYSE Arca Options */
		WcbEnum = 18,   /*!< Indicates Chicago Board Options Exchange */
		AsqEnum = 19,   /*!< Indicates Consolidated issues listed on NYSE MKT */
		CmeEnum = 20,   /*!< Indicates CME:Chicago Mercantile Commodities */
		ImmEnum = 21,   /*!< Indicates CME:International Monetary Market */
		IomEnum = 22,   /*!< Indicates CME:Index and Options Market */
		CbtEnum = 23,   /*!< Indicates Chicago Board of Trade */
		ItsEnum = 24,   /*!< Indicates NYSE Alerts */
		WpgEnum = 25,   /*!< Indicates Winnipeg Commodity Exchange */
		MgeEnum = 26,   /*!< Indicates Minneapolis Grain Exchange */
		PbtEnum = 27,   /*!< Indicates Philadelphia Board of Trade */
		CmxEnum = 28,   /*!< Indicates CEI:Commodities Exchange Centre */
		CscEnum = 29,   /*!< Indicates CEI:Coffee, Sugar and Cocoa */
		NycEnum = 30,   /*!< Indicates CEI:New York Cotton */
		NymEnum = 31,   /*!< Indicates NEW YORK MERCANTILE EXCHANGE (NYMEX) */
		FisEnum = 32,   /*!< Indicates FTSE SINGAPORE */
		CsoEnum = 33,   /*!< Indicates CEI:Coffee, Sugar and Cocoa Options */
		CeoEnum = 34,   /*!< Indicates CEI:Commodities Exchange Centre Options */
		ShcEnum = 35,   /*!< Indicates Shanghai Commodity Exchange */
		NyfEnum = 36,   /*!< Indicates New York Futures Exchange */
		TfeEnum = 37,   /*!< Indicates Toronto Futures Exchange */
		MioEnum = 38,   /*!< Indicates Montreal SE IOM division */
		NmsEnum = 39,   /*!< Indicates NASDAQ Stock Market Exchange Large Cap (formally known as NASDAQ NATIONAL MARKET SYSTEM) */
		DeaEnum = 40,   /*!< Indicates Direct Edge Holdings - EDGA (CTA) */
		MoeEnum = 41,   /*!< Indicates Montreal Options Exchange */
		BtyEnum = 42,   /*!< Indicates BATS Y Exchange (CTA) */
		NasEnum = 43,   /*!< Indicates NASDAQ Capital Market (from NASDAQ SmallCap) */
		MdmEnum = 44,   /*!< Indicates Montreal SE Mercantile Division */
		SceEnum = 45,   /*!< Indicates Shanghai Cereals & Oils Exchange */
		BsqEnum = 46,   /*!< Indicates Consolidated Issue, listed by Boston SE */
		ChiEnum = 47,   /*!< Indicates CHI - X Europe */
		WcqEnum = 48,   /*!< Indicates Consolidated Issue, listed by CBOE */
		TrcEnum = 49,   /*!< Indicates TRC (THOMSON REUTERS CONTRIBUTED DATA)  */
		KbtEnum = 50,   /*!< Indicates Kansas City Board of Trade */
		SomEnum = 51,   /*!< Indicates Stockholmsborsen - derivatives (was Om Stockholm) */
		AdsEnum = 52,   /*!< Indicates NASD Alternative Display Facility for Nasdaq Capital Market */
		AdfEnum = 53,   /*!< Indicates NASD Alternative Display Facility for Nasdaq Large Cap */
		MifEnum = 54,   /*!< Indicates Manila International Futures Exchange */
		JsfEnum = 55,   /*!< Indicates Japan Securities Finance */
		TceEnum = 56,   /*!< Indicates Tokyo Commodity Exchange */
		TffEnum = 57,   /*!< Indicates Tokyo Financial Futures Exchange */
		RpsEnum = 58,   /*!< Indicates REUTERS PRICING SERVICE */
		HomEnum = 59,   /*!< Indicates Helsinki Options and Futures Exchange */
		FomEnum = 60,   /*!< Indicates Finland Options Market (Suomen Optiomeklait) */
		FirEnum = 61,   /*!< Indicates Florence Stock Exchange */
		AthEnum = 62,   /*!< Indicates Athens Stock Exchange */
		SffEnum = 63,   /*!< Indicates Swiss Options and Financial Futures Exchange */
		LseEnum = 64,   /*!< Indicates London Stock Exchange */
		JnbEnum = 65,   /*!< Indicates Johannesburg Stock Exchange (JSE) Level 1 */
		LifEnum = 66,   /*!< Indicates LIFFE */
		TlvEnum = 67,   /*!< Indicates Tel Aviv Stock Exchange */
		CphEnum = 68,   /*!< Indicates Copenhagen Stock Exchange */
		OslEnum = 69,   /*!< Indicates Oslo Stock Exchange */
		StoEnum = 70,   /*!< Indicates Stockholmsborsen - cash (was Stockholm Stock Exchange) */
		CmaEnum = 71,   /*!< Indicates Madrid SE - Chi-X */
		LuxEnum = 72,   /*!< Indicates Luxembourg Stock Exchange */
		BruEnum = 73,   /*!< Indicates Brussels Stock Exchange */
		ParEnum = 74,   /*!< Indicates Paris Stock Exchange */
		MadEnum = 75,   /*!< Indicates Madrid Stock Exchange */
		BarEnum = 76,   /*!< Indicates Barcelona Stock Exchange */
		AexEnum = 77,   /*!< Indicates Amsterdam Exchanges */
		EoeEnum = 78,   /*!< Indicates European Options Exchange */
		TwbEnum = 79,   /*!< Indicates TRADEWEB */
		VieEnum = 80,   /*!< Indicates Vienna Stock Exchange */
		DusEnum = 81,   /*!< Indicates Dusseldorf Stock Exchange */
		FraEnum = 82,   /*!< Indicates Frankfurt Stock Exchange */
		HamEnum = 83,   /*!< Indicates Hamburg Stock Exchange */
		MunEnum = 84,   /*!< Indicates Munich Stock Exchange */
		ZrhEnum = 85,   /*!< Indicates Zurich Stock Exchange */
		GvaEnum = 86,   /*!< Indicates Geneva Stock Exchange */
		BslEnum = 87,   /*!< Indicates Basel Stock Exchange */
		MilEnum = 88,   /*!< Indicates Milan Stock Exchange */
		CsmEnum = 89,   /*!< Indicates Sibe Mercado Espanola - Chi-X */
		CmlEnum = 90,   /*!< Indicates Milan SE - Chi-X */
		NzcEnum = 91,   /*!< Indicates New Zealand Total - Prices, Indices, News */
		HkgEnum = 92,   /*!< Indicates Stock Exchange of Hong Kong Limited */
		SimEnum = 93,   /*!< Indicates Singapore Exchange Derivatives Trading */
		KlcEnum = 94,   /*!< Indicates Mdex Commodity Market */
		CchEnum = 95,   /*!< Indicates Copenhagen SE - Chi-X */
		CxeEnum = 96,   /*!< Indicates Xetra - Chi-X */
		CamEnum = 97,   /*!< Indicates Amsterdam SE - Chi-X */
		CbsEnum = 98,   /*!< Indicates Brussels SE - Chi-X */
		LmeEnum = 99,   /*!< Indicates LME Data - Real Time */
		ClbEnum = 100,   /*!< Indicates Lisbon SE - Chi-X */
		LceEnum = 101,   /*!< Indicates London Commodity Exchange */
		CpaEnum = 102,   /*!< Indicates Paris SE - Chi-X */
		CfrEnum = 103,   /*!< Indicates Frankfurt SE - Chi-X */
		CloEnum = 104,   /*!< Indicates London SE - Chi-X */
		SfeEnum = 105,   /*!< Indicates Sydney Futures Exchange */
		TyoEnum = 106,   /*!< Indicates Tokyo Stock Exchange */
		NgoEnum = 107,   /*!< Indicates Nagoya Stock Exchange */
		SapEnum = 108,   /*!< Indicates Sapporo Stock Exchange */
		ChjEnum = 109,   /*!< Indicates JAPAN - CHI-X */
		KyoEnum = 110,   /*!< Indicates Kyoto Stock Exchange */
		HirEnum = 111,   /*!< Indicates Hiroshima Stock Exchange */
		FkaEnum = 112,   /*!< Indicates Fukuoka Stock Exchange */
		OsaEnum = 113,   /*!< Indicates OSAKA EXCHANGE INC. */
		HfeEnum = 114,   /*!< Indicates Hong Kong Futures Exchange Limited */
		BerEnum = 115,   /*!< Indicates Berlin Stock Exchange */
		HanEnum = 116,   /*!< Indicates Hanover Stock Exchange */
		StuEnum = 117,   /*!< Indicates Stuttgart Stock Exchange */
		BreEnum = 118,   /*!< Indicates Bremen Stock Exchange */
		CokEnum = 119,   /*!< Indicates Osaka Mercantile SE - Chi-X */
		BrnEnum = 120,   /*!< Indicates Berne Stock Exchange */
		CssEnum = 121,   /*!< Indicates Swiss SE - Chi-X */
		CheEnum = 122,   /*!< Indicates Helsinki SE - Chi-X */
		CvxEnum = 123,   /*!< Indicates Virt-X - Chi-X */
		RomEnum = 124,   /*!< Indicates Rome Stock Exchange */
		TrnEnum = 125,   /*!< Indicates Turin Stock Exchange */
		GoaEnum = 126,   /*!< Indicates Genoa Stock Exchange */
		NapEnum = 127,   /*!< Indicates Naples Stock Exchange */
		PalEnum = 128,   /*!< Indicates Palermo Stock Exchange */
		BloEnum = 129,   /*!< Indicates Bologna Stock Exchange */
		VceEnum = 130,   /*!< Indicates Venice Stock Exchange */
		IsfEnum = 131,   /*!< Indicates Istanbul Stock Exchange (Mutual Funds) */
		AqcEnum = 132,   /*!< Indicates NASDAQ Limit Order Matching System (NAQCESS) */
		HexEnum = 133,   /*!< Indicates Helsinki Exchange */
		WseEnum = 134,   /*!< Indicates Warsaw Stock Exchange */
		NxtEnum = 135,   /*!< Indicates Euronext */
		Mx2Enum = 136,   /*!< Indicates MEXICO SE DEPTH MARKET L1 AND L2 */
		CaqEnum = 137,   /*!< Indicates Canadian Composite Quote/Trade */
		F4eEnum = 138,   /*!< Indicates FTSE 4 Good Environmental Leaders Europe 40 Index ICW */
		BetEnum = 139,   /*!< Indicates Bureau of Energy */
		F4jEnum = 140,   /*!< Indicates FTSE 4 Good Japan ICW */
		JdeEnum = 141,   /*!< Indicates Joint Asian Derivatives Exchange  */
		LagEnum = 142,   /*!< Indicates Lagos Stock Exchange */
		ZseEnum = 143,   /*!< Indicates Zimbabwe Stock Exchange */
		ShhEnum = 144,   /*!< Indicates Shanghai Stock Exchange */
		BseEnum = 145,   /*!< Indicates  BSE Ltd */
		CalEnum = 146,   /*!< Indicates Calcutta Stock Exchange */
		DesEnum = 147,   /*!< Indicates Delhi Stock Exchange */
		MdsEnum = 148,   /*!< Indicates Madras Stock Exchange */
		JktEnum = 149,   /*!< Indicates Indonesia Stock Exchange (formerly Jakarta SE) */
		KlsEnum = 150,   /*!< Indicates Bursa Malaysia Consolidated Equities and Derivatives (formerly Kuala Lumpur Stock Exchange) */
		KarEnum = 151,   /*!< Indicates Karachi Stock Exchange */
		MakEnum = 152,   /*!< Indicates Makati Stock Exchange */
		MnlEnum = 153,   /*!< Indicates Manila Stock Exchange */
		FtmEnum = 154,   /*!< Indicates Financiele Termijn Markt, Amsterdam */
		SesEnum = 155,   /*!< Indicates Singapore Exchange Securities Trading Ltd */
		KscEnum = 156,   /*!< Indicates Korea Exchange - KSE */
		MauEnum = 157,   /*!< Indicates Mauritius Stock Exchange */
		SetEnum = 158,   /*!< Indicates The Stock Exchange of Thailand */
		BahEnum = 159,   /*!< Indicates BAHRAIN BOURSE */
		CaiEnum = 160,   /*!< Indicates Cairo Stock Exchange */
		AmmEnum = 161,   /*!< Indicates Amman Stock Exchange */
		KuwEnum = 162,   /*!< Indicates Kuwait Stock Exchange */
		BueEnum = 163,   /*!< Indicates Buenos Aires Stock Exchange */
		RioEnum = 164,   /*!< Indicates Rio Fixed Income Exchange */
		SaoEnum = 165,   /*!< Indicates Sao Paolo Stock Exchange */
		SgoEnum = 166,   /*!< Indicates Santiago Stock Exchange */
		BogEnum = 167,   /*!< Indicates Bogota Stock Exchange */
		MexEnum = 168,   /*!< Indicates Mexico SE Principal Market */
		CcsEnum = 169,   /*!< Indicates Caracas Stock Exchange */
		NfeEnum = 170,   /*!< Indicates New Zealand Futures Exchange */
		IndEnum = 171,   /*!< Indicates World Indices */
		FixEnum = 172,   /*!< Indicates Fixings */
		LotEnum = 173,   /*!< Indicates London Over The Counter Market */
		MatEnum = 174,   /*!< Indicates Paris Financial Futures Exchange ( MATIF ) */
		TaiEnum = 175,   /*!< Indicates Taiwan Stock Exchange */
		IpeEnum = 176,   /*!< Indicates ICE Futures */
		CviEnum = 177,   /*!< Indicates Vienna SE - Chi-X */
		CirEnum = 178,   /*!< Indicates Irish SE - Chi-X */
		CluEnum = 179,   /*!< Indicates Luxemburg SE - Chi-X */
		SopEnum = 180,   /*!< Indicates Suomen Optioporssi */
		ReuEnum = 181,   /*!< Indicates Thomson Reuters */
		LisEnum = 182,   /*!< Indicates Lisbon Stock Exchange */
		OpoEnum = 183,   /*!< Indicates Oporto Stock Exchange */
		NomEnum = 184,   /*!< Indicates Norwegian Options Market (Norsk Opsjonmarked) */
		CltEnum = 185,   /*!< Indicates Latino / America Market in Spain - Chi-X */
		RtsEnum = 186,   /*!< Indicates RTS Quotes */
		AsxEnum = 187,   /*!< Indicates Australian Stock Exchange - SEATS */
		IfxEnum = 188,   /*!< Indicates Irish Options and Futures Exchange */
		PmiEnum = 189,   /*!< Indicates Stockholmsborsen - fixed income (was Penningsmarknads Information AB) */
		MceEnum = 190,   /*!< Indicates BME SPANISH EXCHANGE EQUITIES LEVEL 2 */
		CvaEnum = 191,   /*!< Indicates Valencia SE - Chi-X */
		TgeEnum = 192,   /*!< Indicates Tokyo Grain Exchange */
		TsuEnum = 193,   /*!< Indicates Tokyo Sugar Exchange */
		KreEnum = 194,   /*!< Indicates Kobe Rubber Exchange */
		MrvEnum = 195,   /*!< Indicates BME Spanish Exchange Derivatives L1+L2 */
		KfiEnum = 196,   /*!< Indicates KOREA STOCK EXCHANGE - KOREA BONDS - KOFIA */
		BfxEnum = 197,   /*!< Indicates Brussels Futures and Options Exchange */
		BrtEnum = 198,   /*!< Indicates Brussels SE Forward Market */
		DtbEnum = 199,   /*!< Indicates Deutsche Terminboerse */
		MrfEnum = 200,   /*!< Indicates MEFF Renta Fija */
		CbaEnum = 201,   /*!< Indicates Barcelona SE - Chi-X */
		GerEnum = 202,   /*!< Indicates Xetra Level 1 */
		MtsEnum = 203,   /*!< Indicates MTS Italy */
		IstEnum = 204,   /*!< Indicates Borsa Istanbul */
		MusEnum = 205,   /*!< Indicates Muscat Stock Exchange */
		AbjEnum = 206,   /*!< Indicates Bourse de Valeurs d'Abidjan */
		NaiEnum = 207,   /*!< Indicates Nairobi Stock Exchange */
		TunEnum = 208,   /*!< Indicates Tunis Stock Exchange */
		FsiEnum = 209,   /*!< Indicates FT-SE International */
		NinEnum = 210,   /*!< Indicates NASDAQ International (pending SEC approval) */
		McpEnum = 211,   /*!< Indicates Interbolsa - Portuguese Continuous Market */
		OtbEnum = 212,   /*!< Indicates Osterreichische Termin und Optionenboerse (Austrian FOX) */
		SicEnum = 213,   /*!< Indicates Singapore Commodity Exchange */
		RctEnum = 214,   /*!< Indicates SOURCE IS A THOMSON REUTERS CONTRIBUTOR */
		IgfEnum = 215,   /*!< Indicates Italian Government Forwards */
		MwqEnum = 216,   /*!< Indicates Consolidated issue, listed by Midwest SE */
		PsqEnum = 217,   /*!< Indicates Consolidated issue, listed by Pacific SE */
		PhqEnum = 218,   /*!< Indicates Consolidated issue, listed by Philadelphia SE */
		JsdEnum = 219,   /*!< Indicates JASDAQ */
		EcmEnum = 220,   /*!< Indicates American SE Emerging Company Marketplace */
		ObbEnum = 221,   /*!< Indicates NASD OTC Bulletin Board Market */
		IfmEnum = 222,   /*!< Indicates SIA Futures Market */
		MfiEnum = 223,   /*!< Indicates Madrid Fixed Income */
		SmeEnum = 224,   /*!< Indicates Shanghai Metal Exchange */
		ShzEnum = 225,   /*!< Indicates Shenzhen Stock Exchange */
		RsaEnum = 226,   /*!< Indicates South African Quotations */
		BudEnum = 227,   /*!< Indicates Budapest Stock Exchange */
		PhsEnum = 228,   /*!< Indicates Philippine Stock Exchange */
		MltEnum = 229,   /*!< Indicates Malta Stock Exchange */
		CoxEnum = 230,   /*!< Indicates OFEX - Chi-X */
		SzmEnum = 231,   /*!< Indicates Shenzhen Mercantile Exchange */
		SzfEnum = 232,   /*!< Indicates Shenzhen Futures Exchange */
		JbtEnum = 233,   /*!< Indicates Japan Bond Trading Co. Ltd. */
		MdcEnum = 234,   /*!< Indicates Maebashi Dried Cocoon Exchange */
		NtcEnum = 235,   /*!< Indicates Nagoya Textile Exchange */
		YseEnum = 236,   /*!< Indicates Yokohama Silk Exchange */
		OteEnum = 237,   /*!< Indicates Osaka Textile Exchange */
		NakEnum = 238,   /*!< Indicates Nakadachi Securities */
		CbeEnum = 239,   /*!< Indicates Berlin SE - Chi-X */
		CexEnum = 240,   /*!< Indicates CHI-EAST Exchange */
		AhmEnum = 241,   /*!< Indicates Ahemdebad Stock Exchange */
		KlfEnum = 242,   /*!< Indicates Kuala Lumpur Options and Financial Futures Exchange */
		CseEnum = 243,   /*!< Indicates Colombo Stock Exchange */
		BecEnum = 244,   /*!< Indicates Chile Electronic Exchange, Santiago */
		MxiEnum = 245,   /*!< Indicates Mexico SE Intermediate Market */
		ZhcEnum = 246,   /*!< Indicates China Zhengzhou Commodity Exchange */
		BjcEnum = 247,   /*!< Indicates Beijing Commodities Exchange */
		EodEnum = 248,   /*!< Indicates Credit Default Swaps EOD */
		GufEnum = 249,   /*!< Indicates Guangdong United Futures Exchange */
		BmfEnum = 250,   /*!< Indicates Brazilian Commodties and Futures Exchange */
		HsxEnum = 251,   /*!< Indicates HO CHI MINH STOCK EXCHANGE */
		SfxEnum = 252,   /*!< Indicates South African Futures Exchange */
		CmuEnum = 253,   /*!< Indicates Munich SE - Chi-X */
		DlcEnum = 254,   /*!< Indicates Dalian Commodities Exchange */
		CfsEnum = 255   /*!< Indicates China Foreign Exchange Trade System */
	};


	/**
		@class thomsonreuters::eoa::domain::marketmaker::Mm::PriceRankRule Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::PriceRankRule is an enumeration specifying price ranking rule.

		@see Mm::Summary
	*/
	enum class EOA_DOMAIN_API PriceRankRule
	{
		UndefinedEnum = 0,      /*!< Indicates undefined market */
		NorEnum = 1,			/*!< Indicates Normal Market */
		InvEnum = 2,			/*!< Indicates Inverted Market */
		SqxEnum = 3,			/*!< Indicates Firm Quote, Limit Order (LSE SETSqx) */
		PonEnum = 4,			/*!< Indicates Price, Order Type, Time Priority Sequence Number */
		FiveEnum = 5			/*!< Indicates Not allocated (was Quote Type, Market Maker ID, Price, Quote Access Payment, Size, Time) */
	};

	
	/**
		@class thomsonreuters::eoa::domain::marketmaker::Mm::TradeUnit Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::TradeUnit is an enumeration specifying Trade Unit or precision at which prices are set.

		@see Mm::Summary
	*/
	enum class EOA_DOMAIN_API TradeUnit
	{
		IntEnum = 0,				/*!< Indicates whole number */
		OneDpEnum = 1,				/*!< Indicates one decimal place */
		TwoDpEnum = 2,				/*!< Indicates two decimal places */
		ThreeDpEnum = 3,			/*!< Indicates three decimal places */
		FourDpEnum = 4,				/*!< Indicates four decimal places */
		FiveDpEnum = 5,				/*!< Indicates five decimal places */
		SixDpEnum = 6,				/*!< Indicates six decimal places */
		SevenDpEnum = 7,			/*!< Indicates seven decimal places */
		UndefinedEnum = 8,			/*!< Indicates undefined */
		NineEnum = 9,				/*!< Indicates Not allocated */
		TenEnum = 10,				/*!< Indicates Not allocated */
		HalvesEnum = 11,			/*!< Indicates halves */
		QuartersEnum = 12,			/*!< Indicates quarters */
		EightsEnum = 13,			/*!< Indicates eighths */
		SixteenthsEnum = 14,		/*!< Indicates sixteenths */
		ThirtySecondsEnum = 15,		/*!< Indicates 1/32nds */
		SixtyFourthsEnum = 16,		/*!< Indicates 1/64ths */
		OneTwentyEigthsEnum = 17,	/*!< Indicates 1/128ths */
		TwoFiftySixthsEnum = 18,	/*!< Indicates 1/256ths */
		NineteenEnum = 19,			/*!< Indicates Not allocated */
		VarEnum = 20,				/*!< Indicates variable */
		FiveCentEnum = 21,			/*!< Indicates 2 Decimal places, MPV = 5 cents */
		OneCentEnum = 22,			/*!< Indicates 2 Decimal places, MPV = 1 cent */
		TenCentEnum = 23,			/*!< Indicates 2 Decimal places, MPV = 10 cents */
		DolarEnum = 24,				/*!< Indicates 2 Decimal places, MPV = whole dollars */
		HalfCentEnum = 25			/*!< Indicates 3 Decimal places, MPV = 0.5 cents (or 0.005) */
	};

	
	/**
		@class thomsonreuters::eoa::domain::marketmaker::Mm::MarketSource Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::MarketSource is an enumeration specifying Market Source.

		@see Mm::MarketMakerQuote
	*/
	enum class EOA_DOMAIN_API MarketSource
	{
		UnknownEnum = 0,		/*!< Indicates undefined orunknown market source */
		NasdEnum = 1,			/*!< Indicates NASDAQ */
		BerEnum = 18,			/*!< Indicates Berlin */
		BomEnum = 27,			/*!< Indicates Bombay */
		BosEnum = 28,			/*!< Indicates Boston */
		ChgEnum = 40,			/*!< Indicates Chicago */
		LaxEnum = 80,			/*!< Indicates Los Angeles */
		NycEnum = 118,			/*!< Indicates New York */
		PhlEnum = 127,			/*!< Indicates Philadelphia */
		SfrEnum = 145,			/*!< Indicates San Francisco */
		EcnEnum = 183			/*!< Indicates Electronic Communication Network */
	};


	/**
		@class thomsonreuters::eoa::domain::marketmaker::Mm::PriceQualifier Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::PriceQualifier is an enumeration specifying price qualifier code.

		@see Mm::MarketMakerQuote
	*/
	enum class EOA_DOMAIN_API PriceQualifier
	{
		NormalEnum = 0,			/*!< Indicates normal market or not allocated */
		AskEnum = 1,			/*!< Indicates ask price */
		BidEnum = 5,			/*!< Indicates bid price */
		McpEnum = 8,			/*!< Indicates market clearing price */
		CanEnum = 11,			/*!< Indicates cancelled stock */
		DefEnum = 13			/*!< Indicates defered market */
	};


	/**
		@class thomsonreuters::eoa::domain::marketmaker::Mm::MarketMakerQuote Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::MarketMakerQuote specifies an entry of MarketMakerList.
	
		\remark All methods in this class are \ref SingleThreaded.

		@see Mm::MarketMakerList
	*/
	class EOA_DOMAIN_API MarketMakerQuote
	{
	public:

		///@name Accessors
		//@{
		/** Returns ID of this MarketMakerQuote
			@return EoaBuffer containing MarketMakerQuote's id
		*/
		const foundation::EoaBuffer& getId() const;

		/** Returns ID of this MarketMakerQuote's
			@return EoaString containing MarketMakerQuote's id
		*/
		const foundation::EoaString& getIdAsString() const;

		/** Returns action associated with this MarketMakerQuote
			@return Action
		*/
		foundation::Action getAction() const;

		/** Returns the action associated with this MarketMakerQuote as a string
			@return EoaString containing action name
		*/
		const foundation::EoaString& getActionAsString() const;

		/** Indicates presence of MarketMakerQuote's bid.
			@return true if MarketMakerQuote's bid is set; false otherwise
		*/
		bool hasBid() const;

		/** returns double representation of MarketMakerQuote's bid
			@throw OmmInvalidUsageException if hasBid() returns false
			@return double value of MarketMakerQuote's bid
		*/
		double getBid() const;

		/** returns string representation of MarketMakerQuote's bid
			@return EoaString containing value of MarketMakerQuote's bid if it is present
		*/
		const foundation::EoaString& getBidAsString() const;

		/** Indicates presence of MarketMakerQuote's ask
			@return true if MarketMakerQuote's ask is set; false otherwise
		*/
		bool hasAsk() const;

		/** returns double representation of MarketMakerQuote's ask
			@throw OmmInvalidUsageException if hasAsk() returns false
			@return double value of MarketMakerQuote's ask
		*/
		double getAsk() const;

		/** returns string representation of MarketMakerQuote's ask
			@return EoaString containing value of MarketMakerQuote's ask if it is present
		*/
		const foundation::EoaString& getAskAsString() const;

		/** Indicates presence of MarketMakerQuote's ask size
			@return true if MarketMakerQuote's ask size is set; false otherwise
		*/
		bool hasAskSize() const;

		/** Returns maker's ask size
			@throw OmmInvalidUsageException if hasAskSize() returns false
			@return double representing MarketMakerQuote's ask size
		*/
		double getAskSize() const;

		/** Returns MarketMakerQuote's ask size
			@return EoaString containing MarketMakerQuote's ask size if it is present
		*/
		const foundation::EoaString& getAskSizeAsString() const;

		/** Indicates presence of MarketMakerQuote's bid size
			@return true if MarketMakerQuote's bid size is set; false otherwise
		*/
		bool hasBidSize() const;

		/** Returns MarketMakerQuote's bid size
			@throw OmmInvalidUsageException if hasBidSize() returns false
			@return double representing MarketMakerQuote's bid size
		*/
		double getBidSize() const;

		/** Returns MarketMakerQuote's bid size
			@return EoaString containing MarketMakerQuote's bid size if it is present
		*/
		const foundation::EoaString& getBidSizeAsString() const;

		/** Indicates presence of MarketMakerQuote's quote time
			@return true if MarketMakerQuote's quote time is set; false otherwise
		*/
		bool hasQuoteTime() const;

		/** Returns MarketMakerQuote's quote time
			@throw OmmInvalidUsageException if hasQuoteTime() returns false
			@return UInt64 representing MarketMakerQuote's quote time
		*/
		foundation::UInt64 getQuoteTime() const;

		/** Returns MarketMakerQuote's quote time
			@return EoaString containing MarketMakerQuote's quote time if it is present
		*/
		const foundation::EoaString& getQuoteTimeAsString() const;

		/** Indicates presence of MarketMakerQuote's MarketSource
			@return true if MarketMakerQuote's MarketSource is set; false otherwise
		*/
		bool hasMarketSource() const;

		/** Returns MarketMakerQuote's MarketSource
			@throw OmmInvalidUsageException if hasMarketSource() returns false
			@return MarketSource enumeration
		*/
		MarketSource getMarketSource() const;

		/** Returns MarketMakerQuote's MarketSource
			@return EoaString containing MarketMakerQuote's MarketSource if it is present
		*/
		const foundation::EoaString& getMarketSourceAsString() const;

		/** Indicates presence of the MarketMaker's name
		@return true if MarketMaker's name is set; false otherwise
		*/
		bool hasMarketMakerName() const;

		/** Returns the MarketMaker's name
			@throw OmmInvalidUsageException if hasMarketMakerName() returns false
			@return EoaString containing MarketMaker's name
		*/
		const foundation::EoaString& getMarketMakerName() const;

		/** Indicates presence of MarketMakerQuote's price qualifier
			@return true if MarketMakerQuote's price qualifier is set; false otherwise
		*/
		bool hasPriceQualifier() const;

		/** Returns MarketMakerQuote's price qualifier
			@throw OmmInvalidUsageException if hasPriceQualifier() returns false
			@return PriceQualifier enumeration
		*/
		PriceQualifier getPriceQualifier() const;

		/** Returns MarketMakerQuote's price qualifier
			@return EoaString containing MarketMakerQuote's price qualifier if it is present
		*/
		const foundation::EoaString& getPriceQualifierAsString() const;

		/** returns MarketMakerQuote as Node
			\remark allows retrieval of any field from the MarketMakerQuote
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

		friend class MmRefreshInfoImpl;
		friend class MmStatusInfoImpl;
		friend class MmUpdateInfoImpl;
		friend class MmConsumerItemImpl;
		friend class MarketMakerList;

		const foundation::EoaString& toString( foundation::UInt64 indent ) const;

		MarketMakerQuote();
		virtual ~MarketMakerQuote();

		const foundation::Node*			_pNode;
		mutable foundation::EoaString	_toString;
		mutable foundation::EoaString	_idString;

		MarketMakerQuote( const MarketMakerQuote& );
		MarketMakerQuote& operator=(MarketMakerQuote);
	};


	/**
		@class thomsonreuters::eoa::domain::marketmaker::Mm::Summary Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::Summary specifies summary of MarketMakerList.
	
		\remark All methods in this class are \ref SingleThreaded.

		@see Mm::MarketMakerList
	*/
	class EOA_DOMAIN_API Summary
	{
	public:

		///@name Accessors
		//@{
		//@{
		/** Indicates presence of the permission information
			@return true if the permission information is set; false otherwise
		*/
		bool hasPermission() const;

		/** Returns the permission information
			@throw OmmInvalidUsageException if hasPermission() returns false
			@return EoaBuffer containing permission information
		*/
		const foundation::EoaBuffer& getPermission() const;

		/** Indicates presence of the QuoteDate
			@return true if the Quotedate is set; false otherwise
		*/
		bool hasQuoteDate() const;

		/** Returns the QuoteDate
			@throw OmmInvalidUsageException if hasQuoteDate() returns false
			@return QuoteDate
		*/
		const foundation::OmmDate& getQuoteDate() const;

		/** Returns the QuoteDate
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

		/** Returns the TradeUnits
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

		/** Returns the Exchange ID
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

		/** Returns the Market State
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

		/** Returns the Currency
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

		/** Returns the Price Ranking Rule
			@return EoaString containing Price Ranking Rule name if it is present
		*/
		const foundation::EoaString& getPriceRankRuleAsString() const;

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

		friend class MmRefreshInfoImpl;
		friend class MmStatusInfoImpl;
		friend class MmUpdateInfoImpl;
		friend class MmConsumerItemImpl;
		friend class MarketMakerList;

		const foundation::EoaString& toString( foundation::UInt64 indent ) const;

		Summary();
		virtual ~Summary();

		const foundation::Node*			_pNode;
		mutable foundation::EoaString	_toString;

		Summary( const Summary& );
		Summary& operator=( const Summary& );
	};


	/**
		@class thomsonreuters::eoa::domain::marketmaker::Mm::MarketMakerList Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::MarketMakerList specifies MarketMaker domain payload.
	
		\remark All methods in this class are \ref SingleThreaded.

		@see Mm::MarketMakerQuote,
			Mm::Summary,
			Mm::RefreshInfo,
			Mm::UpdateInfo,
			Mm::ConsumerItem
	*/
	class EOA_DOMAIN_API MarketMakerList
	{
	public:

		/**
			@class thomsonreuters::eoa::domain::marketmaker::Mm::MarketMakerList::const_iterator Mm.h "Domain/MarketMaker/Include/Mm.h"
			@brief Mm::MarketMakerList::const_iterator is an external and explicit Mm::MarketMakerList read iterator.

			Mm::MarketMakerList::const_iterator allows for concurent and independent iteration over the same Mm::MarketMakerList instance.

			The following code snippet shows getting data from Mm::MarketMakerList using the "for-each" loop.

			\code

			for ( const auto& maker : marketMakerList )
				cout << maker << endl;

			\endcode
	
			The following code snippet shows extracting data from Mm::MarketMakerList using the const_iterator.

			\code

			for ( auto iter = marketMakerList.begin(); iter != marketMakerList.end(); ++iter )
				cout << *iter << endl;

			\endcode

			\remark Undefined behavior may occur while attempting to dereference Mm::MarketMakerList::const_iterator objects
					equal to the Mm::MarketMakerList::end().
			\remark Undefined behavior may occur while using objects of Mm::MarketMakerList::const_iterator associated with 
					the Mm::MarketMakerList object who is out of scope.
			\remark All methods in this class are \ref SingleThreaded.

			@see Mm::MarketMakerQuote,
				Mm::MarketMakerList
		*/
		class EOA_DOMAIN_API const_iterator
		{
		public :

			///@name Constructor
			//@{
			/** Default constructor.
				\remark equivalent to Mm::MarketMakerList::end()
			*/
			const_iterator();
		
			/** Constructor set to the begining of Mm::MarketMakerList
				@param[in] MarketMakerList Mm::MarketMakerList object to whose begining's this iterator will point to
			*/
			const_iterator( const Mm::MarketMakerList& marketMakerList );

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
				@return const reference to Mm::MarketMakerQuote
			*/
			const Mm::MarketMakerQuote& operator*() const;

			/** Pointer operator.
				@return const pointer to Mm::MarketMakerQuote
			*/
			const Mm::MarketMakerQuote* operator->() const;

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

			friend class MarketMakerList;

			foundation::Node::const_iterator	_nodeIterator;
			mutable MarketMakerQuote			_marketMakerQuote;
		};
	
		///@name External Iterators
		//@{
		/** Iterator to beginning.
			@return Mm::MarketMakerList::const_iterator object pointing to the first Mm::MarketMakerQuote
			\remark Undefined behavior may occur while dereferencing returned object pointing to Mm::MarketMakerList::end()
		*/
		const_iterator begin() const;

		/** Iterator to end.
			@return Mm::MarketMakerList::const_iterator object pointing beyond the last Mm::MarketMakerQuote in this Mm::MarketMakerList
			\remark Undefined behavior may occur while dereferencing returned object pointing to Mm::MarketMakerList::end()
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

		/** returns MarketMakerList as Node object
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

		friend class MmRefreshInfoImpl;
		friend class MmStatusInfoImpl;
		friend class MmUpdateInfoImpl;
		friend class MmConsumerItemImpl;

		const foundation::EoaString& toString( foundation::UInt64 indent ) const;

		MarketMakerList();
		virtual ~MarketMakerList();

		const foundation::Node*			_pNode;
		mutable Summary					_summary;
		mutable foundation::EoaString	_toString;

		MarketMakerList( const MarketMakerList& );
		MarketMakerList& operator=( const MarketMakerList& );
	};


	/**
		@class thomsonreuters::eoa::domain::marketmaker::Mm::Qos Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::Qos represents Quality Of Service information in MarketMaker domain.

		\remark All methods in this class are \ref SingleThreaded.

		@see Mm::RefreshInfo
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

		friend class MmRefreshInfoImpl;

		Qos();
		virtual ~Qos();
		Qos( const Qos& );
		Qos& operator=( const Qos& );

		MmQosImpl*	_pImpl;
	};


	/**
		@class thomsonreuters::eoa::domain::marketmaker::Mm::State Mm.h "Domain/MarketMaker/Include/Mm.h"
		@brief Mm::State represents State information in MarketMaker domain. 

		\remark All methods in this class are \ref SingleThreaded.

		@see Mm::RefreshInfo,
			Mm::StatusInfo
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

		friend class MmRefreshInfoImpl;
		friend class MmStatusInfoImpl;

		State();
		virtual ~State();
		State( const State& );
		State& operator=( const State& );

		MmStateImpl*		_pImpl;
	};


	/** Returns a string with the version of the Mm class.
		@return string with version
	*/
	static const foundation::EoaString& getVersion();

private:

	static const foundation::EoaString		_version;

	Mm();
	virtual ~Mm();

	Mm( const Mm& );
	Mm& operator=( const Mm& );
};

}

}

}

}

#endif // __thomsonreuters_eoa_domain_marketmaker_mm_h
