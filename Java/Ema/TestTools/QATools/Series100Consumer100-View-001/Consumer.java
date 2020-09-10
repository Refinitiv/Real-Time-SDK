///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.rtsdk.ema.examples.training.consumer.series100.example100__MarketPrice__Streaming;

import com.rtsdk.ema.access.Msg;
import com.rtsdk.ema.access.AckMsg;
import com.rtsdk.ema.access.GenericMsg;
import com.rtsdk.ema.access.RefreshMsg;
import com.rtsdk.ema.access.ReqMsg;
import com.rtsdk.ema.access.StatusMsg;
import com.rtsdk.ema.access.UpdateMsg;
import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.OmmConsumer;
import com.rtsdk.ema.access.OmmConsumerClient;
import com.rtsdk.ema.access.OmmConsumerConfig;
import com.rtsdk.ema.access.OmmConsumerEvent;
import com.rtsdk.ema.access.OmmException;
import com.rtsdk.ema.access.ElementList;
import com.rtsdk.ema.access.OmmArray;
import com.rtsdk.ema.rdm.EmaRdm;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.concurrent.ThreadLocalRandom;


class AppClient implements OmmConsumerClient
{
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println(refreshMsg);
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		System.out.println("updateMsg");
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		System.out.println(statusMsg);
	}

	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}
}


public class Consumer 
{
	public static int[] getView() {
		int size = ThreadLocalRandom.current().nextInt(1,16);
		int a[] = new int[size];
		for (int k = 0; k < size; ++k) {
			int v = ThreadLocalRandom.current().nextInt(10, 41);
			a[k] = v;
		}
		return a;
	}
	
	public static String[] getElementView() { // vector of fid name
		final String[] stringFids = {
			"BID",
			"ASK",
			"DSPLY_NAME",
			"RDN_EXCHD2",
			"TRDPRC_1",
			"TRDPRC_2",
			"TRDPRC_3",
			"TRDPRC_4",
			"TRDPRC_5",
			"HIGH_1",
			"LOW_1",
			"CURRENCY",
			"TRADE_DATE",
			"NEWS",
			"NEWS_TIME",
			"BIDSIZE",
			"ASKSIZE",
			"ACVOL_1",
			"BLKCOUNT",
			"BLKVOLUM",
			"OPEN_BID",
			"OPEN_ASK",
			"CLOSE_BID",
			"CLOSE_ASK",
			"YIELD",
			"EARNINGS",
			"PERATIO",
			"DIVIDENDTP",
			"DIVPAYDATE",
			"EXDIVDATE",
			"CTS_QUAL",
		};
		
		int size = ThreadLocalRandom.current().nextInt(1,21);
		String[] view = new String[size];
		int stringFidsSize = stringFids.length;
		for (int i = 0; i < size; ++i)
			view[i] = stringFids[ThreadLocalRandom.current().nextInt(1, stringFidsSize)];
		return view;	
	}

	public static void printView(String text, int[] a) {
		Arrays.sort(a);  // we are done with this by this point
		System.out.print("array sorted sans duplicates (text: " + text + "): ");
		int previous = 0; // non-legal
		for (int I : a)
			if (I != previous) {
				System.out.print(I + " ");
				previous = I;
			}
		System.out.println();
	}

	public static void main(String[] args)
	{
		boolean sendingFidsAsStrings = false;
		for (int i = 0; i < args.length; ++i ) {
			if (args[i].equals("-e"))
				sendingFidsAsStrings = true;
		}

		OmmConsumer consumer = null;
		try
		{
			AppClient appClient = new AppClient();
			
			OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig("EmaConfig.xml");
			
			consumer  = EmaFactory.createOmmConsumer(config.username("user"));			
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();

			ElementList view = EmaFactory.createElementList();
			OmmArray array = EmaFactory.createOmmArray();

			Long handle;
			array.clear();
			view.clear();
			reqMsg.clear();
			int requestCount = 0;
			int reissueCount = 0;

			ArrayList<Long> handles = new ArrayList<Long>();
			int fids[];
			String fidsAsStrings[];
			int events = 0;
			while (events < 60) {
				int selector = ThreadLocalRandom.current().nextInt(0,20); // 0, 1, ..., 19
				if (selector < 7) { // unregister
					if (handles.size() < 3)
						continue;
					++events;
					int itemToUnregister = ThreadLocalRandom.current().nextInt(0, handles.size());
					System.out.println("event: removing handle " + handles.get(itemToUnregister));
					consumer.unregister(handles.get(itemToUnregister));
					handles.remove(itemToUnregister);
				}
				else if (selector < 12) {  // reissue
					if (handles.isEmpty())
						continue;
					++events;
					array.clear();
					
					String eventMsg = "event: requesting fids for reissue " + ++reissueCount + ": ";
					if (sendingFidsAsStrings) {
						fidsAsStrings = getElementView();
						for (String S : fidsAsStrings) {
							array.add(EmaFactory.createOmmArrayEntry().ascii(S));
							eventMsg += S + " ";
						}
					}
					else {
						fids = getView();
						for (int I : fids) {
							array.add(EmaFactory.createOmmArrayEntry().intValue(I));
							eventMsg += I + " ";
						}
					}
					System.out.println(eventMsg);

					view.clear();
					view.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_VIEW_TYPE, (sendingFidsAsStrings == true ? 2 : 1)));
					view.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_VIEW_DATA, array));
					reqMsg.clear();
					reqMsg.payload(view);

					int itemToReissue = ThreadLocalRandom.current().nextInt(0, handles.size());
					System.out.println("event: reissue for handle " + handles.get(itemToReissue));
					consumer.reissue(reqMsg, handles.get(itemToReissue));
					Thread.sleep(10000);
				}
				else {			// registerClient
					++events;
					array.clear();
					view.clear();
					reqMsg.clear();
					String eventMsg = "event: requesting fids for request " + ++requestCount + ": ";
					if (sendingFidsAsStrings) {	
						fidsAsStrings = getElementView();
						for (String S : fidsAsStrings) {
							array.add(EmaFactory.createOmmArrayEntry().ascii(S));
							eventMsg += S + " ";
						}
					}
					else {
						fids = getView();
						for (int I : fids) {
							array.add(EmaFactory.createOmmArrayEntry().intValue(I));
							eventMsg += I + " ";
						}
					}
					System.out.println(eventMsg);
					
					view.clear();
					view.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_VIEW_TYPE, (sendingFidsAsStrings == true ? 2 : 1)));
					view.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_VIEW_DATA, array));

					reqMsg.serviceName("ELEKTRON_DD").name("IBM.N").hasView();
					reqMsg.interestAfterRefresh(true);
					reqMsg.payload(view);
					handle = consumer.registerClient(reqMsg, appClient);
					System.out.println("event: handle " + handle + " created");
					handles.add(handle);
					Thread.sleep(10000);
				}
			}

			for (int i = 0; i < handles.size(); ++i) {
				System.out.println("event: removing handle (end) " + handles.get(i));
				consumer.unregister(handles.get(i));
				Thread.sleep(10000);
			}
		} 
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally 
		{
			if (consumer != null) consumer.uninitialize();
		}
	}
}


