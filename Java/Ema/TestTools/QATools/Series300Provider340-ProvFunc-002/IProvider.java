///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.iprovider.series300.ex340_MP_Posting;

import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmIProviderConfig;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmProviderClient;
import com.refinitiv.ema.access.OmmProviderEvent;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.examples.training.consumer.series500.ex510_RequestRouting_FileCfg.Consumer;
import com.refinitiv.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
	AppClient() {}

	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent)
	{
		switch(reqMsg.domainType())
		{
		case EmaRdm.MMT_LOGIN:
			processLoginRequest(reqMsg,providerEvent);
			break;
		case EmaRdm.MMT_MARKET_PRICE:
			processMarketPriceRequest(reqMsg,providerEvent);
			break;
		default:
			processInvalidItemRequest(reqMsg,providerEvent);
			break;
		}
	}
	
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent event) {}
	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event) {}
	public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent providerEvent) {}
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {
		System.out.println("Received PostMsg with id: " + postMsg.postId());
		System.out.println(postMsg);
		if(postMsg.solicitAck()) {
			AckMsg ackMsg = EmaFactory.createAckMsg();
			if(postMsg.hasSeqNum()){
				ackMsg.seqNum(postMsg.seqNum());
			}
			if(postMsg.hasName()){
				ackMsg.name(postMsg.name());
			}
			if(postMsg.hasServiceId()){
				ackMsg.serviceId(postMsg.serviceId());
			}
			ackMsg.ackId(postMsg.postId())
					.domainType(postMsg.domainType());
			providerEvent.provider().submit(ackMsg, providerEvent.handle());
		}
	}
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onAllMsg(Msg msg, OmmProviderEvent providerEvent) {}
	
	void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		ElementList elementList = EmaFactory.createElementList();
		elementList.add(EmaFactory.createElementEntry().uintValue("SupportOMMPost", 1));
		event.provider().submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).nameType(EmaRdm.USER_NAME).
				complete(true).solicited(true).state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted").
				attrib(elementList), event.handle());
	}

	void processMarketPriceRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		FieldList fieldList = EmaFactory.createFieldList();
		fieldList.add(EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add(EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add(EmaFactory.createFieldEntry().real(30, 9, OmmReal.MagnitudeType.EXPONENT_0));
		fieldList.add(EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
		event.provider().submit(
			EmaFactory.createRefreshMsg().serviceName(reqMsg.serviceName()).name(reqMsg.name())
			.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed")
			.solicited(true)
			.payload(fieldList).complete(true), event.handle());
	}

	void processInvalidItemRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		event.provider().submit(EmaFactory.createStatusMsg().name(reqMsg.name()).serviceName(reqMsg.serviceName()).
				domainType(reqMsg.domainType()).
				state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NOT_FOUND, "Item not found"),
				event.handle());
	}
}


public class IProvider
{
	//API QA
		public static String _portNumber = "14002";
		static void printHelp()
		{
			System.out.println("\nOptions:\n" + "  -?\tShows this usage\n" 
					+ "  -p port number.\n"
		    		+ "\n");
		}
		static boolean readCommandlineArgs(String[] args)
		{
			    try
			    {
			        int argsCount = 0;
			        while (argsCount < args.length)
			        {
			            if (0 == args[argsCount].compareTo("-?"))
			            {
			                printHelp();
			                return false;
			            }
			            else if ("-p".equals(args[argsCount]))
						{
							IProvider._portNumber = argsCount < (args.length-1) ? args[++argsCount] : null;
							++argsCount;
						}			       	            
		    			else // unrecognized command line argument
		    			{
		    				printHelp();
		    				return false;
		    			}			
		    		}		        		
		        }
		        catch (Exception e)
		        {
		        	printHelp();
		            return false;
		        }
				
				return true;
		}
		// END API QA
	public static void main(String[] args)
	{
		OmmProvider provider = null;
		try
		{
			//API QA
			if ( !readCommandlineArgs(args) ) return;
			//END API QA
			
			AppClient appClient = new AppClient();

			OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig();

			provider = EmaFactory.createOmmProvider(config.port(IProvider._portNumber), appClient);

			Thread.sleep(120000);
		}
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally
		{
			if (provider != null) provider.uninitialize();
		}
	}
}
