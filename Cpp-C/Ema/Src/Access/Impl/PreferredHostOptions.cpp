/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 LSEG. All rights reserved.	                  --
 *|-----------------------------------------------------------------------------
 */

#include "PreferredHostOptions.h"

using namespace refinitiv::ema::access;

PreferredHostOptions::PreferredHostOptions() : _enablePreferredHostOptions(false),
	 _phDetectionTimeSchedule(""),
	 _phDetectionTimeInterval(0),
	 _preferredChannelName(""),
	 _preferredWSBChannelName(""),
	 _phFallBackWithInWSBGroup(0)
{}

PreferredHostOptions& PreferredHostOptions::enablePreferredHostOptions( bool enablePreferredHostOptions)
{
	_enablePreferredHostOptions = enablePreferredHostOptions;
	return *this;
}

PreferredHostOptions& PreferredHostOptions::phDetectionTimeSchedule(const EmaString& phDetectionTimeSchedule)
{
	_phDetectionTimeSchedule = phDetectionTimeSchedule;
	return *this;
}

PreferredHostOptions& PreferredHostOptions::phDetectionTimeInterval(const UInt32 phDetectionTimeInterval)
{
	_phDetectionTimeInterval = phDetectionTimeInterval;
	return *this;
}

PreferredHostOptions& PreferredHostOptions::preferredChannelName(const EmaString& preferredChannelName)
{
	_preferredChannelName = preferredChannelName;
	return *this;
}

PreferredHostOptions& PreferredHostOptions::preferredWSBChannelName(const EmaString& preferredWSBChannelName)
{
	_preferredWSBChannelName = preferredWSBChannelName;
	return *this;
}

PreferredHostOptions& PreferredHostOptions::phFallBackWithInWSBGroup(bool phFallBackWithInWSBGroup)
{
	_phFallBackWithInWSBGroup = phFallBackWithInWSBGroup;
	return *this;
}

PreferredHostOptions& PreferredHostOptions::clear()
{
	_enablePreferredHostOptions = false;
	_phDetectionTimeSchedule.clear();
	_phDetectionTimeInterval = 0;
	_preferredChannelName.clear();
	_preferredWSBChannelName.clear();
	_phFallBackWithInWSBGroup = 0;

	return *this;
}

PreferredHostOptions::~PreferredHostOptions() 
{
}
