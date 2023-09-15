package com.refinitiv.ema.access;

import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyMode;

import java.util.ArrayList;
import java.util.List;

public class WarmStandbyChannelConfig
{
    String name;
    WarmStandbyServerInfoConfig startingActiveServer;
    List<WarmStandbyServerInfoConfig> standbyServerSet;
    boolean downloadConnectionConfig;
    int warmStandbyMode;

    WarmStandbyChannelConfig(String name)
    {
        this.name = name;
        clear();
    }

    WarmStandbyChannelConfig()
    {
    	clear();
    }

    void clear()
    {
        startingActiveServer = null;
        if (standbyServerSet != null)
        	standbyServerSet.clear();
        else
        	standbyServerSet = new ArrayList<WarmStandbyServerInfoConfig>();
        downloadConnectionConfig = ActiveConfig.DEFAULT_WSB_DOWNLOAD_CONNECTION_CONFIG;
        warmStandbyMode = ReactorWarmStandbyMode.LOGIN_BASED;
    }
}
