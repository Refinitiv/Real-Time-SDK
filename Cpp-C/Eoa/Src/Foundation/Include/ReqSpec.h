/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_reqspec_h
#define __thomsonreuters_eoa_foundation_reqspec_h

/**
	@class thomsonreuters::eoa::foundation::ReqSpec ReqSpec.h "Foundation/Include/ReqSpec.h"
	@brief ReqSpec is used to specify item's attributes.
	
	ReqSpec allows application to register its interest in a specified item.
	Among other attributes, ReqSpec conveys item's name, domain type, and
	desired quality of service.
	
	\remark All methods in this class are \ref SingleThreaded.

	@see Node,
		EoaString,
		EoaBuffer,
		OmmUnsupportedDomainTypeException
*/

#include "Foundation/Include/Common.h"

namespace thomsonreuters {

namespace eoa {

namespace foundation {

class EoaBuffer;
class EoaString;
class Node;

class ReqSpecImpl;

class EOA_FOUNDATION_API ReqSpec
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
	*/
	ReqSpec();

	/** Copy Constructor.
		@param[in] other object to copy from
	*/
	ReqSpec( const ReqSpec& );

	/** Constructs ReqSpec.
		@param[in] name specifies an item name.
	*/
	ReqSpec( const EoaString& name );
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
	ReqSpec& operator=( const ReqSpec& );

	/** Specifies stream ID.
		@param[in] id specifies a stream ID
		@return reference to this object
	*/
	ReqSpec& streamId( Int32 id );

	/** Specifies domain type.
		@throw OmmUnsupportedDomainTypeException if domainType is greater then 255
		@param[in] domainType specifies RDM Message Model Type (default value is MMT_MARKET_PRICE)
		@return reference to this object
	*/
	ReqSpec& domainType( UInt16 domainType = 6 );

	/** Specifies item name.
		@throw OmmMemoryExhaustionException if app runs out of memory
		@param[in] name represents item name
		@return reference to this object
	*/
	ReqSpec& name( const EoaString& name );

	/** Specifies name type.
		@param[in] nameType represents name type
		@return reference to this object
	*/
	ReqSpec& nameType( UInt8 nameType );

	/** Specifies id.
		@param[in] id represents item id.
		@return reference to this object
	*/
	ReqSpec& id( Int32 id );

	/** Specifies filter.
		@param[in] filter represents a filter value.
		@return reference to this object
	*/
	ReqSpec& filter( UInt32 filter );

	/** Specifies item priority.
		@param[in] priorityClass represents item's priority class. 
		@param[in] priorityCount represents item's priority count.
		@return reference to this object
	*/
	ReqSpec& priority( UInt8 priorityClass = 1, UInt16 priorityCount = 1 );

	/** Specifies Qos.
		@param[in] timeliness represents Qos timeliness.
		@param[in] rate represents Qos rate.
		@return reference to this object
	*/
	ReqSpec& qos( Int32 timeliness = BestTimelinessEnum , Int32 rate = BestRateEnum );

	/** Specifies attribute.
		@throw OmmMemoryExhaustionException if app runs out of memory
		@param[in] node represents attribute information.
		@return reference to this object
	*/
	ReqSpec& attrib( const Node& attrib );

	/** Specifies payload.
		@throw OmmMemoryExhaustionException if app runs out of memory
		@param[in] node represents item payload.
		@return reference to this object
	*/
	ReqSpec& payload( const Node& payload );

	/** Specifies extendedHeader.
		@throw OmmMemoryExhaustionException if app runs out of memory
		@param[in] extendedHeader represents an optional extended header.
		@return reference to this object
	*/
	ReqSpec& extendedHeader( const EoaBuffer& extendedHeader );

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

	ReqSpecImpl*		_pReqSpecImpl;

	friend class ConsumerService;
};

}

}

}

#endif //__thomsonreuters_eoa_foundation_reqspec_h
