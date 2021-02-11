package com.refinitiv.eta.codec;


import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;

public interface XmlTraceDump {
    int dumpBuffer(Channel channel, int protocolType, TransportBuffer buffer, DataDictionary dataDictionary, StringBuilder msgBuilder, Error error);
    int dumpBuffer(int majorVersion, int minorVersion, int protocolType, Buffer buffer, DataDictionary dataDictionary, StringBuilder msgBuilder, Error error);
}
