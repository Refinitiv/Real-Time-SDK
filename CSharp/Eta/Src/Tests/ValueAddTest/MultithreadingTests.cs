/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using Xunit;
using System.Collections.Generic;

using System;
using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.Rdm;
using Buffer = LSEG.Eta.Codec.Buffer;
using Array = LSEG.Eta.Codec.Array;
using LSEG.Eta.Common;
using System.Threading;
using AspectInjector.Broker;

namespace LSEG.Eta.ValuedAdd.Tests;

[Collection("ValueAdded")]
public class MultithreadingTests
{
	private static void DecodeViewDataForFieldId(ReactorChannel reactorChannel, IRequestMsg msg, List<int> viewFieldIdList)
	{
		DecodeIterator decodeIt = new();
		ElementList elementList = new ElementList();
		ElementEntry elementEntry = new ElementEntry();
		UInt viewType = new();
		Int fieldId = new();
		Array viewArray = new();
		ArrayEntry viewEntry = new();
		Buffer viewData = new Buffer();
		CodecReturnCode ret;

		viewFieldIdList.Clear();
		Assert.Equal(DataTypes.ELEMENT_LIST, msg.ContainerType);

		decodeIt.Clear();
		decodeIt.SetBufferAndRWFVersion(msg.EncodedDataBody, reactorChannel.MajorVersion, reactorChannel.MinorVersion);

		elementList.Clear();
		Assert.True((ret = elementList.Decode(decodeIt, null)) == CodecReturnCode.SUCCESS);

		elementEntry.Clear();
		// Decoding the list of Field IDs
		while ((ret = elementEntry.Decode(decodeIt)) != CodecReturnCode.END_OF_CONTAINER)
		{
			Assert.False(ret < CodecReturnCode.SUCCESS);

			if (elementEntry.Name.Equals(ElementNames.VIEW_TYPE) || elementEntry.Name.Equals(ElementNames.VIEW_DATA))
			{
				switch (elementEntry.DataType)
				{
					case DataTypes.UINT:
						{
							Assert.True(viewType.Decode(decodeIt) == CodecReturnCode.SUCCESS);
							break;
						}
					case DataTypes.ARRAY:
						{
							viewData = elementEntry.EncodedData;
							break;
						}
				}
			}
		}

		decodeIt.Clear();
		decodeIt.SetBufferAndRWFVersion(viewData, reactorChannel.MajorVersion,
				reactorChannel.MinorVersion);

		// Support only as a list of Field IDs
		Assert.True(viewType.ToLong() == ViewTypes.FIELD_ID_LIST);
		viewArray.Clear();
		Assert.True(viewArray.Decode(decodeIt) == CodecReturnCode.SUCCESS);
		Assert.True(viewArray.PrimitiveType == DataTypes.INT);
		while ((ret = viewEntry.Decode(decodeIt)) != CodecReturnCode.END_OF_CONTAINER)
		{
			Assert.False(ret < CodecReturnCode.SUCCESS);
			if ((ret = fieldId.Decode(decodeIt)) == CodecReturnCode.SUCCESS)
			{
				Assert.False(fieldId.ToLong() < short.MinValue || fieldId.ToLong() > short.MaxValue);
				viewFieldIdList.Add((int)fieldId.ToLong());
			}
		}
	}

	private static void EncodeViewFieldIdList(ReactorChannel reactorChannel, List<int> fieldIdList, IRequestMsg msg)
	{
		Buffer buf = new();
		buf.Data(new ByteBuffer(1024));
		Int tempInt = new();
		UInt tempUInt = new();
		Array viewArray = new();
		EncodeIterator encodeIter = new();
		encodeIter.SetBufferAndRWFVersion(buf, reactorChannel.MajorVersion, reactorChannel.MinorVersion);
		ElementList elementList = new();
		ElementEntry elementEntry = new();
		ArrayEntry arrayEntry = new();

		elementList.ApplyHasStandardData();
		Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(encodeIter, null, 0));

		elementEntry.Clear();
		elementEntry.Name = ElementNames.VIEW_TYPE;
		elementEntry.DataType = DataTypes.UINT;

		tempUInt.Value(ViewTypes.FIELD_ID_LIST);

		Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(encodeIter, tempUInt));

		elementEntry.Clear();
		elementEntry.Name = ElementNames.VIEW_DATA;
		elementEntry.DataType = DataTypes.ARRAY;
		Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.EncodeInit(encodeIter, 0));
		viewArray.PrimitiveType = DataTypes.INT;
		viewArray.ItemLength = 2;

		Assert.Equal(CodecReturnCode.SUCCESS, viewArray.EncodeInit(encodeIter));

		foreach (int viewField in fieldIdList)
		{
			arrayEntry.Clear();
			tempInt.Value(viewField);
			Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(encodeIter, tempInt));
		}
		Assert.Equal(CodecReturnCode.SUCCESS, viewArray.EncodeComplete(encodeIter, true));
		Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.EncodeComplete(encodeIter, true));
		Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(encodeIter, true));

		msg.ContainerType = DataTypes.ELEMENT_LIST;
		msg.EncodedDataBody = buf;
	}

	public class AppClient
	{
		public volatile int Handle;
		public string Item { get; private set; }

		public AppClient(string item)
		{
			Item = item;
			Handle = 0;
		}
	}

	public class MultithreadedOmmConsumer : Consumer
	{
		internal class ItemSubscriber
		{
			private int _streamID = 0;
			private Msg _msg = new Msg();
			private IRequestMsg _requestMsg;
			private ICloseMsg _closeMsg;
			private ReactorSubmitOptions _submitOptions = new ReactorSubmitOptions();
			private Consumer _consumer;
			internal ItemSubscriber(Consumer consumer, int streamID)
			{
				_consumer = consumer;
				_streamID = streamID;

				_requestMsg = (IRequestMsg)_msg;
				_closeMsg = (ICloseMsg)_msg;
			}

			internal void OpenRequest(string serviceName, string itemName, int domainType, bool streaming, List<int> viewFids)
			{
				_requestMsg.Clear();
				_requestMsg.MsgClass = MsgClasses.REQUEST;
				_requestMsg.StreamId = _streamID;
				_requestMsg.DomainType = domainType;

				if (streaming)
					_requestMsg.ApplyStreaming();

				_requestMsg.MsgKey.ApplyHasName();
				_requestMsg.MsgKey.Name.Data(itemName);

				if (viewFids != null)
				{
					_requestMsg.ApplyHasView();
					EncodeViewFieldIdList(_consumer.ReactorChannel, viewFids, _requestMsg);
				}

				_submitOptions.Clear();
				_submitOptions.ServiceName = serviceName;
				_submitOptions.RequestMsgOptions.UserSpecObj = itemName;

				Assert.True(_consumer.Submit((Msg)_requestMsg, _submitOptions) >= ReactorReturnCode.SUCCESS);
			}
			internal void CloseRequest()
			{
				_closeMsg.Clear();
				_closeMsg.MsgClass = MsgClasses.CLOSE;
				_closeMsg.ContainerType = DataTypes.NO_DATA;
				_closeMsg.StreamId = _streamID;
				_submitOptions.Clear();

				Assert.True(_consumer.Submit((Msg)_closeMsg, _submitOptions) >= ReactorReturnCode.SUCCESS);
			}
		}

		ReaderWriterLockSlim m_UserLock = new ReaderWriterLockSlim();
		private Dictionary<int, ItemSubscriber> m_ItemHashTable;
		private int m_NextStreamID = 5; // for atomic operations
		private Int intValue = new Int();
		private volatile bool m_Dispatching = false;
		private DecodeIterator m_DecodeIterator = new DecodeIterator();
		private FieldList m_FieldList = new FieldList();
		private FieldEntry m_FieldEntry = new FieldEntry();
		private List<int> m_ViewFids;
		
		public int NumRefreshMessage = 0;
		public int NumRefreshCompleteMessage = 0;

		private Thread m_RunnerThread;

		public MultithreadedOmmConsumer(TestReactor testReactor) : base(testReactor)
		{
			m_ItemHashTable = new Dictionary<int, ItemSubscriber>();
		}
		public void UnsubAndSub(AppClient appClient, string serviceName, int domainType, bool streaming, List<int> viewFids)
		{
			m_ViewFids = viewFids;
			if (appClient.Handle > 0)
			{
				Unsubscribe(appClient.Handle);
			}

			int handle = Subscribe(serviceName, appClient.Item, domainType, streaming, m_ViewFids);
			appClient.Handle = handle;
		}
		public int Subscribe(string serviceName, string itemName, int domainType, bool streaming, List<int> viewFids)
		{
			m_ViewFids = viewFids;
			m_UserLock.EnterWriteLock();

			int streamId = Interlocked.Increment(ref m_NextStreamID);
			ItemSubscriber item = new ItemSubscriber(this, streamId);

			try
			{
				item.OpenRequest(serviceName, itemName, domainType, streaming, m_ViewFids);
				m_ItemHashTable.Add(streamId, item);
			}
			finally
			{
				m_UserLock.ExitWriteLock();
			}

			return streamId;
		}
		public void Unsubscribe(int handle)
		{
			m_UserLock.EnterWriteLock();
			try
			{
				ItemSubscriber item = m_ItemHashTable[handle];

				if (item != null)
				{
					item.CloseRequest();
					m_ItemHashTable.Remove(handle);
				}
			}
			finally
			{
				m_UserLock.ExitWriteLock();
			}
		}
		public override ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent evt)
		{
			string closure = (string)(evt.StreamInfo.UserSpec);

            if(evt.Msg.MsgClass == MsgClasses.STATUS)
            {
                return ReactorCallbackReturnCode.SUCCESS;
            }

			Assert.True(evt.Msg.MsgClass == MsgClasses.REFRESH);
			IRefreshMsg refreshMsg = (IRefreshMsg)evt.Msg;

			++NumRefreshMessage;

			if (refreshMsg.CheckRefreshComplete())
				++NumRefreshCompleteMessage;

			/* Checks the requested and received item name must be the same */
			Assert.Equal(closure, refreshMsg.MsgKey.Name.ToString());

			if (m_ViewFids != null)
			{
				m_FieldList.Clear();
				m_FieldEntry.Clear();
				m_DecodeIterator.Clear();
				Assert.True(refreshMsg.ContainerType == DataTypes.FIELD_LIST);
				Assert.True(refreshMsg.EncodedDataBody != null);
				Assert.True(refreshMsg.EncodedDataBody.Data() != null);

				CodecReturnCode ret;
				int fieldCount = 0;

				Assert.True(m_DecodeIterator.SetBufferAndRWFVersion(refreshMsg.EncodedDataBody, evt.ReactorChannel.MajorVersion, evt.ReactorChannel.MinorVersion)
									== CodecReturnCode.SUCCESS);

				Assert.True(m_FieldList.Decode(m_DecodeIterator, null) == CodecReturnCode.SUCCESS);

				while ((ret = m_FieldEntry.Decode(m_DecodeIterator)) != CodecReturnCode.END_OF_CONTAINER)
				{
					Assert.True(ret == CodecReturnCode.SUCCESS);

					Assert.Contains(m_FieldEntry.FieldId, m_ViewFids);

					++fieldCount;
				}

				Assert.True(fieldCount == m_ViewFids.Count);
			}
			else
			{
				Assert.True(refreshMsg.ContainerType == DataTypes.NO_DATA);
			}

			return ReactorCallbackReturnCode.SUCCESS;
		}
		public void Start()
		{
			m_Dispatching = true;
			m_RunnerThread = new Thread(new ThreadStart(this.Run));
			m_RunnerThread.Start();
		}
		public void Stop()
		{
			m_Dispatching = false;
		}
		public void Run()
		{
            do
            {
                TestReactor.Dispatch(-1);

            } while (m_Dispatching);
        }
	}

	public interface TestWithMultithreadedOmmConsumer
	{
		public void RunTest(MultithreadedOmmConsumer consumer, 
			AppClient appClient, 
			string serviceName, 
			int domainType, 
			bool streaming, 
			List<int> viewFids);
	}

	public class ViewRequestProvider : Provider
	{
		private IRefreshMsg m_RefreshMsg = new Msg();
		private ReactorSubmitOptions m_SubmitOptions = new ReactorSubmitOptions();
		private DecodeIterator m_DecodeIter = new DecodeIterator();
		private ElementList m_ElementList = new ElementList();
		private ElementEntry m_ElementEntry = new ElementEntry();
		private UInt m_ViewType = new UInt();
		private Buffer m_ViewData = new Buffer();
		private Array m_ViewArray = new Array();
		private ArrayEntry m_ViewArrayEntry = new ArrayEntry();
		private Int m_FieldId = new Int();
		private List<int> m_ViewFieldIdList = new List<int>();
		private EncodeIterator m_EncodeIterator = new EncodeIterator();
		private FieldList m_FieldList = new FieldList();
		private FieldEntry m_FieldEntry = new FieldEntry();
		private Buffer m_Buffer = new Buffer();
		private Real m_Real = new Real();
		private volatile bool m_Dispatching = false;
		private bool m_Multipart;
		private int m_RefreshCompleteMessage = 0;
		public int NumRequestMessage = 0;
		public int NumRefreshMessage = 0;

		private Thread runnerThread;

		public ViewRequestProvider(TestReactor testReactor, bool multipart) : base(testReactor)
		{
			m_Multipart = multipart;
		}
		public void Start()
		{
			m_Dispatching = true;
			runnerThread = new Thread(new ThreadStart(this.Run));
			runnerThread.Start();
		}
		public void Stop()
		{
			m_Dispatching = false;
		}
		public override ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent evt)
		{
			CodecReturnCode ret;
			m_DecodeIter.Clear();
			m_ElementList.Clear();
			m_ElementEntry.Clear();
			m_ViewType.Clear();
			m_ViewData.Clear();
			m_ViewArray.Clear();
			m_ViewArrayEntry.Clear();
			m_FieldId.Clear();
			m_ViewFieldIdList.Clear();

			if (evt.Msg.MsgClass == MsgClasses.REQUEST)
			{
				++NumRequestMessage;
				IRequestMsg requestMsg = (IRequestMsg)evt.Msg;

				if (requestMsg.CheckHasView())
                {
					DecodeViewDataForFieldId(evt.ReactorChannel, requestMsg, m_ViewFieldIdList);
				}

				bool applyRefreshComplete = m_Multipart ? false : true;

				do
				{
					m_RefreshMsg.Clear();
					m_SubmitOptions.Clear();
					m_RefreshMsg.MsgClass = MsgClasses.REFRESH;
					m_RefreshMsg.ApplySolicited();
					m_RefreshMsg.ApplyClearCache();
					m_RefreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
					m_RefreshMsg.StreamId = evt.Msg.StreamId;
					m_RefreshMsg.ApplyHasMsgKey();
					m_RefreshMsg.MsgKey.ApplyHasServiceId();
					m_RefreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
					m_RefreshMsg.MsgKey.ApplyHasName();
					m_RefreshMsg.MsgKey.Name.Data(evt.Msg.MsgKey.Name.ToString());

					if (requestMsg.CheckStreaming())
					{
						m_RefreshMsg.State.StreamState(StreamStates.OPEN);
						m_RefreshMsg.State.DataState(DataStates.OK);
					}
					else
					{
						m_RefreshMsg.State.StreamState(StreamStates.NON_STREAMING);
						m_RefreshMsg.State.DataState(DataStates.SUSPECT);
					}

					m_EncodeIterator.Clear();
					m_FieldList.Clear();
					m_FieldEntry.Clear();
					m_Buffer.Clear();
					if (m_ViewFieldIdList.Count > 0)
					{
						ByteBuffer byteBuffer = new ByteBuffer(8192);
						m_Buffer.Data(byteBuffer);
						m_EncodeIterator.Clear();
						Assert.True(m_EncodeIterator.SetBufferAndRWFVersion(m_Buffer, evt.ReactorChannel.MajorVersion, evt.ReactorChannel.MinorVersion) == CodecReturnCode.SUCCESS);

						m_FieldList.Clear();
						m_FieldList.ApplyHasStandardData();
						ret = m_FieldList.EncodeInit(m_EncodeIterator, null, 0);

						Assert.False(ret < CodecReturnCode.SUCCESS);

						IEnumerator<int> it = m_ViewFieldIdList.GetEnumerator();
						while (it.MoveNext())
						{
							int fid = it.Current;
							m_FieldEntry.Clear();
							switch (fid)
							{
								case 6:
								case 12:
								case 13:
								case 19:
								case 21:
								case 22:
								case 25:
								case 30:
								case 31:
								case 1465:
									{
										m_FieldEntry.FieldId = fid;
										m_FieldEntry.DataType = DataTypes.REAL;
										m_Real.Clear();
										m_Real.Value(fid, RealHints.EXPONENT1);
										Assert.True(m_FieldEntry.Encode(m_EncodeIterator, m_Real) == CodecReturnCode.SUCCESS);
										break;
									}
								default:
									Assert.False(true); // Support only the above fields
									break;
							}
						}

						Assert.True(m_FieldList.EncodeComplete(m_EncodeIterator, true) == CodecReturnCode.SUCCESS);
						m_RefreshMsg.ContainerType = DataTypes.FIELD_LIST;
						m_RefreshMsg.EncodedDataBody = m_Buffer;
					}
					else
					{
						m_RefreshMsg.ContainerType = DataTypes.NO_DATA;
					}


					try
					{
						Thread.Sleep(10);
					}
					catch { }

					if (applyRefreshComplete)
					{
						++m_RefreshCompleteMessage;
						m_RefreshMsg.ApplyRefreshComplete();
					}

					++NumRefreshMessage;
					Assert.True(Submit((Msg)m_RefreshMsg, m_SubmitOptions) >= ReactorReturnCode.SUCCESS);

					if (applyRefreshComplete)
						break;
					else
						applyRefreshComplete = true;

				} while (m_Multipart);
			}

			return ReactorCallbackReturnCode.SUCCESS;
		}
		public void Run()
		{
			try
			{
				do
				{
					TestReactor.Dispatch(-1);

				} while (m_Dispatching);
			}
			catch { }
		}
	}

	public MultithreadedOmmConsumer MultithreadedSubscriptionTest(bool streaming, 
		string stockList, 
		List<int> viewFieldList, 
		int numIteration, 
		bool multipart, 
		TestWithMultithreadedOmmConsumer test)
	{
		/* Create consumer. */
		MultithreadedOmmConsumer consumer = new MultithreadedOmmConsumer(new TestReactor());
		ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
		consumerRole.InitDefaultRDMLoginRequest();
		consumerRole.InitDefaultRDMDirectoryRequest();
		consumerRole.ChannelEventCallback = consumer;
		consumerRole.LoginMsgCallback = consumer;
		consumerRole.DirectoryMsgCallback = consumer;
		consumerRole.DictionaryMsgCallback = consumer;
		consumerRole.DefaultMsgCallback = consumer;
		consumerRole.WatchlistOptions.EnableWatchlist = true;
		consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
		consumerRole.WatchlistOptions.RequestTimeout = 3000;

		/* Create provider. */
		ViewRequestProvider provider = new ViewRequestProvider(new TestReactor(), multipart);
		ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
		providerRole.ChannelEventCallback = provider;
		providerRole.LoginMsgCallback = provider;
		providerRole.DirectoryMsgCallback = provider;
		providerRole.DictionaryMsgCallback = provider;
		providerRole.DefaultMsgCallback = provider;

		/* Connect the consumer and provider. Setup login & directory streams automatically. */
		ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
		opts.SetupDefaultLoginStream = true;
		opts.SetupDefaultDirectoryStream = true;
		provider.Bind(opts);
		TestReactor.OpenSession(consumer, provider, opts);

		provider.Start();
		consumer.Start();

		/* Consumer sends request. */
		String[] stocks = stockList.Split(",");
        List<Thread> threads = new ();
		foreach (string stock in stocks)
		{
			AppClient appClient = new AppClient(stock);
            var thread = new Thread(new ThreadStart(() =>
            {
                for (int i = 0; i < numIteration; i++)
                {

                    test.RunTest(consumer,
                        appClient,
                        Provider.DefaultService.Info.ServiceName.ToString(),
                        (int)DomainType.MARKET_PRICE,
                        streaming,
                        viewFieldList
                        );
                }
            }));
            threads.Add(thread);
            thread.Start();
        }

        Thread.Sleep(15000); // Wait 15 seconds to complete all requests
        Thread.JoinAll(threads);
        consumer.Stop();
        provider.Stop();
        return consumer;
    }

	class MultithreadedUnsubAndSubTheSameItemListTest : TestWithMultithreadedOmmConsumer
	{
		public void RunTest(MultithreadedOmmConsumer consumer, AppClient appClient, string serviceName, int domainType, bool streaming, List<int> viewFids)
		{
			consumer.UnsubAndSub(appClient, serviceName, domainType, streaming, viewFids);
		}
	}

	[Fact]
	public void MultithreadedUnsubAndSubTheSameItemList_StreamingTest()
	{
		string stockList = "AEMN.SI,DBSM.SI,BOUS.SI,SGXL.SI,CMDG.SI";
		MultithreadedOmmConsumer consumer = MultithreadedSubscriptionTest(true, stockList, null, 30, false, new MultithreadedUnsubAndSubTheSameItemListTest());
		Assert.True(consumer.NumRefreshMessage == consumer.NumRefreshCompleteMessage);
	}

	[Fact]
	public void MultithreadedUnsubAndSubTheSameItemList_NonStreamingTest()
	{
		string stockList = "AEMN.SI,DBSM.SI,BOUS.SI,SGXL.SI,CMDG.SI";
		MultithreadedOmmConsumer consumer = MultithreadedSubscriptionTest(false, stockList, null, 30, false, new MultithreadedUnsubAndSubTheSameItemListTest());
		Assert.True(consumer.NumRefreshMessage == consumer.NumRefreshCompleteMessage);
	}

	[Fact]
	public void MultithreadedUnsubAndSubTheSameItemListWithView_StreamingTest()
	{
		List<int> viewFieldList = new List<int>();
		viewFieldList.Add(6);
		viewFieldList.Add(12);
		viewFieldList.Add(13);
		viewFieldList.Add(19);
		viewFieldList.Add(21);
		viewFieldList.Add(22);
		viewFieldList.Add(25);
		viewFieldList.Add(30);
		viewFieldList.Add(31);
		viewFieldList.Add(1465);

		string stockList = "AEMN.SI,DBSM.SI,BOUS.SI,SGXL.SI,CMDG.SI";
		MultithreadedOmmConsumer consumer = MultithreadedSubscriptionTest(true, stockList, viewFieldList, 30, false, new MultithreadedUnsubAndSubTheSameItemListTest());
		Assert.True(consumer.NumRefreshMessage == consumer.NumRefreshCompleteMessage);
	}

	[Fact]
	public void MultithreadedUnsubAndSubTheSameItemListWithView_NonStreamingTest()
	{
		List<int> viewFieldList = new List<int>();
		viewFieldList.Add(6);
		viewFieldList.Add(12);
		viewFieldList.Add(13);
		viewFieldList.Add(19);
		viewFieldList.Add(21);
		viewFieldList.Add(22);
		viewFieldList.Add(25);
		viewFieldList.Add(30);
		viewFieldList.Add(31);
		viewFieldList.Add(1465);

		string stockList = "AEMN.SI,DBSM.SI,BOUS.SI,SGXL.SI,CMDG.SI";
		MultithreadedOmmConsumer consumer = MultithreadedSubscriptionTest(false, stockList, viewFieldList, 30, false, new MultithreadedUnsubAndSubTheSameItemListTest());
		Assert.True(consumer.NumRefreshMessage == consumer.NumRefreshCompleteMessage);
	}

	internal class MultithreadedSubTheSameItemListTest : TestWithMultithreadedOmmConsumer
	{
		public void RunTest(MultithreadedOmmConsumer consumer,
			AppClient appClient,
			string serviceName,
			int domainType,
			bool streaming,
			List<int> viewFids)
		{
			consumer.Subscribe(serviceName, appClient.Item, domainType, streaming, viewFids);
		}
	}

	[Fact]
	public void MultithreadedSubTheSameItemList_StreamingTest()
	{
		string stockList = "AEMN.SI,DBSM.SI,BOUS.SI,SGXL.SI,CMDG.SI";
		MultithreadedOmmConsumer consumer = MultithreadedSubscriptionTest(true, stockList, null, 30, false, new MultithreadedSubTheSameItemListTest());
		Assert.True(consumer.NumRefreshMessage == (stockList.Split(",").Length * 30));
	}

	[Fact]
	public void MultithreadedSubTheSameItemList_MultiPartStreamingTest()
	{
		string stockList = "AEMN.SI,DBSM.SI,BOUS.SI,SGXL.SI,CMDG.SI";
		MultithreadedOmmConsumer consumer = MultithreadedSubscriptionTest(true, stockList, null, 30, true, new MultithreadedSubTheSameItemListTest());
        Assert.True(consumer.NumRefreshCompleteMessage == (stockList.Split(",").Length * 30));
		Assert.True(consumer.NumRefreshMessage == (consumer.NumRefreshCompleteMessage * 2));
	}

	[Fact]
	public void MultithreadedSubTheSameItemList_NonStreamingTest()
	{
		string stockList = "AEMN.SI,DBSM.SI,BOUS.SI,SGXL.SI,CMDG.SI,AEMN.SI,DBSM.SI,BOUS.SI,SGXL.SI,CMDG.SI";
		MultithreadedOmmConsumer consumer = MultithreadedSubscriptionTest(false, stockList, null, 30, false, new MultithreadedSubTheSameItemListTest());
		Assert.True(consumer.NumRefreshMessage == (stockList.Split(",").Length * 30));
	}

	[Fact]
	public void MultithreadedSubTheSameItemList_MultipartNonStreamingTest()
	{
		string stockList = "AEMN.SI,DBSM.SI,BOUS.SI,SGXL.SI,CMDG.SI,AEMN.SI,DBSM.SI,BOUS.SI,SGXL.SI,CMDG.SI";
		MultithreadedOmmConsumer consumer = MultithreadedSubscriptionTest(false, stockList, null, 30, true, new MultithreadedSubTheSameItemListTest());
		Assert.True(consumer.NumRefreshCompleteMessage == (stockList.Split(",").Length * 30));
		Assert.True(consumer.NumRefreshMessage == (consumer.NumRefreshCompleteMessage * 2));
	}

	[Fact]
	public void MultithreadedSubTheSameItemListWithView_StreamingTest()
	{
		List<int> viewFieldList = new List<int>();
		viewFieldList.Add(6);
		viewFieldList.Add(12);
		viewFieldList.Add(13);
		viewFieldList.Add(19);
		viewFieldList.Add(21);
		viewFieldList.Add(22);
		viewFieldList.Add(25);
		viewFieldList.Add(30);
		viewFieldList.Add(31);
		viewFieldList.Add(1465);

        string stockList = "AEMN.SI,DBSM.SI,BOUS.SI,SGXL.SI,CMDG.SI";
        MultithreadedOmmConsumer consumer = MultithreadedSubscriptionTest(true, stockList, viewFieldList, 4, false, new MultithreadedSubTheSameItemListTest());
		Assert.True(consumer.NumRefreshMessage == (stockList.Split(",").Length + (stockList.Split(",").Length * 2) + (stockList.Split(",").Length * 3)
				+ (stockList.Split(",").Length * 4)));
	}

	[Fact]
	public void MultithreadedSubTheSameItemListWithView_NonStreamingTest()
	{
		List<int> viewFieldList = new List<int>();
		viewFieldList.Add(6);
		viewFieldList.Add(12);
		viewFieldList.Add(13);
		viewFieldList.Add(19);
		viewFieldList.Add(21);
		viewFieldList.Add(22);
		viewFieldList.Add(25);
		viewFieldList.Add(30);
		viewFieldList.Add(31);
		viewFieldList.Add(1465);

		string stockList = "AEMN.SI,DBSM.SI,BOUS.SI,SGXL.SI,CMDG.SI,AEMN.SI,DBSM.SI,BOUS.SI,SGXL.SI,CMDG.SI";
		MultithreadedOmmConsumer consumer = MultithreadedSubscriptionTest(false, stockList, viewFieldList, 35, false, new MultithreadedSubTheSameItemListTest());
		Assert.Equal(stockList.Split(",").Length * 35, consumer.NumRefreshMessage);
	}
}
