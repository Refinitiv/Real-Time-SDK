/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Domain.Login;
using LSEG.Ema.Rdm;
using System;
using System.Text;
using System.Threading;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent _)
	{
		Console.WriteLine("Received Refresh Message\n");
		
		Console.WriteLine( refreshMsg );		
		Console.WriteLine();
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent _) 
	{
		Console.WriteLine("Received Update Message\n");
		
		Console.WriteLine( updateMsg );		
		Console.WriteLine();
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent _) 
	{
		Console.WriteLine("Received Status Message\n");
		
		Console.WriteLine( statusMsg );		
		Console.WriteLine();
	}
}

class AppLoginClient : IOmmConsumerClient
{
	public long handle = 0;
	public ulong ttReissue = 0;
	
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine("Received Login Refresh Message\n");
		
		Console.WriteLine( refreshMsg );		
		Console.WriteLine();
		
		/* Get the handle from the event and save it for a future reissue */
		handle = @event.Handle;
        /* Get the time to reissue from the refresh and save it */
        LoginRefresh loginRefresh = new LoginRefresh().Message(refreshMsg);
        if (loginRefresh.HasAuthenticationTTReissue)
            ttReissue = loginRefresh.AuthenticationTTReissue();
    }
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent _) 
	{
		Console.WriteLine("Received Login Update Message\n");
		
		Console.WriteLine(updateMsg);		
		Console.WriteLine();
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent _) 
	{
		Console.WriteLine("Received Login Status Message\n");
		
		Console.WriteLine( statusMsg );		
		Console.WriteLine();
	}
}
	
public class Consumer 
{
	private static string authenticationToken = "";
	private static string appId = "256";
	private static string authenticationExtended = "";
	
	public static void PrintHelp()
	{
		
		Console.WriteLine("\nOptions:\n"
		+ "  -?                            Shows this usage\n\n"
		+ "  -at <token>                   Authentication token to use in login request [default = \"\"]\n"
		+ "  -ax <extended information>    Extended authentication information to use in login request [default = \"\"]\n"
		+ "  -aid <applicationId>        ApplicationId set as login Attribute [default = 256]\n"
		+ "\n" );
	}
	
	public static void PrintInvalidOption()
	{
		Console.WriteLine("Detected a missing argument. Please verify command line options [-?]");
	}
	
	public static bool Init(string[] argv)
	{
		int count = argv.Length;
		int idx = 0;
		
		while ( idx < count )
		{
			if ( argv[idx].Equals("-?") )
			{
				PrintHelp();
				return false;
			}
			else if ( argv[idx].Equals("-aid") )
			{
				if ( ++idx >= count )
				{
					PrintInvalidOption();
					return false;
				}
				appId = argv[idx];
				++idx;
			}
			else if ( argv[idx].Equals("-at") )
			{
				if ( ++idx >= count )
				{
					PrintInvalidOption();
					return false;
				}
				authenticationToken = argv[idx];
				++idx;
			}
			else if ( argv[idx].Equals("-ax") )
			{
				if ( ++idx >= count )
				{
					PrintInvalidOption();
					return false;
				}
				authenticationExtended = argv[idx];
				++idx;
			}
			else
			{
				Console.WriteLine( "Unrecognized option. Please see command line help. [-?]");
				return false;
			}
		}
		
		return true;
	}
	
	private static void PrintActiveConfig()
	{
		Console.WriteLine("Following options are selected:");
		
		Console.WriteLine("appId = " + appId);
		Console.WriteLine("Authentication Token = " + authenticationToken);
		Console.WriteLine("Authentication Extended = " + authenticationExtended);
	}
	
	public static void Main(string[] args)
	{
		OmmConsumer? consumer = null;
		try
		{
			if ( !Init(args) ) return;
			AppClient appClient = new();
			AppLoginClient appLoginClient = new();
            LoginReq loginReq = new();

            PrintActiveConfig();
			
			OmmConsumerConfig config = new OmmConsumerConfig();
			loginReq.Clear().Name(authenticationToken).NameType(EmaRdm.USER_AUTH_TOKEN).ApplicationId(appId);

            if (!string.IsNullOrEmpty(authenticationExtended))
			{
				Console.WriteLine("setting authnextended\n");
                loginReq.AuthenticationExtended(new EmaBuffer(Encoding.ASCII.GetBytes(authenticationExtended)));
			}

            config.AddAdminMsg(loginReq.Message());
			consumer = new(config, appLoginClient);
			
			RequestMsg reqMsg = new();
			reqMsg.ServiceName("DIRECT_FEED").Name("TRI.N");
            consumer.RegisterClient(reqMsg, appClient);

			for(int i = 0; i < 60; i++)
			{
                if (appLoginClient.ttReissue != 0 && appLoginClient.ttReissue <= (ulong)DateTimeOffset.Now.ToUnixTimeSeconds())
				{
                    loginReq.Clear().Name(authenticationToken).NameType(EmaRdm.USER_AUTH_TOKEN).ApplicationId(appId);

                    if (!string.IsNullOrEmpty(authenticationExtended))
                        loginReq.AuthenticationExtended(new EmaBuffer(Encoding.ASCII.GetBytes(authenticationExtended)));

                    consumer.Reissue(loginReq.Message(), appLoginClient.handle);
					appLoginClient.ttReissue = 0;
				}
				
				Thread.Sleep(1000);
			}
		} 
		catch (OmmException excp)
		{
			Console.WriteLine(excp.Message);
		}
		finally 
		{
			consumer?.Uninitialize();
		}
	}
}
