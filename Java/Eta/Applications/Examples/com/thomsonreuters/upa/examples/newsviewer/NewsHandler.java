package com.thomsonreuters.upa.examples.newsviewer;

import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Date;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Vector;

import com.thomsonreuters.upa.codec.AckMsg;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.DictionaryEntry;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.EnumType;
import com.thomsonreuters.upa.codec.FieldEntry;
import com.thomsonreuters.upa.codec.FieldList;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.PostUserInfo;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RmtesBuffer;
import com.thomsonreuters.upa.codec.RmtesCacheBuffer;
import com.thomsonreuters.upa.codec.RmtesDecoder;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.codec.UpdateMsg;
import com.thomsonreuters.upa.examples.common.ChannelSession;
import com.thomsonreuters.upa.examples.common.LoginHandler;
import com.thomsonreuters.upa.examples.common.StreamIdWatchList;
import com.thomsonreuters.upa.examples.common.StreamIdWatchList.StreamIdKey;
import com.thomsonreuters.upa.examples.common.StreamIdWatchList.WatchListEntry;
import com.thomsonreuters.upa.shared.rdm.marketprice.MarketPriceClose;
import com.thomsonreuters.upa.shared.rdm.marketprice.MarketPriceRequest;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.UpdateEventTypes;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRefresh;

/**
 * This is the news handler for the UPA NewsViewer application. It provides
 * methods for sending the market price request(s) to a provider and processing
 * the response(s). News items are requested via these market price requests.
 * Methods for decoding a field entry from a response, and closing market price
 * streams are also provided.
 * 
 */
public class NewsHandler
{
    public static int TRANSPORT_BUFFER_SIZE_REQUEST = ChannelSession.MAX_MSG_SIZE;
    public static int TRANSPORT_BUFFER_SIZE_CLOSE = ChannelSession.MAX_MSG_SIZE;

    private int domainType;

    /* Channel to use for private stream redirect */
    protected ChannelSession redirectChnl;

    /* Login information to use for private stream redirect */
    private LoginRefresh redirectLoginInfo;

    /* Source directory information to use for private stream redirect */
    private Service redirectSrcDirInfo;

    private MarketPriceRequest marketPriceRequest;

    private MarketPriceClose closeMessage;

    protected final StreamIdWatchList watchList;

    private boolean viewRequested = false;
    private boolean snapshotRequested = false;
    private List<String> viewFieldList;

    // temp. reusable variables used for encoding
    protected FieldList fieldList = CodecFactory.createFieldList();
    protected FieldEntry fieldEntry = CodecFactory.createFieldEntry();
    private UInt fidUIntValue = CodecFactory.createUInt();
    private Int fidIntValue = CodecFactory.createInt();
    private Real fidRealValue = CodecFactory.createReal();
    private com.thomsonreuters.upa.codec.Enum fidEnumValue = CodecFactory.createEnum();
    private com.thomsonreuters.upa.codec.Date fidDateValue = CodecFactory.createDate();
    private Time fidTimeValue = CodecFactory.createTime();
    private DateTime fidDateTimeValue = CodecFactory.createDateTime();
    private com.thomsonreuters.upa.codec.Float fidFloatValue = CodecFactory.createFloat();
    private com.thomsonreuters.upa.codec.Double fidDoubleValue = CodecFactory.createDouble();
    private Qos fidQosValue = CodecFactory.createQos();
    private State fidStateValue = CodecFactory.createState();
    private EncodeIterator encIter = CodecFactory.createEncodeIterator();
    private RmtesDecoder decoder = CodecFactory.createRmtesDecoder();
    private int bufferLength = 2000;
    private RmtesBuffer rmtesBuffer = CodecFactory.createRmtesBuffer(bufferLength);
    private RmtesCacheBuffer cacheBuf = CodecFactory.createRmtesCacheBuffer(bufferLength);
    private String fEntryString;
    private List<String> tempStringList = new Vector<String>();
    private StringBuilder fieldString = new StringBuilder();

    private NewsViewerFrame newsViewer;
    private Headline headline = new Headline();
    private ChannelSession _chnl;

    public NewsHandler(int fontSize, String font, StreamIdWatchList watchList)
    {
        this(fontSize, font, DomainTypes.MARKET_PRICE, watchList);
    }

    protected NewsHandler(int fontSize, String font, int domainType, StreamIdWatchList watchList)
    {
        this.watchList = watchList;
        this.domainType = domainType;
        marketPriceRequest = createMarketPriceRequest();
        closeMessage = new MarketPriceClose();
        viewFieldList = new ArrayList<String>();
        viewFieldList.add("6");
        viewFieldList.add("22");
        viewFieldList.add("25");
        viewFieldList.add("32");
        newsViewer = new NewsViewerFrame(fontSize, font, this);
    }

    protected MarketPriceRequest createMarketPriceRequest()
    {
        return new MarketPriceRequest();
    }

    public void viewRequest(boolean doViewRequest)
    {
        viewRequested = doViewRequest;
    }

    public void snapshotRequest(boolean snapshotRequested)
    {
        this.snapshotRequested = snapshotRequested;
    }

    protected void removeMarketPriceItemEntry(int streamId)
    {
        watchList.remove(streamId);
    }

    protected int closeStream(ChannelSession chnl, int streamId, Error error)
    {
        /* get a buffer for the item close */
        TransportBuffer msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, error);
        if (msgBuf == null)
            return CodecReturnCodes.FAILURE;

        /* encode item close */
        closeMessage.clear();
        closeMessage.streamId(streamId);
        closeMessage.domainType(domainType);
        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel()
                .minorVersion());

        int ret = closeMessage.encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("encodeMarketPriceClose(): Failed <"
                    + CodecReturnCodes.toString(ret) + ">");
        }
        return chnl.write(msgBuf, error);
    }

    /*
     * this method is used while posting to query the first requested item, if
     * any. It will populate the passed in buffer with the name and length
     * information and return the streamId associated with the stream. If
     * itemName->length is 0 and streamId is returned as 0, this indicates that
     * there is no valid name available.
     */
    public int getFirstItem(Buffer itemName)
    {
        return watchList.getFirstItem(itemName);
    }

    private boolean hasMarketPriceCapability(List<Long> capabilities)
    {
        for (Long capability : capabilities)
        {
            if (capability.equals((long)marketPriceRequest.domainType()))
                return true;
        }
        return false;
    }

    /**
     * Encodes and sends item requests for news items
     * 
     * @param chnl - The channel to send a source directory request to
     * 
     * @param itemNames - List of item names
     * 
     * @param isPrivateStream - flag indicating if requested items are private
     *            stream or not.
     * 
     * @param loginInfo - RDM login information
     * @param serviceInfo - RDM directory response information
     * 
     * @return success if item requests can be made, can be encoded and sent
     *         successfully. Failure if service does not support market price
     *         capability or failure for encoding/sending request.
     */
    public int sendItemRequests(ChannelSession chnl, List<String> itemNames,
            boolean isPrivateStream, LoginRefresh loginInfo, Service serviceInfo, Error error)
    {
        if (itemNames == null || itemNames.isEmpty())
            return CodecReturnCodes.SUCCESS;

        /* check to see if the provider supports the market price domain */
        if (!hasMarketPriceCapability(serviceInfo.info().capabilitiesList()))
        {
            error.text("'" + DomainTypes.toString(marketPriceRequest.domainType())
                    + "' not supported by the indicated provider");
            return CodecReturnCodes.FAILURE;
        }

        /* set redirect channel for private stream redirect */
        redirectChnl = chnl;

        /* set login information for private stream redirect */
        redirectLoginInfo = loginInfo;

        /* set source directory information for private stream redirect */
        redirectSrcDirInfo = serviceInfo;

        generateRequest(marketPriceRequest, isPrivateStream, serviceInfo, loginInfo);

        // If there is only one item in the itemList, it is a waste of bandwidth
        // to send a batch request
        if (itemNames.size() == 1)
        {
            return sendRequest(chnl, itemNames, error);
        }

        if (!(loginInfo.checkHasFeatures() && loginInfo.features().checkHasSupportBatchRequests() && loginInfo
                .features().supportBatchRequests() == 1))
        {
            System.out
                    .println("Connected Provider does not support Batch Requests. Sending Market Price requests as individual request messages.");
            return sendRequest(chnl, itemNames, error);
        }

        // batch
        return sendBatchRequest(chnl, itemNames, error);
    }

    private void generateRequest(MarketPriceRequest marketPriceRequest, boolean isPrivateStream,
            Service srcDirInfo, LoginRefresh loginInfo)
    {
        marketPriceRequest.clear();

        if (!snapshotRequested)
            marketPriceRequest.applyStreaming();
        marketPriceRequest.applyHasServiceId();
        marketPriceRequest.serviceId(srcDirInfo.serviceId());
        marketPriceRequest.applyHasPriority();
        marketPriceRequest.priority(1, 1);
		if (srcDirInfo.info().qosList().size() > 0)
		{
			marketPriceRequest.applyHasQos();
			marketPriceRequest.qos().dynamic(false);
			marketPriceRequest.qos().timeInfo(srcDirInfo.info().qosList().get(0).timeInfo());
			marketPriceRequest.qos().timeliness(srcDirInfo.info().qosList().get(0).timeliness());
			marketPriceRequest.qos().rateInfo(srcDirInfo.info().qosList().get(0).rateInfo());
			marketPriceRequest.qos().rate(srcDirInfo.info().qosList().get(0).rate());
		}
        if (isPrivateStream)
            marketPriceRequest.applyPrivateStream();

        if (loginInfo.checkHasFeatures() && loginInfo.features().checkHasSupportViewRequests()
                && loginInfo.features().supportViewRequests() == 1 && viewRequested)
        {
            marketPriceRequest.applyHasView();
            marketPriceRequest.viewFields().addAll(viewFieldList);
        }
    }

    // sends items as batch request
    private int sendBatchRequest(ChannelSession chnl, List<String> itemNames, Error error)
    {
        int batchStreamId = watchList.add(domainType, "BATCH_" + new Date(),
                                          marketPriceRequest.checkPrivateStream());
        marketPriceRequest.streamId(batchStreamId);
        for (String itemName : itemNames)
        {
            watchList.add(domainType, itemName, marketPriceRequest.checkPrivateStream());
            marketPriceRequest.itemNames().add(itemName);
        }

        return encodeAndSendRequest(chnl, marketPriceRequest, error);
    }

    // sends one item at a time
    private int sendRequest(ChannelSession chnl, List<String> itemNames, Error error)
    {
        int ret = CodecReturnCodes.SUCCESS;
        for (String itemName : itemNames)
        {
            Integer streamId = watchList.add(domainType, itemName,
                                             marketPriceRequest.checkPrivateStream());

            marketPriceRequest.itemNames().clear();
            marketPriceRequest.itemNames().add(itemName);

            marketPriceRequest.streamId(streamId);
            ret = encodeAndSendRequest(chnl, marketPriceRequest, error);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    private int encodeAndSendRequest(ChannelSession chnl, MarketPriceRequest marketPriceRequest,
            Error error)
    {
        // get a buffer for the item request
        TransportBuffer msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false,
                                                         error);

        if (msgBuf == null)
        {
            return CodecReturnCodes.FAILURE;
        }

        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel()
                .minorVersion());

        int ret = marketPriceRequest.encode(encIter);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            error.text("MarketPriceRequest.encode() failed");
            error.errorId(ret);
            return ret;
        }

        System.out.println(marketPriceRequest.toString());
        return chnl.write(msgBuf, error);
    }

    /**
     * Publicly visible response handler
     * 
     * Processes a market price response containing news info. This consists of
     * extracting the key, printing out the item name contained in the key,
     * decoding the field list and field entry, and calling decodeFieldEntry()
     * to decode the field entry data.
     * 
     * @param msg - The partially decoded message
     * @param dIter - The decode iterator
     * 
     * @return success if decoding succeeds, failure if it fails.
     */
    public int processResponse(ChannelSession chnl, Msg msg, DecodeIterator dIter,
            DataDictionary dictionary, Error error)
    {
        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
                return handleRefresh(chnl, msg, dIter, dictionary, error);
            case MsgClasses.UPDATE:
                return handleUpdate(chnl, msg, dIter, dictionary, error);
            case MsgClasses.STATUS:
                return handleStatus(msg, error);
            case MsgClasses.ACK:
                return handleAck(msg);
            default:
                System.out.println("Received Unhandled Item Msg Class: " + msg.msgClass());
                break;
        }

        return CodecReturnCodes.SUCCESS;
    }

    protected int handleAck(Msg msg)
    {
        System.out.println("Received AckMsg for stream " + msg.streamId());

        StringBuilder fieldValue = new StringBuilder();
        getItemName(msg, fieldValue);

        AckMsg ackMsg = (AckMsg)msg;

        fieldValue.append("\tackId=" + ackMsg.ackId());
        if (ackMsg.checkHasSeqNum())
            fieldValue.append("\tseqNum=" + ackMsg.seqNum());
        if (ackMsg.checkHasNakCode())
            fieldValue.append("\tnakCode=" + ackMsg.nakCode());
        if (ackMsg.checkHasText())
            fieldValue.append("\ttext=" + ackMsg.text().toString());

        System.out.println(fieldValue.toString());
        return CodecReturnCodes.SUCCESS;
    }

    protected void getItemName(Msg msg, StringBuilder fieldValue)
    {
        // get key
        MsgKey key = msg.msgKey();

        // print out item name from key if it has it
        if (key != null && key.checkHasName())
        {
            if (msg.msgClass() == MsgClasses.REFRESH)
            {
                RefreshMsg refreshMsg = (RefreshMsg)msg;
                fieldValue.append(key.name().toString()
                        + (refreshMsg.checkPrivateStream() ? " (PRIVATE STREAM)" : "")
                        + "\nDOMAIN: " + DomainTypes.toString(msg.domainType()) + "\n");
            }
            else
            {
                fieldValue.append(key.name().toString() + "\nDOMAIN: "
                        + DomainTypes.toString(msg.domainType()) + "\n");
                if (msg.msgClass() == MsgClasses.UPDATE)
                {
                    fieldValue.append("UPDATE TYPE: "
                            + UpdateEventTypes.toString(((UpdateMsg)msg).updateType()) + "\n");
                }
            }

        }
        else
        // cached item name
        {
            WatchListEntry wle = watchList.get(msg.streamId());

            if (wle != null)
            {
                fieldValue.append(wle.itemName + (wle.isPrivateStream ? " (PRIVATE STREAM)" : " ")
                        + "\nDOMAIN: " + DomainTypes.toString(msg.domainType()) + "\n");
                if (msg.msgClass() == MsgClasses.UPDATE)
                {
                    fieldValue.append("UPDATE TYPE: "
                            + UpdateEventTypes.toString(((UpdateMsg)msg).updateType()) + "\n");
                }
            }
            else
            {
                // check if this is login stream for offstream posting
                if (msg.streamId() == LoginHandler.LOGIN_STREAM_ID)
                {
                    fieldValue.append("OFFPOST " + "\nDOMAIN: "
                            + DomainTypes.toString(msg.domainType()) + "\n");
                }
            }
        }
    }

    protected int handleStatus(Msg msg, com.thomsonreuters.upa.transport.Error error)
    {
        StatusMsg statusMsg = (StatusMsg)msg;
        System.out.println("Received Item StatusMsg for stream " + msg.streamId());
        if (!statusMsg.checkHasState())
            return CodecReturnCodes.SUCCESS;

        // get state information
        State state = statusMsg.state();
        System.out.println("	" + state);

        WatchListEntry wle = watchList.get(msg.streamId());
        if (wle != null)
        {
            /* update our state table with the new state */
            if (!statusMsg.checkPrivateStream()) /* non-private stream */
            {
                /*
                 * check if this response should be on private stream but is not
                 */
                if (!statusMsg.checkPrivateStream()) /* non-private stream */
                {
                    /*
                     * check if this response should be on private stream but is
                     * not batch responses for private stream may be sent on
                     * non-private stream
                     */
                    /* if this is the case, close the stream */
                    if (wle.isPrivateStream && !wle.itemName.contains("BATCH_"))
                    {
                        System.out.println("Received non-private response for stream "
                                + msg.streamId() + " that should be private - closing stream");
                        // close stream
                        closeStream(redirectChnl, msg.streamId(), error);
                        // remove private stream entry from list
                        removeMarketPriceItemEntry(msg.streamId());
                        return CodecReturnCodes.FAILURE;
                    }
                }
            }
            wle.itemState.dataState(statusMsg.state().dataState());
            wle.itemState.streamState(statusMsg.state().streamState());
        }

        // redirect to private stream if indicated
        if (statusMsg.state().streamState() == StreamStates.REDIRECTED
                && (statusMsg.checkPrivateStream()))
        {
            int ret = redirectToPrivateStream(msg.streamId(), error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }

        return CodecReturnCodes.SUCCESS;
    }

    protected int handleUpdate(ChannelSession chnl, Msg msg, DecodeIterator dIter,
            DataDictionary dictionary, Error error)
    {
        UpdateMsg updateMsg = (UpdateMsg)msg;
        PostUserInfo pu = updateMsg.postUserInfo();
        if (pu != null)
        {
            System.out.println(" Received UpdateMsg for stream " + updateMsg.streamId()
                    + " from publisher with user ID: " + pu.userId() + " at user address: "
                    + pu.userAddrToString(pu.userAddr()));
        }
        return decode(chnl, msg, dIter, dictionary, error);
    }

    protected int decode(ChannelSession chnl, Msg msg, DecodeIterator dIter,
            DataDictionary dictionary, Error error)
    {
        int ret = fieldList.decode(dIter, null);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("DecodeFieldList() failed with return code: " + ret);
            return ret;
        }

        StringBuilder fieldValue = new StringBuilder();
        headline = new Headline();
        getItemName(msg, fieldValue);
        if (msg.msgClass() == MsgClasses.REFRESH)
            fieldValue.append((((RefreshMsg)msg).state()).toString() + "\n");

        // decode each field entry in list
        while ((ret = fieldEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("decodeFieldEntry() failed with return code: " + ret);
                return ret;
            }
            ret = decodeFieldEntry(chnl, fieldEntry, dIter, dictionary, fieldValue, error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("decodeFieldEntry() failed");
                return ret;
            }
            fieldValue.append("\n");
        }
        if (headline.getLang() != null)
            newsViewer.addHeadline(headline);

        if (headline.getPnac() != null)
        {
            // Set channel session to current channel session
            _chnl = chnl;
        }
        System.out.println(fieldValue.toString());

        return CodecReturnCodes.SUCCESS;
    }

    // sends one item at a time to current channelsession
    public int sendPnacRequest(List<String> itemNames, Error error)
    {
        int ret = CodecReturnCodes.SUCCESS;

        for (String itemName : itemNames)
        {
            Integer streamId = watchList.add(domainType, itemName,
                                             marketPriceRequest.checkPrivateStream());

            marketPriceRequest.itemNames().clear();
            marketPriceRequest.itemNames().add(itemName);

            marketPriceRequest.streamId(streamId);
            ret = encodeAndSendRequest(_chnl, marketPriceRequest, error);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    protected int handleRefresh(ChannelSession chnl, Msg msg, DecodeIterator dIter,
            DataDictionary dictionary, Error error)
    {
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        PostUserInfo pu = refreshMsg.postUserInfo();
        if (pu != null)
        {
            System.out.println(" Received RefreshMsg for stream " + refreshMsg.streamId()
                    + " from publisher with user ID: " + pu.userId() + " at user address: "
                    + pu.userAddrToString(pu.userAddr()));
        }

        WatchListEntry wle = watchList.get(msg.streamId());

        /* check if this response should be on private stream but is not */
        /* if this is the case, close the stream */
        if (wle != null)
            if (!refreshMsg.checkPrivateStream() && wle.isPrivateStream)
            {
                System.out.println("Received non-private response for stream " + msg.streamId()
                        + " that should be private - closing stream");
                // close stream
                closeStream(redirectChnl, msg.streamId(), error);

                // remove private stream entry from list
                removeMarketPriceItemEntry(msg.streamId());
                error.text("Received non-private response for stream " + msg.streamId()
                        + " that should be private - closing stream");
                return CodecReturnCodes.FAILURE;
            }
        /*
         * update our item state list if its a refresh, then process just like
         * update
         */
        wle.itemState.dataState(refreshMsg.state().dataState());
        wle.itemState.streamState(refreshMsg.state().streamState());

        return this.decode(chnl, msg, dIter, dictionary, error);
    }

    /*
     * This is used by news item handler to output field lists.
     * 
     * Decodes the field entry data and prints out the field entry data with
     * help of the dictionary. Returns success if decoding succeeds or failure
     * if decoding fails. fEntry - The field entry data dIter - The decode
     * iterator
     */
    protected int decodeFieldEntry(ChannelSession chnl, FieldEntry fEntry, DecodeIterator dIter,
            DataDictionary dictionary, StringBuilder fieldValue, Error error)
    {
        // get dictionary entry
        DictionaryEntry dictionaryEntry = dictionary.entry(fEntry.fieldId());

        // return if no entry found
        if (dictionaryEntry == null)
        {
            fieldValue.append("\tFid " + fEntry.fieldId() + " not found in dictionary");
            return CodecReturnCodes.SUCCESS;
        }

        // print out fid name
        fieldValue.append("\t" + fEntry.fieldId() + "/" + dictionaryEntry.acronym().toString()
                + ": ");

        // decode and print out fid value
        int dataType = dictionaryEntry.rwfType();
        int ret = 0;
        
        switch (dataType)
        {
            case DataTypes.UINT:
                ret = fidUIntValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    fieldValue.append(fidUIntValue.toLong());
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeUInt() failed: <" + CodecReturnCodes.toString(ret)
                            + ">");
                    return ret;
                }
                break;
            case DataTypes.INT:
                ret = fidIntValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    fieldValue.append(fidIntValue.toLong());
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeInt() failed: <" + CodecReturnCodes.toString(ret)
                            + ">");

                    return ret;
                }
                break;
            case DataTypes.FLOAT:
                ret = fidFloatValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    fieldValue.append(fidFloatValue.toFloat());
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeFloat() failed: <" + CodecReturnCodes.toString(ret)
                            + ">");

                    return ret;
                }
                break;
            case DataTypes.DOUBLE:
                ret = fidDoubleValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    fieldValue.append(fidDoubleValue.toDouble());
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeDouble() failed: <" + CodecReturnCodes.toString(ret)
                            + ">");

                    return ret;
                }
                break;
            case DataTypes.REAL:
                ret = fidRealValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    fieldValue.append(fidRealValue.toString());
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeReal() failed: <" + CodecReturnCodes.toString(ret)
                            + ">");

                    return ret;
                }
                break;
            case DataTypes.ENUM:
                ret = fidEnumValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    EnumType enumType = dictionary.entryEnumType(dictionaryEntry, fidEnumValue);

                    if (enumType == null)
                    {
                        fieldValue.append(fidEnumValue.toInt());
                    }
                    else
                    {
                        fieldValue.append(enumType.display().toString() + "("
                                + fidEnumValue.toInt() + ")");
                    }
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeEnum() failed: <" + CodecReturnCodes.toString(ret)
                            + ">");

                    return ret;
                }
                break;
            case DataTypes.DATE:
                ret = fidDateValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    fieldValue.append(fidDateValue.toString());
                    if (fEntry.fieldId() == 1027)
                        headline.setStoryDate(fidDateValue.toString());
                    break;
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeDate() failed: <" + CodecReturnCodes.toString(ret)
                            + ">");

                    return ret;
                }
                break;
            case DataTypes.TIME:
                ret = fidTimeValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    fieldValue.append(fidTimeValue.toString());
                    if (fEntry.fieldId() == 1024)
                        headline.setStoryTime(fidTimeValue.toString());
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeTime() failed: <" + CodecReturnCodes.toString(ret)
                            + ">");

                    return ret;
                }
                break;
            case DataTypes.DATETIME:
                ret = fidDateTimeValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    fieldValue.append(fidDateTimeValue.toString());
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeDateTime() failed: <"
                            + CodecReturnCodes.toString(ret) + ">");
                    return ret;
                }
                break;
            case DataTypes.QOS:
                ret = fidQosValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    fieldValue.append(fidQosValue.toString());
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeQos() failed: <" + CodecReturnCodes.toString(ret)
                            + ">");

                    return ret;
                }
                break;
            case DataTypes.STATE:
                ret = fidStateValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    fieldValue.append(fidStateValue.toString());
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeState() failed: <" + CodecReturnCodes.toString(ret)
                            + ">");

                    return ret;
                }
                break;
            case DataTypes.ARRAY:
                break;
            case DataTypes.BUFFER:
            case DataTypes.ASCII_STRING:
            case DataTypes.UTF8_STRING:
            case DataTypes.RMTES_STRING:
                ret = fEntry.encodedData().decode(dIter);

                if (ret == CodecReturnCodes.SUCCESS)
                {
                    fEntryString = fEntry.encodedData().toString();
                    switch (fEntry.fieldId())
                    {
                        case 3: // DSPLY_NAME
                            headline.setOption(Byte.parseByte(fEntryString));
                            break;
                        case 235: // PNAC
                            headline.setPnac(fEntryString);
                            break;
                        case 238: // NEXT_LR in chain
                            if (fEntryString != null && fEntryString.length() != 0)
                            {
                                tempStringList.clear();
                                tempStringList.add(fEntryString);
                                sendPnacRequest(tempStringList, error);
                            }
                            break;
                        case 258: // SEG_TEXT that is the story
                            cacheBuf.clear();
                            if (decoder.RMTESApplyToCache(fEntry.encodedData(), cacheBuf) == CodecReturnCodes.SUCCESS)
                            {
                            	rmtesBuffer.clear();
                                if (decoder.RMTESToUCS2(rmtesBuffer, cacheBuf) == CodecReturnCodes.SUCCESS)
                                {
                                	byte[] array = new byte[rmtesBuffer.length()];
                                	rmtesBuffer.byteData().get(array, 0, rmtesBuffer.length());
                                	newsViewer.getStoryViewer().writeBody(new String(array, Charset.forName("UTF-16")));
                                }
                                else
                                    newsViewer.getStoryViewer().writeBody("RMTES Parse error");
                            }
                            else
                                newsViewer.getStoryViewer().writeBody("RMTES Parse error");
                            break;
                        case 264: // BCAST_TEXT
                            cacheBuf.clear();
                            if (decoder.RMTESApplyToCache(fEntry.encodedData(), cacheBuf) == CodecReturnCodes.SUCCESS)
                            {
                            	rmtesBuffer.clear();
                                if (decoder.RMTESToUCS2(rmtesBuffer, cacheBuf) == CodecReturnCodes.SUCCESS)
                                {
                                	byte[] array = new byte[rmtesBuffer.length()];
                                	rmtesBuffer.byteData().get(array, 0, rmtesBuffer.length());
                                    headline.setText(new String(array, Charset.forName("UTF-16")));
                                }
                                else
                                    headline.setText("RMTES Parse error");
                            }
                            else
                                headline.setText("RMTES Parse error");
                            break;
                        case 725: // Attribution
                            for (int i = 0; i < fEntryString.length(); ++i)
                            {
                                if (fEntryString.toCharArray()[i] != ' '
                                        && i < fEntryString.length() - 1)
                                    fieldString.append(fEntryString.toCharArray()[i]);
                                else
                                {
                                    headline.setAttribution(fieldString.toString());
                                    fieldString.setLength(0);
                                }

                            }
                            break;
                        case 749: // Prod Codes
                            for (int i = 0; i < fEntryString.length(); ++i)
                            {
                                if (fEntryString.toCharArray()[i] != ' '
                                        && i < fEntryString.length() - 1)
                                    fieldString.append(fEntryString.toCharArray()[i]);
                                else
                                {
                                    headline.setProdCodes(headline.getProdCodes() + " "
                                            + fieldString.toString());
                                    fieldString.setLength(0);
                                }

                            }
                            break;
                        case 750: // Topic Codes
                            for (int i = 0; i < fEntryString.length(); ++i)
                            {
                                if (fEntryString.toCharArray()[i] != ' '
                                        && i < fEntryString.length() - 1)
                                    fieldString.append(fEntryString.toCharArray()[i]);
                                else
                                {
                                    headline.setTopicCodes(headline.getTopicCodes() + " "
                                            + fieldString.toString());
                                    fieldString.setLength(0);
                                }

                            }
                            break;
                        case 751: // Company Codes
                            for (int i = 0; i < fEntryString.length(); ++i)
                            {
                                if (fEntryString.toCharArray()[i] != ' '
                                        && i < fEntryString.length() - 1)
                                    fieldString.append(fEntryString.toCharArray()[i]);
                                else
                                {
                                    headline.setCompanyCodes(headline.getCompanyCodes() + " "
                                            + fieldString.toString());
                                    fieldString.setLength(0);
                                }

                            }
                            break;
                        case 752: // LANG_IND
                            headline.setLang(fEntryString);
                            break;
                        default:
                            break;
                    }
                }
                fieldValue.append(fEntryString);

                break;
            default:
                fieldValue.append("Unsupported data type (" + DataTypes.toString(dataType) + ")");
                break;
        }

        if (ret == CodecReturnCodes.BLANK_DATA)
        {
            fieldValue.append("<blank data>");
        }

        return CodecReturnCodes.SUCCESS;
    }

    /**
     * Close all item streams.
     * 
     * @param chnl - The channel to send a item stream close to
     */
    public int closeStreams(ChannelSession chnl, Error error)
    {
        int ret = 0;

        Iterator<Map.Entry<StreamIdKey, WatchListEntry>> iter = watchList.iterator();
        while (iter.hasNext())
        {
            Map.Entry<StreamIdKey, WatchListEntry> entry = iter.next();
            /*
             * we only want to close a stream if it was not already closed (e.g.
             * rejected by provider, closed via refresh or status, or
             * redirected)
             */
            if (entry.getValue().itemState.isFinal())
                continue;
            if (entry.getValue().domainType == domainType)
            {
                ret = closeStream(chnl, entry.getKey().streamId(), error);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
                iter.remove();
            }
        }

        return CodecReturnCodes.SUCCESS;
    }

    /*
     * Redirect a request to a private stream. streamId - The stream id to be
     * redirected to private stream
     */
    private int redirectToPrivateStream(int streamId, com.thomsonreuters.upa.transport.Error error)
    {
        WatchListEntry wle = watchList.get(streamId);

        /* remove non-private stream entry from list */
        removeMarketPriceItemEntry(streamId);

        /* add item name to private stream list */
        Integer psStreamId = watchList.add(domainType, wle.itemName, true);

        generateRequest(marketPriceRequest, true, redirectSrcDirInfo, redirectLoginInfo);
        marketPriceRequest.itemNames().add(wle.itemName);
        marketPriceRequest.streamId(psStreamId);
        return encodeAndSendRequest(redirectChnl, marketPriceRequest, error);
    }
}
