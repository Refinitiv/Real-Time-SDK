package com.thomsonreuters.upa.examples.consumerperf;

import java.util.ArrayList;
import java.util.List;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.InstrumentNameTypes;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;

public class ItemHandler
{
	private static final int INITIAL_CAPACITY = 350000; 

	/* stream id starting point */
	private static final int MARKETPRICE_STREAM_ID_START = 6;

	/* number of items in the item list */
	private int itemCount;

	/* item info list */
	private final List<ItemInfo> itemInfoList;
	
	/* item request burst size */
	private int itemReqBurstSize;
	
	/* start burst stream id index */
	private int startBurstIndex;
	
	// message and encode iterator for item requests
	RequestMsg msg;
	EncodeIterator encodeIter;

	/* item information */
	private class ItemInfo
	{
		String	itemname;
		State	itemState;
	}

	{
		itemInfoList = new ArrayList<ItemInfo>(INITIAL_CAPACITY);
		itemCount = 0;
		itemReqBurstSize = 1000;
		startBurstIndex = 0;
		msg = (RequestMsg)CodecFactory.createMsg();
		encodeIter = CodecFactory.createEncodeIterator();
	}
	
	public int itemCount()
	{
		return itemCount;
	}
	
	/*
	 * Publically visable market price request method 
	 *
	 * Sends item requests to a channel.  For each item, this
	 * consists of getting a message buffer, encoding the item
	 * request, and sending the item request to the server.
	 * chnl - The channel to send an item request to
	 */
	public int sendItemRequests(Channel chnl, int serviceId, LoginResponseInfo loginInfo, SourceDirectoryResponseInfo srcDirInfo)
	{
		Error error = TransportFactory.createError();
		TransportBuffer msgBuf = null;
		int ret = 0;

		/* Do not send a request if there are no items in the item list. */
		if(itemCount == 0)
			return CodecReturnCodes.SUCCESS;

		/* check to see if the provider supports the market price domain */
		if(!srcDirInfo.hasCapability(DomainTypes.MARKET_PRICE))
		{
			System.out.println("MARKET_PRICE is not supported by the indicated provider");
			return CodecReturnCodes.FAILURE;
		}

		for (int i = 0; i < itemInfoList.size(); i++)
		{
			/* initialize state management array */
			/* these will be updated as refresh and status messages are received */
			itemInfoList.get(i).itemState.dataState(DataStates.NO_CHANGE);
			itemInfoList.get(i).itemState.streamState(StreamStates.UNSPECIFIED);
		}

		/* send item request(s) */
		for (int i = startBurstIndex; i < startBurstIndex + itemReqBurstSize && i < itemInfoList.size(); i++)
		{
			/* get a buffer for the item request */
			msgBuf = chnl.getBuffer(SendMsgUtils.MAX_MSG_SIZE, false, error);

			if (msgBuf != null)
			{
				/* encode item request */
				if (encodeItemRequest(chnl, msgBuf, (i + MARKETPRICE_STREAM_ID_START), serviceId, loginInfo, srcDirInfo, false) != CodecReturnCodes.SUCCESS)
				{
					System.out.println("\nMarket Price encodeItemRequest() failed");
					return CodecReturnCodes.FAILURE;
				}
				ret = SendMsgUtils.sendMessage(chnl, msgBuf);
			}
			else
			{
				System.out.println("getBuffer(): Failed <" + error.text() + ">");
				return CodecReturnCodes.FAILURE;
			}
		}
		
		itemCount -= itemReqBurstSize;
		startBurstIndex += itemReqBurstSize;
		
		return ret;
	}

	/*
	 * Encodes the item request.  Returns success if
	 * encoding succeeds or failure if encoding fails.
	 * chnl - The channel to send an item request to
	 * msgBuf - The message buffer to encode the item request into
	 * streamId - The stream id of the item request
	 * isPrivateStream - Flag for private stream request
	 *
	 * This method is only used within the Market Price Handler
	 * and each handler has its own implementation, although much is similar
	 */
	private int encodeItemRequest(Channel chnl, TransportBuffer msgBuf, int streamId, int serviceId, LoginResponseInfo loginInfo, SourceDirectoryResponseInfo srcDirInfo, boolean isPrivateStream)
	{
		int ret = 0;

		/* clear encode iterator */
		encodeIter.clear();
		
		/* clear the message */
		msg.clear();

		/* set-up message */
		msg.msgClass(MsgClasses.REQUEST);
		msg.streamId(streamId);
		msg.domainType(DomainTypes.MARKET_PRICE);
		msg.containerType(DataTypes.NO_DATA);
		msg.applyHasQos();
		
		msg.applyHasPriority();
		msg.applyStreaming();
		msg.qos().dynamic(false);
		msg.qos().rate(srcDirInfo.serviceGeneralInfo.qos.get(0).rate());
		msg.qos().timeliness(srcDirInfo.serviceGeneralInfo.qos.get(0).timeliness());
		msg.priority().priorityClass(1);
		msg.priority().count(1);
		
		/* specify msgKey members */
		msg.msgKey().applyHasNameType();
		msg.msgKey().applyHasName();
		msg.msgKey().applyHasServiceId();
		msg.msgKey().nameType(InstrumentNameTypes.RIC);
		msg.msgKey().name().data(itemInfoList.get(streamId - MARKETPRICE_STREAM_ID_START).itemname);
		if (streamId == MARKETPRICE_STREAM_ID_START)
		{
			msg.msgKey().applyHasIdentifier();
			msg.msgKey().identifier(3);
		}
		msg.msgKey().serviceId(serviceId);
		
		/* encode message */
		encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
		if ((ret = msg.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeMsgInit() failed with return code: " + ret);
			return ret;
		}

		if ((ret = msg.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeMsgComplete() failed with return code: " + ret);
			return ret;
		}

		return CodecReturnCodes.SUCCESS;
	}
	
	/**
	 * Adds an item name requested by the application to the item name list.
	 * @param itemname Item name requested by the application
	 */
	public void addItemName(String itemname)
	{
        ItemInfo itemInfo = new ItemInfo();
        itemInfo.itemState = CodecFactory.createState();
        itemInfo.itemname = itemname;
	    
		/* take the next streamId and increment it */
        itemInfoList.add(itemInfo);
        ++itemCount;
	}
}
