/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024-2025 LSEG. All rights reserved.	                  --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_PreferredHostOptions_h
#define __refinitiv_ema_access_PreferredHostOptions_h

 /**
	 @class refinitiv::ema::access::PreferredHostOptions PreferredHostOptions.h "Access/Include/PreferredHostOptions.h"
	 @brief PreferredHostOptions provides API to set/get preferred host option. This object is used to set preferred host option with IOCtl

	 @see  refinitiv::ema::access::OmmConsumer modifyReactorChannelIOCtl

	 \remark All methods in this class are \ref SingleThreaded.
 */

#include "Common.h"
#include "EmaString.h"

namespace refinitiv {

namespace ema {

namespace access {

class EMA_ACCESS_API PreferredHostOptions
{
public:
	///@name Constructor
	//@{
	/** Constructs PreferredHostOptions
	*/
	PreferredHostOptions();
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~PreferredHostOptions();
	//@}

	///@name Accessors
	//@{

	/** Returns status enable/disable of preferred host
		@return status
	*/
	bool getEnablePreferredHostOptions() const { return _enablePreferredHostOptions; };

	/** Returns string of detection time schedule in cron format.
		@return cron string
	*/
	const EmaString& getPHDetectionTimeSchedule() const { return _phDetectionTimeSchedule; };

	/** Returns detection time interval in seconds.
		@return time in seconds
	*/
	UInt32 getPHDetectionTimeInterval() const { return _phDetectionTimeInterval; };

	/** Returns name of preferred host channel.
		@return name of preferred host channel
	*/
	const EmaString& getPreferredChannelName() const { return _preferredChannelName; };

	/** Returns name of warm standby preferred host channel.
		@return name of warm standby preferred host channel
	*/
	const EmaString& getPreferredWSBChannelName() const { return _preferredWSBChannelName; };

	/** Returns status enable/disable of fall back in warm standby group.
		@return status of fall back in warm standby group
	*/
	bool getPHFallBackWithInWSBGroup() const { return _phFallBackWithInWSBGroup; };

	/** Returns the name of the session channel that this structure's options will apply to.
		@return name of the session channel
	*/
	const EmaString& getSessionChannelName() const { return _sessionChannelName; };

	///@name Operations
	//@{

	/** Specifies enable/disable preferred host feature.
		@param[in] enablePreferredHostOptions specifies enable/disable preferred host
		@return reference to this object
	*/
	PreferredHostOptions& enablePreferredHostOptions(bool enablePreferredHostOptions);

	/** Specifies detection time schedule for preferred host.
		@param[in] phDetectionTimeSchedule specifies cron formated string of time schedule
		@return reference to this object
	*/
	PreferredHostOptions& phDetectionTimeSchedule(const EmaString& phDetectionTimeSchedule);

	/** Specifies detection time interval for preferred host.
		@param[in] phDetectionTimeInterval specifies time in seconds
		@return reference to this object
	*/
	PreferredHostOptions& phDetectionTimeInterval(UInt32 phDetectionTimeInterval);

	/** Specifies channel name for preferred host.
		@param[in] preferredChannelName specifies channel name
		@return reference to this object
	*/
	PreferredHostOptions& preferredChannelName(const EmaString& preferredChannelName);

	/** Specifies warm standby channel name for preferred host.
		@param[in] preferredWSBChannelName specifies warm standby channel name
		@return reference to this object
	*/
	PreferredHostOptions& preferredWSBChannelName(const EmaString& preferredWSBChannelName);

	/** Specifies perform preferred host fallback for warm stand by group.
		@param[in] phFallBackWithInWSBGroup specifies enable/disable preferred host
		@return reference to this object
	*/
	PreferredHostOptions& phFallBackWithInWSBGroup(bool phFallBackWithInWSBGroup);

	/** Specifies the name of the session channel that this structure's options will apply to.
		This is required to be set to a valid configured session channel name if session channels are used in the configuration
		@param[in] sessionChannelName specifies the name of the session channel
		@return reference to this object
	*/
	PreferredHostOptions& sessionChannelName(const EmaString& sessionChannelName);
	//@}

	/** Clears the PreferredHostOptions
		\remark invoking clear() resets all member variables to their default values
	*/
	PreferredHostOptions& clear();

	//@}

private:
	bool _enablePreferredHostOptions;
	EmaString _phDetectionTimeSchedule;
	UInt32 _phDetectionTimeInterval;
	EmaString _preferredChannelName;
	EmaString _preferredWSBChannelName;
	EmaString _sessionChannelName;
	bool _phFallBackWithInWSBGroup;
};

}
}
}

#endif //__refinitiv_ema_access_PreferredHostOptions_h
