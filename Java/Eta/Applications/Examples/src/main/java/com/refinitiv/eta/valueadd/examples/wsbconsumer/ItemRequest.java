/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.wsbconsumer;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.eta.codec.Array;
import com.refinitiv.eta.codec.ArrayEntry;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.ElementEntry;
import com.refinitiv.eta.codec.ElementList;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.RequestMsgFlags;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceRequestFlags;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.rdm.InstrumentNameTypes;

/**
 * The Class ItemRequest.
 */
public class ItemRequest
{
   private List<String> itemNames;
   private int streamId;
   private Qos qos;
   private Qos worstQos;
   private int priorityClass;
   private int priorityCount;
   private int serviceId;
   private int identifier;
   private int domainType;
   private int flags;
   private boolean isSymbolListData;
    
   /**
    * The Constant VIEW_TYPE.
    */
   public static final Buffer VIEW_TYPE = CodecFactory.createBuffer();
   
   /**
    *  :ViewData.
    */
   public static final Buffer VIEW_DATA = CodecFactory.createBuffer();
   
   RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
   private ElementList elementList = CodecFactory.createElementList();
   private ElementEntry elementEntry = CodecFactory.createElementEntry();
   private Array array = CodecFactory.createArray();
   private ArrayEntry arrayEntry = CodecFactory.createArrayEntry();
   private Buffer itemNameBuf = CodecFactory.createBuffer();

         
   /**
    * Instantiates a new item request.
    */
   public ItemRequest()
   {
       this(DomainTypes.MARKET_PRICE);
       this.isSymbolListData = false;
   }

   /**
    * Instantiates a new item request.
    *
    * @param domainType the domain type
    */
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
    * Stream id.
    *
    * @return the int
    */
   public int streamId()
   {
       return streamId;
   }

   /**
    * Stream id.
    *
    * @param streamId the stream id
    */
   public void streamId(int streamId)
   {
       this.streamId = streamId;
   }
   
   /**
    * Symbol list data.
    *
    * @param isSymbolListData the is symbol list data
    */
   public void symbolListData(boolean isSymbolListData)
   {
	   this.isSymbolListData = isSymbolListData;
   }
      
   /**
    * Clears the current contents of this object and prepares it for re-use.
    */
   public void clear()
   {
       flags = 0;
       qos.clear();
       worstQos.clear();
       itemNames.clear();
       priorityClass = 1;
       priorityCount = 1;
       identifier = -1;
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
    * Service id.
    *
    * @return service id
    */
   public int serviceId()
   {
       return serviceId;
   }

   /**
    * Service id.
    *
    * @param serviceId the service id
    * @return the item request
    */
   public ItemRequest serviceId(int serviceId)
   {
       this.serviceId = serviceId;
       return this;
   }

   /**
    * Item names.
    *
    * @return list of item names
    */
   public List<String> itemNames()
   {
       return itemNames;
   }
   
   /**
    * Item names.
    *
    * @param itemNames list of item names
    */
   public void itemNames(List<String> itemNames)
   {
       this.itemNames = itemNames;
   }   

   /**
    * Adds the item.
    *
    * @param itemName item name
    */
   public void addItem(String itemName)
   {
       itemNames.add(itemName);
   }   
   
   /**
    * Priority class.
    *
    * @return priority class used by request
    */
   public int priorityClass()
   {
       return priorityClass;
   }

   /**
    * Priority count.
    *
    * @return priority count used by request
    */
   public int priorityCount()
   {
       return priorityCount;
   }

   /**
    * Priority.
    *
    * @param priorityClass the priority class
    * @param priorityCount the priority count
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
    * Applies Pause flag.
    */
   public void applyPause()
   {
       flags |= RequestMsgFlags.PAUSE;
   }
   
   /**
    * Checks the presence of Pause.
    * 
    * @return true - if exists; false if does not exist.
    */
   public boolean checkHasPause()
   {
       return (flags & RequestMsgFlags.PAUSE) != 0;
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
    * Qos.
    *
    * @return Qos used by request
    */
   public Qos qos()
   {
       return qos;
   }

   /**
    * Worst qos.
    *
    * @return WorstQos used by request
    */
   public Qos worstQos()
   {
       return worstQos;
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
    * Domain type.
    *
    * @return Domain type
    */
   public int domainType()
   {
       return domainType;
   }
   
   /**
    * Domain type.
    *
    * @param domainType Domain type
    */
   public void domainType(int domainType)
   {
       this.domainType = domainType;
   }
   
   /**
    * Set the identifier for the msg Key.
    *
    * @param setIdentifier the set identifier
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
    * Checks the presence of an identifier.
    *
    * @return true - if exists; false if does not exist;
    */
   public boolean checkHasIdentifier()
   {
       if (identifier >= 0)
           return true;
       else
           return false;
   }

   /**
    * Encode.
    */
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

       if (checkHasPause())
       {
           requestMsg.applyPause();
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
       int ret = CodecReturnCodes.SUCCESS;
       
       encode();

       /* encode request message payload */
       if (checkHasView() || isSymbolListData || isBatchRequest)
       {
           ret = encodeRequestPayload(isBatchRequest, encodeIter);
           if (ret < CodecReturnCodes.SUCCESS)
               return ret;
       }

       return CodecReturnCodes.SUCCESS;
   }

   private int encodeRequestPayload(boolean isBatchRequest, EncodeIterator encodeIter)
   {
       elementList.clear();
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

       if (checkHasView() || isSymbolListData || isBatchRequest)
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
}

