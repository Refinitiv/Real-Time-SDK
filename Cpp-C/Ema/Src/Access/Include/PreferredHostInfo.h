/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.                   --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_PreferredHostInfo_h
#define __refinitiv_ema_access_PreferredHostInfo_h

 /**
	 @class refinitiv::ema::access::PreferredHostInfo PreferredHostInfo.h "Access/Include/PreferredHostInfo.h"
	 @brief PreferredHostInfo provides preferred host information for OmmConsumer.

	 \remark All methods in this class are \ref SingleThreaded.
 */

#include "Access/Include/EmaString.h"

namespace refinitiv {

namespace ema {

namespace access {

class EMA_ACCESS_API PreferredHostInfo
{
public:
	///@name Constructor
	//@{
	/** Constructs PreferredHostInfo
	*/
	PreferredHostInfo();
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~PreferredHostInfo();
	//@}

	///@name Accessors
	//@{

	/** Gets status of preferred host
		@return The status of preferred host: enable/disable.
	*/
	bool getEnablePreferredHostOptions() const { return _enablePreferredHostOptions; };

	/** Gets the preferred host detection time schedule
	  @return The detection time schedule in cron format.
	*/
	const EmaString& getPHDetectionTimeSchedule() const { return _phDetectionTimeSchedule; };

	/** Gets the preferred host detection time interval
	  @return The detection time interval in seconds.
	*/
	UInt32 getPHDetectionTimeInterval() const { return _phDetectionTimeInterval; };

	/** Gets the preferred host channel name
	  @return The preferred host channel name.
	*/
	const EmaString& getPreferredChannelName() const { return _preferredChannelName; };

	/** Gets the preferred host warm standby channel name
	  @return The status of preferred host fall back: enable/disable.
	*/
	const EmaString& getPreferredWSBChannelName() const { return _preferredWSBChannelName; };

	/** Gets status of preferred host fall back
	  @return The preferred host warm standby channel name.
	*/
	bool getPHFallBackWithInWSBGroup() const { return _phFallBackWithInWSBGroup; };

	/** Returns remaining detection time in seconds to perform fallback to preferred host.
		@return remaining detection time in seconds to perform fallback
	*/
	UInt32 getRemainingDetectionTime() const { return _remainingDetectionTime; }

	/** Returns state whether channel is preferred.
		@return state whether channel is preferred.
	*/
	bool getIsChannelPreferred() const { return _isChannelPreferred; }

	/** Gets a string representation of the class instance
		@return string representation of the class instance.
	*/
	const EmaString& toString() const;

	//@}

	///@name Operations
	//@{

	/** Specifies enable/disable preferred host
		@param[in] enable/disable preferred host
		@return reference to this object
	*/
	PreferredHostInfo& enablePreferredHostOptions(bool enablePreferredHostOptions);

	/** Specifies detection time schedule to roll over preferred host.
	  @param[in] time schedule in cron format
	  @return reference to this object
	*/
	PreferredHostInfo& phDetectionTimeSchedule(EmaString phDetectionTimeSchedule);

	/** Specifies detection time to roll over preferred host.
	  @param[in] time in seconds
	  @return reference to this object
	*/
	PreferredHostInfo& phDetectionTimeInterval(UInt32 phDetectionTimeInterval);

	/** Specifies enable/disable fall back with warm standby group.
	  @param[in] enable/disable fallback with in warm stand by group.
	  @return reference to this object
	*/
	PreferredHostInfo& phFallBackWithInWSBGroup(bool phFallBackWithInWSBGroup);

	/** Specifies remaining detection time in seconds to perform fallback to preferred host
		@param[in] remaining detection time in seconds
		@return reference to this object
	*/
	PreferredHostInfo& remainingDetectionTime(UInt32 remainingDetectionTime);
	//@}

	/** Clears the PreferredHostInfo
	  \remark invoking clear() resets all member variables to their default values
	  @return reference to this object
	*/
	PreferredHostInfo& clear();

	/** Specifies whether current channel is preferred
		@param[in] is channel preferred
		@return reference to this object
	*/
	PreferredHostInfo& isChannelPreferred(bool isChannelPreferred);
	//@}

private:
	bool _enablePreferredHostOptions;
	EmaString _phDetectionTimeSchedule;
	UInt32 _phDetectionTimeInterval;
	EmaString _preferredChannelName;
	EmaString _preferredWSBChannelName;
	bool _phFallBackWithInWSBGroup;
	UInt32 _remainingDetectionTime;
	bool   _isChannelPreferred;
	mutable EmaString _toString;

	PreferredHostInfo& preferredChannelName(UInt32 channellIndex, const void* pChannel);
	PreferredHostInfo& preferredWSBChannelName(UInt32 wsbChannellIndex, const void* pChannel);

	friend class ChannelInformation;
};

}
}
}

#endif // __refinitiv_ema_access_PreferredHostInfo_h