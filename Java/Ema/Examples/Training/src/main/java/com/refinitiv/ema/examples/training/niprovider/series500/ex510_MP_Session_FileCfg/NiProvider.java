/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.niprovider.series500.ex510_MP_Session_FileCfg;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.ema.access.ChannelInformation;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmInvalidUsageException;
import com.refinitiv.ema.access.OmmNiProviderConfig;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmProviderClient;
import com.refinitiv.ema.access.OmmProviderEvent;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;

class AppClient implements OmmProviderClient
{
	List<ChannelInformation> channelInList = new ArrayList<ChannelInformation>();
	
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent event)
	{
		System.out.println(refreshMsg + "\nevent session info (refresh)\n");
		
		printSessionInfo(event);
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmProviderEvent event) 
	{
		System.out.println(updateMsg + "\nevent session info (update)\n");
		
		printSessionInfo(event);
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event) 
	{
		System.out.println(statusMsg + "\nevent session info (status)\n");
		
		printSessionInfo(event);
	}
	
	void printSessionInfo(OmmProviderEvent event)
	{
		event.sessionChannelInfo(channelInList);
		
		for(ChannelInformation channelInfo : channelInList) 
		{
			System.out.println(channelInfo);
		}
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent providerEvent) {}
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {}
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onAllMsg(Msg msg, OmmProviderEvent providerEvent) {}
}

public class NiProvider {

	public static void main(String[] args)
	{
		OmmProvider provider = null;
		AppClient appClient = new AppClient();
		try
		{
			OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig();
			
			provider = EmaFactory.createOmmProvider(config.providerName("Provider_Session").username("user"), appClient);
			
			long ibmHandle = 5;
			long triHandle = 6;
			
			FieldList fieldList = EmaFactory.createFieldList();
			
			fieldList.add( EmaFactory.createFieldEntry().real(22, 14400, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 14700, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("TEST_NI_PUB").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), ibmHandle);
			
			fieldList.clear();
			
			fieldList.add( EmaFactory.createFieldEntry().real(22, 4100, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 4200, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 20,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 40, OmmReal.MagnitudeType.EXPONENT_0));
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("TEST_NI_PUB").name("TRI.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), triHandle);
			
			for( int i = 0; i < 60; i++ )
			{
				Thread.sleep(1000);
				try
				{
					fieldList.clear();
					fieldList.add(EmaFactory.createFieldEntry().real(22, 14400 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
					fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
					
					provider.submit( EmaFactory.createUpdateMsg().serviceName("TEST_NI_PUB").name("IBM.N").payload( fieldList ), ibmHandle );
					
					fieldList.clear();
					fieldList.add(EmaFactory.createFieldEntry().real(22, 4100 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
					fieldList.add(EmaFactory.createFieldEntry().real(30, 21 + i, OmmReal.MagnitudeType.EXPONENT_0));
					
					provider.submit( EmaFactory.createUpdateMsg().serviceName("TEST_NI_PUB").name("TRI.N").payload( fieldList ), triHandle );
				}
				catch(OmmInvalidUsageException excp) 
				{
					// There is no active channel currently associated with this session, so just continue until the channels are back up.
					if(excp.errorCode() == OmmInvalidUsageException.ErrorCode.NO_ACTIVE_CHANNEL)
					{
						continue;
					}
					else
					{
						System.out.println(excp.getMessage());
						break;
					}
				}
			}
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