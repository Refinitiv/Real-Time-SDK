/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

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
