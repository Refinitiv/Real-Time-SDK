///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2017. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.examples.training.niprovider.series400.example430__MarketPrice__TrepAuthentication;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.OmmProviderClient;
import com.thomsonreuters.ema.access.OmmProviderEvent;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.OmmNiProviderConfig;
import com.thomsonreuters.ema.access.OmmProvider;
import com.thomsonreuters.ema.access.OmmReal;
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.access.PostMsg;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.UpdateMsg;
import com.thomsonreuters.ema.domain.login.Login.LoginRefresh;
import com.thomsonreuters.ema.domain.login.Login.LoginReq;
import com.thomsonreuters.ema.rdm.EmaRdm;

class AppLoginClient implements OmmProviderClient
{
	
	public long handle = 0;
	public long ttReissue = 0;
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent event)
	{
		System.out.println("Received Login Refresh Message\n");
		
		System.out.println( refreshMsg );		
		System.out.println();
		
		/* Get the handle from the event and save it for a future reissue */
		handle = event.handle();
		
		LoginRefresh loginRefresh = EmaFactory.Domain.createLoginRefresh().message(refreshMsg);
		
		if(loginRefresh.hasAuthenticationTTReissue())
			ttReissue = loginRefresh.authenticationTTReissue();
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmProviderEvent event) 
	{
		System.out.println("Received Login Update Message\n");
		
		System.out.println( updateMsg );		
		System.out.println();
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event) 
	{
		System.out.println("Received Login Status Message\n");
		
		System.out.println( statusMsg );		
		System.out.println();
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent event){}
	public void onAllMsg(Msg msg, OmmProviderEvent event){}

	@Override
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {}

	@Override
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}

	@Override
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}

	@Override
	public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
}	

public class NiProvider
{
	private static String authenticationToken = "";
	private static String appId = "256";
	private static String authenticationExtended = "";
	
	public static void printHelp()
	{
		
		System.out.println("\nOptions:\n"
		+ "  -?                            Shows this usage\n\n"
		+ "  -at <token>           Authentication token to use in login request [default = \"\"]\n"
		+ "  -ax <name>         Extended authentication information to use in login request [default = \"\"]\n"
		+ "  -aid <applicationId>        ApplicationId set as login Attribute [default = 256]\n"
		+ "\n" );
	}
	
	public static void printInvalidOption()
	{
		System.out.println("Detected a missing argument. Please verify command line options [-?]");
	}
	
	public static boolean init(String[] argv)
	{
		int count = argv.length;
		int idx = 0;
		
		while ( idx < count )
		{
			if ( 0 == argv[idx].compareTo("-?") )
			{
				printHelp();
				return false;
			}
			else if ( 0 == argv[idx].compareTo("-aid") )
			{
				if ( ++idx >= count )
				{
					printInvalidOption();
					return false;
				}
				appId = argv[idx];
				++idx;
			}
			else if ( 0 == argv[idx].compareTo("-at") )
			{
				if ( ++idx >= count )
				{
					printInvalidOption();
					return false;
				}
				authenticationToken = argv[idx];
				++idx;
			}
			else if ( 0 == argv[idx].compareTo("-ax") )
			{
				if ( ++idx >= count )
				{
					printInvalidOption();
					return false;
				}
				authenticationExtended = argv[idx];
				++idx;
			}
			else
			{
				System.out.println( "Unrecognized option. Please see command line help. [-?]");
				return false;
			}
		}
		
		return true;
	}
	
	private static void printActiveConfig()
	{
		System.out.println("Following options are selected:");
		
		System.out.println("appId = " + appId);
		System.out.println("Authentication Token = " + authenticationToken);
		System.out.println("Authentication Extended = " + authenticationExtended);
	}
	
	public static void main(String[] args)
	{
		OmmProvider provider = null;
		try
		{
			AppLoginClient appLoginClient = new AppLoginClient();
			LoginReq loginReq = EmaFactory.Domain.createLoginReq();
			
			if ( !init(args) ) return;
			
			printActiveConfig();

			OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig();
			
			loginReq.clear().role(EmaRdm.LOGIN_ROLE_PROV).name(authenticationToken).nameType(EmaRdm.USER_AUTH_TOKEN).applicationId(appId);
			if(authenticationExtended != "")
				loginReq.authenticationExtended(ByteBuffer.wrap(authenticationExtended.getBytes()));
			
			config.addAdminMsg(loginReq.message());
			
			provider = EmaFactory.createOmmProvider(config, appLoginClient);
			
			long itemHandle = 5;
			
			FieldList fieldList = EmaFactory.createFieldList();
			
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB").name("TRI.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(1000);
			
			for( int i = 0; i < 60; i++ )
			{
				if(appLoginClient.ttReissue != 0 && appLoginClient.ttReissue <= (System.currentTimeMillis()/1000))
				{
					loginReq.clear().role(EmaRdm.LOGIN_ROLE_PROV).name(authenticationToken).nameType(EmaRdm.USER_AUTH_TOKEN).applicationId(appId);
					if(!authenticationExtended.isEmpty())
						loginReq.authenticationExtended(ByteBuffer.wrap(authenticationExtended.getBytes()));
					
					provider.reissue(loginReq.message(), appLoginClient.handle);
				}
				fieldList.clear();
				fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
				
				provider.submit( EmaFactory.createUpdateMsg().serviceName("NI_PUB").name("TRI.N").payload( fieldList ), itemHandle );
				Thread.sleep(1000);
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
