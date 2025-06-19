/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.iprovider.series200.ex201_MP_Encrypted;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
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
import com.refinitiv.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
    //holds all item requests for each client
    public Map<Long, List<Long>> clientItemHandlesMap = new HashMap<>();


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
	
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent providerEvent) {}
	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent providerEvent) {}
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
    public void onClose(ReqMsg reqMsg, OmmProviderEvent event){
        switch (reqMsg.domainType())
        {
            case EmaRdm.MMT_LOGIN :
                clientItemHandlesMap.remove(event.clientHandle());
                break;
            case EmaRdm.MMT_MARKET_PRICE :
                List<Long> list = clientItemHandlesMap.get(event.clientHandle());
                list.remove(event.handle());
                if (list.isEmpty()) {
                    clientItemHandlesMap.remove(event.clientHandle());
                }
                break;
            default :
                break;
        }
    }
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
        fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
        fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));

        event.provider().submit( EmaFactory.createRefreshMsg().name(reqMsg.name()).serviceId(reqMsg.serviceId()).solicited(true).
                        state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").
                        payload(fieldList).complete(true),
                event.handle() );

        if (clientItemHandlesMap.containsKey(event.clientHandle())) {
            clientItemHandlesMap.get(event.clientHandle()).add(event.handle());
        } else {
            LinkedList<Long> list = new LinkedList<>();
            list.add(event.handle());
            clientItemHandlesMap.put(event.clientHandle(), list);
        }
	}
	
	void processInvalidItemRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		event.provider().submit(EmaFactory.createStatusMsg().name(reqMsg.name()).serviceName(reqMsg.serviceName()).
				domainType(reqMsg.domainType()).
				state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NOT_FOUND, "Item not found"),
				event.handle());
	}
	
    public void sendUpdates(OmmProvider provider, FieldList fieldList) {
        for (Long cl_h : clientItemHandlesMap.keySet()) {
            List<Long> list = clientItemHandlesMap.get(cl_h);
            for (Long ih : list)  {
            	try {
            		provider.submit( EmaFactory.createUpdateMsg().payload( fieldList ), ih );
            	}
            	catch (Exception e)
            	{
            		System.out.println(e.getLocalizedMessage());
            	}
            }
        }
    }
    
    public boolean standBy() {
        return clientItemHandlesMap.size() == 0;
    }
}


public class IProvider 
{

	static void printHelp()
	{

	    System.out.println("\nOptions:\n" + "  -?\tShows this usage\n" 
	    		+ "  -keyfile keystore file containing the server private key and certificate for encryption.\n"
	    		+ "  -keypasswd keystore password for encryption.\n"
	    		+ "  -spTLSv1.2 Enable TLS 1.2 security protocol. Default enables both TLS 1.2 and TLS 1.3 (optional). \n"
	    		+ "  -spTLSv1.3 Enable TLS 1.3 security protocol. Default enables both TLS 1.2 and TLS 1.3 (optional). \n"
	    		+ "\n");
	}

	static boolean readCommandlineArgs(String[] args, OmmIProviderConfig config)
	{
		    try
		    {
		        int argsCount = 0;
		        boolean tls12 = false;
		        boolean tls13 = false;

		        while (argsCount < args.length)
		        {
		            if (0 == args[argsCount].compareTo("-?"))
		            {
		                printHelp();
		                return false;
		            }
	    			else if ("-keyfile".equals(args[argsCount]))
	    			{
	    				config.keystoreFile(argsCount < (args.length-1) ? args[++argsCount] : null);
	    				++argsCount;				
	    			}	
	    			else if ("-keypasswd".equals(args[argsCount]))
	    			{
	    				config.keystorePasswd(argsCount < (args.length-1) ? args[++argsCount] : null);
	    				++argsCount;				
	    			}
	    			else if ("-spTLSv1.2".equals(args[argsCount]))	   
	    			{
	    				tls12 = true;
	    				++argsCount;
	    			}
	    			else if ("-spTLSv1.3".equals(args[argsCount]))
	    			{
	    				tls13 = true;
	    				++argsCount;
	    			}
	    			else // unrecognized command line argument
	    			{
	    				printHelp();
	    				return false;
	    			}			
	    		}
		        
		        // Set security protocol versions of TLS based on configured values, with default having TLS 1.2 and 1.3 enabled
		        if ((tls12 && tls13) || (!tls12 && !tls13))
		        {
		        	config.securityProtocol("TLS");
		        	config.securityProtocolVersions(new String[]{"1.2", "1.3"});
		        }
		        else if (tls12)
		        {
		        	config.securityProtocol("TLS");
		        	config.securityProtocolVersions(new String[]{"1.2"});
		        }
		        else if (tls13)
		        {
		        	config.securityProtocol("TLS");
		        	config.securityProtocolVersions(new String[]{"1.3"});
		        }
	        }
	        catch (Exception e)
	        {
	        	printHelp();
	            return false;
	        }
			
			return true;
	}
	
	public static void main(String[] args)
	{
		AppClient appClient = new AppClient();
		OmmProvider provider = null;
		try
		{
			FieldList fieldList = EmaFactory.createFieldList();
			OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig().providerName("EncryptedProvider").operationModel(OmmIProviderConfig.OperationModel.USER_DISPATCH);
			
			if (!readCommandlineArgs(args, config))
                return;

			provider = EmaFactory.createOmmProvider(config, appClient);
			
            while (appClient.standBy()) {
                provider.dispatch(500);
                Thread.sleep(500);
            }
			
			for(int i = 0; i < 60000; i++)
			{
				provider.dispatch(1000);
		
				fieldList.clear();
				fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real(25, 3994 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
				fieldList.add(EmaFactory.createFieldEntry().real(31, 19 + i, OmmReal.MagnitudeType.EXPONENT_0));
			
                appClient.sendUpdates(provider, fieldList);

				Thread.sleep(1000);
			}
		}
		catch (Exception excp)
		{
			System.out.println(excp.getMessage());
		} 
		finally 
		{
			if (provider != null) provider.uninitialize();
		}
	}

}
