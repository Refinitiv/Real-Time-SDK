/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System;
using System.IO;
using System.Threading;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine(refreshMsg);
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine(updateMsg);
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine(statusMsg);
    }
}

public class Consumer 
{
	static string? clientId;
	static string? clientSecret;
	static string? clientJwk;
	static string? audience;
	static string itemName = "IBM.N";

	static void PrintHelp()
	{
		Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
						   + "  -clientId client ID for application making the request to (mandatory) \r\n"
						   + "  -clientSecret client secret for application making the request to (mandatory for V2 oAuth client credentials)\r\n"
				    	   + "  -jwkFile file containing the private JWK encoded in JSON format. (mandatory for V2 client credentials grant with JWT)\n"
				    	   + "  -audience Audience value for JWT (optional for V2 oAuth client credentials with JWT).\n"
						   + "  -tokenURLV2 URL to perform authentication to get access and refresh tokens for V2 oAuth password credentials (optional).\n"
						   + "  -serviceDiscoveryURL URL for RDP service discovery to get global endpoints (optional).\n"
						   + "\nOptional parameters for establishing a connection and sending requests through a proxy server:\n"
						   + "  -itemName Request item name (optional).\n"
						   + "  -ph Proxy host name (optional).\n"
						   + "  -pp Proxy port number (optional).\n"
						   + "  -plogin User name on proxy server (optional).\n"
						   + "  -ppasswd Password on proxy server (optional).\n"
                           // API QA
                           + "  -rph REST proxy host name (optional).\n"
                           + "  -rpp REST proxy port number (optional).\n"
                           + "  -rplogin User name on REST proxy server (optional).\n"
                           + "  -rppasswd Password on REST proxy server (optional).\n"
                           // END API QA
                           + "\n");
	}

	static bool ReadCommandlineArgs(string[] args, OmmConsumerConfig config)
	{
		try
		{
			int argsCount = 0;

			while (argsCount < args.Length)
			{
				if (0 == args[argsCount].CompareTo("-?"))
				{
					PrintHelp();
					return false;
				}
				else if ("-clientId".Equals(args[argsCount]))
				{
					clientId = argsCount < (args.Length-1) ? args[++argsCount] : null;
					config.ClientId(clientId!);
					++argsCount;
				}
				else if ("-clientSecret".Equals(args[argsCount]))
				{
					clientSecret = argsCount < (args.Length-1) ? args[++argsCount] : null;
					config.ClientSecret(clientSecret!);
					++argsCount;
				}
				else if ("-jwkFile".Equals(args[argsCount]))
    			{
	            	String? jwkFile = argsCount < (args.Length-1) ? args[++argsCount] : null;
	            	if(jwkFile != null)
	            	{
		            	try
						{
							// Get the full contents of the JWK file.
							byte[] jwkBuffer = File.ReadAllBytes(Path.GetFullPath(jwkFile));
							clientJwk = System.Text.Encoding.Default.GetString(jwkBuffer);
							config.ClientJwk(clientJwk);
						}
						catch(Exception e)
						{
							Console.Error.WriteLine("Error loading JWK file: " + e.Message);
							Console.Error.WriteLine();
							Console.WriteLine("Consumer exits...");
							System.Environment.Exit(-1);
						} 
	            	}
    				++argsCount;				
    			}
				else if ("-audience".Equals(args[argsCount]))
				{
					audience = argsCount < (args.Length-1) ? args[++argsCount] : null;
					config.Audience(audience!);
					++argsCount;
				}
				else if ("-itemName".Equals(args[argsCount]))
				{
					if(argsCount < (args.Length-1))	itemName = args[++argsCount];
					++argsCount;
				}
                else if ("-ph".Equals(args[argsCount]))
                {
					if(argsCount < (args.Length - 1))
						config.ProxyHost(args[++argsCount]);
                    ++argsCount;
                }
                else if ("-pp".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                        config.ProxyPort(args[++argsCount]);
                    ++argsCount;
                }
                else if ("-plogin".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                        config.ProxyUserName(args[++argsCount]);
                    ++argsCount;
                }
                else if ("-ppasswd".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                        config.ProxyPassword(args[++argsCount]);
                    ++argsCount;
                }
                // API QA
                else if ("-rph".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                        config.RestProxyHost(args[++argsCount]);
                    ++argsCount;
                }
                else if ("-rpp".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                        config.RestProxyPort(args[++argsCount]);
                    ++argsCount;
                }
                else if ("-rplogin".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                        config.RestProxyUserName(args[++argsCount]);
                    ++argsCount;
                }
                else if ("-rppasswd".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                        config.RestProxyPassword(args[++argsCount]);
                    ++argsCount;
                }
                // END API QA
                else if ("-tokenURLV2".Equals(args[argsCount]))
				{
					if ( argsCount < (args.Length-1) )
					{
						config.TokenUrlV2( args[++argsCount] );
					}
					++argsCount;
				}
				else if ("-serviceDiscoveryURL".Equals(args[argsCount]))
				{
					if ( argsCount < (args.Length-1) )
					{
						config.ServiceDiscoveryUrl( args[++argsCount] );
					}
					++argsCount;
				}
				else // unrecognized command line argument
				{
					PrintHelp();
					return false;
				}
			}

			if (clientId == null || (clientSecret == null && clientJwk == null))
			{
				Console.WriteLine("clientId and clientSecret/jwkFile must be specified on the command line. Exiting...");
				PrintHelp();
				return false;
			}
		}
		catch (Exception)
		{
			PrintHelp();
			return false;
		}

		return true;
	}

	public static void Main(string[] args)
	{
		OmmConsumer? consumer = null;
        try
		{
			OmmConsumerConfig config = new();
			if (!ReadCommandlineArgs(args, config))
				return;
            consumer = new (config.ConsumerName("Consumer_4"));
			consumer.RegisterClient(new RequestMsg().ServiceName("ELEKTRON_DD").Name(itemName), new AppClient());
			Thread.Sleep(900000);			// API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
		}
        catch (OmmException ommException)
        {
            Console.WriteLine(ommException.Message);
        }
		finally
		{
			consumer?.Uninitialize();

        }
    }
}
