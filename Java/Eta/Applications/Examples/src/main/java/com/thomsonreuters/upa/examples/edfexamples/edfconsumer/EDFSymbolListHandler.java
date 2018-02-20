package com.thomsonreuters.upa.examples.edfexamples.edfconsumer;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.ListIterator;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.ElementList;
import com.thomsonreuters.upa.codec.FieldEntry;
import com.thomsonreuters.upa.codec.FieldList;
import com.thomsonreuters.upa.codec.LocalElementSetDefDb;
import com.thomsonreuters.upa.codec.LocalFieldSetDefDb;
import com.thomsonreuters.upa.codec.Map;
import com.thomsonreuters.upa.codec.MapEntry;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.codec.Vector;
import com.thomsonreuters.upa.codec.VectorEntry;
import com.thomsonreuters.upa.examples.common.ChannelSession;
import com.thomsonreuters.upa.examples.common.SymbolListHandler;
import com.thomsonreuters.upa.examples.edfexamples.common.EDFWatchList;
import com.thomsonreuters.upa.shared.rdm.marketprice.MarketPriceClose;
import com.thomsonreuters.upa.shared.rdm.symbollist.SymbolListRequest;
import com.thomsonreuters.upa.rdm.ElementNames;
import com.thomsonreuters.upa.transport.TransportBuffer;

/**
 * This is the symbol list handler for the UPA EDF consumer application. It provides
 * methods for sending the symbol list request(s) to the Ref Data server and processing
 * the response(s). Method closing stream is also provided.
 */
public class EDFSymbolListHandler extends SymbolListHandler
{
    private UInt mapKey = CodecFactory.createUInt();
    private FieldList fieldList = CodecFactory.createFieldList();
    private FieldEntry fieldEntry = CodecFactory.createFieldEntry();
    private LocalFieldSetDefDb localFieldSetDefDb = CodecFactory.createLocalFieldSetDefDb();
    private Buffer tmpBuffer = CodecFactory.createBuffer();
    private String tmpName;
    private UInt tmpChannelId = CodecFactory.createUInt();
    private UInt tmpDomain = CodecFactory.createUInt();
    private Vector vector = CodecFactory.createVector();
    private VectorEntry vectorEntry = CodecFactory.createVectorEntry();
    private ElementList vectorElementList = CodecFactory.createElementList();
    private ElementEntry vectorElementEntry = CodecFactory.createElementEntry();
    private LocalElementSetDefDb vectorElementSetDefDb = CodecFactory.createLocalElementSetDefDb();
    private HashMap<String, SymbolListEntry> symbolList;

    private boolean snapshotRequested;
    
    public class SymbolListEntry
    {
        int Id;
        List<ChannelInfo> channels = new ArrayList<ChannelInfo>();
                
        SymbolListEntry()
        {
            Id = 0;
            channels.clear();
            
        }
        
        void clear()
        {
            Id = 0;
            channels.clear();
        }
        
        public int getId()
        {
            return Id;
        }

        public void setId(int id)
        {
            Id = id;
        }
        
        public void addRealTimeChannelId(int domain, int channelId)
        {
            ListIterator<ChannelInfo> iter = channels.listIterator();
            
            while(iter.hasNext())
            {
                ChannelInfo temp = iter.next();
                
                if(temp.getDomain() == domain)
                {
                    temp.setRealTimechannelId(channelId);
                    
                    return;
                }
            }
            
            ChannelInfo newInfo = new ChannelInfo();
            
            newInfo.setDomain(domain);
            newInfo.setRealTimechannelId(channelId);
            
            channels.add(newInfo);
            
            return;
        }
        
        public void addGapFillChannelId(int domain, int channelId)
        {
            ListIterator<ChannelInfo> iter = channels.listIterator();
            
            while(iter.hasNext())
            {
                ChannelInfo temp = iter.next();
                
                if(temp.getDomain() == domain)
                {
                    temp.setGapFillChannelId(channelId);
                    
                    return;
                }
            }
            
            ChannelInfo newInfo = new ChannelInfo();
            
            newInfo.setDomain(domain);
            newInfo.setGapFillChannelId(channelId);
            
            channels.add(newInfo);
            
            return;
        }

        public int getGapFillChannelId(int domain)
        {
            ListIterator<ChannelInfo> iter = channels.listIterator();
            
            while(iter.hasNext())
            {
                ChannelInfo temp = iter.next();
                
                if(temp.getDomain() == domain)
                    return temp.getGapFillChannelId();
            }
            
            return 0;
        }
        
        public int getRealTimeChannelId(int domain)
        {
            ListIterator<ChannelInfo> iter = channels.listIterator();
            
            while(iter.hasNext())
            {
                ChannelInfo temp = iter.next();
                
                if(temp.getDomain() == domain)
                    return temp.getRealTimechannelId();
            }
            
            return 0;
        }
    }
    
    class ChannelInfo
    {
        int realTimechannelId;
        int gapFillChannelId;
        int domain;
        
        ChannelInfo()
        {
            realTimechannelId = 0;
            gapFillChannelId = 0;
            domain = 0;
        }

        public int getRealTimechannelId()
        {
            return realTimechannelId;
        }

        public void setRealTimechannelId(int realTimechannelId)
        {
            this.realTimechannelId = realTimechannelId;
        }

        public int getGapFillChannelId()
        {
            return gapFillChannelId;
        }

        public void setGapFillChannelId(int gapFillChannelId)
        {
            this.gapFillChannelId = gapFillChannelId;
        }

        public int getDomain()
        {
            return domain;
        }

        public void setDomain(int domain)
        {
            this.domain = domain;
        }
    }

    public EDFSymbolListHandler()
    {
        state = CodecFactory.createState();
        qos = CodecFactory.createQos();
        capabilities = new ArrayList<Long>();
        symbolListName = CodecFactory.createBuffer();
        symbolListRequest = new SymbolListRequest();
        closeMessage = new MarketPriceClose();
        symbolList = new HashMap<String, SymbolListEntry>();
    }
    
    public EDFSymbolListHandler(EDFWatchList watchlist)
    {
        state = CodecFactory.createState();
        qos = CodecFactory.createQos();
        capabilities = new ArrayList<Long>();
        symbolListName = CodecFactory.createBuffer();
        symbolListRequest = new SymbolListRequest();
        closeMessage = new MarketPriceClose();
        symbolList = new HashMap<String, SymbolListEntry>();
    }

    public HashMap<String, SymbolListEntry> symbolList()
    {
        return symbolList;
    }
    
    public int sendRequest(ChannelSession chnl, com.thomsonreuters.upa.transport.Error error)
    {
        /* check to see if the provider supports the symbol list domain */
        if (!hasSymbolListCapability(capabilities()))
        {
            error.text("SYMBOL_LIST domain is not supported by the indicated provider");
            // Symbol list should always be supported by Reference Data Server,
            //so while the server may say it does not provide it, it does
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
        localFieldSetDefDb.clear();
        fieldList.clear();
        fieldEntry.clear();
        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
                RefreshMsg refreshMsg = (RefreshMsg)msg;
                state.dataState(refreshMsg.state().dataState());
                state.streamState(refreshMsg.state().streamState());
            case MsgClasses.UPDATE:
                return handleUpdate(msg, dIter, map, mapEntry, mapKey, localFieldSetDefDb, fieldList, fieldEntry);

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
            MapEntry mapEntry, UInt mapKey, LocalFieldSetDefDb localFieldSetDefDb,
            FieldList fieldList, FieldEntry fieldEntry)
    {
        int ret = map.decode(dIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("DecodeMap() failed: < " + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }
        
        if (map.checkHasSetDefs())
            if (map.containerType() == DataTypes.FIELD_LIST)
            {
                localFieldSetDefDb.clear();
                ret = localFieldSetDefDb.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    System.out.println("DecodeLocalElementSetDefDb() failed: < " + CodecReturnCodes.toString(ret) + ">");
                    return ret;
                }
            }
                
        // decode the map
        while ((ret = mapEntry.decode(dIter, mapKey)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("DecodeMapEntry() failed: < " + CodecReturnCodes.toString(ret) + ">");
                return ret;
            }

            // Decode the field list
        
            ret = fieldList.decode(dIter, localFieldSetDefDb);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("DecodeFieldList() failed: < " + CodecReturnCodes.toString(ret) + ">");
                return ret;
            }
            
            SymbolListEntry tmpEntry = new SymbolListEntry();
            
            while ((ret = fieldEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    /* Get snapshot server host information */
                    /* SnapshotServerHost */
                    
                    tmpEntry.setId((int)mapKey.toLong());
                    
                    if (fieldEntry.fieldId() == 3422)   // Prov Symbol
                    {
                        ret = tmpBuffer.decode(dIter);
                        if (ret != CodecReturnCodes.SUCCESS)
                        {
                            System.out.println("DecodeFieldEntry() failed: < " + CodecReturnCodes.toString(ret) + ">");
                            return ret;
                        }
                        tmpName = tmpBuffer.toString();
                    }
                    else if (fieldEntry.fieldId() == 32639) // Streaming Channel ID
                    {
                        vector.clear();
                        ret = vector.decode(dIter);
                        if (ret != CodecReturnCodes.SUCCESS)
                        {
                            System.out.println("DecodeVector() failed: < " + CodecReturnCodes.toString(ret) + ">");
                            return ret;
                        }
                        
                        if (vector.checkHasSetDefs())
                        {
                            if (vector.containerType() == DataTypes.ELEMENT_LIST)
                            {
                                vectorElementSetDefDb.clear();
                                ret = vectorElementSetDefDb.decode(dIter);
                                if (ret != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.println("DecodeLocalElementSetDefDb() failed: < " + CodecReturnCodes.toString(ret) + ">");
                                    return ret;
                                }
                            }
                        }
                        // Decode the vector
                        while ((ret = vectorEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                        {
                            if (ret == CodecReturnCodes.SUCCESS)
                            {
                                vectorElementList.clear();
                                ret = vectorElementList.decode(dIter, vectorElementSetDefDb);
                                if (ret != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.println("DecodeElementList() failed: < " + CodecReturnCodes.toString(ret) + ">");
                                    return ret;
                                }
                                while ((ret = vectorElementEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                                {
                                    if (ret == CodecReturnCodes.SUCCESS)
                                    {
                                        if (vectorElementEntry.name().equals(ElementNames.CHANNEL_ID))
                                        {
                                            ret = tmpChannelId.decode(dIter);
                                            if (ret != CodecReturnCodes.SUCCESS)
                                            {
                                                System.out.println("DecodeUInt() failed: < " + CodecReturnCodes.toString(ret) + ">");
                                                return ret;
                                            }
                                         }
                                        else if (vectorElementEntry.name().equals(ElementNames.DOMAIN))
                                        {
                                            ret = tmpDomain.decode(dIter);
                                            if (ret != CodecReturnCodes.SUCCESS)
                                            {
                                                System.out.println("DecodeUInt() failed: < " + CodecReturnCodes.toString(ret) + ">");
                                                return ret;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        System.out.println("DecodeElementEntry() failed: < " + CodecReturnCodes.toString(ret) + ">");
                                        return ret;
                                    }
                                }
                                tmpEntry.addRealTimeChannelId((int)tmpDomain.toLong(), (int)tmpChannelId.toLong());
                            }
                            else
                            {
                                System.out.println("DecodeVectorEntry() failed: < " + CodecReturnCodes.toString(ret) + ">");
                                return ret;
                            }
                        }
                    }
                    else if (fieldEntry.fieldId() == 32640) // Gap Channel ID
                    {
                        vector.clear();
                        ret = vector.decode(dIter);
                        if (ret != CodecReturnCodes.SUCCESS)
                        {
                            System.out.println("DecodeVector() failed: < " + CodecReturnCodes.toString(ret) + ">");
                            return ret;
                        }
                        
                        if (vector.checkHasSetDefs())
                        {
                            if (vector.containerType() == DataTypes.ELEMENT_LIST)
                            {
                                vectorElementSetDefDb.clear();
                                ret = vectorElementSetDefDb.decode(dIter);
                                if (ret != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.println("DecodeLocalElementSetDefDb() failed: < " + CodecReturnCodes.toString(ret) + ">");
                                    return ret;
                                }
                            }
                        }
                        // Decode the vector
                        while ((ret = vectorEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                        {
                            if (ret == CodecReturnCodes.SUCCESS)
                            {
                                vectorElementList.clear();
                                ret = vectorElementList.decode(dIter, vectorElementSetDefDb);
                                if (ret != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.println("DecodeElementList() failed: < " + CodecReturnCodes.toString(ret) + ">");
                                    return ret;
                                }
                                while ((ret = vectorElementEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                                {
                                    if (ret == CodecReturnCodes.SUCCESS)
                                    {
                                        if (vectorElementEntry.name().equals(ElementNames.CHANNEL_ID))
                                        {
                                            ret = tmpChannelId.decode(dIter);
                                            if (ret != CodecReturnCodes.SUCCESS)
                                            {
                                                System.out.println("DecodeUInt() failed: < " + CodecReturnCodes.toString(ret) + ">");
                                                return ret;
                                            }
                                        }
                                        else if (vectorElementEntry.name().equals(ElementNames.DOMAIN))
                                        {
                                            ret = tmpDomain.decode(dIter);
                                            if (ret != CodecReturnCodes.SUCCESS)
                                            {
                                                System.out.println("DecodeUInt() failed: < " + CodecReturnCodes.toString(ret) + ">");
                                                return ret;
                                            }

                                        }
                                    }
                                    else
                                    {
                                        System.out.println("DecodeElementEntry() failed: < " + CodecReturnCodes.toString(ret) + ">");
                                        return ret;
                                    }
                                }
                                
                                tmpEntry.addGapFillChannelId((int)tmpDomain.toLong(), (int)tmpChannelId.toLong());
                            }
                            else
                            {
                                System.out.println("DecodeVectorEntry() failed: < " + CodecReturnCodes.toString(ret) + ">");
                                return ret;
                            }
                        }
                    
                    }
                    symbolList.put(tmpName, tmpEntry);
                }
                else
                {
                    System.out.println("DecodeElementEntry() failed: < " + CodecReturnCodes.toString(ret) + ">");
                    return ret;
                }
            }
        }

        return CodecReturnCodes.SUCCESS;
    }

}
