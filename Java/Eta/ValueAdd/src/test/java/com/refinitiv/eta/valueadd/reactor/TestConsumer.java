/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.io.IOException;
import java.nio.channels.CancelledKeyException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Set;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;

public class TestConsumer implements ConsumerCallback,
ReactorJsonConversionEventCallback, ReactorServiceNameToIdCallback
{
	private final String FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
	private final String ENUM_TABLE_FILE_NAME = "enumtype.def";
	
	private Reactor reactor;
	private ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
	private ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
	private ReactorDispatchOptions dispatchOptions = ReactorFactory.createReactorDispatchOptions();

	private Selector selector;
	
	long cacheTime;
	long cacheInterval;
	long statisticTime;
	long statisticInterval;
	StringBuilder cacheDisplayStr;
	Buffer cacheEntryBuffer;
	
	private Error error;    // error information
	
	private DataDictionary dictionary;
	
	boolean closeHandled;

	private boolean fieldDictionaryLoadedFromFile;
	private boolean enumTypeDictionaryLoadedFromFile;
	
	private ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
	
	ArrayList<TestChannelInfo> chnlInfoList = new ArrayList<TestChannelInfo>();

	boolean stopConsumer = false;


	@Override
	public int reactorChannelEventCallback(ReactorChannelEvent event) {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int defaultMsgCallback(ReactorMsgEvent event) {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int rdmLoginMsgCallback(RDMLoginMsgEvent event) {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int rdmDirectoryMsgCallback(RDMDirectoryMsgEvent event) {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int rdmDictionaryMsgCallback(RDMDictionaryMsgEvent event) {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int reactorServiceNameToIdCallback(ReactorServiceNameToId serviceNameToId,
			ReactorServiceNameToIdEvent serviceNameToIdEvent) {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int reactorJsonConversionEventCallback(ReactorJsonConversionEvent jsonConversionEvent) {
		// TODO Auto-generated method stub
		return 0;
	}
	
	/* Load dictionary from file. */
	void loadDictionary()
	{
		dictionary.clear();
		if (dictionary.loadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, error) < 0)
		{
			System.out.println("Unable to load field dictionary.  Will attempt to download from provider.\n\tText: "
							   + error.text());
		}
		else
		{
			fieldDictionaryLoadedFromFile = true;
		}

		if (dictionary.loadEnumTypeDictionary(ENUM_TABLE_FILE_NAME, error) < 0)
		{
			System.out.println("Unable to load enum dictionary.  Will attempt to download from provider.\n\tText: "
							   + error.text());
		}
		else
		{
			enumTypeDictionaryLoadedFromFile = true;
		}
	}
	
	/* Initializes the Test Consumer. */
	private void init(ReactorOptions reactorOptions, TestChannelInfo chnlInfo)
	{
		// display product version information
		System.out.println(Codec.queryVersion().toString());
		System.out.println("Consumer initializing...");

		// load dictionary
		loadDictionary();
		
		this.reactorOptions = reactorOptions;

		// create reactor
		reactor = ReactorFactory.createReactor(reactorOptions, errorInfo);
		if (errorInfo.code() != ReactorReturnCodes.SUCCESS)
		{
			System.out.println("createReactor() failed: " + errorInfo.toString());
			System.exit(ReactorReturnCodes.FAILURE);
		}

		// register selector with reactor's reactorChannel.
		try
		{
			reactor.reactorChannel().selectableChannel().register(selector,
					SelectionKey.OP_READ,
					reactor.reactorChannel());
		}
		catch (ClosedChannelException e)
		{
			System.out.println("selector register failed: " + e.getLocalizedMessage());
			System.exit(ReactorReturnCodes.FAILURE);
		}

		/* Connect channels for each connection specified */
		
		// connect channel
		int ret;
		if ((ret = reactor.connect(chnlInfo.connectOptions, chnlInfo.consumerRole, errorInfo)) < ReactorReturnCodes.SUCCESS)
		{
			System.out.println("Reactor.connect failed with return code: " + ret + " error = " + errorInfo.error().text());
			System.exit(ReactorReturnCodes.FAILURE);
		}

		// add to ChannelInfo list
		chnlInfoList.add(chnlInfo);
	}

	/* Runs the Test Consumer. */
	private void run()
	{
		int selectRetVal, selectTime = 1000;
		while (true)
		{
			Set<SelectionKey> keySet = null;
			try
			{
				selectRetVal = selector.select(selectTime);
				if (selectRetVal > 0)
				{
					keySet = selector.selectedKeys();
				}
			}
			catch (IOException e)
			{
				System.out.println("select failed: " + e.getLocalizedMessage());
				System.exit(ReactorReturnCodes.FAILURE);
			}

			long currentTime = System.currentTimeMillis();
			if (currentTime >= cacheTime && cacheInterval > 0)
			{
				cacheTime = System.currentTimeMillis() + cacheInterval*1000;

				cacheTime = currentTime + cacheInterval*1000;
			}

			if (currentTime >= statisticTime && statisticInterval > 0)
			{
				statisticTime = System.currentTimeMillis() + statisticInterval*1000;

				TestChannelInfo chnlInfo = chnlInfoList.get(0);

				statisticTime = currentTime + statisticInterval*1000;
			}


			// nothing to read
			if (keySet != null)
			{
				Iterator<SelectionKey> iter = keySet.iterator();
				int ret = ReactorReturnCodes.SUCCESS;
				while (iter.hasNext())
				{
					SelectionKey key = iter.next();
					iter.remove();
					try
					{
						if (key.isReadable())
						{
							// retrieve associated reactor channel and dispatch on that channel
							ReactorChannel reactorChnl = (ReactorChannel)key.attachment();


							// dispatch until no more messages
							while ((ret = reactorChnl.dispatch(dispatchOptions, errorInfo)) > 0) {}
							if (ret == ReactorReturnCodes.FAILURE)
							{
								if (reactorChnl.state() != ReactorChannel.State.CLOSED &&
									reactorChnl.state() != ReactorChannel.State.DOWN_RECONNECTING)
								{
									System.out.println("ReactorChannel dispatch failed: " + ret + "(" + errorInfo.error().text() + ")");
									uninitialize();
									System.exit(ReactorReturnCodes.FAILURE);
								}
							}
						}
					}
					catch (CancelledKeyException e)
					{
					} // key can be canceled during shutdown
				}
			}

			// Handle stopping consumer
			if (stopConsumer)
			{
				System.out.println("Consumer has been stopped, closing now...");
				closeHandled = true;
				break;
			}
			
			if (!closeHandled)
			{
				// send login reissue if login reissue time has passed
				for (TestChannelInfo chnlInfo : chnlInfoList)
				{
					if (chnlInfo.reactorChannel == null ||
						(chnlInfo.reactorChannel.state() != ReactorChannel.State.UP &&
						 chnlInfo.reactorChannel.state() != ReactorChannel.State.READY))
					{
						continue;
					}

					if (chnlInfo.canSendLoginReissue &&
						System.currentTimeMillis() >= chnlInfo.loginReissueTime)
					{
						LoginRequest loginRequest = chnlInfo.consumerRole.rdmLoginRequest();
						submitOptions.clear();
						if (chnlInfo.reactorChannel.submit(loginRequest, submitOptions, errorInfo) !=  CodecReturnCodes.SUCCESS)
						{
							System.out.println("Login reissue failed. Error: " + errorInfo.error().text());
						}
						else
						{
							System.out.println("Login reissue sent");
						}
						chnlInfo.canSendLoginReissue = false;
					}
				}
			}
		}
	}

	/* Uninitializes the Test Consumer. */
	private void uninitialize()
	{
		System.out.println("Consumer unitializing and exiting...");

		for (TestChannelInfo chnlInfo : chnlInfoList)
		{
			// close ReactorChannel
			if (chnlInfo.reactorChannel != null)
			{
				chnlInfo.reactorChannel.close(errorInfo);
			}
		}

		// shutdown reactor
		if (reactor != null)
		{
			reactor.shutdown(errorInfo);
		}
	}
	
	public void startConsumer(ReactorOptions reactorOptions, TestChannelInfo chnlInfo)
	{
		TestConsumer consumer = new TestConsumer();
		consumer.init(reactorOptions, chnlInfo);
		consumer.run();
		consumer.uninitialize();
	}
}
