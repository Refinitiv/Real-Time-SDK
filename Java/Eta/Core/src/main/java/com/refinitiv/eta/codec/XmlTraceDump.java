/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;


import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;

public interface XmlTraceDump {
    int dumpBuffer(Channel channel, int protocolType, TransportBuffer buffer, DataDictionary dataDictionary, StringBuilder msgBuilder, Error error);
    int dumpBuffer(int majorVersion, int minorVersion, int protocolType, Buffer buffer, DataDictionary dataDictionary, StringBuilder msgBuilder, Error error);
}
