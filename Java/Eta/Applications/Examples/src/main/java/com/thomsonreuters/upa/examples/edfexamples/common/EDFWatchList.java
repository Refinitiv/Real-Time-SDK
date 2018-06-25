package com.thomsonreuters.upa.examples.edfexamples.common;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.CopyMsgFlags;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.GlobalFieldSetDefDb;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.UpdateMsg;
import com.thomsonreuters.upa.examples.common.ChannelSession;
import com.thomsonreuters.upa.examples.edfexamples.edfconsumer.GapFillSession;
import com.thomsonreuters.upa.examples.edfexamples.edfconsumer.RealTimeSession;
import com.thomsonreuters.upa.examples.edfexamples.edfconsumer.SnapshotSession;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;

/** Watchlist used by the EDFConsumer application. */
public class EDFWatchList
{
    private List<Item> watchlist = new ArrayList<Item>();
    private Service serviceInfo;
    
    private MarketPriceHandler marketPriceHandler = new MarketPriceHandler();
    private MarketByOrderHandler marketByOrderHandler = new MarketByOrderHandler();
    private MarketByPriceHandler marketByPriceHandler = new MarketByPriceHandler();
    
    private DataDictionary dictionary;
    
    public EDFWatchList()
    {
        watchlist.clear();
    }
    
    public void dictionary(DataDictionary setDictionary, GlobalFieldSetDefDb globalSetDefDb)
    {
        dictionary = setDictionary;
        marketByOrderHandler.setGlobalSetDefDb(globalSetDefDb);
        marketByPriceHandler.setGlobalSetDefDb(globalSetDefDb);
        marketPriceHandler.setGlobalSetDefDb(globalSetDefDb);
    }
    
    public int count()
    {
        return watchlist.size();
    }
    
    public Item getItem(int realTimeStreamId, int domainType)
    {
        ListIterator<Item> iter = watchlist.listIterator();
        
        while(iter.hasNext() == true)
        {
           Item tmp = iter.next();
           
           if(tmp.getRealTimeStreamId() == realTimeStreamId && tmp.getDomainType() == domainType)
           {
               return tmp;
           }
               
        }          
        
        return null;
    }
    
    public void setServiceInfo(Service serviceInfo)
    {
        this.serviceInfo = serviceInfo;
    }
    
    public Item getItemSnapshotId(int streamId)
    {
        ListIterator<Item> iter = watchlist.listIterator();
        
        while(iter.hasNext() == true)
        {
           Item tmp = iter.next();
           
           if(tmp.snapshotServerStreamId == streamId)
           {
               return tmp;
           }
               
        }
        
        return null;
    }
    
    public Item getItem(int streamId)
    {
        ListIterator<Item> iter = watchlist.listIterator();
        
        while(iter.hasNext() == true)
        {
           Item tmp = iter.next();
           
           if(tmp.realTimeStreamId == streamId)
           {
               return tmp;
           }
               
        }
        
        return null;
    }
    
    
    public Item getItem(Item item, Buffer symbolName, int domainType)
    {
        ListIterator<Item> iter = watchlist.listIterator();
        
        while(iter.hasNext() == true)
        {
           Item tmp = iter.next();
           
           if(tmp.name.equals(symbolName) == true && tmp.getDomainType() == domainType)
           {
               return tmp;
           }
               
        }
        
        return null;
    }
    
    public boolean containsItem(Buffer symbolName)
    {
        ListIterator<Item> iter = watchlist.listIterator();
        while (iter.hasNext() == true)
        {
            Item tmp = iter.next();
            if (tmp.name.equals(symbolName))
                return true;
        }
        return false;
    }
    
    
    public int processMsg(Msg msg, DecodeIterator dIter, SnapshotSession snapSession)
    {
        Item foundItem = null;
        Msg queueMsg;
        long msgSeqNum;
        
        StringBuilder outputString = new StringBuilder();

        if((foundItem = getItemSnapshotId(msg.streamId())) != null)
        {
            
            msgSeqNum = foundItem.msgGetSeqNum(msg);
            
           outputString.append("SEQ. NO: " + msgSeqNum + "\n");

            if(foundItem.getSequenceState() == ItemSequenceState.REQUEST_SENT)
            {
                /* Pop off messages up until the current sequence number */
                while(foundItem.msgQueuePop(MsgQueuePopCommand.UP_TO, msgSeqNum) != null)
                {
                    continue;
                }
                
                foundItem.sequenceState = ItemSequenceState.HAVE_SNAPSHOT;
                foundItem.setSeqNum(msgSeqNum);
            }
            else
            {
                if(foundItem.hasBufferedSnapshotMsgs() == true)
                {
                    foundItem.msgQueueAdd(msg);
                    return CodecReturnCodes.SUCCESS;
                }
                
                /* We're not buffering refreshes, so replay all messages up to this point */
                while((queueMsg = foundItem.msgQueuePop(MsgQueuePopCommand.UP_TO, msgSeqNum)) != null)
                {
                    
                    foundItem.setSeqNum(foundItem.msgGetSeqNum(queueMsg));
                    if(decodeItemMsgData(foundItem, dIter, queueMsg, outputString) != CodecReturnCodes.SUCCESS)
                    {
                        return CodecReturnCodes.FAILURE;
                    }
                }
                
                if(foundItem.getSequenceState() == ItemSequenceState.HAVE_SNAPSHOT && 
                    foundItem.seqNumCompare(msgSeqNum, foundItem.getSeqNum()) > 0)
                {
                    foundItem.setSeqNum(foundItem.getNextSeqNum(foundItem.getSeqNum()));
                    foundItem.setSequenceState(ItemSequenceState.REALTIME_STARTED);
                }
                
                if(foundItem.getSequenceState() == ItemSequenceState.REALTIME_STARTED &&
                    foundItem.seqNumCompare(foundItem.getSeqNum(), foundItem.getNextSeqNum(msgSeqNum)) < 0)
                {
                    foundItem.setHasBufferedSnapshotMsgs(true);
                    foundItem.msgQueueAdd(msg);
                    return CodecReturnCodes.SUCCESS;
                }
            }
            
            if(decodeItemMsgData(foundItem, dIter, msg, outputString) != CodecReturnCodes.SUCCESS)
            {
                return CodecReturnCodes.FAILURE;
            }
            
            if(((RefreshMsg)msg).checkRefreshComplete() == true)
            {                
                while((queueMsg = foundItem.msgQueuePop(MsgQueuePopCommand.ALL, foundItem.msgGetSeqNum(msg))) != null)
                {
                    foundItem.setSeqNum(foundItem.msgGetSeqNum(queueMsg));
                    
                    outputString.append("  (Replaying buffered message)" + "\n");
                   
                    if(decodeItemMsgData(foundItem, dIter, queueMsg, outputString) != CodecReturnCodes.SUCCESS)
                    {
                        return CodecReturnCodes.FAILURE;
                    }
                    
                    if(foundItem.seqNumCompare(foundItem.getSeqNum(), foundItem.msgGetSeqNum(msg)) >= 0)
                    {
                        foundItem.setSequenceState(ItemSequenceState.DONE_REORDERING);
                    }
                }
                foundItem.setHasRefreshComplete(true);
            }
            
            return CodecReturnCodes.SUCCESS;
        }
        else
        {
            System.out.println("Unknown stream Idreceived from snapshot session");
            return CodecReturnCodes.FAILURE;
        }

    }
    
    public int processMsg(ChannelSession chnl, Msg msg, DecodeIterator dIter, RealTimeSession realTimeSession, StringBuilder outputString)
    {
        Item foundItem = null;
        Msg queueMsg;
        long msgSeqNum;
        
        if((foundItem = getItem(msg.streamId())) != null)
        {
            ((EDFChannelSession)chnl).channelInfo().gapInfo().address = serviceInfo.seqMcastInfo().streamingMCastChanServerList().get(foundItem.getGapChannelId());
            ((EDFChannelSession)chnl).channelInfo().gapInfo().port = serviceInfo.seqMcastInfo().streamingMCastChanPortList().get(foundItem.getGapChannelId());
            msgSeqNum = foundItem.msgGetSeqNum(msg);
            outputString.append("Item: " + foundItem.getName().toString() + "  StreamId: " + foundItem.getRealTimeStreamId() + " Seq: " + msgSeqNum + " ");
            switch(foundItem.getSequenceState())
            {
                case ItemSequenceState.REQUEST_SENT:
                {
                    foundItem.msgQueueAdd(msg);
                    return CodecReturnCodes.SUCCESS;
                }
                case ItemSequenceState.HAVE_SNAPSHOT:
                {
                    if(foundItem.seqNumCompare(foundItem.msgGetSeqNum(msg), foundItem.getSeqNum()) <= 0)
                    {
                        /* Discard this message, because we've already gotten a refresh containing this message's data */
                        return CodecReturnCodes.SUCCESS;
                    }
                    
                    if(foundItem.isHasRefreshComplete())
                    {
                        /* Realtime data has caught up to the snapshot server.  Process this update, and we're now done reordering the streams */
                        foundItem.setSequenceState(ItemSequenceState.DONE_REORDERING);
                        return decodeItemMsgData(foundItem, dIter, msg, outputString);
                    }
                    else
                    {
                        /* We have not received the final part of a multi-part refresh, so queue this message */
                        foundItem.msgQueueAdd(msg);
                        return CodecReturnCodes.SUCCESS;
                    }
                }
                case ItemSequenceState.REALTIME_STARTED:
                {
                    if(foundItem.seqNumCompare(foundItem.msgGetSeqNum(msg), foundItem.getSeqNum()) <= 0)
                    {
                        /* Discard this message, because we've already gotten a refresh containing this message's data */
                        return CodecReturnCodes.SUCCESS;
                    }
                    
                    if(foundItem.hasBufferedSnapshotMsgs() == true)
                    {
                        while((queueMsg = foundItem.msgQueuePop(MsgQueuePopCommand.BEFORE, msgSeqNum)) != null)
                        {
                            if(queueMsg.msgClass() == MsgClasses.REFRESH && ((RefreshMsg)queueMsg).checkRefreshComplete() == true)
                            {
                                foundItem.setHasRefreshComplete(true);
                            }
                            
                            outputString.append("  (Replaying buffered message)" + "\n");
                            
                            if(decodeItemMsgData(foundItem, dIter, queueMsg, outputString) != CodecReturnCodes.SUCCESS)
                            {
                                return CodecReturnCodes.FAILURE;
                            }
                        }
                        
                        if(foundItem.hasQueuedMsgs() == true)
                        {
                            if(decodeItemMsgData(foundItem, dIter, msg, outputString) != CodecReturnCodes.SUCCESS)
                            {
                                return CodecReturnCodes.FAILURE;
                            }
                            
                            while((queueMsg = foundItem.msgQueuePop(MsgQueuePopCommand.BEFORE, msgSeqNum)) != null)
                            {
                                if(queueMsg.msgClass() == MsgClasses.REFRESH && ((RefreshMsg)queueMsg).checkRefreshComplete() == true)
                                {
                                    foundItem.setHasRefreshComplete(true);
                                }
                                
                                outputString.append("  (Replaying buffered message)" + "\n");
                                
                                if(decodeItemMsgData(foundItem, dIter, queueMsg, outputString) != CodecReturnCodes.SUCCESS)
                                {
                                    return CodecReturnCodes.FAILURE;
                                }
                            }
                            
                            if(foundItem.hasQueuedMsgs() == false)
                            {
                                foundItem.setHasBufferedSnapshotMsgs(false);
                            }
                            
                            if(foundItem.isHasRefreshComplete() == true)
                            {
                                foundItem.setSequenceState(ItemSequenceState.DONE_REORDERING);
                                
                                while((queueMsg = foundItem.msgQueuePop(MsgQueuePopCommand.ALL, msgSeqNum)) != null)
                                {
                                    outputString.append("  (Replaying buffered message)" + "\n");
                                    
                                    if(decodeItemMsgData(foundItem, dIter, queueMsg, outputString) != CodecReturnCodes.SUCCESS)
                                    {
                                        return CodecReturnCodes.FAILURE;
                                    }
                                }
                            }
                            
                        }
                        
                    }
                    break;
                }
                case ItemSequenceState.DONE_REORDERING:
                  decodeItemMsgData(foundItem, dIter, msg, outputString);
                  break;
                case ItemSequenceState.UNINITIALIZED:
                   return CodecReturnCodes.SUCCESS;
                default:
                	break;
            }
            
            return CodecReturnCodes.SUCCESS;
        }
        else
        {
            /* Item is not in our watchlist, so just return success */
            return CodecReturnCodes.SUCCESS;
        }
    }
    
    public int processMsg(Msg msg, DecodeIterator dIter, GapFillSession gapSession, StringBuilder outputString)
    {
        Item foundItem;
        
        if((foundItem = getItem(msg.streamId())) != null)
        {   
            if(decodeItemMsgData(foundItem, dIter, msg, outputString) != CodecReturnCodes.SUCCESS)
            {
                return CodecReturnCodes.FAILURE;
            }
        }
        
        return CodecReturnCodes.SUCCESS;
    }
    
    public int addItem(int realTimeStreamId, Buffer symbolName, int domainType, int serviceId, int gapFillFeedId, int realTimeFeedId)
    {
        ListIterator<Item> iter = watchlist.listIterator();
        while(iter.hasNext() == true)
        {
            Item temp = iter.next();
            if(temp.realTimeStreamId > realTimeStreamId)
            {
                break;
            }
        }
        
        Item newItem = new Item(symbolName, realTimeStreamId, domainType, serviceId);
        newItem.setSnapshotServerStreamId(realTimeFeedId);
        iter.add(newItem);
            
        return CodecReturnCodes.SUCCESS;
    }
    
    public int addItem(Item item)
    {
        ListIterator<Item> iter = watchlist.listIterator();
        while(iter.hasNext() == true)
        {
            Item temp = iter.next();
            if(temp.realTimeStreamId > item.getRealTimeStreamId())
            {
                break;
            }
        }
        
        Item newItem = new Item(item);
        
        iter.add(newItem);
            
        return CodecReturnCodes.SUCCESS;
    }
    
    public int sendRequests(ChannelSession chnl, Error error)
    {
        ListIterator<Item> iter = watchlist.listIterator();
        int ret;
        
        while(iter.hasNext() == true)
        {
            Item temp = iter.next();
            
            if(temp.getSequenceState() == ItemSequenceState.UNINITIALIZED)
            {
                switch(temp.getDomainType())
                {
                    case DomainTypes.MARKET_PRICE:
                        ret = marketPriceHandler.sendItemRequest(chnl, temp, serviceInfo, error);
                        
                        if(ret != CodecReturnCodes.SUCCESS)
                        {  
                            return ret;
                        }
                        break;
                    case DomainTypes.MARKET_BY_PRICE:
                        ret = marketByPriceHandler.sendItemRequest(chnl, temp, serviceInfo, error);
                        
                        if(ret != CodecReturnCodes.SUCCESS)
                        {  
                            return ret;
                        }
                        break;
                    case DomainTypes.MARKET_BY_ORDER:
                        ret = marketByOrderHandler.sendItemRequest(chnl, temp, serviceInfo, error);
                        
                        if(ret != CodecReturnCodes.SUCCESS)
                        {  
                            return ret;
                        }
                        break;
                    default:
                    	break;
                }
                temp.sequenceState = ItemSequenceState.REQUEST_SENT;
                iter.set(temp);
            }
        }
        
        
        return CodecReturnCodes.SUCCESS;
    }
    
    private int decodeItemMsgData(Item item, DecodeIterator dIter,  Msg msg, StringBuilder outputString)
    {
        dIter.clear();
        dIter.setBufferAndRWFVersion(msg.encodedDataBody(), Codec.majorVersion(), Codec.minorVersion());
        
        System.out.println(outputString);
        switch(msg.domainType())
        {
            case DomainTypes.MARKET_PRICE:
                return marketPriceHandler.decode(msg, dIter, item, dictionary);
            case DomainTypes.MARKET_BY_PRICE:
                return marketByPriceHandler.decode(msg, dIter, item, dictionary);
            case DomainTypes.MARKET_BY_ORDER:
                return marketByOrderHandler.decode(msg, dIter, item, dictionary);
            default:
                /* Print error */
                return CodecReturnCodes.FAILURE;
        }
    }

    
    public class ItemSequenceState
    {
        static final int UNINITIALIZED = 0;
        static final int REQUEST_SENT = 1;     /* No snapshot server messages received. Any
                                                * realtime messages have been buffered. */
        static final int HAVE_SNAPSHOT = 2;    /* We have received our first snapshot message. */
        static final int REALTIME_STARTED = 3; /* We have determined the starting point for
                                                * sequencing realtime messages. */
        static final int DONE_REORDERING = 4;  /* Image from snapshot server has been
                                                * replayed in order with realtime updates; no
                                                * more reordering is needed. */
    }
    
    private class MsgQueuePopCommand
    {
        static final int BEFORE = 0;
        static final int UP_TO = 1;
        static final int ALL = 2;
    }
    
    public class Item
    {
        private int domainType;
        private int snapshotServerStreamId;
        private int realTimeStreamId;
        private boolean hasRefreshComplete;
        private int sequenceState;
        private long seqNum;
        private boolean bufferedSnapshotMsgs;
        private Buffer name;
        private int realTimeChannelId;
        private int gapChannelId;
        private int serviceId;
        
        
        
        private List<Msg> msgQueue = new ArrayList<Msg>();
        
        public Item()
        {
            domainType = 0;
            snapshotServerStreamId = 0;
            realTimeStreamId = 0;
            hasRefreshComplete = false;
            sequenceState = ItemSequenceState.UNINITIALIZED;
            seqNum = 0;
            bufferedSnapshotMsgs   = false;
            name = null;
            realTimeChannelId = 0;
            gapChannelId = 0;
            
            msgQueue.clear();
        }
        
        Item(Buffer symbolName, int feedStreamId, int domainType, int serviceId)
        {
            this.domainType = domainType;               // Domain type from symbol list
            snapshotServerStreamId = 0;                     // Channel Id from symbol list
            this.realTimeStreamId = feedStreamId;   // Key from symbol list
            this.serviceId = serviceId;                             // Unused
            hasRefreshComplete = false;
            sequenceState = ItemSequenceState.UNINITIALIZED;
            seqNum = 0;
            bufferedSnapshotMsgs = false;
            name = CodecFactory.createBuffer();
            name.data(ByteBuffer.allocate(symbolName.length()));
            symbolName.copy(name);
            realTimeChannelId = 0;
            gapChannelId = 0;
            msgQueue.clear();
        }
        
        Item(Item item)
        {
            this.domainType = item.getDomainType();
            this.snapshotServerStreamId = item.getSnapshotServerStreamId();
            this.realTimeStreamId = item.getRealTimeStreamId();
            this.serviceId = item.getServiceId();
            this.hasRefreshComplete = false;
            this.sequenceState = ItemSequenceState.UNINITIALIZED;
            this.seqNum = 0;
            this.bufferedSnapshotMsgs = false;
            this.name = CodecFactory.createBuffer();
            this.name.data(ByteBuffer.allocate(item.getName().length()));
            item.getName().copy(this.name);
            this.realTimeChannelId = item.getRealTimeChannelId();
            this.gapChannelId = item.getGapChannelId();
            this.msgQueue.clear();
        }
        
        public void msgQueueAdd(Msg msg)
        {
            Msg newMsg = CodecFactory.createMsg();
            msg.copy(newMsg, CopyMsgFlags.ALL_FLAGS);
                        
            msgQueue.add(newMsg);
        }
        
        public Msg msgQueuePop(int command, long seqNum)
        {
            if(msgQueue.size() > 0)
            {
                Msg tmp = msgQueue.get(0);
                switch(command)
                {
                    case(MsgQueuePopCommand.BEFORE):
                        if(msgGetSeqNum(tmp) - seqNum >= 0)
                            return msgQueue.remove(0);
                        break;
                    case(MsgQueuePopCommand.UP_TO):
                        if(msgGetSeqNum(tmp) - seqNum > 0)
                            return msgQueue.remove(0);
                        break;
                    case(MsgQueuePopCommand.ALL):
                        return msgQueue.remove(0);
                    default:
                    	break;
                }
            }

            return null;
        }

        public int getDomainType()
        {
            return domainType;
        }

        public int getSnapshotServerStreamId()
        {
            return snapshotServerStreamId;
        }

        public void setSnapshotServerStreamId(int snapshotServerStreamId)
        {
            this.snapshotServerStreamId = snapshotServerStreamId;
        }

        public int getRealTimeStreamId()
        {
            return realTimeStreamId;
        }

        public boolean isHasRefreshComplete()
        {
            return hasRefreshComplete;
        }

        public void setHasRefreshComplete(boolean hasRefreshComplete)
        {
            this.hasRefreshComplete = hasRefreshComplete;
        }

        public int getSequenceState()
        {
            return sequenceState;
        }

        public void setSequenceState(int sequenceState)
        {
            this.sequenceState = sequenceState;
        }

        public long getSeqNum()
        {
            return seqNum;
        }

        public void setSeqNum(long seqNum)
        {
            this.seqNum = seqNum;
        }

        public boolean hasBufferedSnapshotMsgs()
        {
            return bufferedSnapshotMsgs;
        }

        public void setHasBufferedSnapshotMsgs(boolean bufferedSnapshotMsgs)
        {
            this.bufferedSnapshotMsgs = bufferedSnapshotMsgs;
        }

        public Buffer getName()
        {
            return name;
        }
        
        public void setName(Buffer name)
        {
            this.name = name;
        }

        public int getRealTimeChannelId()
        {
            return realTimeChannelId;
        }

        public void setRealTimeChannelId(int realTimeChannelId)
        {
            this.realTimeChannelId = realTimeChannelId;
        }

        public int getGapChannelId()
        {
            return gapChannelId;
        }

        public void setGapChannelId(int gapChannelId)
        {
            this.gapChannelId = gapChannelId;
        }

        public int getServiceId()
        {
            return serviceId;
        }

        public void setServiceId(int serviceId)
        {
            this.serviceId = serviceId;
        }
        
        public boolean hasQueuedMsgs()
        {
            return msgQueue.size() > 0;
        }
        

        public boolean msgHasSeqNum(Msg msg)
        {
            if(msg.msgClass() == MsgClasses.REFRESH)
            {
                return ((RefreshMsg)msg).checkHasSeqNum();
            }
            if(msg.msgClass() == MsgClasses.UPDATE)
            {
                return ((UpdateMsg)msg).checkHasSeqNum();    
            }
            
            return false;
        }
        
        public long msgGetSeqNum(Msg msg)
        {
            if(msg.msgClass() == MsgClasses.REFRESH)
            {
                if(((RefreshMsg)msg).checkHasSeqNum() == true)
                {
                    return ((RefreshMsg)msg).seqNum();
                }
            }
            
            if(msg.msgClass() == MsgClasses.UPDATE)
            {
                if(((UpdateMsg)msg).checkHasSeqNum() == true)
                {
                    return ((UpdateMsg)msg).seqNum();
                }
            }
            
            return 0;
        }
        
        public long seqNumCompare(long seqNum1, long seqNum2)
        {
            return seqNum1-seqNum2;
        }
        
        public long getNextSeqNum(long seqNum)
        {

            if(seqNum == 0xFFFFFFFF)
                return 1;
            else
                return seqNum + 1;
        }
        
    }

}
