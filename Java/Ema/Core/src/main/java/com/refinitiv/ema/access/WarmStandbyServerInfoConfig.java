package com.refinitiv.ema.access;

import java.util.ArrayList;
import java.util.List;

public class WarmStandbyServerInfoConfig
{
    String          name;
    ChannelConfig   channelConfig;
    List<String>    perServiceNameSet;

    WarmStandbyServerInfoConfig(String name)
    {
        this.name = name;
        clear();
    }

    WarmStandbyServerInfoConfig()
    {
    	clear();
    }

    void clear() {
        channelConfig = null;
        if (perServiceNameSet != null)
        	perServiceNameSet.clear();
        else
        	perServiceNameSet = new ArrayList<String>();
    }
}
