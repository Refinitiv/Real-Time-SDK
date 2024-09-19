/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Domain.Login;
using LSEG.Ema.Rdm;
using System;
using System.Threading;
using static LSEG.Ema.Access.DataType;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent _)
	{
		Console.WriteLine(refreshMsg);
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent _) 
	{
		Console.WriteLine(updateMsg);
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent _) 
	{
		Console.WriteLine(statusMsg);
	}
	
	public void OnGenericMsg(GenericMsg genericMsg, IOmmConsumerEvent consumerEvent){
		if (genericMsg.DomainType() == EmaRdm.MMT_LOGIN && genericMsg.Payload().DataType == DataTypes.ELEMENT_LIST) {
			Console.WriteLine("Received login RTT message from Provider");
			ElementList data = genericMsg.Payload().ElementList();
			foreach ( ElementEntry elem in data) {
				if (elem.Name.Equals(EmaRdm.ENAME_TICKS)) {
					Console.WriteLine("        Ticks: " + elem.UIntValue());
				}
				if (elem.Name.Equals(EmaRdm.ENAME_LATENCY)) {
					Console.WriteLine("        Last Latency: " + elem.UIntValue());
				}
			}
		}
	}
}

public class Consumer 
{
	public static void Main()
	{
		OmmConsumer? consumer = null;
		try 
		{
			AppClient appClient = new();
			OmmConsumerConfig config = new();
			consumer  = new(config.ConsumerName("Consumer_7"));
            LoginReq loginReq = new();
            consumer.RegisterClient(loginReq.Message(), appClient);
			consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name("IBM.N"), appClient, 0);
			Thread.Sleep(60000);			// API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
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


