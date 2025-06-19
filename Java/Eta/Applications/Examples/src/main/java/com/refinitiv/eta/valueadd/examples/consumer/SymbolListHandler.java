/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.consumer;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.eta.codec.AckMsg;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Map;
import com.refinitiv.eta.codec.MapEntry;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceClose;
import com.refinitiv.eta.shared.rdm.symbollist.SymbolListRequest;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorSubmitOptions;

/*
 * This is the symbol list handler for the ETA Value Add consumer application.
 * It provides methods for sending the symbol list request(s) to a provider
 * and processing the response(s). Method closing stream is also provided.
 */
class SymbolListHandler
{
    private final int SYMBOL_LIST_STREAM_ID_START = 400;
    private final int TRANSPORT_BUFFER_SIZE_REQUEST = 1000;
    private final int TRANSPORT_BUFFER_SIZE_CLOSE = 1000;

    private SymbolListRequest symbolListRequest;
    private MarketPriceClose closeMessage;
    private Buffer symbolListName;
    private State state;
    private Qos qos;
    private List<Long> capabilities;
    private int serviceId;
    private Map map = CodecFactory.createMap();
    private MapEntry mapEntry = CodecFactory.createMapEntry();
    private Buffer mapKey = CodecFactory.createBuffer();
    private EncodeIterator encIter = CodecFactory.createEncodeIterator();

    private boolean snapshotRequested;

    private ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

    SymbolListHandler()
    {
        state = CodecFactory.createState();
        qos = CodecFactory.createQos();
        capabilities = new ArrayList<Long>();
        symbolListName = CodecFactory.createBuffer();
        symbolListRequest = new SymbolListRequest();
        closeMessage = new MarketPriceClose();
    }

    void snapshotRequest(boolean snapshotRequested)
    {
        this.snapshotRequested = snapshotRequested;
    }

    Buffer symbolListName()
    {
        return symbolListName;
    }

    int serviceId()
    {
        return serviceId;
    }

    void serviceId(int serviceId)
    {
        this.serviceId = serviceId;
    }

    Qos qos()
    {
        return qos;
    }

    List<Long> capabilities()
    {
        return capabilities;
    }

    private boolean hasSymbolListCapability(List<Long> capabilities)
    {
        for (Long capability : capabilities)
        {
            if (capability == DomainTypes.SYMBOL_LIST)
                return true;
        }
        return false;
    }

    int sendRequest(ReactorChannel chnl, ReactorErrorInfo errorInfo)
    {
        /* check to see if the provider supports the symbol list domain */
        if (!hasSymbolListCapability(capabilities()))
        {
            errorInfo.error().text("SYMBOL_LIST domain is not supported by the indicated provider");
            return CodecReturnCodes.FAILURE;
        }

        /* get a buffer for the item request */
        TransportBuffer msgBuf = chnl.getBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, errorInfo);
        if (msgBuf == null)
        {
            return CodecReturnCodes.FAILURE;
        }

        /* initialize state management array */
        /* these will be updated as refresh and status messages are received */
        state.dataState(DataStates.NO_CHANGE);
        state.streamState(StreamStates.UNSPECIFIED);

        /* encode symbol list request */
        symbolListRequest.clear();

        if (!snapshotRequested)
            symbolListRequest.applyStreaming();
        symbolListRequest.symbolListName().data(symbolListName.data(), symbolListName.position(), symbolListName.length());
        symbolListRequest.streamId(SYMBOL_LIST_STREAM_ID_START);
        symbolListRequest.serviceId(serviceId());
        symbolListRequest.applyHasServiceId();
        symbolListRequest.qos().dynamic(qos.isDynamic());
        symbolListRequest.qos().rate(qos.rate());
        symbolListRequest.qos().timeliness(qos.timeliness());
        symbolListRequest.applyHasQos();
        symbolListRequest.priority(1, 1);

        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());

        System.out.println(symbolListRequest.toString());
        int ret = symbolListRequest.encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        return chnl.submit(msgBuf, submitOptions, errorInfo);
    }

    @SuppressWarnings("fallthrough")
	int processResponse(Msg msg, DecodeIterator dIter, DataDictionary dictionary)
    {
        map.clear();
        mapEntry.clear();
        mapKey.clear();
        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
                RefreshMsg refreshMsg = (RefreshMsg)msg;
                state.dataState(refreshMsg.state().dataState());
                state.streamState(refreshMsg.state().streamState());
            case MsgClasses.UPDATE:
                return handleUpdate(msg, dIter, map, mapEntry, mapKey);

            case MsgClasses.STATUS:
                StatusMsg statusMsg = (StatusMsg)msg;
                System.out.println("Received Item StatusMsg for stream " + msg.streamId());
                if (statusMsg.checkHasState())
                {
                    this.state.dataState(statusMsg.state().dataState());
                    this.state.streamState(statusMsg.state().streamState());
                    System.out.println("    " + state);
                }
                return CodecReturnCodes.SUCCESS;
            case MsgClasses.ACK:
                handleAck(msg);
                return CodecReturnCodes.SUCCESS;
            default:
                System.out.println("Received Unhandled Item Msg Class: " + msg.msgClass());
                break;
        }

        return CodecReturnCodes.SUCCESS;
    }

    protected int handleUpdate(Msg msg, DecodeIterator dIter, Map map,
            MapEntry mapEntry, Buffer mapKey)
    {
        // print the name of the symbolist and the domain
        System.out.println(symbolListName.toString() +
                "\nDOMAIN: " + DomainTypes.toString(msg.domainType()));

        int ret = map.decode(dIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("DecodeMap() failed: < " + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        // decode the map
        while ((ret = mapEntry.decode(dIter, mapKey)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("DecodeMapEntry() failed: < " + CodecReturnCodes.toString(ret) + ">");
                return ret;
            }
            System.out.println(mapKey.toString() + "\t" +
                        mapEntryActionToString(mapEntry.action()));
        }

        return CodecReturnCodes.SUCCESS;
    }

    protected int handleAck(Msg msg)
    {
        /*
         * although this application only posts on Market Price, ACK handler is
         * provided for other domains to allow user to extend and post on MBO
         * and MBP domains
         */
        System.out.println("Received AckMsg for stream " + msg.streamId());
        MsgKey key = msg.msgKey();

        //print out item name from key if it has it
        if (key != null && key.checkHasName())
        {
            System.out.println(key.name() + "\nDOMAIN: " +
                        DomainTypes.toString(msg.domainType()));
        }

        AckMsg ackMsg = (AckMsg)msg;
        System.out.println("\tackId=" + ackMsg.ackId() + (ackMsg.checkHasSeqNum() ? "\tseqNum=" + ackMsg.seqNum() : "") + (ackMsg.checkHasNakCode() ? "\tnakCode=" + ackMsg.nakCode() : "") + (ackMsg.checkHasText() ? "\ttext=" + ackMsg.text().toString() : ""));
        return CodecReturnCodes.SUCCESS;
    }

    /*
     * Close the symbol list stream.
     */
    int closeStream(ReactorChannel chnl, ReactorErrorInfo errorInfo)
    {
        /*
         * we only want to close a stream if it was not already closed (e.g.
         * rejected by provider, closed via refresh or status, or redirected)
         */
        if (state.streamState() != StreamStates.OPEN && state.dataState() != DataStates.OK)
            return CodecReturnCodes.SUCCESS;

        //get a buffer for the item close
        TransportBuffer msgBuf = chnl.getBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, errorInfo);

        if (msgBuf == null)
        {
            return CodecReturnCodes.FAILURE;
        }

        //encode item close
        closeMessage.clear();

        closeMessage.streamId(SYMBOL_LIST_STREAM_ID_START);
        closeMessage.domainType(DomainTypes.SYMBOL_LIST);
        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());

        int ret = closeMessage.encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            errorInfo.error().text("encodeSymbolListClose(): Failed <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }
        return chnl.submit(msgBuf, submitOptions, errorInfo);
    }

    private String mapEntryActionToString(int mapEntryAction)
    {
        switch (mapEntryAction)
        {
            case MapEntryActions.UPDATE:
                return "UPDATE";
            case MapEntryActions.ADD:
                return "ADD";
            case MapEntryActions.DELETE:
                return "DELETE";
            default:
                return "Unknown Map Entry Action";
        }
    }
}
