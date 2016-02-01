/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_ommstate_h
#define __thomsonreuters_eoa_foundation_ommstate_h
/**
	@class thomsonreuters::eoa::foundation::OmmState OmmState.h "Foundation/Include/OmmState.h"
	@brief OmmState represents State information in Omm. 

	OmmState is used to represent state of item, item group and service.
	OmmState encapsulates stream state, data state, status code and status text information.

	\reoark OmmState is a read only class.
	\reoark All methods in this class are \ref SingleThreaded.

	@see Data,
		EoaString,
		EoaBuffer,
		OmmMemoryExhaustionException
*/

#include "Foundation/Include/DataType.h"
#include "Foundation/Include/EoaString.h"

namespace thomsonreuters {

namespace eoa {

namespace foundation {

class EoaBuffer;

class EOA_FOUNDATION_API OmmState 
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

		ClosedRedirectedEnum = 5	/*!<Indicates the stream is closed and has been renamed.
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
		@return string representation of this object StreamState
	*/
	const EoaString& getStreamStateAsString() const throw();
		
	/** Returns the DataState value as a string format.
		@return string representation of this object DataState
	*/
	const EoaString& getDataStateAsString() const throw();
		
	/** Returns the StatusCode value as a string format.
		@return string representation of this object StatusCode
	*/
	const EoaString& getStatusCodeAsString() const throw();

	/** Returns StreamState.
		@return value of StreamState
	*/
	StreamState getStreamState() const throw();

	/** Returns DataState.
		@return value of DataState
	*/
	DataState getDataState() const throw();

	/** Returns StatusCode.
		@return value of StatusCode
	*/
	UInt16 getStatusCode() const throw();

	/** Returns StatusText.
		@return EoaString containing status text information
	*/
	const EoaString& getStatusText() const throw();

	/** Returns a buffer that in turn provides an alphanumeric null-terminated hexadecimal string representation.
		@return EoaBuffer with the object hex information
	*/
	const EoaBuffer& getAsHex() const throw();

	/** Returns a string representation of the class instance.
		@throw OmmMemoryExhaustionException if app runs out of memory
		@return string representation of the class instance
	*/
	const EoaString& toString() const;

	/** Operator const char* overload.
		@throw OmmMemoryExhaustionException if app runs out of memory
	*/
	operator const char* () const;
	//@}

private :

	friend class Tag;
	friend class Decoder;
	friend class LeafImplDecoder;
	friend class NoLeafDecoder;
	friend class RefreshInfoImpl;
	friend class StatusInfoImpl;
	friend class CacheLeaf;

	StreamState				_streamState;
	DataState				_dataState;
	UInt16					_statusCode;
	bool					_isLocal;

	const void*				_ptr;
	mutable UInt16			_streamStateString[24];
	mutable UInt16			_dataStateString[24];
	mutable UInt16			_statusCodeString[24];
	mutable UInt16			_statusTextString[24];
	mutable EoaString		_toString;
	mutable UInt16			_eoaBuffer[24];

	const EoaString& toString( UInt64 indent ) const;

	OmmState( StreamState streamState, DataState dataState, UInt16 statusCode );

	OmmState();
	virtual ~OmmState();
	OmmState( const OmmState& );
	OmmState& operator=( const OmmState& );
};

}

}

}

#endif //__thomsonreuters_eoa_foundation_ommstate_h
