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
	        //APIQA	
		Console.WriteLine("Clone Refresh Msg");
	        RefreshMsg cloneRefreshMsg = new (refreshMsg); 
		Console.WriteLine("Item Name: " + (cloneRefreshMsg.HasName ? cloneRefreshMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (cloneRefreshMsg.HasServiceName ? cloneRefreshMsg.ServiceName() : "<not set>"));
		
		Console.WriteLine("Item State: " + cloneRefreshMsg.State());
		
		if (DataTypes.MAP == cloneRefreshMsg.Payload().DataType)
			Decode(cloneRefreshMsg.Payload().Map());
		
	        //END APIQA	
		Console.WriteLine();
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event) 
	{
	        //APIQA	
		Console.WriteLine("Clone Update Msg");
	        UpdateMsg cloneUpdateMsg = new(updateMsg); 
		Console.WriteLine("Item Name: " + (cloneUpdateMsg.HasName ? cloneUpdateMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (cloneUpdateMsg.HasServiceName ? cloneUpdateMsg.ServiceName() : "<not set>"));
		
		if (DataTypes.MAP == cloneUpdateMsg.Payload().DataType)
			Decode(cloneUpdateMsg.Payload().Map());
		
	        //END APIQA	
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

    static void Decode(FieldList fieldList)
	{
		foreach (FieldEntry fieldEntry in fieldList)
		{
			Console.WriteLine("Fid: " + fieldEntry.FieldId + " Name: " + fieldEntry.Name + " value: " + fieldEntry.Load);
		}
	}
	
	void Decode(Map map)
	{
		if (DataTypes.FIELD_LIST == map.SummaryData().DataType)
		{
			Console.WriteLine("Map Summary data:");
            Decode(map.SummaryData().FieldList());
			Console.WriteLine();
		}
		
		foreach (MapEntry mapEntry in map)
		{
			if (DataTypes.BUFFER == mapEntry.Key.DataType)
				Console.WriteLine("Action: " + mapEntry.MapActionAsString() + " key value: " + EmaUtility.AsHexString(mapEntry.Key.Buffer().Value));

			if (DataTypes.FIELD_LIST == mapEntry.LoadType)
			{
				Console.WriteLine("Entry data:");
                Decode(mapEntry.FieldList());
				Console.WriteLine();
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
			
			consumer = new(new OmmConsumerConfig().Host("localhost:14002").UserName("user"));
			
			consumer.RegisterClient( new RequestMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER)
															.ServiceName("DIRECT_FEED").Name("AAO.V"), appClient, 0);
			
			Thread.Sleep(60000);			// API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
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
