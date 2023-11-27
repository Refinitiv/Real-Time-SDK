/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using System;
using System.Collections.Generic;
using System.Text;
using Array = LSEG.Eta.Codec.Array;
using Buffer = LSEG.Eta.Codec.Buffer;
using DateTime = LSEG.Eta.Codec.DateTime;

namespace LSEG.Eta.Example.Common
{
	/// <summary>
	/// This is the yield curve handler for the ETA consumer application. 
	/// It provides methods for sending the yield curve request(s) to a provider 
	/// and processing the response(s). Methods for decoding vectors containing 
	/// field lists and/or arrays, decoding field entries from a response, 
	/// closing yield curve streams, and adding/removing items to/from the item list are also provided. 
	/// Methods for decoding a field entry from a response, and closing market price streams 
	/// are also provided.
	/// </summary>
	public class YieldCurveHandler
    {
		public static int TRANSPORT_BUFFER_SIZE_REQUEST = ChannelSession.MAX_MSG_SIZE;
		public static int TRANSPORT_BUFFER_SIZE_CLOSE = ChannelSession.MAX_MSG_SIZE;

		private int domainType;

		// Channel to use for private stream redirect 
		private ChannelSession? redirectChnl;

		// Login information to use for private stream redirect
		private LoginRefresh? redirectLoginInfo;

		// Source directory information to use for private stream redirect
		private Service? redirectSrcDirInfo;

		private YieldCurveRequest yieldCurveRequest;

		private MarketPriceClose closeMessage;

		private StreamIdWatchList watchList;

		public bool SnapshotRequested { get; set; } = false;

		private FieldList fieldList = new FieldList();
		private FieldEntry fieldEntry = new FieldEntry();
		private FieldList embeddedFieldList = new FieldList();
		private FieldEntry embeddedFieldEntry = new FieldEntry();
		private VectorEntry vectorEntry = new VectorEntry();
		private ArrayEntry arrayEntry = new ArrayEntry();
		private LocalFieldSetDefDb localFieldSetDefDb = new LocalFieldSetDefDb();
		private Int fidIntValue = new Int();
		private Real fidRealValue = new Real();
		private Date fidDateValue = new Date();
		private Time fidTimeValue = new Time();
		private DateTime fidDateTimeValue = new DateTime();
		private Vector fidVectorValue = new Vector();
		private Array fidArrayValue = new Array();
		private Buffer fidBufferValue = new Buffer();
		private EncodeIterator encIter = new EncodeIterator();
		private StringBuilder fieldValue = new StringBuilder();

		private int indentCount;
		private string[] indents = { "", "    ", "        ", "            " };

		private YieldCurveRequest CreateYieldCurveRequest()
		{
			return new YieldCurveRequest();
		}
		/// <summary>
		/// Instantiates a new yield curve handler.
		/// </summary>
		/// <param name="watchList">The WatchList</param>
		public YieldCurveHandler(StreamIdWatchList watchList) : this((int)DomainType.YIELD_CURVE, watchList) {}

		private YieldCurveHandler(int domainType, StreamIdWatchList watchList)
		{
			this.watchList = watchList;
			this.domainType = domainType;
			yieldCurveRequest = CreateYieldCurveRequest();
			closeMessage = new MarketPriceClose();
		}

		private void RemoveYieldCurveItemEntry(int streamId)
		{
			watchList.Remove(streamId);
		}

		private TransportReturnCode CloseStream(ChannelSession chnl, int streamId, out Error? error)
		{
			ITransportBuffer? msgBuf = chnl.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, out error);
			if (msgBuf == null)
				return TransportReturnCode.FAILURE;

			closeMessage.Clear();
			closeMessage.StreamId = streamId;
			closeMessage.DomainType = domainType;
			encIter.Clear();
			encIter.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);

			CodecReturnCode ret = closeMessage.Encode(encIter);
			if (ret < CodecReturnCode.SUCCESS)
			{
				error = new Error()
				{
					Text = "EncodeYieldCurveClose(): Failed <" + ret + ">"
				};
				return TransportReturnCode.FAILURE;
			}
			return chnl.Write(msgBuf, out error);
		}

		private bool HasYieldCurveCapability(List<long> capabilities)
		{
			foreach (long capability in capabilities)
			{
				if (capability.Equals(yieldCurveRequest.DomainType))
					return true;
			}
			return false;
		}

		/// <summary>
		/// Encodes and sends item requests for the yield curve domain.
		/// </summary>
		/// <param name="chnl">The channel to send a yield curve request to</param>
		/// <param name="itemNames">List of item names</param>
		/// <param name="isPrivateStream">Flag indicating if requested items are private stream or not</param>
		/// <param name="loginInfo">RDM login information</param>
		/// <param name="serviceInfo">RDM directory response information</param>
		/// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
		/// <returns><see cref="TransportReturnCode"/> value that indicates the result of the operation</returns>
		public TransportReturnCode SendItemRequests(ChannelSession chnl, List<string> itemNames, bool isPrivateStream, LoginRefresh loginInfo, Service serviceInfo, out Error? error)
		{
			error = null;
			if (itemNames == null || itemNames.Count == 0)
				return TransportReturnCode.SUCCESS;

			// Check to see if the provider supports the market price domain
			if (!HasYieldCurveCapability(serviceInfo.Info.CapabilitiesList))
			{
				error = new Error()
				{
					Text = "'" + DomainTypes.ToString(yieldCurveRequest.DomainType) + "' not supported by the indicated provider"
				};
				return TransportReturnCode.FAILURE;
			}

			// set redirect channel for private stream redirect
			redirectChnl = chnl;

			// set login information for private stream redirect
			redirectLoginInfo = loginInfo;

			// set source directory information for private stream redirect
			redirectSrcDirInfo = serviceInfo;

			GenerateRequest(yieldCurveRequest, isPrivateStream, serviceInfo, loginInfo);

			// If there is only one item in the itemList, it is a waste of bandwidth
			// to send a batch request
			if (itemNames.Count == 1)
			{
				return SendRequest(chnl, itemNames, error);
			}

			if (!(loginInfo.HasFeatures &&
				loginInfo.SupportedFeatures.HasSupportBatchRequests &&
				loginInfo.SupportedFeatures.SupportBatchRequests == 1))
			{
				Console.WriteLine("Connected Provider does not support Batch Requests. Sending Market Price requests as individual request messages.");
				return SendRequest(chnl, itemNames, error);
			}

			// batch
			return SendBatchRequest(chnl, itemNames, error);
		}

		private TransportReturnCode SendBatchRequest(ChannelSession chnl, List<string> itemNames, Error? error)
		{
			int batchStreamId = watchList.Add(domainType, "BATCH_" + new Date(), yieldCurveRequest.IsPrivateStream);
			yieldCurveRequest.StreamId = batchStreamId;
			foreach (string itemName in itemNames)
			{
				watchList.Add(domainType, itemName, yieldCurveRequest.IsPrivateStream);
				yieldCurveRequest.ItemNames.Add(itemName);
			}

			return EncodeAndSendRequest(chnl, yieldCurveRequest, out error);
		}

		private TransportReturnCode SendRequest(ChannelSession chnl, List<String> itemNames, Error? error)
		{
			TransportReturnCode ret;
			foreach (string itemName in itemNames)
			{
				int streamId = watchList.Add(domainType, itemName, yieldCurveRequest.IsPrivateStream);

				yieldCurveRequest.ItemNames.Clear();
				yieldCurveRequest.ItemNames.Add(itemName);

				yieldCurveRequest.StreamId = streamId;
				ret = EncodeAndSendRequest(chnl, yieldCurveRequest, out error);
				if (ret < TransportReturnCode.SUCCESS)
					return ret;
			}

			return TransportReturnCode.SUCCESS;
		}

		private TransportReturnCode EncodeAndSendRequest(ChannelSession chnl, YieldCurveRequest yieldCurveRequest, out Error? error)
		{
			ITransportBuffer? msgBuf = chnl.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out error);

			if (msgBuf == null)
			{
				return TransportReturnCode.FAILURE;
			}

			encIter.Clear();
			encIter.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);

			CodecReturnCode ret = yieldCurveRequest.Encode(encIter);
			if (ret < CodecReturnCode.SUCCESS)
			{
				error = new Error()
				{
					Text = "YieldCurveRequest.Encode() failed"
				};
				return TransportReturnCode.FAILURE;
			}

			Console.WriteLine(yieldCurveRequest.ToString());
			return chnl.Write(msgBuf, out error);
		}

		/// <summary>
		/// Handles yield curve response.
		/// </summary>
		/// <param name="msg">The partially decoded message</param>
		/// <param name="dIter">The decode iterator</param>
		/// <param name="dictionary">The dictionary that will be used to decode the message</param>
		/// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
		/// <returns><see cref="TransportReturnCode"/> value that indicates the result of the operation</returns>
		public TransportReturnCode ProcessResponse(Msg msg, DecodeIterator dIter, DataDictionary dictionary, out Error? error)
		{
			error = null;
			indentCount = 0;

			switch (msg.MsgClass)
			{
				case MsgClasses.REFRESH:
					return HandleRefresh(msg, dIter, dictionary, out error);
				case MsgClasses.UPDATE:
					return HandleUpdate(msg, dIter, dictionary);
				case MsgClasses.STATUS:
					return HandleStatus(msg, out error);
				case MsgClasses.ACK:
					return HandleAck(msg);
				default:
					Console.WriteLine("Received Unhandled Item Msg Class: " + msg.MsgClass);
					break;
			}

			return TransportReturnCode.SUCCESS;
		}

		private TransportReturnCode HandleAck(Msg msg)
		{
			Console.WriteLine("Received AckMsg for stream " + msg.StreamId);

			fieldValue.Clear();
			GetItemName(msg, fieldValue);

			IAckMsg ackMsg = msg;

			fieldValue.Append("\tackId=" + ackMsg.AckId);
			if (ackMsg.CheckHasSeqNum())
				fieldValue.Append("\tseqNum=" + ackMsg.SeqNum);
			if (ackMsg.CheckHasNakCode())
				fieldValue.Append("\tnakCode=" + ackMsg.NakCode);
			if (ackMsg.CheckHasText())
				fieldValue.Append("\ttext=" + ackMsg.Text.ToString());

			Console.WriteLine(fieldValue.ToString());
			return TransportReturnCode.SUCCESS;
		}

		private TransportReturnCode HandleStatus(Msg msg, out Error? error)
		{
			error = null;
			IStatusMsg statusMsg = msg;
			Console.WriteLine("Received Item StatusMsg for stream " + msg.StreamId);
			if (!statusMsg.CheckHasState())
				return TransportReturnCode.SUCCESS;

			State state = statusMsg.State;
			Console.WriteLine("	" + state);

			WatchListEntry? wle = watchList.Get(msg.StreamId);
			if (wle != null)
			{
				// update our state table with the new state
				if (!statusMsg.CheckPrivateStream()) // non-private stream
				{
					
					// Check if this response should be on private stream but is not
					if (!statusMsg.CheckPrivateStream()) // non-private stream
					{
						// Check if this response should be on private stream but is
						// not batch responses for private stream may be sent on
						// non-private stream
						// if this is the case, close the stream
						if (wle.IsPrivateStream && !wle.ItemName!.Contains("BATCH_"))
						{
							Console.WriteLine("Received non-private response for stream "
											+ msg.StreamId
											+ " that should be private - closing stream");
							// close stream
							if (CloseStream(redirectChnl!, msg.StreamId, out var closeError) != TransportReturnCode.SUCCESS)
							{
								Console.WriteLine($"Failed closing the channel: {closeError?.Text}");
							}
							// remove private stream entry from list
							RemoveYieldCurveItemEntry(msg.StreamId);
							error = new Error()
							{
								Text = "Received non-private response for stream "
											+ msg.StreamId
											+ " that should be private - closing stream",
								ErrorId = TransportReturnCode.FAILURE
							};
							return TransportReturnCode.FAILURE;
						}
					}
				}
				wle.ItemState!.DataState(statusMsg.State.DataState());
				wle.ItemState.StreamState(statusMsg.State.StreamState());
			}

			// redirect to private stream if indicated
			if (statusMsg.State.StreamState() == StreamStates.REDIRECTED
					&& (statusMsg.CheckPrivateStream()))
			{
				return RedirectToPrivateStream(msg.StreamId, out error);
			}

			return TransportReturnCode.SUCCESS;
		}

		private TransportReturnCode HandleRefresh(Msg msg, DecodeIterator dIter, DataDictionary dictionary, out Error? error)
		{
			error = null;
			IRefreshMsg refreshMsg = msg;
			WatchListEntry? wle = watchList!.Get(msg.StreamId);

			if (wle == null)
			{
				error = new Error()
				{
					Text = "Non existing stream id: " + msg.StreamId
				};
				return TransportReturnCode.FAILURE;
			}

			// Check if this response should be on private stream but is not
			// if this is the case, close the stream
			if (!refreshMsg.CheckPrivateStream() && wle!.IsPrivateStream)
			{
				Console.WriteLine("Received non-private response for stream "
						+ msg.StreamId
						+ " that should be private - closing stream");
				// close stream
				if (CloseStream(redirectChnl!, msg.StreamId, out var closeError) != TransportReturnCode.SUCCESS)
					Console.WriteLine($"Failed closing the stream: {closeError?.Text}");

				// remove private stream entry from list
				RemoveYieldCurveItemEntry(msg.StreamId);
				error = new Error()
				{
					Text = "Received non-private response for stream "
						+ msg.StreamId
						+ " that should be private - closing stream"
				};
				return TransportReturnCode.FAILURE;
			}
		
			// update our item state list if its a refresh, then process just like update
			wle!.ItemState!.DataState(refreshMsg.State.DataState());
			wle!.ItemState.StreamState(refreshMsg.State.StreamState());

			CodecReturnCode ret;
			if ((ret = Decode(msg, dIter, dictionary)) == CodecReturnCode.SUCCESS)
            {
				return TransportReturnCode.SUCCESS;
			}
			else
            {
				error = new Error()
				{
					Text = "Failed decoding message: " + ret
				};
				return TransportReturnCode.FAILURE;
			}
		}

		private TransportReturnCode HandleUpdate(Msg msg, DecodeIterator dIter, DataDictionary dictionary)
		{
			return (TransportReturnCode)Decode(msg, dIter, dictionary);
		}

		private CodecReturnCode Decode(Msg msg, DecodeIterator dIter, DataDictionary dictionary)
		{
			CodecReturnCode ret;

			fieldValue.Clear();
			GetItemName(msg, fieldValue);
			Console.WriteLine(fieldValue);

			if (msg.MsgClass == MsgClasses.REFRESH)
				Console.WriteLine((((IRefreshMsg)msg).State).ToString());
			else
				Console.WriteLine();

			ret = DecodeFieldList(dIter, dictionary, fieldList, fieldEntry);

			Console.WriteLine();
			return ret;
		}

		private void GetItemName(Msg msg, StringBuilder fieldValue)
		{
			IMsgKey key = msg.MsgKey;

			if (key != null && key.CheckHasName())
			{
				if (msg.MsgClass == MsgClasses.REFRESH)
				{
					IRefreshMsg refreshMsg = msg;
					fieldValue.Append(key.Name.ToString() + (refreshMsg.CheckPrivateStream() ? " (PRIVATE STREAM)" : "") + "\nDOMAIN: " + DomainTypes.ToString(msg.DomainType) + "\n");
				}
				else
				{
					fieldValue.Append(key.Name.ToString() + "\nDOMAIN: " + DomainTypes.ToString(msg.DomainType) + "\n");
					if (msg.MsgClass == MsgClasses.UPDATE)
					{
						fieldValue.Append("UPDATE TYPE: " + UpdateEventTypes.ToString(((IUpdateMsg)msg).UpdateType) + "\n");
					}
				}
			}
			else
			{
				WatchListEntry? wle = watchList.Get(msg.StreamId);

				if (wle != null)
				{
					fieldValue.Append(wle.ItemName
							+ (wle.IsPrivateStream ? " (PRIVATE STREAM)" : " ")
							+ "\nDOMAIN: " + DomainTypes.ToString(msg.DomainType)
							+ "\n");
					if (msg.MsgClass == MsgClasses.UPDATE)
					{
						fieldValue.Append("UPDATE TYPE: "
								+ UpdateEventTypes.ToString(((IUpdateMsg)msg).UpdateType) + "\n");
					}
				}
				else
				{
					// Check if this is login stream for offstream posting
					if (msg.StreamId == LoginHandler.LOGIN_STREAM_ID)
					{
						fieldValue.Append("OFFPOST " + "\nDOMAIN: "
								+ DomainTypes.ToString(msg.DomainType) + "\n");
					}
				}
			}
		}

		private CodecReturnCode DecodeFieldList(DecodeIterator dIter, DataDictionary dictionary, FieldList localFieldList, FieldEntry localFieldEntry)
		{
			localFieldList.Clear();

			CodecReturnCode ret = localFieldList.Decode(dIter, localFieldSetDefDb);
			if (ret < CodecReturnCode.SUCCESS)
			{
				Console.WriteLine("DecodeFieldList() failed with return code: " + ret);
				return ret;
			}

			localFieldEntry.Clear();

			indentCount++;

			while ((ret = localFieldEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
			{
				if (ret < CodecReturnCode.SUCCESS)
				{
					Console.WriteLine("DecodeFieldEntry() failed with return code: " + ret);
					return ret;
				}
				IDictionaryEntry dictionaryEntry = dictionary.Entry(localFieldEntry.FieldId);

				if (dictionaryEntry == null)
				{
					Console.WriteLine("\tFid " + localFieldEntry.FieldId
							+ " not found in dictionary");
					Console.WriteLine(localFieldEntry.EncodedData.ToHexString());
					return CodecReturnCode.SUCCESS;
				}

				Console.Write(indents[indentCount] + dictionaryEntry.GetAcronym().ToString());
				for (int i = 0; i < 40 - indents[indentCount].Length - dictionaryEntry.GetAcronym().Length; i++)
				{
					Console.Write(" ");
				}

				int dataType = dictionaryEntry.GetRwfType();

				switch (dataType)
				{
					case DataTypes.VECTOR:
						ret = DecodeVector(dIter, dictionary);
						if (ret < CodecReturnCode.SUCCESS)
						{
							Console.WriteLine("DecodeVector inside FieldList failed");
							return ret;
						}
						break;
					case DataTypes.ARRAY:
						ret = DecodeArray(dIter);
						if (ret < CodecReturnCode.SUCCESS)
						{
							Console.WriteLine("DecodeArray inside FieldList failed");
							return ret;
						}
						break;
					default:
						ret = DecodePrimitive(dIter, dataType, false);
						if (ret < CodecReturnCode.SUCCESS)
						{
							Console.WriteLine("DecodePrimitive inside FieldList failed");
							return ret;
						}
						break;
				}
			}
			indentCount--;
			return CodecReturnCode.SUCCESS;
		}

		private CodecReturnCode DecodeVector(DecodeIterator dIter, DataDictionary dictionary)
		{
			CodecReturnCode ret = fidVectorValue.Decode(dIter);
			if (ret < CodecReturnCode.SUCCESS)
			{
				Console.WriteLine("DecodeReal() failed: <" + ret + ">");
				return ret;
			}
			if (ret == CodecReturnCode.NO_DATA)
			{
				Console.WriteLine("<no data>");
				return CodecReturnCode.SUCCESS;
			}
			if (fidVectorValue.CheckHasSummaryData())
			{
				Console.WriteLine();	
				ret = DecodeFieldList(dIter, dictionary, embeddedFieldList, embeddedFieldEntry);
				if (ret < CodecReturnCode.SUCCESS)
				{
					Console.WriteLine("DecodeSummaryData failed: <" + ret + ">");
					return ret;
				}
			}
		
			if (fidVectorValue.CheckHasSetDefs())
			{
				if (fidVectorValue.ContainerType == DataTypes.FIELD_LIST)
				{
					localFieldSetDefDb.Clear();
					ret = localFieldSetDefDb.Decode(dIter);
					if (ret < CodecReturnCode.SUCCESS)
					{
						Console.WriteLine("DecodeSetDefDb() failed: <" + ret + ">");
						return ret;
					}
				}
			}

			indentCount++;
			Console.WriteLine();

			vectorEntry.Clear();
			while ((ret = vectorEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
			{
				if (ret < CodecReturnCode.SUCCESS)
				{
					Console.Write("Error while decoding VectorEntry: " + ret);
					return ret;
				}

				Console.WriteLine(indents[indentCount] + "INDEX: " + vectorEntry.Index);
				Console.Write(indents[indentCount] + "ACTION: ");
				switch (vectorEntry.Action)
				{
					case VectorEntryActions.UPDATE:
						Console.WriteLine("UPDATE_ENTRY");
						break;
					case VectorEntryActions.SET:
						Console.WriteLine("SET_ENTRY");
						break;
					case VectorEntryActions.CLEAR:
						Console.WriteLine("CLEAR_ENTRY");
						break;
					case VectorEntryActions.INSERT:
						Console.WriteLine("INSERT_ENTRY");
						break;
					case VectorEntryActions.DELETE:
						Console.WriteLine("DELETE_ENTRY");
						break;
					default:
						Console.WriteLine("UNKNOWN");
						break;
				}

				// Continue decoding vector entries.
				switch (fidVectorValue.ContainerType)
				{
					case DataTypes.FIELD_LIST:
						// fieldList inside vectorEntry within vector
						ret = DecodeFieldList(dIter, dictionary, embeddedFieldList, embeddedFieldEntry);
						if (ret < CodecReturnCode.SUCCESS)
						{
							Console.Write("Error encountered with decoding FieldList within Vector: " + ret);
							return ret;
						}
						break;
					case DataTypes.ARRAY:
						ret = DecodeArray(dIter);
						if (ret < CodecReturnCode.SUCCESS)
						{
							Console.Write("Error encountered with decoding ARRAY within Vector: " + ret);
							return ret;
						}
						break;
					default:
						Console.WriteLine("Error: Vector contained unhandled containerType " + fidVectorValue.ContainerType);
						break;
				}

			}
			indentCount--;
			return CodecReturnCode.SUCCESS;
		}

		private CodecReturnCode DecodePrimitive(DecodeIterator dIter, int dataType, bool isArray)
		{
			CodecReturnCode ret = 0;

			switch (dataType)
			{
				case DataTypes.INT:
					ret = fidIntValue.Decode(dIter);
					if (ret == CodecReturnCode.SUCCESS)
					{
						Console.Write(fidIntValue.ToLong());
					}
					else if (ret != CodecReturnCode.BLANK_DATA)
					{
						Console.WriteLine("DecodeInt() failed: <" + ret + ">");
						return ret;
					}
					break;
				case DataTypes.REAL:
					ret = fidRealValue.Decode(dIter);
					if (ret == CodecReturnCode.SUCCESS)
					{
						Console.Write(fidRealValue.ToDouble());
					}
					else if (ret != CodecReturnCode.BLANK_DATA)
					{
						Console.WriteLine("DecodeReal() failed: <" + ret + ">");
						return ret;
					}
					break;
				case DataTypes.DATE:
					ret = fidDateValue.Decode(dIter);
					if (ret == CodecReturnCode.SUCCESS)
					{
						Console.Write(fidDateValue.ToString());

					}
					else if (ret != CodecReturnCode.BLANK_DATA)
					{
						Console.WriteLine("DecodeDate() failed: <" + ret + ">");
						return ret;
					}
					break;
				case DataTypes.TIME:
					ret = fidTimeValue.Decode(dIter);
					if (ret == CodecReturnCode.SUCCESS)
					{
						Console.Write(fidTimeValue.ToString());
					}
					else if (ret != CodecReturnCode.BLANK_DATA)
					{
						Console.WriteLine("DecodeTime() failed: <" + ret + ">");
						return ret;
					}
					break;
				case DataTypes.DATETIME:
					ret = fidDateTimeValue.Decode(dIter);
					if (ret == CodecReturnCode.SUCCESS)
					{
						Console.Write(fidDateTimeValue.ToString());
					}
					else if (ret != CodecReturnCode.BLANK_DATA)
					{
						Console.WriteLine("DecodeDateTime() failed: <" + ret + ">");
						return ret;
					}
					break;
				case DataTypes.ARRAY:
					ret = DecodeArray(dIter);
					if (ret < CodecReturnCode.SUCCESS)
					{
						Console.Write("Error encountered with decoding ARRAY was primitive: " + ret);
						return ret;
					}
					break;
				case DataTypes.BUFFER:
				case DataTypes.ASCII_STRING:
				case DataTypes.UTF8_STRING:
				case DataTypes.RMTES_STRING:
					ret = fidBufferValue.Decode(dIter);
					if (ret == CodecReturnCode.SUCCESS)
					{
						if (isArray)
							Console.Write("\"");
						Console.Write(fidBufferValue.ToString());
						if (isArray)
							Console.Write("\"");
					}
					else if (ret != CodecReturnCode.BLANK_DATA)
					{
						Console.WriteLine("DecodeString() failed: <" + ret + ">");
						return ret;
					}
					break;
				default:
					Console.Write("Unsupported data type (" + DataTypes.ToString(dataType) + ")");
					break;
			}
			if (ret == CodecReturnCode.BLANK_DATA)
			{
				Console.Write("<blank data>");
			}

			if (!isArray)
				Console.Write("\n");

			return CodecReturnCode.SUCCESS;
		}

		private CodecReturnCode DecodeArray(DecodeIterator dIter)
		{
			bool firstArrayEntry = true;

			Console.WriteLine("{ ");

			fidArrayValue.Clear();
			CodecReturnCode ret = fidArrayValue.Decode(dIter);
			if (ret < CodecReturnCode.SUCCESS)
			{
				Console.WriteLine("DecodeArray() failed: <" + ret + ">");
				return ret;
			}

			int dataType = fidArrayValue.PrimitiveType;

			arrayEntry.Clear();
			while ((ret = arrayEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
			{
				if (ret < CodecReturnCode.SUCCESS)
				{
					Console.WriteLine("Error encountered with DecodeArrayEntry: " + ret);
					return ret;
				}

				if (firstArrayEntry)
					firstArrayEntry = false;
				else
					Console.WriteLine(", ");
				ret = DecodePrimitive(dIter, dataType, true);
				if (ret < CodecReturnCode.SUCCESS)
				{
					Console.WriteLine("Error encountered with DecodeArrayEntryPrimitives: " + ret);
					return ret;
				}
			}

			Console.WriteLine(" }\n");

			return CodecReturnCode.SUCCESS;
		}

		/// <summary>
		/// Close all item streams.
		/// </summary>
		/// <param name="chnl">The channel to send a item stream close to</param>
		/// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
		/// <returns><see cref="TransportReturnCode"/> value</returns>
		public TransportReturnCode CloseStreams(ChannelSession chnl, out Error? error)
		{
			TransportReturnCode ret = 0;

			List<StreamIdKey> itemsToRemove = new List<StreamIdKey>();
			var enumerator = watchList.GetEnumerator();
			while (enumerator.MoveNext())
            {
				var entry = (KeyValuePair<StreamIdKey, WatchListEntry>)enumerator.Current;
				if (entry.Value.ItemState!.IsFinal())
                {
					continue;
                }
				if (entry.Value.DomainType == domainType)
                {
					ret = CloseStream(chnl, entry.Key.StreamId, out error);
					if (ret < TransportReturnCode.SUCCESS)
					{
						return ret;
					}
					itemsToRemove.Add(entry.Key);
				}
            }
			itemsToRemove.ForEach(item => watchList.Remove(item.StreamId));

			error = null;
			return TransportReturnCode.SUCCESS;
		}

		private void GenerateRequest(YieldCurveRequest yieldCurveRequest, bool isPrivateStream, Service srcDirInfo, LoginRefresh loginInfo)
		{
			yieldCurveRequest.Clear();

			if (!SnapshotRequested)
				yieldCurveRequest.Streaming = true;
			yieldCurveRequest.HasServiceId = true;
			yieldCurveRequest.ServiceId = srcDirInfo.ServiceId;
			yieldCurveRequest.HasPriority = true;
			yieldCurveRequest.PriorityCount = 1;
			yieldCurveRequest.PriorityClass = 1;
			yieldCurveRequest.HasQos = true;
			srcDirInfo.Info.QosList[0].Copy(yieldCurveRequest.Qos);
			if (isPrivateStream)
				yieldCurveRequest.IsPrivateStream = true;
		}

		private TransportReturnCode RedirectToPrivateStream(int streamId, out Error? error)
		{
			WatchListEntry? wle = watchList.Get(streamId);
			RemoveYieldCurveItemEntry(streamId);
			int psStreamId = watchList.Add(domainType, wle!.ItemName!, true);

			GenerateRequest(yieldCurveRequest, true, redirectSrcDirInfo!, redirectLoginInfo!);
			yieldCurveRequest.ItemNames.Add(wle.ItemName!);
			yieldCurveRequest.StreamId = psStreamId;
			return EncodeAndSendRequest(redirectChnl!, yieldCurveRequest, out error);
		}

	}
}
