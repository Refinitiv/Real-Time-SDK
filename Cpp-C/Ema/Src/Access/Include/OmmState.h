/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmState_h
#define __refinitiv_ema_access_OmmState_h
/**
	@class rtsdk::ema::access::OmmState OmmState.h "Access/Include/OmmState.h"
	@brief OmmState represents State information in Omm. 

	OmmState is used to represent state of item, item group and service.
	OmmState encapsulates stream state, data state, status code and status text information.

	\remark OmmState is a read only class.
	\remark This class is used for extraction of OmmState info only.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		EmaString,
		EmaBuffer
*/

#include "Access/Include/Data.h"

namespace rtsdk {

namespace ema {

namespace domain {

namespace login {

class LoginReqImpl;
class LoginRefreshImpl;
class LoginStatusImpl;

}

}

namespace access {

class OmmStateDecoder;


class EMA_ACCESS_API OmmState : public Data
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
										accessor methods. */
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
		DacsUserAccessToAppDeniedEnum	= 32,	/*!< User is not allowed to use application */
		GapFillEnum						= 34,	/*!< Content is intended to fill a recognized gap */
		AppAuthorizationFailedEnum		= 35,	/*!< Application Authorization Failed */
		InvalidFormedMsgEnum			= 256,  /*!< DEPRECATED: Not Used */
		ChannelUnavailableEnum			= 257, /*!< DEPRECATED: Not Used */
		ServiceUnavailableEnum			= 258, /*!< DEPRECATED: Not Used */
		ServiceDownEnum					= 259, /*!< DEPRECATED: Not Used */
		ServiceNotAcceptingRequestsEnum = 260, /*!< DEPRECATED: Not Used */
		LoginClosedEnum					= 261, /*!< DEPRECATED: Not Used */
		DirectoryClosedEnum				= 262, /*!< DEPRECATED: Not Used */
		ItemNotFoundEnum				= 263, /*!< DEPRECATED: Not Used */
		DictionaryUnavailableEnum		= 264, /*!< DEPRECATED: Not Used */
		FieldIdNotFoundDictionaryUnavailableEnum = 265, /*!< DEPRECATED: Not Used */
		ItemRequestTimeoutEnum			= 266 /*!< DEPRECATED: Not Used */
	};

	///@name Accessors
	//@{	
	/** Returns the StreamState value as a string format.
		@return string representation of this object StreamState
	*/
	const EmaString& getStreamStateAsString() const;
		
	/** Returns the DataState value as a string format.
		@return string representation of this object DataState
	*/
	const EmaString& getDataStateAsString() const;
		
	/** Returns the StatusCode value as a string format.
		@return string representation of this object StatusCode
	*/
	const EmaString& getStatusCodeAsString() const;
		
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::StateEnum
	*/
	DataType::DataTypeEnum getDataType() const;

	/** Returns the Code, which indicates a special state of a DataType.
		@return Data::BlankEnum if received data is blank; Data::NoCodeEnum otherwise
	*/
	Data::DataCode getCode() const;

	/** Returns a buffer that in turn provides an alphanumeric null-terminated hexadecimal string representation.
		@return EmaBuffer with the object hex information
	*/
	const EmaBuffer& getAsHex() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EmaString& toString() const;

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
	UInt8 getStatusCode() const;

	/** Returns StatusText.
		@return EmaString containing status text information
	*/
	const EmaString& getStatusText() const;
	//@}

private :

	friend class Decoder;
	friend class StaticDecoder;
	friend class RefreshMsgDecoder;
	friend class StatusMsgDecoder;
	friend class rtsdk::ema::domain::login::LoginStatusImpl;
	friend class rtsdk::ema::domain::login::LoginRefreshImpl;
	friend class rtsdk::ema::domain::login::LoginReqImpl;

	Decoder& getDecoder();
	bool hasDecoder() const;

	const EmaString& toString( UInt64 ) const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	OmmState();
	virtual ~OmmState();
	OmmState( const OmmState& );
	OmmState& operator=( const OmmState& );

	OmmStateDecoder*		_pDecoder;
	UInt64					_space[19];
};

}

}

}

#endif //__refinitiv_ema_access_OmmState_h
