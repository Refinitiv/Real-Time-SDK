package com.thomsonreuters.upa.valueadd.examples.watchlistconsumer;

import java.util.ArrayList;
import java.util.List;

import com.thomsonreuters.upa.codec.Array;
import com.thomsonreuters.upa.codec.ArrayEntry;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.ElementList;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.examples.rdm.marketprice.MarketPriceRequestFlags;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.ElementNames;
import com.thomsonreuters.upa.rdm.InstrumentNameTypes;
import com.thomsonreuters.upa.rdm.ViewTypes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBaseImpl;


public class ItemRequest extends MsgBaseImpl
{
   private List<String> itemNames;
   private Qos qos;
   private Qos worstQos;
   private int priorityClass;
   private int priorityCount;
   private int serviceId;
   private int identifier;
   private List<Integer> viewFieldList;
   private int domainType;
   private int flags;

 
   public static final Buffer VIEW_TYPE = CodecFactory.createBuffer();
   /** :ViewData */
   public static final Buffer VIEW_DATA = CodecFactory.createBuffer();
   
   public static final Buffer SYMBOL_LIST_BEHAVIORS = CodecFactory.createBuffer();

   public static final Buffer SYMBOL_LIST_DATA_STREAMS = CodecFactory.createBuffer();
   static
   {
	   SYMBOL_LIST_BEHAVIORS.data(":SymbolListBehaviors");
       
	   SYMBOL_LIST_DATA_STREAMS.data(":DataStreams");
   }

   RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
   private ElementList elementList = CodecFactory.createElementList();
   private ElementList behaviorList = CodecFactory.createElementList();
   private ElementEntry elementEntry = CodecFactory.createElementEntry();
   private ElementEntry dataStreamEntry = CodecFactory.createElementEntry();
   private Array array = CodecFactory.createArray();
   private ArrayEntry arrayEntry = CodecFactory.createArrayEntry();
   private Buffer itemNameBuf = CodecFactory.createBuffer();

   private Int tempInt = CodecFactory.createInt();
   private UInt tempUInt = CodecFactory.createUInt();
   private Array viewArray = CodecFactory.createArray();
         
   public ItemRequest()
   {
       this(DomainTypes.MARKET_PRICE);
   }

   public ItemRequest(int domainType)
   {
       qos = CodecFactory.createQos();
       worstQos = CodecFactory.createQos();
       itemNames = new ArrayList<String>();

       priorityClass = 1;
       priorityCount = 1;
       flags = 0;
       identifier = -1;
       this.domainType = domainType;
   }

   /**
    * Clears the current contents of this object and prepares it for re-use.
    */
   public void clear()
   {
       super.clear();
       flags = 0;
       qos.clear();
       worstQos.clear();
       itemNames.clear();
       priorityClass = 1;
       priorityCount = 1;
       identifier = -1;
       if (viewFieldList != null )  viewFieldList.clear();
   }

   /**
    * Checks the presence of private stream flag.
    * 
    * @return true - if exists; false if does not exist.
    */
   public boolean checkPrivateStream()
   {
       return (flags & MarketPriceRequestFlags.PRIVATE_STREAM) != 0;
   }

   /**
    * Applies private stream flag.
    */
   public void applyPrivateStream()
   {
       flags |= MarketPriceRequestFlags.PRIVATE_STREAM;
   }

   /**
    * 
    * @return service id
    */
   public int serviceId()
   {
       return serviceId;
   }

   /**
    * @param serviceId
    */
   public ItemRequest serviceId(int serviceId)
   {
       this.serviceId = serviceId;
       return this;
   }

   /**
    * 
    * @return list of item names
    */
   public List<String> itemNames()
   {
       return itemNames;
   }
   
   /**
    * 
    * @param list of item names
    */
   public void itemNames(List<String> itemNames)
   {
       this.itemNames = itemNames;
   }   

   /**
    * 
    * @param item name
    */
   public void addItem(String itemName)
   {
       itemNames.add(itemName);
   }   
   
   /**
    * 
    * @return priority class used by request
    */
   public int priorityClass()
   {
       return priorityClass;
   }

   /**
    * 
    * @return priority count used by request
    */
   public int priorityCount()
   {
       return priorityCount;
   }

   /**
    * 
    * @param priorityClass
    * @param priorityCount
    * 
    */
   public void priority(int priorityClass, int priorityCount)
   {
       this.priorityClass = priorityClass;
       this.priorityCount = priorityCount;
   }

   /**
    * Checks the presence of streaming.
    * 
    * @return true - if exists; false if does not exist.
    */
   public boolean checkStreaming()
   {
       return (flags & MarketPriceRequestFlags.STREAMING) != 0;
   }

   /**
    * Applies streaming flag.
    */
   public void applyStreaming()
   {
       flags |= MarketPriceRequestFlags.STREAMING;
   }

   /**
    * Checks the presence of Qos.
    * 
    * @return true - if exists; false if does not exist.
    */
   public boolean checkHasQos()
   {
       return (flags & MarketPriceRequestFlags.HAS_QOS) != 0;
   }

   /**
    * Applies Qos flag.
    */
   public void applyHasQos()
   {
       flags |= MarketPriceRequestFlags.HAS_QOS;
   }

   /**
    * Checks the presence of WorstQos.
    * 
    * @return true - if exists; false if does not exist.
    */
   public boolean checkHasWorstQos()
   {
       return (flags & MarketPriceRequestFlags.HAS_WORST_QOS) != 0;
   }

   /**
    * Applies WorstQos flag.
    */
   public void applyHasWorstQos()
   {
       flags |= MarketPriceRequestFlags.HAS_WORST_QOS;
   }

   /**
    * Checks the presence of Priority flag.
    * 
    * @return true - if exists; false if does not exist.
    */
   public boolean checkHasPriority()
   {
       return (flags & MarketPriceRequestFlags.HAS_PRIORITY) != 0;
   }

   /**
    * Applies Priority flag.
    */
   public void applyHasPriority()
   {
       flags |= MarketPriceRequestFlags.HAS_PRIORITY;
   }

   /**
    * Applies View flag.
    */
   public void applyHasView()
   {
       flags |= MarketPriceRequestFlags.HAS_VIEW;
   }

   /**
    * Checks the presence of View flag.
    * 
    * @return true - if exists; false if does not exist.
    */
   public boolean checkHasView()
   {
       return (flags & MarketPriceRequestFlags.HAS_VIEW) != 0;
   }

   /**
    * 
    * @return Qos used by request
    */
   public Qos qos()
   {
       return qos;
   }

   /**
    * 
    * @return WorstQos used by request
    */
   public Qos worstQos()
   {
       return worstQos;
   }

   /**
    * 
    * @return list of view fields
    */
   public List<Integer> viewFields()
   {
       return viewFieldList;
   }
   /**
    * 
    * @param list of view fields
    */
   public void viewFields(List<Integer> viewList)
   {
       viewFieldList = viewList;
   }
      
   /**
    * Checks the presence of service id flag.
    * @return true - if exists; false if does not exist.
    */
   public boolean checkHasServiceId()
   {
       return (flags & MarketPriceRequestFlags.HAS_SERVICE_ID) != 0;
   }

   /**
    * Applies service id flag.
    */
   public void applyHasServiceId()
   {
       flags |= MarketPriceRequestFlags.HAS_SERVICE_ID;
   }

   /**
    * 
    * @return Domain type
    */
   public int domainType()
   {
       return domainType;
   }
   
   /**
    * 
    * @param Domain type
    */
   public void domainType(int domainType)
   {
       this.domainType = domainType;
   }
   
   /**
    * Set the identifier for the msg Key
    */
   
   public void identifier(int setIdentifier)
   {
       identifier = setIdentifier;
   }
   
   /** 
    * @return Identifier
    */
   public int identifier()
   {
       return identifier;
   }
   
   /**
    * Checks the presence of an identifier
    * @return true - if exists; false if does not exist;
    */
   public boolean checkHasIdentifier()
   {
       if (identifier >= 0)
           return true;
       else
           return false;
   }

   public void encode()
   {
       requestMsg.clear();

       /* set-up message */
       requestMsg.msgClass(MsgClasses.REQUEST);
       requestMsg.streamId(streamId());
       requestMsg.domainType(domainType);
       
       requestMsg.containerType(DataTypes.NO_DATA);
       
       if (checkHasQos())
       {
           requestMsg.applyHasQos();
           requestMsg.qos().dynamic(qos.isDynamic());
           requestMsg.qos().rate(qos.rate());
           requestMsg.qos().timeliness(qos.timeliness());
           requestMsg.qos().rateInfo(qos.rateInfo());
           requestMsg.qos().timeInfo(qos.timeInfo());
       }

       if (checkHasWorstQos())
       {
           requestMsg.applyHasWorstQos();
           requestMsg.worstQos().dynamic(worstQos.isDynamic());
           requestMsg.worstQos().rate(worstQos.rate());
           requestMsg.worstQos().timeliness(worstQos.timeliness());
           requestMsg.worstQos().rateInfo(worstQos.rateInfo());
           requestMsg.worstQos().timeInfo(worstQos.timeInfo());
       }

       if (checkHasPriority())
       {
           requestMsg.applyHasPriority();
           requestMsg.priority().priorityClass(priorityClass());
           requestMsg.priority().count(priorityCount());
       }

       if (checkStreaming())
       {
           requestMsg.applyStreaming();
       }

       boolean isBatchRequest = itemNames.size() > 1;
       applyFeatureFlags(isBatchRequest);

       /* specify msgKey members */
       requestMsg.msgKey().applyHasNameType();
       requestMsg.msgKey().nameType(InstrumentNameTypes.RIC);
       
       /* If user set Identifier */
       if (checkHasIdentifier())
       {
           requestMsg.msgKey().applyHasIdentifier();
           requestMsg.msgKey().identifier(identifier);
       }

       if (!isBatchRequest && !checkHasIdentifier())
       {
           requestMsg.msgKey().applyHasName();
           requestMsg.msgKey().name().data(itemNames.get(0));
       }
   }
   
   /**
    * Encodes the item request.
    * 
    * @param encodeIter The Encode Iterator
    * @return {@link CodecReturnCodes#SUCCESS} if encoding succeeds or failure
    *         if encoding fails.
    * 
    *         This method is only used within the Market By Price Handler and
    *         each handler has its own implementation, although much is similar
    */
   public int encode(EncodeIterator encodeIter)
   {
       boolean isBatchRequest = itemNames.size() > 1;
       
       encode();

       int ret = requestMsg.encodeInit(encodeIter, 0);
       if (ret < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }

       /* encode request message payload */
       if (checkHasView() || isBatchRequest)
       {
           ret = encodeRequestPayload(isBatchRequest, encodeIter);
           if (ret < CodecReturnCodes.SUCCESS)
               return ret;
       }
       ret = requestMsg.encodeComplete(encodeIter, true);
       if (ret < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }

       return CodecReturnCodes.SUCCESS;
   }

   private int encodeRequestPayload(boolean isBatchRequest, EncodeIterator encodeIter)
   {
       elementList.clear();
       requestMsg.containerType(DataTypes.ELEMENT_LIST);
       elementList.applyHasStandardData();

       int ret = elementList.encodeInit(encodeIter, null, 0);
       if (ret < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }

       if (isBatchRequest
               && (encodeBatchRequest(encodeIter) < CodecReturnCodes.SUCCESS))
       {
           return CodecReturnCodes.FAILURE;
       }

       if (checkHasView() && viewFieldList != null &&
               (encodeViewRequest(encodeIter) < CodecReturnCodes.SUCCESS))
       {
           return CodecReturnCodes.FAILURE;
       }

       ret = elementList.encodeComplete(encodeIter, true);
       if (ret < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }
       return CodecReturnCodes.SUCCESS;
   }

   private void applyFeatureFlags(boolean isBatchRequest)
   {
       if (checkPrivateStream())
       {
           requestMsg.applyPrivateStream();
       }

       if (checkHasView() || isBatchRequest)
       {
           requestMsg.containerType(DataTypes.ELEMENT_LIST);
           if (checkHasView())
           {
               requestMsg.applyHasView();
           }
           if (isBatchRequest)
           {
               requestMsg.applyHasBatch();
           }
       }
   }

   private int encodeBatchRequest(EncodeIterator encodeIter)
   {
       /*
        * For Batch requests, the message has a payload of an element list that
        * contains an array of the requested items
        */

       elementEntry.clear();
       array.clear();
       itemNameBuf.clear();

       elementEntry.name(ElementNames.BATCH_ITEM_LIST);
       elementEntry.dataType(DataTypes.ARRAY);
       int ret = elementEntry.encodeInit(encodeIter, 0);
       if (ret < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }

       /* Encode the array of requested item names */
       array.primitiveType(DataTypes.ASCII_STRING);
       array.itemLength(0);

       ret = array.encodeInit(encodeIter);
       if (ret < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }

       for (String itemName : itemNames)
       {
           arrayEntry.clear();
           itemNameBuf.data(itemName);
           ret = arrayEntry.encode(encodeIter, itemNameBuf);
           if (ret < CodecReturnCodes.SUCCESS)
           {
               return ret;
           }
       }

       ret = array.encodeComplete(encodeIter, true);
       if (ret < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }

       ret = elementEntry.encodeComplete(encodeIter, true);
       if (ret < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }

       return CodecReturnCodes.SUCCESS;
   }

  
 
   /*
    * Encodes the View Element Entry. This entry contains an array of FIDs that
    * the consumer wishes to receive from the provider.
    * 
    * This method is only used within the Market Price Handler
    */
   private int encodeViewRequest(EncodeIterator encodeIter)
   {
       elementEntry.clear();
       elementEntry.name(ElementNames.VIEW_TYPE);
       elementEntry.dataType(DataTypes.UINT);
       tempUInt.value(ViewTypes.FIELD_ID_LIST);
       int ret = elementEntry.encode(encodeIter, tempUInt);
       if (ret < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }

       elementEntry.clear();
       elementEntry.name(ElementNames.VIEW_DATA);
       elementEntry.dataType(DataTypes.ARRAY);
       if ((ret = elementEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }

       viewArray.primitiveType(DataTypes.INT);
       viewArray.itemLength(2);

       if ((ret = viewArray.encodeInit(encodeIter)) < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }

       for (Integer viewField : viewFieldList)
       {
           arrayEntry.clear();
           tempInt.value(viewField);
           ret = arrayEntry.encode(encodeIter, tempInt);
           if (ret < CodecReturnCodes.SUCCESS)
           {
               return ret;
           }
       }
       ret = viewArray.encodeComplete(encodeIter, true);

       if (ret < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }

       ret = elementEntry.encodeComplete(encodeIter, true);
       if (ret < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }

       return CodecReturnCodes.SUCCESS;
   }
   
   public int encodeSymbollistDataStreamPayload(EncodeIterator encodeIter)
   {
	   elementList.clear();
	   elementList.applyHasStandardData();
	   int ret = elementList.encodeInit(encodeIter, null, 0);
       if (ret < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }
	   
       elementEntry.clear();
       elementEntry.name(SYMBOL_LIST_BEHAVIORS);
       elementEntry.dataType(DataTypes.ELEMENT_LIST);
     
       ret = elementEntry.encodeInit(encodeIter, 0);
       if (ret < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }

	   behaviorList.clear();
	   behaviorList.applyHasStandardData();
	   // maxSize 0 good ? xia
	   ret = behaviorList.encodeInit(encodeIter, null, 0);
       if (ret < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }       
       
       dataStreamEntry.clear();
       dataStreamEntry.name(SYMBOL_LIST_DATA_STREAMS);
       dataStreamEntry.dataType(DataTypes.UINT);
       tempUInt.value(SymbolListRequestFlags.SYMBOL_LIST_DATA_STREAMS);
       if ((ret = dataStreamEntry.encode(encodeIter, tempUInt)) < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }
  
       ret = behaviorList.encodeComplete(encodeIter, true);

       if (ret < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }

       ret = elementEntry.encodeComplete(encodeIter, true);
       if (ret < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }
       
       ret = elementList.encodeComplete(encodeIter, true);

       if (ret < CodecReturnCodes.SUCCESS)
       {
           return ret;
       }      

       return CodecReturnCodes.SUCCESS;
   }
   
   
   @Override
   public int decode(DecodeIterator dIter, Msg msg)
   {
       throw new UnsupportedOperationException();
   }
   
   public String toString()
   {
      StringBuilder buf = buildStringBuffer();
      buf.append(" itemName: ");
      buf.append(itemNames.get(0));
      buf.append(" DomainType: ");
      buf.append(domainType);
      return buf.toString();
   }
         
}

