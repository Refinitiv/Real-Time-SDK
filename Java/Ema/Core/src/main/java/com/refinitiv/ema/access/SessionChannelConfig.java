///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|              Copyright (C) 2024-2025 LSEG. All rights reserved.                --
///*|-----------------------------------------------------------------------------


package com.refinitiv.ema.access;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.refinitiv.eta.valueadd.reactor.ReactorConnectOptions;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;

class SessionChannelConfig {
	
	String					name;
	List<ChannelConfig>		configChannelSet;
	List<WarmStandbyChannelConfig> configWarmStandbySet;
	int reconnectAttemptLimit;
	int reconnectMinDelay;
	int reconnectMaxDelay;
	
	/* Preferred host configuration options */
	boolean enablePerferredHostOptions;
	String preferredChannelName = "";
	String preferredWSBChannelName = "";
	String detectionTimeSchedule = "";
	long detectionTimeInterval;
	boolean fallBackWithInWSBGroup;
	int preferredChannelIndex = 0;
	int preferredWSBChannelIndex = 0;
	
	Map<String, Integer>  _wsbModeMap;
	
	private ReactorConnectOptions 		_rsslReactorConnOptions = ReactorFactory.createReactorConnectOptions();
	
	SessionChannelConfig(String name)
    {
        this.name = name;
        clear();
    }
	
	ReactorConnectOptions connectOptions()
	{
		return _rsslReactorConnOptions;
	}

    void clear()
    {
		 reconnectAttemptLimit = ActiveConfig.DEFAULT_RECONNECT_ATTEMPT_LIMIT;
		 reconnectMinDelay = ActiveConfig.DEFAULT_RECONNECT_MIN_DELAY;
		 reconnectMaxDelay = ActiveConfig.DEFAULT_RECONNECT_MAX_DELAY;
    	
    	if(configChannelSet != null)
    		configChannelSet.clear();
    	else
    		configChannelSet = new ArrayList<ChannelConfig>();
    	
    	if(configWarmStandbySet != null)
    		configWarmStandbySet.clear();
    	else
    		configWarmStandbySet = new ArrayList<WarmStandbyChannelConfig>();
    	
    	enablePerferredHostOptions = false;
    	preferredChannelName = "";
    	preferredWSBChannelName = "";
    	detectionTimeSchedule = "";
    	detectionTimeInterval = 0;
    	fallBackWithInWSBGroup = false;
    	preferredChannelIndex = 0;
    	preferredWSBChannelIndex = 0;
    }
    
    /* Gets a WSB mode by a WSB channel name. Return -1 if the WSB mode is not found */
    int getWSBModeByChannelName(String channelName)
    {
    	if(configWarmStandbySet.size() != 0)
    	{
    		if(_wsbModeMap != null)
    		{
    			Integer wsbMode = _wsbModeMap.get(channelName);
    			
    			if(wsbMode != null)
    			{
    				return wsbMode.intValue();
    			}
    		}
    		
    		for(WarmStandbyChannelConfig wsbConfig : configWarmStandbySet)
    		{
    			if(wsbConfig.name.equals(channelName))
    			{
    				if(_wsbModeMap == null)
    				{
    					_wsbModeMap = new HashMap<String, Integer>();
    				}
    				
    				_wsbModeMap.put(channelName, wsbConfig.warmStandbyMode);
    				
    				return wsbConfig.warmStandbyMode;
    			}
    		}
    	}
    	    	
    	return -1;
    }
    

    <T> void validatePreferChannelAndWSBChannes(OmmBaseImpl<T> ommBaseImpl)
    {
    	boolean foundIndex = false;
    	StringBuilder names = ommBaseImpl.strBuilder();
    	if(preferredChannelName.length() != 0 && configChannelSet.size() > 0)
    	{
	    	for(int index = 0; index < configChannelSet.size(); index++)
			{
	    		ChannelConfig entry = configChannelSet.get(index);
	    		names.append(entry.name);
				if(entry.name.equalsIgnoreCase(preferredChannelName))
				{
					foundIndex = true;
					preferredChannelIndex = index;
				}
				
				if( (index + 1) < configChannelSet.size())
				{
					names.append(",");
				}
			}
	    	
	    	if(!foundIndex)
	    	{
	    		ommBaseImpl.throwOmmIUExceptIfNameIsNotInList(preferredChannelName, "PreferredChannelName", names.toString(), "SessionChannelInfo.ChannelSet");
	    	}
    	}
    	
    	names.setLength(0);
    	foundIndex = false;
    	if(preferredWSBChannelName.length() != 0 && configWarmStandbySet.size() > 0)
    	{
	    	for(int index = 0; index < configWarmStandbySet.size(); index++)
			{
	    		WarmStandbyChannelConfig entry = configWarmStandbySet.get(index);
	    		names.append(entry.name);
				if(entry.name.equalsIgnoreCase(preferredWSBChannelName))
				{
					foundIndex = true;
					preferredWSBChannelIndex = index;
				}
				
				if( (index + 1) < configWarmStandbySet.size())
				{
					names.append(",");
				}
			}
	    	
	    	if(!foundIndex)
	    	{
	    		ommBaseImpl.throwOmmIUExceptIfNameIsNotInList(preferredWSBChannelName, "PreferredWSBChannelName", names.toString(), "SessionChannelInfo.WarmStandbyChannelSet");
	    	}
    	}
    }
}
