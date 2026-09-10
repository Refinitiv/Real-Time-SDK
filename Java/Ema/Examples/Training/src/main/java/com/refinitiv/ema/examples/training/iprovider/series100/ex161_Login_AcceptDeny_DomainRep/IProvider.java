/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.iprovider.series100.ex161_Login_AcceptDeny_DomainRep;

import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmIProviderConfig;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmProviderClient;
import com.refinitiv.ema.access.OmmProviderEvent;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.domain.login.Login;
import com.refinitiv.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{	
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent event)
	{
		switch (reqMsg.domainType())
		{
			case EmaRdm.MMT_LOGIN :
				processLoginRequest(reqMsg, event);
				break;
			default :
				processInvalidDomainRequest(reqMsg, event);
				break;
		}
	}
	
	public void onRefreshMsg(RefreshMsg refreshMsg,	OmmProviderEvent event){}
	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event){}
	public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent event){}
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent event){}
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent event){}
	public void onClose(ReqMsg reqMsg, OmmProviderEvent event){}
	public void onAllMsg(Msg msg, OmmProviderEvent event){}
	
	void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
        Login.LoginReq loginRequest = EmaFactory.Domain.createLoginReq( reqMsg );
	    
		if ( !loginRequest.name().equals("user") )
		{		
		    Login.LoginStatus loginStatus = EmaFactory.Domain.createLoginStatus();
		    
		    if (loginRequest.hasNameType())
		        loginStatus.nameType(loginRequest.nameType());
		    
			event.provider().submit( loginStatus.name(loginRequest.name()).
					state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NOT_AUTHORIZED, "Login denied").
					message(), event.handle() );
		}
		else
		{
			Login.LoginRefresh loginRefresh = EmaFactory.Domain.createLoginRefresh();
			
			if(loginRequest.hasAllowSuspectData())
				loginRefresh.allowSuspectData(loginRequest.allowSuspectData());
			
			if(loginRequest.hasSingleOpen())
				loginRefresh.singleOpen(loginRequest.singleOpen());

			if(loginRequest.hasPosition())
				loginRefresh.position(loginRequest.position());
			
			if(loginRequest.hasApplicationId())
				loginRefresh.applicationId(loginRequest.applicationId());
			
			if (loginRequest.hasNameType())
			    loginRefresh.nameType(loginRequest.nameType());
			
			event.provider().submit( loginRefresh.name(loginRequest.name()).
					state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted").message(),
					event.handle() );
		}
	}
	
	void processInvalidDomainRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		event.provider().submit( EmaFactory.Domain.createLoginStatus().name(reqMsg.name()).nameType(reqMsg.nameType()).
				state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NOT_AUTHORIZED, "Invalid domain").
				message(), event.handle() );
	}
}

public class IProvider
{
	public static void main(String[] args)
	{
		OmmProvider provider = null;
		try
		{	
			AppClient appClient = new AppClient();
			
			OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig();
			
			provider = EmaFactory.createOmmProvider(config.port("14002"), appClient);
			
			Thread.sleep(60000);
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
