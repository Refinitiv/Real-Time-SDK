/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.consumer;

import java.util.Date;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import com.refinitiv.eta.codec.AckMsg;
import com.refinitiv.eta.codec.Array;
import com.refinitiv.eta.codec.ArrayEntry;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DateTime;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.DictionaryEntry;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.FieldEntry;
import com.refinitiv.eta.codec.FieldList;
import com.refinitiv.eta.codec.Int;
import com.refinitiv.eta.codec.LocalFieldSetDefDb;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.Real;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.codec.Time;
import com.refinitiv.eta.codec.UpdateMsg;
import com.refinitiv.eta.codec.Vector;
import com.refinitiv.eta.codec.VectorEntry;
import com.refinitiv.eta.codec.VectorEntryActions;
import com.refinitiv.eta.examples.common.ChannelSession;
import com.refinitiv.eta.examples.common.LoginHandler;
import com.refinitiv.eta.examples.common.StreamIdWatchList;
import com.refinitiv.eta.examples.common.StreamIdWatchList.StreamIdKey;
import com.refinitiv.eta.examples.common.StreamIdWatchList.WatchListEntry;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceClose;
import com.refinitiv.eta.shared.rdm.yieldcurve.YieldCurveRequest;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.UpdateEventTypes;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;

/**
 * This is the yield curve handler for the ETA consumer application. It provides
 * methods for sending the yield curve request(s) to a provider and processing
 * the response(s). Methods for decoding vectors containing field lists and/or
 * arrays, decoding field entries from a response, closing yield curve streams,
 * and adding/removing items to/from the item list are also provided. Methods
 * for decoding a field entry from a response, and closing market price streams
 * are also provided.
 * 
 */
public class YieldCurveHandler
{
	public static int TRANSPORT_BUFFER_SIZE_REQUEST = ChannelSession.MAX_MSG_SIZE;
	public static int TRANSPORT_BUFFER_SIZE_CLOSE = ChannelSession.MAX_MSG_SIZE;

	private int domainType;

	/* Channel to use for private stream redirect */
	private ChannelSession redirectChnl;

	/* Login information to use for private stream redirect */
	private LoginRefresh redirectLoginInfo;

	/* Source directory information to use for private stream redirect */
	private Service redirectSrcDirInfo;

	private YieldCurveRequest yieldCurveRequest;

	private MarketPriceClose closeMessage;

	private final StreamIdWatchList watchList;

	private boolean snapshotRequested = false;

	// temp. reusable variables used for encoding
	private FieldList fieldList = CodecFactory.createFieldList();
	private FieldEntry fieldEntry = CodecFactory.createFieldEntry();
	private FieldList embeddedFieldList = CodecFactory.createFieldList();
	private FieldEntry embeddedFieldEntry = CodecFactory.createFieldEntry();
	private VectorEntry vectorEntry = CodecFactory.createVectorEntry();
	private ArrayEntry arrayEntry = CodecFactory.createArrayEntry();
	private LocalFieldSetDefDb localFieldSetDefDb = CodecFactory.createLocalFieldSetDefDb();
	private Int fidIntValue = CodecFactory.createInt();
	private Real fidRealValue = CodecFactory.createReal();
	private com.refinitiv.eta.codec.Date fidDateValue = CodecFactory.createDate();
	private Time fidTimeValue = CodecFactory.createTime();
	private DateTime fidDateTimeValue = CodecFactory.createDateTime();
	private Vector fidVectorValue = CodecFactory.createVector();
	private Array fidArrayValue = CodecFactory.createArray();
	private Buffer fidBufferValue = CodecFactory.createBuffer();
	private EncodeIterator encIter = CodecFactory.createEncodeIterator();
	private StringBuilder fieldValue = new StringBuilder();
	
	private int indentCount;
	private String[] indents = {"", "    ", "        ", "            "};

	/**
	 * Instantiates a new yield curve handler.
	 *
	 * @param watchList the watch list
	 */
	public YieldCurveHandler(StreamIdWatchList watchList)
	{
		this(DomainTypes.YIELD_CURVE, watchList);
	}

	private YieldCurveHandler(int domainType, StreamIdWatchList watchList)
	{
		this.watchList = watchList;
		this.domainType = domainType;
		yieldCurveRequest = createYieldCurveRequest();
		closeMessage = new MarketPriceClose();
	}

	private YieldCurveRequest createYieldCurveRequest()
	{
		return new YieldCurveRequest();
	}

	/**
	 * Used to enable snapshot requesting. Send a set of
	 * items as a snapshot request.
	 *
	 * @param snapshotRequested the snapshot requested
	 */
	public void snapshotRequest(boolean snapshotRequested)
	{
		this.snapshotRequested = snapshotRequested;
	}

	private void removeYieldCurveItemEntry(int streamId)
	{
		watchList.remove(streamId);
	}

	private int closeStream(ChannelSession chnl, int streamId, Error error)
	{
		/* get a buffer for the item close */
		TransportBuffer msgBuf = chnl.getTransportBuffer(
				TRANSPORT_BUFFER_SIZE_CLOSE, false, error);
		if (msgBuf == null)
			return CodecReturnCodes.FAILURE;

		/* encode item close */
		closeMessage.clear();
		closeMessage.streamId(streamId);
		closeMessage.domainType(domainType);
		encIter.clear();
		encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(),
				chnl.channel().minorVersion());

		int ret = closeMessage.encode(encIter);
		if (ret < CodecReturnCodes.SUCCESS)
		{
			System.out.println("encodeYieldCurveClose(): Failed <"
					+ CodecReturnCodes.toString(ret) + ">");
		}
		return chnl.write(msgBuf, error);
	}

	private boolean hasYieldCurveCapability(List<Long> capabilities)
	{
		for (Long capability : capabilities)
		{
			if (capability.equals((long) yieldCurveRequest.domainType()))
				return true;
		}
		return false;
	}

	/**
	 * Encodes and sends item requests for yield curve domains.
	 *
	 * @param chnl            - The channel to send a source directory request to
	 * @param itemNames            - List of item names
	 * @param isPrivateStream            - flag indicating if requested items are private stream or
	 *            not.
	 * @param loginInfo            - RDM login information
	 * @param serviceInfo            - RDM directory response information
	 * @param error the error
	 * @return success if item requests can be made, can be encoded and sent
	 *         successfully. Failure if duplicate item request, service does not
	 *         support market price capability or failure for encoding/sending
	 *         request.
	 */
	public int sendItemRequests(ChannelSession chnl, List<String> itemNames,
			boolean isPrivateStream, LoginRefresh loginInfo,
			Service serviceInfo, Error error)
	{
		if (itemNames == null || itemNames.isEmpty())
			return CodecReturnCodes.SUCCESS;

		/* check to see if the provider supports the market price domain */
		if (!hasYieldCurveCapability(serviceInfo.info().capabilitiesList()))
		{
			error.text("'"
					+ DomainTypes.toString(yieldCurveRequest.domainType())
					+ "' not supported by the indicated provider");
			return CodecReturnCodes.FAILURE;
		}

		/* set redirect channel for private stream redirect */
		redirectChnl = chnl;

		/* set login information for private stream redirect */
		redirectLoginInfo = loginInfo;

		/* set source directory information for private stream redirect */
		redirectSrcDirInfo = serviceInfo;

		generateRequest(yieldCurveRequest, isPrivateStream, serviceInfo, loginInfo);

		// If there is only one item in the itemList, it is a waste of bandwidth
		// to send a batch request
		if (itemNames.size() == 1)
		{
			return sendRequest(chnl, itemNames, error);
		}

		if (!(loginInfo.checkHasFeatures() &&
			loginInfo.features().checkHasSupportBatchRequests() &&
			loginInfo.features().supportBatchRequests() == 1))
		{
			System.out.println("Connected Provider does not support Batch Requests. Sending Market Price requests as individual request messages.");
			return sendRequest(chnl, itemNames, error);
		}

		// batch
		return sendBatchRequest(chnl, itemNames, error);
	}

	private void generateRequest(YieldCurveRequest yieldCurveRequest,
			boolean isPrivateStream, Service srcDirInfo, LoginRefresh loginInfo)
	{
		yieldCurveRequest.clear();

		if (!snapshotRequested)
			yieldCurveRequest.applyStreaming();
		yieldCurveRequest.applyHasServiceId();
		yieldCurveRequest.serviceId(srcDirInfo.serviceId());
		yieldCurveRequest.applyHasPriority();
		yieldCurveRequest.priority(1, 1);
		yieldCurveRequest.applyHasQos();
		yieldCurveRequest.qos().dynamic(false);
		yieldCurveRequest.qos().timeInfo(srcDirInfo.info().qosList().get(0).timeInfo());
		yieldCurveRequest.qos().timeliness(srcDirInfo.info().qosList().get(0).timeliness());
		yieldCurveRequest.qos().rateInfo(srcDirInfo.info().qosList().get(0).rateInfo());
		yieldCurveRequest.qos().rate(srcDirInfo.info().qosList().get(0).rate());
		if (isPrivateStream)
			yieldCurveRequest.applyPrivateStream();
	}

	// sends items as batch request
	private int sendBatchRequest(ChannelSession chnl, List<String> itemNames, Error error)
	{
		int batchStreamId = watchList.add(domainType, "BATCH_" + new Date(),
				yieldCurveRequest.checkPrivateStream());
		yieldCurveRequest.streamId(batchStreamId);
		for (String itemName : itemNames)
		{
			watchList.add(domainType, itemName,	yieldCurveRequest.checkPrivateStream());
			yieldCurveRequest.itemNames().add(itemName);
		}

		return encodeAndSendRequest(chnl, yieldCurveRequest, error);
	}

	// sends one item at a time
	private int sendRequest(ChannelSession chnl, List<String> itemNames, Error error)
	{
		int ret = CodecReturnCodes.SUCCESS;
		for (String itemName : itemNames)
		{
			Integer streamId = watchList.add(domainType, itemName, yieldCurveRequest.checkPrivateStream());

			yieldCurveRequest.itemNames().clear();
			yieldCurveRequest.itemNames().add(itemName);

			yieldCurveRequest.streamId(streamId);
			ret = encodeAndSendRequest(chnl, yieldCurveRequest, error);
			if (ret < CodecReturnCodes.SUCCESS)
				return ret;
		}

		return CodecReturnCodes.SUCCESS;
	}

	private int encodeAndSendRequest(ChannelSession chnl, YieldCurveRequest yieldCurveRequest, Error error)
	{
		// get a buffer for the item request
		TransportBuffer msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, error);

		if (msgBuf == null)
		{
			return CodecReturnCodes.FAILURE;
		}

		encIter.clear();
		encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());

		int ret = yieldCurveRequest.encode(encIter);
		if (ret < CodecReturnCodes.SUCCESS)
		{
			error.text("YieldCurveRequest.encode() failed");
			error.errorId(ret);
			return ret;
		}

		System.out.println(yieldCurveRequest.toString());
		return chnl.write(msgBuf, error);
	}

	/**
	 * Yield curve response handler.
	 * 
	 * Processes a yield curve response. This consists of extracting the key,
	 * printing out the item name contained in the key, decoding the field list
	 * and field entry, and calling decodeFieldEntry() to decode the field entry
	 * data.
	 *
	 * @param msg            - The partially decoded message
	 * @param dIter            - The decode iterator
	 * @param dictionary the dictionary
	 * @param error the error
	 * @return success if decoding succeeds, failure if it fails.
	 */
	public int processResponse(Msg msg, DecodeIterator dIter, DataDictionary dictionary, Error error)
	{
		indentCount = 0;

		switch (msg.msgClass())
		{
			case MsgClasses.REFRESH:
				return handleRefresh(msg, dIter, dictionary, error);
			case MsgClasses.UPDATE:
				return handleUpdate(msg, dIter, dictionary);
			case MsgClasses.STATUS:
				return handleStatus(msg, error);
			case MsgClasses.ACK:
				return handleAck(msg);
			default:
				System.out.println("Received Unhandled Item Msg Class: "
						+ msg.msgClass());
				break;
		}

		return CodecReturnCodes.SUCCESS;
	}

	private int handleAck(Msg msg)
	{
		System.out.println("Received AckMsg for stream " + msg.streamId());

		fieldValue.setLength(0);
		getItemName(msg, fieldValue);

		AckMsg ackMsg = (AckMsg) msg;

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

	private void getItemName(Msg msg, StringBuilder fieldValue)
	{
		// get key
		MsgKey key = msg.msgKey();

		// print out item name from key if it has it
		if (key != null && key.checkHasName())
		{
			if (msg.msgClass() == MsgClasses.REFRESH)
			{
				RefreshMsg refreshMsg = (RefreshMsg) msg;
				fieldValue.append(key.name().toString()
								+ (refreshMsg.checkPrivateStream() ? " (PRIVATE STREAM)" : "") + "\nDOMAIN: "
								+ DomainTypes.toString(msg.domainType()) + "\n");
			}
			else
			{
				fieldValue.append(key.name().toString() + "\nDOMAIN: "
						+ DomainTypes.toString(msg.domainType()) + "\n");
				if (msg.msgClass() == MsgClasses.UPDATE)
				{
					fieldValue.append("UPDATE TYPE: "
							+ UpdateEventTypes.toString(((UpdateMsg) msg).updateType()) + "\n");
				}
			}
		}
		else
		// cached item name
		{
			WatchListEntry wle = watchList.get(msg.streamId());

			if (wle != null)
			{
				fieldValue.append(wle.itemName
						+ (wle.isPrivateStream ? " (PRIVATE STREAM)" : " ")
						+ "\nDOMAIN: " + DomainTypes.toString(msg.domainType())
						+ "\n");
				if (msg.msgClass() == MsgClasses.UPDATE)
				{
					fieldValue.append("UPDATE TYPE: "
							+ UpdateEventTypes.toString(((UpdateMsg) msg).updateType()) + "\n");
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

	private int handleStatus(Msg msg, com.refinitiv.eta.transport.Error error)
	{
		StatusMsg statusMsg = (StatusMsg) msg;
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
										+ msg.streamId()
										+ " that should be private - closing stream");
						// close stream
						closeStream(redirectChnl, msg.streamId(), error);
						// remove private stream entry from list
						removeYieldCurveItemEntry(msg.streamId());
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
			if (ret < CodecReturnCodes.SUCCESS)
			{
				return ret;
			}
		}

		return CodecReturnCodes.SUCCESS;
	}

	private int handleUpdate(Msg msg, DecodeIterator dIter, DataDictionary dictionary)
	{
		return decode(msg, dIter, dictionary);
	}

	private int decode(Msg msg, DecodeIterator dIter, DataDictionary dictionary)
	{
		int ret = 0;
		
		fieldValue.setLength(0);
		getItemName(msg, fieldValue);
		System.out.print(fieldValue);
		
		if (msg.msgClass() == MsgClasses.REFRESH)
			System.out.println((((RefreshMsg)msg).state()).toString());
		else
			System.out.println();

		ret = decodeFieldList(dIter, dictionary, fieldList, fieldEntry);
		
		System.out.println();
		return ret;
	}

	private int handleRefresh(Msg msg, DecodeIterator dIter, DataDictionary dictionary, Error error)
	{
		RefreshMsg refreshMsg = (RefreshMsg) msg;
		WatchListEntry wle = watchList.get(msg.streamId());

		/* check if this response should be on private stream but is not */
		/* if this is the case, close the stream */
		if (!refreshMsg.checkPrivateStream() && wle.isPrivateStream)
		{
			System.out.println("Received non-private response for stream "
					+ msg.streamId()
					+ " that should be private - closing stream");
			// close stream
			closeStream(redirectChnl, msg.streamId(), error);

			// remove private stream entry from list
			removeYieldCurveItemEntry(msg.streamId());
			error.text("Received non-private response for stream "
					+ msg.streamId()
					+ " that should be private - closing stream");
			return CodecReturnCodes.FAILURE;
		}
		/*
		 * update our item state list if its a refresh, then process just like
		 * update
		 */
		wle.itemState.dataState(refreshMsg.state().dataState());
		wle.itemState.streamState(refreshMsg.state().streamState());

		return this.decode(msg, dIter, dictionary);
	}

	private int decodeFieldList(DecodeIterator dIter, DataDictionary dictionary, FieldList localFieldList, FieldEntry localFieldEntry)
	{
		localFieldList.clear();
				
		int ret = localFieldList.decode(dIter, localFieldSetDefDb);
		if (ret < CodecReturnCodes.SUCCESS)
		{
			System.out.println("DecodeFieldList() failed with return code: " + ret);
			return ret;
		}

		localFieldEntry.clear();

		indentCount++;
		
		// decode each field entry in list
		while ((ret = localFieldEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret < CodecReturnCodes.SUCCESS)
			{
				System.out.println("decodeFieldEntry() failed with return code: " + ret);
				return ret;
			}
			// get dictionary entry
			DictionaryEntry dictionaryEntry = dictionary.entry(localFieldEntry.fieldId());
			
			// return if no entry found
			if (dictionaryEntry == null)
			{
				System.out.println("\tFid " + localFieldEntry.fieldId()
						+ " not found in dictionary");
				System.out.println(localFieldEntry.encodedData().toHexString());
				return CodecReturnCodes.SUCCESS;
			}

			// print out fid name
			System.out.print(indents[indentCount] + dictionaryEntry.acronym().toString());
			for (int i = 0; i < 40 - indents[indentCount].length() - dictionaryEntry.acronym().length(); i++)
			{
				System.out.print(" ");
			}
			
			// decode and print out fid value
			int dataType = dictionaryEntry.rwfType();

			switch (dataType)
			{
				case DataTypes.VECTOR:
					ret = decodeVector(dIter, dictionary);
					if (ret < CodecReturnCodes.SUCCESS)
					{
						System.out.println("decodeVector inside FieldList failed");
						return ret;
					}
					break;
				case DataTypes.ARRAY:
					ret = decodeArray(dIter);
					if (ret < CodecReturnCodes.SUCCESS)
					{
						System.out.println("decodeArray inside FieldList failed");
						return ret;
					}	
					break;
				default:
					ret = decodePrimitive(dIter, dataType, false);
					if (ret < CodecReturnCodes.SUCCESS)
					{
						System.out.println("decodePrimitive inside FieldList failed");
						return ret;
					}	
					break;
			}
		}
		indentCount--;
		return CodecReturnCodes.SUCCESS;
	}

	private int decodeVector(DecodeIterator dIter, DataDictionary dictionary)
	{
		int ret = fidVectorValue.decode(dIter);
		if (ret < CodecReturnCodes.SUCCESS)
		{
			System.out.println("DecodeReal() failed: <"	+ CodecReturnCodes.toString(ret) + ">");
			return ret;
		}
		if (ret == CodecReturnCodes.NO_DATA)
		{
			System.out.println("<no data>");
			return CodecReturnCodes.SUCCESS;
		}
		if (fidVectorValue.checkHasSummaryData())
		{
			System.out.println();
			// fieldList inside summaryData within vector			
			ret = decodeFieldList(dIter, dictionary, embeddedFieldList, embeddedFieldEntry);
			if (ret < CodecReturnCodes.SUCCESS)
			{
				System.out.println("DecodeSummaryData failed: <" + CodecReturnCodes.toString(ret) + ">");
				return ret;
			}
		}
		// If the vector flags indicate that set definition content is present,
		// decode the set def db
		if (fidVectorValue.checkHasSetDefs())
		{
			if (fidVectorValue.containerType() == DataTypes.FIELD_LIST)
			{
				localFieldSetDefDb.clear();
				ret = localFieldSetDefDb.decode(dIter);
				if (ret < CodecReturnCodes.SUCCESS)
				{
					System.out.println("DecodeSetDefDb() failed: <"	+ CodecReturnCodes.toString(ret) + ">");
					return ret;
				}
			}
		}
		
		indentCount++;
		System.out.println();
		
		vectorEntry.clear();
		while ((ret = vectorEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with DecodeVectorEntry.  Error Text: %s\n",
								CodecReturnCodes.toString(ret), ret, CodecReturnCodes.info(ret));
				return ret;
			}

			System.out.println(indents[indentCount] + "INDEX: " + vectorEntry.index());
			System.out.print(indents[indentCount] + "ACTION: ");
			switch (vectorEntry.action())
			{
				case VectorEntryActions.UPDATE:
					System.out.println("UPDATE_ENTRY");
					break;
				case VectorEntryActions.SET:
					System.out.println("SET_ENTRY");
					break;
				case VectorEntryActions.CLEAR:
					System.out.println("CLEAR_ENTRY");
					break;
				case VectorEntryActions.INSERT:
					System.out.println("INSERT_ENTRY");
					break;
				case VectorEntryActions.DELETE:
					System.out.println("DELETE_ENTRY");
					break;
				default:
					System.out.println("UNKNOWN");
					break;
			}

			/* Continue decoding vector entries. */
			switch (fidVectorValue.containerType())
			{
				case DataTypes.FIELD_LIST:
					// fieldList inside vectorEntry within vector
					ret = decodeFieldList(dIter, dictionary, embeddedFieldList, embeddedFieldEntry);
					if (ret < CodecReturnCodes.SUCCESS)
					{
						System.out.printf("Error %s (%d) encountered with decoding FieldList within Vector: %s\n",
										CodecReturnCodes.toString(ret), ret, CodecReturnCodes.info(ret));
						return ret;
					}
					break;
				case DataTypes.ARRAY:
					ret = decodeArray(dIter);
					if (ret < CodecReturnCodes.SUCCESS)
					{
						System.out.printf("Error %s (%d) encountered with decoding ARRAY within Vector: %s\n",
										CodecReturnCodes.toString(ret), ret, CodecReturnCodes.info(ret));
						return ret;
					}
					break;
				default:
					System.out.println("Error: Vector contained unhandled containerType " + fidVectorValue.containerType());
					break;
			}

		}
		indentCount--;
		return CodecReturnCodes.SUCCESS;
	}

	private int decodePrimitive(DecodeIterator dIter, int dataType, boolean isArray)
	{
		int ret = 0;

		switch (dataType)
		{
			case DataTypes.INT:
				ret = fidIntValue.decode(dIter);
				if (ret == CodecReturnCodes.SUCCESS)
				{
					System.out.print(fidIntValue.toLong());
				}
				else if (ret != CodecReturnCodes.BLANK_DATA)
				{
					System.out.println("DecodeInt() failed: <" + CodecReturnCodes.toString(ret) + ">");
					return ret;
				}
				break;
			case DataTypes.REAL:
				ret = fidRealValue.decode(dIter);
				if (ret == CodecReturnCodes.SUCCESS)
				{
					System.out.print(fidRealValue.toDouble());
				}
				else if (ret != CodecReturnCodes.BLANK_DATA)
				{
					System.out.println("DecodeReal() failed: <" + CodecReturnCodes.toString(ret) + ">");
					return ret;
				}
				break;
			case DataTypes.DATE:
				ret = fidDateValue.decode(dIter);
				if (ret == CodecReturnCodes.SUCCESS)
				{
					System.out.print(fidDateValue.toString());
	
				}
				else if (ret != CodecReturnCodes.BLANK_DATA)
				{
					System.out.println("DecodeDate() failed: <" + CodecReturnCodes.toString(ret) + ">");
					return ret;
				}
				break;
			case DataTypes.TIME:
				ret = fidTimeValue.decode(dIter);
				if (ret == CodecReturnCodes.SUCCESS)
				{
					System.out.print(fidTimeValue.toString());
				}
				else if (ret != CodecReturnCodes.BLANK_DATA)
				{
					System.out.println("DecodeTime() failed: <" + CodecReturnCodes.toString(ret) + ">");
					return ret;
				}
				break;
			case DataTypes.DATETIME:
				ret = fidDateTimeValue.decode(dIter);
				if (ret == CodecReturnCodes.SUCCESS)
				{
					System.out.print(fidDateTimeValue.toString());
				}
				else if (ret != CodecReturnCodes.BLANK_DATA)
				{
					System.out.println("DecodeDateTime() failed: <" + CodecReturnCodes.toString(ret) + ">");
					return ret;
				}
				break;
			case DataTypes.ARRAY:
				ret = decodeArray(dIter);
				if (ret < CodecReturnCodes.SUCCESS)
				{
					System.out.printf("Error %s (%d) encountered with decoding ARRAY was primitive: %s\n",
											CodecReturnCodes.toString(ret), ret, CodecReturnCodes.info(ret));
					return ret;
				}
				break;			
			case DataTypes.BUFFER:
			case DataTypes.ASCII_STRING:
			case DataTypes.UTF8_STRING:
			case DataTypes.RMTES_STRING:
				ret = fidBufferValue.decode(dIter);
				if (ret == CodecReturnCodes.SUCCESS)
				{
					if (isArray)
						System.out.print("\"");
					System.out.print(fidBufferValue.toString());
					if (isArray)
						System.out.print("\"");
				}
				else if (ret != CodecReturnCodes.BLANK_DATA)
				{
					System.out.println("DecodeString() failed: <" + CodecReturnCodes.toString(ret) + ">");
					return ret;
				}
				break;
			default:
				System.out.print("Unsupported data type (" + DataTypes.toString(dataType) + ")");
				break;
		}
		if (ret == CodecReturnCodes.BLANK_DATA)
		{
			System.out.print("<blank data>");
		}	

		if(!isArray)
			System.out.print("\n");
		
		return CodecReturnCodes.SUCCESS;
	}

	private int decodeArray(DecodeIterator dIter)
	{
		boolean firstArrayEntry = true;

		System.out.print("{ ");

		fidArrayValue.clear();
		int ret = fidArrayValue.decode(dIter);
		if (ret < CodecReturnCodes.SUCCESS)
		{
			System.out.println("DecodeArray() failed: <" + CodecReturnCodes.toString(ret) + ">");
			return ret;
		}
		
		int dataType = fidArrayValue.primitiveType();

		arrayEntry.clear();
		while ((ret = arrayEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with DecodeArrayEntry.  Error Text: %s\n",
								CodecReturnCodes.toString(ret), ret, CodecReturnCodes.info(ret));
				return ret;
			}

			if (firstArrayEntry)
				firstArrayEntry = false;
			else
				System.out.print(", ");
			ret = decodePrimitive(dIter, dataType, true);
			if (ret < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with DecodeArrayEntryPrimitives.  Error Text: %s\n",
								CodecReturnCodes.toString(ret), ret, CodecReturnCodes.info(ret));
				return ret;
			}
		}

		System.out.print(" }\n");

		return CodecReturnCodes.SUCCESS;
	}

	/**
	 * Close all item streams.
	 *
	 * @param chnl The channel to send a item stream close to
	 * @param error the error
	 * @return the int
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
				if (ret < CodecReturnCodes.SUCCESS)
				{
					return ret;
				}
				iter.remove();
			}
		}

		return CodecReturnCodes.SUCCESS;
	}

	/*
	 * Redirect a request to a private stream. streamId is the stream id to be
	 * redirected to private stream
	 */
	private int redirectToPrivateStream(int streamId, com.refinitiv.eta.transport.Error error)
	{
		WatchListEntry wle = watchList.get(streamId);

		/* remove non-private stream entry from list */
		removeYieldCurveItemEntry(streamId);

		/* add item name to private stream list */
		Integer psStreamId = watchList.add(domainType, wle.itemName, true);

		generateRequest(yieldCurveRequest, true, redirectSrcDirInfo, redirectLoginInfo);
		yieldCurveRequest.itemNames().add(wle.itemName);
		yieldCurveRequest.streamId(psStreamId);
		return encodeAndSendRequest(redirectChnl, yieldCurveRequest, error);
	}
}
