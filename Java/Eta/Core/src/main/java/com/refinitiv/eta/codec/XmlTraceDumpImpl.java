package com.refinitiv.eta.codec;

import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportReturnCodes;

import java.nio.ByteBuffer;
import java.util.Objects;

class XmlTraceDumpImpl implements XmlTraceDump {

    protected DecodeIterator xmlDumpIterator = CodecFactory.createDecodeIterator();
    protected Buffer xmlDumpBuffer = CodecFactory.createBuffer();
    protected Msg xmlDumpMsg = CodecFactory.createMsg();

    @Override
    public int dumpBuffer(Channel channel, int protocolType, TransportBuffer buffer, DataDictionary dataDictionary, StringBuilder msgBuilder, Error error) {
        if (Objects.isNull(buffer) || Objects.isNull(error)) {
            return TransportReturnCodes.FAILURE;
        }

        //Check when buffer empty.
        if (buffer.length() <= 0) {
            return setErrorFailure(error,"Buffer of length zero cannot be dumped.\n");
        }

        String result = "";
        if (Codec.RWF_PROTOCOL_TYPE == protocolType) {
            xmlDumpIterator.clear();
            xmlDumpIterator.setBufferAndRWFVersion(buffer, channel.majorVersion(), channel.minorVersion());
            result = xmlDumpMsg.decodeToXml(xmlDumpIterator, dataDictionary);
        } else if (Codec.JSON_PROTOCOL_TYPE == protocolType) {
            result = traceJsonMsg(buffer.data(), buffer.dataStartPosition(), buffer.length());
        } else {
            return setErrorFailure(error,"Unsupported protocol type: " + protocolType);
        }

        msgBuilder.append(result);
        return TransportReturnCodes.SUCCESS;
    }

    @Override
    public int dumpBuffer(int majorVersion, int minorVersion, int protocolType, Buffer buffer, DataDictionary dataDictionary, StringBuilder msgBuilder, Error error) {
        if (Objects.isNull(buffer) || Objects.isNull(error)) {
            return TransportReturnCodes.FAILURE;
        }

        if (buffer.length() <= 0) {
            return setErrorFailure(error,"Buffer of length zero cannot be dumped.\n");
        }

        String result = "";
        if (Codec.RWF_PROTOCOL_TYPE == protocolType) {
            xmlDumpIterator.clear();
            xmlDumpIterator.setBufferAndRWFVersion(buffer, majorVersion, minorVersion);
            result = xmlDumpMsg.decodeToXml(xmlDumpIterator, dataDictionary);
        } else if (Codec.JSON_PROTOCOL_TYPE == protocolType) {
            result = traceJsonMsg(buffer.data(), buffer.position(), buffer.length());
        } else {
            return setErrorFailure(error,"Unsupported protocol type: " + protocolType);
        }

        msgBuilder.append(result);
        return TransportReturnCodes.SUCCESS;
    }

    private String traceJsonMsg(ByteBuffer transportData, int start, int length) {
        xmlDumpBuffer.clear();
        xmlDumpBuffer.data(transportData, start, length);
        return DecodersToXML.xmlDumpString(xmlDumpBuffer, false);
    }

    private int setErrorFailure(Error error, String text) {
        error.errorId(TransportReturnCodes.FAILURE);
        error.sysError(0);
        error.text(text);
        return TransportReturnCodes.FAILURE;
    }
}
