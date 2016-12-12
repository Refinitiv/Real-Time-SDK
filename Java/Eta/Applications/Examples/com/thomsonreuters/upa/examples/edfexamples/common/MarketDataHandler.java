package com.thomsonreuters.upa.examples.edfexamples.common;

import java.util.List;

import com.thomsonreuters.upa.codec.AckMsg;
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
import com.thomsonreuters.upa.codec.GlobalFieldSetDefDb;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.LocalFieldSetDefDb;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.PostUserInfo;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.codec.UpdateMsg;
import com.thomsonreuters.upa.examples.common.ChannelSession;
import com.thomsonreuters.upa.examples.edfexamples.common.EDFWatchList.Item;
import com.thomsonreuters.upa.shared.rdm.marketprice.MarketPriceClose;
import com.thomsonreuters.upa.shared.rdm.marketprice.MarketPriceRequest;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;

abstract class MarketDataHandler
{

    public static int TRANSPORT_BUFFER_SIZE_REQUEST = ChannelSession.MAX_MSG_SIZE;
    public static int TRANSPORT_BUFFER_SIZE_CLOSE = ChannelSession.MAX_MSG_SIZE;
    protected int domainType;
    protected MarketPriceRequest marketPriceRequest;
    protected MarketPriceClose   closeMessage;
    protected FieldList fieldList = CodecFactory.createFieldList();
    protected FieldEntry fieldEntry = CodecFactory.createFieldEntry();
    protected UInt fidUIntValue = CodecFactory.createUInt();
    protected Int fidIntValue = CodecFactory.createInt();
    protected Real fidRealValue = CodecFactory.createReal();
    protected com.thomsonreuters.upa.codec.Enum fidEnumValue = CodecFactory.createEnum();
    protected com.thomsonreuters.upa.codec.Date fidDateValue = CodecFactory.createDate();
    protected Time fidTimeValue = CodecFactory.createTime();
    protected DateTime fidDateTimeValue = CodecFactory.createDateTime();
    protected com.thomsonreuters.upa.codec.Float fidFloatValue = CodecFactory.createFloat();
    protected com.thomsonreuters.upa.codec.Double fidDoubleValue = CodecFactory.createDouble();
    protected Qos fidQosValue = CodecFactory.createQos();
    protected State fidStateValue = CodecFactory.createState();
    protected EncodeIterator encIter = CodecFactory.createEncodeIterator();
    protected LocalFieldSetDefDb localSetDefDb = CodecFactory.createLocalFieldSetDefDb();
    
    protected GlobalFieldSetDefDb globalSetDefDb = null;
    
    public MarketDataHandler(int domainType)
    {
         this.domainType = domainType;
         marketPriceRequest = createMarketDataRequest();
         closeMessage = new MarketPriceClose();
    }
    
    abstract MarketPriceRequest createMarketDataRequest();
    
    private boolean hasMarketDataCapability(List<Long> capabilities)
    {
        for (Long capability : capabilities)
        {
            if (capability.equals((long)domainType))
                return true;
        }
        return false;
    }

    /**
     * Encodes and sends item requests for three market price domains
     * (MarketPrice, MarketByPrice, MarketByOrder).
     * 
     * @param chnl - The channel to send a source directory request to
     * 
     * @param item - Item name to send
     * 
     * @param serviceInfo - RDM directory response information
     * 
     * @return success if item requests can be made, can be encoded and sent
     *         successfully. Failure if service does not support market price capability
     *         or failure for encoding/sending request.
     */
    public int sendItemRequest(ChannelSession chnl, Item item, Service serviceInfo, Error error)
    {
        int ret = CodecReturnCodes.SUCCESS;
        /* check to see if the provider supports the market price domain */
        if (!hasMarketDataCapability(serviceInfo.info().capabilitiesList()))
        {
            error.text("'" +
                    DomainTypes.toString(marketPriceRequest.domainType()) +
                    "' not supported by the indicated provider");
            return CodecReturnCodes.FAILURE;
        }
        
        marketPriceRequest.clear();
    
        marketPriceRequest.applyHasServiceId();
        marketPriceRequest.serviceId(serviceInfo.serviceId());
        marketPriceRequest.applyHasPriority();
        marketPriceRequest.priority(1, 1);
        marketPriceRequest.applyHasQos();
        marketPriceRequest.qos().dynamic(false);
        marketPriceRequest.qos().timeInfo(serviceInfo.info().qosList().get(0).timeInfo());
        marketPriceRequest.qos().timeliness(serviceInfo.info().qosList().get(0)
                .timeliness());
        marketPriceRequest.qos().rateInfo(serviceInfo.info().qosList().get(0).rateInfo());
        marketPriceRequest.qos().rate(serviceInfo.info().qosList().get(0).rate());
    
        marketPriceRequest.identifier(item.getRealTimeStreamId());
        marketPriceRequest.streamId(item.getSnapshotServerStreamId());
        ret = encodeAndSendRequest(chnl, marketPriceRequest, error);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;
    
        return CodecReturnCodes.SUCCESS;
    }

    private int encodeAndSendRequest(ChannelSession chnl, MarketPriceRequest marketPriceRequest, Error error)
    {
        //get a buffer for the item request
        TransportBuffer msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, error);
    
        if (msgBuf == null)
        {
            return CodecReturnCodes.FAILURE;
        }
    
        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());
    
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
     * Publicly visible market price response handler
     * 
     * Processes a market price response. This consists of extracting the key,
     * printing out the item name contained in the key, decoding the field list
     * and field entry, and calling decodeFieldEntry() to decode the field entry
     * data.
     * 
     * @param msg - The partially decoded message
     * @param dIter - The decode iterator
     * @param item - The item we are working on
     * @param dictionary - Data dictionary
     * 
     * @return success if decoding succeeds, failure if it fails.
     */
    public int processResponse(Msg msg, DecodeIterator dIter, Item item, DataDictionary dictionary, Error error)
    {
        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
                return handleRefresh(msg, dIter, item, dictionary, error);
            case MsgClasses.UPDATE:
                return handleUpdate(msg, dIter, item, dictionary);
            case MsgClasses.STATUS:
                return handleStatus(msg, item, error);
            case MsgClasses.ACK:
                return handleAck(msg, item);
            default:
                System.out.println("Received Unhandled Item Msg Class: " + msg.msgClass());
                break;
        }
    
        return CodecReturnCodes.SUCCESS;
    }

    protected int handleAck(Msg msg, Item item)
    {
        System.out.println("Received AckMsg for stream " + msg.streamId());
    
        StringBuilder fieldValue = new StringBuilder();
        fieldValue.append(item.getName().toString());
    
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

    protected int handleStatus(Msg msg, Item item, Error error)
    {
        StatusMsg statusMsg = (StatusMsg)msg;
        System.out.println("Received Item StatusMsg for stream " + msg.streamId());
        if (!statusMsg.checkHasState())
            return CodecReturnCodes.SUCCESS;
    
        // get state information
        State state = statusMsg.state();
        System.out.println("	" + state);
    
        return CodecReturnCodes.SUCCESS;
    }

    protected int handleUpdate(Msg msg, DecodeIterator dIter, Item item, DataDictionary dictionary)
    {
        UpdateMsg updateMsg = (UpdateMsg)msg;
    
        System.out.println(" Received UpdateMsg for stream " + updateMsg.streamId());
        System.out.println("UPDATE TYPE: " + updateMsg.updateType());
        
        if(updateMsg.checkHasSeqNum() == true)
        {
            System.out.println("SEQ. NO.: " + updateMsg.seqNum());
        }
        
        System.out.println("DOMAIN: " + DomainTypes.toString(updateMsg.domainType()));
    
        return decode(msg, dIter, item, dictionary);
    }

    protected int handleRefresh(Msg msg, DecodeIterator dIter, Item item, DataDictionary dictionary, Error error)
    {
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        PostUserInfo pu = refreshMsg.postUserInfo();
        if ( pu != null)
        {
        	System.out.println(" Received RefreshMsg for stream " + refreshMsg.streamId());
        	
        	if(refreshMsg.checkHasSeqNum() == true)
        	{
        	    System.out.println("SEQ. NO.: " + refreshMsg.seqNum());
        	}
        }
        
        System.out.println("DOMAIN: " + DomainTypes.toString(refreshMsg.domainType()));
        
        System.out.println("STREAM STATE: " + refreshMsg.state().toString());
                
        return this.decode(msg, dIter, item, dictionary);
    }
    
    abstract int decode(Msg msg, DecodeIterator dIter, Item item, DataDictionary dictionary);
    
    protected int decodeFieldList(DecodeIterator dIter, DataDictionary dictionary, StringBuilder outputString)
    {
        //decode field list
        int ret = fieldList.decode(dIter, localSetDefDb);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("DecodeFieldList() failed: <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        //decode each field entry in list
        while ((ret = fieldEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("DecodeFieldEntry() failed: <" + CodecReturnCodes.toString(ret) + ">");
                return ret;
            }

            ret = decodeFieldEntry(fieldEntry, dIter, dictionary, outputString);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("decodeFieldEntry() failed");
                return ret;
            }
            outputString.append("\n");
        }

        return CodecReturnCodes.SUCCESS;
    }

    protected int decodeFieldEntry(FieldEntry fEntry, DecodeIterator dIter, DataDictionary dictionary, StringBuilder outputString)
    {
        // get dictionary entry
        DictionaryEntry dictionaryEntry = dictionary.entry(fEntry.fieldId());
    
        // return if no entry found
        if (dictionaryEntry == null)
        {
            outputString.append("\tFid " + fEntry.fieldId() + " not found in dictionary");
            return CodecReturnCodes.SUCCESS;
        }
    
        // print out fid name
        outputString.append("\t" + fEntry.fieldId() + "/" + dictionaryEntry.acronym().toString() + ": ");
    
        // decode and print out fid value
        int dataType = dictionaryEntry.rwfType();
        int ret = 0;
        switch (dataType)
        {
            case DataTypes.UINT:
                ret = fidUIntValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    outputString.append(fidUIntValue.toLong());
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeUInt() failed: <" + CodecReturnCodes.toString(ret) + ">");
                    return ret;
                }
                break;
            case DataTypes.INT:
                ret = fidIntValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    outputString.append(fidIntValue.toLong());
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeInt() failed: <" + CodecReturnCodes.toString(ret) + ">");
    
                    return ret;
                }
                break;
            case DataTypes.FLOAT:
                ret = fidFloatValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    outputString.append(fidFloatValue.toFloat());
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeFloat() failed: <" + CodecReturnCodes.toString(ret) + ">");
    
                    return ret;
                }
                break;
            case DataTypes.DOUBLE:
                ret = fidDoubleValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    outputString.append(fidDoubleValue.toDouble());
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeDouble() failed: <" + CodecReturnCodes.toString(ret) + ">");
    
                    return ret;
                }
                break;
            case DataTypes.REAL:
                ret = fidRealValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    outputString.append(fidRealValue.toString());
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeReal() failed: <" + CodecReturnCodes.toString(ret) + ">");
    
                    return ret;
                }
                break;
            case DataTypes.ENUM:
                ret = fidEnumValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    EnumType enumType = dictionary.entryEnumType(dictionaryEntry,
                                                                 fidEnumValue);
    
                    if (enumType == null)
                    {
                        outputString.append(fidEnumValue.toInt());
                    }
                    else
                    {
                        outputString.append(enumType.display().toString() + "(" +
                                fidEnumValue.toInt() + ")");
                    }
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeEnum() failed: <" + CodecReturnCodes.toString(ret) + ">");
    
                    return ret;
                }
                break;
            case DataTypes.DATE:
                ret = fidDateValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    outputString.append(fidDateValue.toString());
    
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeDate() failed: <" +
                            CodecReturnCodes.toString(ret) + ">");
    
                    return ret;
                }
                break;
            case DataTypes.TIME:
                ret = fidTimeValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    outputString.append(fidTimeValue.toString());
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeTime() failed: <" +
                            CodecReturnCodes.toString(ret) + ">");
    
                    return ret;
                }
                break;
            case DataTypes.DATETIME:
                ret = fidDateTimeValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    outputString.append(fidDateTimeValue.toString());
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeDateTime() failed: <" + CodecReturnCodes.toString(ret) + ">");
                    return ret;
                }
                break;
            case DataTypes.QOS:
                ret = fidQosValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    outputString.append(fidQosValue.toString());
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeQos() failed: <" + CodecReturnCodes.toString(ret) + ">");
    
                    return ret;
                }
                break;
            case DataTypes.STATE:
                ret = fidStateValue.decode(dIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    outputString.append(fidStateValue.toString());
                }
                else if (ret != CodecReturnCodes.BLANK_DATA)
                {
                    System.out.println("DecodeState() failed: <" + CodecReturnCodes.toString(ret) + ">");
    
                    return ret;
                }
                break;
            // For an example of array decoding, see
            // FieldListCodec.exampleDecode()
            case DataTypes.ARRAY:
                break;
            case DataTypes.BUFFER:
            case DataTypes.ASCII_STRING:
            case DataTypes.UTF8_STRING:
            case DataTypes.RMTES_STRING:
                if (fEntry.encodedData().length() > 0)
                {
                    outputString.append(fEntry.encodedData().toString());
                }
                else
                {
                    ret = CodecReturnCodes.BLANK_DATA;
                }
                break;
            default:
                outputString.append("Unsupported data type (" + DataTypes.toString(dataType) + ")");
                break;
        }
        if (ret == CodecReturnCodes.BLANK_DATA)
        {
            outputString.append("<blank data>");
        }
    
        return CodecReturnCodes.SUCCESS;
    }
    
    public void setGlobalSetDefDb(GlobalFieldSetDefDb setFieldSetDefDb)
    {
        globalSetDefDb = setFieldSetDefDb;
    }

}
