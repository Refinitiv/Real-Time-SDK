/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using System;
using System.Threading;
using static LSEG.Ema.Access.DataType;

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.
internal class AppClient : IOmmConsumerClient
{
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));
		
		Console.WriteLine("Item State: " + refreshMsg.State());
		
		if (DataTypes.MAP == refreshMsg.Payload().DataType)
			Decode(refreshMsg.Payload().Map());
		
		Console.WriteLine();
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));
		
		if (DataTypes.MAP == updateMsg.Payload().DataType)
			Decode(updateMsg.Payload().Map());
		
		Console.WriteLine();
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

		if (statusMsg.HasState)
			Console.WriteLine("Item State: " +statusMsg.State());
		
		Console.WriteLine();
	}
	
	public void OnGenericMsg(GenericMsg genericMsg, IOmmConsumerEvent consumerEvent){}
	public void OnAckMsg(AckMsg ackMsg, IOmmConsumerEvent consumerEvent){}
	public void OnAllMsg(Msg msg, IOmmConsumerEvent consumerEvent){}

    static void Decode(FieldList fieldList)
	{
		foreach (FieldEntry fieldEntry in fieldList)
		{
			Console.WriteLine("Fid: " + fieldEntry.FieldId + " Name: " + fieldEntry.Name + " value: " + fieldEntry.Load);
		}
	}
	
	void Decode(Map map)
	{
		foreach (MapEntry map_entry in map) 
		{
            //String element = map_entry.Key.Buffer.ToString();
        #region APIQA
            string? element = map_entry?.Key?.Buffer().ToString();
			Console.WriteLine(element);
		}
		/*
		if (DataTypes.FIELD_LIST == map.SummaryData().DataType)
		{
			Console.WriteLine("Map Summary data:");
            Decode(map.SummaryData().FieldList());
			Console.WriteLine();
		}
		
		foreach (MapEntry mapEntry in map)
		{					
			if (DataTypes.BUFFER == mapEntry.Key.DataType)
			//	Console.WriteLine("Action: " + mapEntry.MapActionAsString() + " key value: " + EmaUtility.AsHexString(mapEntry.Key.Buffer.Buffer()));
				Console.WriteLine("Action: " + mapEntry.MapActionAsString() + " key value: " + mapEntry.Key.Buffer().AsHex().AsHexString());
					
			
			if (DataTypes.ASCII == mapEntry.Key.DataType)
				//		Console.WriteLine("Action: " + mapEntry.MapActionAsString() + " key value: " + EmaUtility.AsHexString(mapEntry.Key.Buffer.Buffer()));
						Console.WriteLine("Action: " + mapEntry.MapActionAsString() + " key value: " + mapEntry.Key.Ascii().ToString());
			
			if (DataTypes.FIELD_LIST == mapEntry.LoadType)
			{
				Console.WriteLine("Entry data:");
                Decode(mapEntry.FieldList());
				Console.WriteLine();
			}
		}
		*/
        #endregion APIQA
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
			
			consumer  = new OmmConsumer(new OmmConsumerConfig().Host("localhost:14002").UserName("user"));
			
			consumer.RegisterClient( new RequestMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER)
															.ServiceName("DIRECT_FEED").Name("AGG.V"), appClient, 0);
			
			Thread.Sleep(60000);            // API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
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
