package com.thomsonreuters.upa.examples.consumer;

import java.util.ArrayList;
import java.util.List;

import com.thomsonreuters.upa.codec.AckMsg;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Map;
import com.thomsonreuters.upa.codec.MapEntry;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.examples.common.ChannelSession;
import com.thomsonreuters.upa.examples.rdm.marketprice.MarketPriceClose;
import com.thomsonreuters.upa.examples.rdm.symbollist.SymbolListRequest;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.Error;

/**
 * This is the symbol list handler for the UPA consumer application. It provides
 * methods for sending the symbol list request(s) to a provider and processing
 * the response(s). Method closing stream is also provided.
 */
public class SymbolListHandler
{
    private static final int SYMBOL_LIST_STREAM_ID_START = 400;
    public static final int TRANSPORT_BUFFER_SIZE_REQUEST = ChannelSession.MAX_MSG_SIZE;
    public static final int TRANSPORT_BUFFER_SIZE_CLOSE = ChannelSession.MAX_MSG_SIZE;

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

    public SymbolListHandler()
    {
        state = CodecFactory.createState();
        qos = CodecFactory.createQos();
        capabilities = new ArrayList<Long>();
        symbolListName = CodecFactory.createBuffer();
        symbolListRequest = new SymbolListRequest();
        closeMessage = new MarketPriceClose();
    }

    public void snapshotRequest(boolean snapshotRequested)
    {
        this.snapshotRequested = snapshotRequested;
    }

    public Buffer symbolListName()
    {
        return symbolListName;
    }

    public int serviceId()
    {
        return serviceId;
    }

    public void serviceId(int serviceId)
    {
        this.serviceId = serviceId;
    }

    public Qos qos()
    {
        return qos;
    }

    public List<Long> capabilities()
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

    public int sendRequest(ChannelSession chnl, com.thomsonreuters.upa.transport.Error error)
    {
        /* check to see if the provider supports the symbol list domain */
        if (!hasSymbolListCapability(capabilities()))
        {
            error.text("SYMBOL_LIST domain is not supported by the indicated provider");
            return CodecReturnCodes.FAILURE;
        }

        /* get a buffer for the item request */
        TransportBuffer msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, error);
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
        encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());

        System.out.println(symbolListRequest.toString());
        int ret = symbolListRequest.encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        return chnl.write(msgBuf, error);
    }

    public int processResponse(Msg msg, DecodeIterator dIter, DataDictionary dictionary)
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
        System.out.println(symbolListName +
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

    /**
     * Close the symbol list stream.
     * 
     * @param chnl - The channel to send a symbol list close to
     */
    public int closeStream(ChannelSession chnl, Error error)
    {
        /*
         * we only want to close a stream if it was not already closed (e.g.
         * rejected by provider, closed via refresh or status, or redirected)
         */
        if (state.streamState() != StreamStates.OPEN && state.dataState() != DataStates.OK)
            return CodecReturnCodes.SUCCESS;

        //get a buffer for the item close
        TransportBuffer msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false,
                                                         error);

        if (msgBuf == null)
        {
            return CodecReturnCodes.FAILURE;
        }

        //encode item close
        closeMessage.clear();

        closeMessage.streamId(SYMBOL_LIST_STREAM_ID_START);
        closeMessage.domainType(DomainTypes.SYMBOL_LIST);
        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());

        int ret = closeMessage.encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("encodeSymbolListClose(): Failed <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }
        return chnl.write(msgBuf, error);
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