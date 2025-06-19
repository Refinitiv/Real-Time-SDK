/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2020,2024 LSEG. All rights reserved.
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
          //APIQA
                        Login.LoginReq loginRequest = EmaFactory.Domain.createLoginReq( reqMsg );
	    
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
                        loginRefresh.supportOMMPost(true);
			event.provider().submit( loginRefresh.name(loginRequest.name()).
					state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted").message(),
					event.handle() );
          //END APIQA
	}
	
	void processInvalidDomainRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
          //APIQA
		  System.out.println("Not supporting this domain");
          //END APIQA
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
