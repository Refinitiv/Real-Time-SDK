/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Rdm;
using static LSEG.Ema.Access.OmmConsumerConfig;

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using LSEG.Ema.PerfTools.Common;
using System;
using System.Collections.Generic;
using System.Threading;
//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.
internal class AppClient : IOmmConsumerClient
{
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));
		
		Console.WriteLine("Item State: " + refreshMsg.State());
		
		if (DataType.DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
			Decode(refreshMsg.Payload().FieldList());
		
		Console.WriteLine();
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event) 
	{
//		Console.WriteLine("+++++++  ThreadId: " + Thread.currentThread().getId());
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

		if (statusMsg.HasState)
			Console.WriteLine("Item State: " +statusMsg.State());
		
		Console.WriteLine();
	}
	
	void Decode(FieldList fieldList)
	{
		foreach (FieldEntry fieldEntry in fieldList)
		{
			Console.WriteLine("Fid: " + fieldEntry.FieldId + " Name: " + fieldEntry.Name + " value: " + fieldEntry.Load);
		}
	}
}


class DispatchThread {
    readonly OmmConsumer myCons;
    readonly Thread thread;
	
	public DispatchThread(OmmConsumer cons) {
		myCons = cons;
        thread = new Thread(() => Run());
    }

	public void Start() => thread.Start();

    private void Run() 
	{
        DateTime startTime = DateTime.Now;
		try {
			while (DateTime.Now < startTime.AddMilliseconds(500000)) 
				myCons.Dispatch();		// calls to OnRefreshMsg(), OnUpdateMsg(), or OnStatusMsg() execute on this thread
		} catch (Exception e) {
			Console.WriteLine(e.StackTrace);
		}
		finally 
		{
		//	Console.WriteLine("====  Runtime expired ====  ThreadId: " + Thread.currentThread().getId());
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
			
			XmlItemInfoList _xmlItemInfoList;
			_xmlItemInfoList = new XmlItemInfoList(1000);
			string fileName = "1k.xml";
			if (_xmlItemInfoList.ParseFile(fileName) == PerfToolsReturnCode.FAILURE)
	        {
                Console.WriteLine($"Failed to load item list from file {fileName}.");
				Environment.Exit(-1);
			}
				List<string> items1 = new();
				List<string> items2 = new();
				List<string> items3 = new();
				
				for (int i=0; i<_xmlItemInfoList.ItemInfoList.Length; i++) {
					switch (i%3) 
					{
					case 0: 
						items1.Add(_xmlItemInfoList.ItemInfoList[i].Name!);
						break;
					case 1:
						items2.Add(_xmlItemInfoList.ItemInfoList[i].Name!);
						break;
					case 2:
						items3.Add(_xmlItemInfoList.ItemInfoList[i].Name!);
						break;
						default:
							Console.WriteLine("Skipping items");
							break;
					}
				}
			
			consumer  = new OmmConsumer(new OmmConsumerConfig()
													.OperationModel(OperationModelMode.USER_DISPATCH)
													.Host("localhost:14002").UserName("user"));
			
			ElementList batch = new();
			ElementList batchSnapshot = new();			
			OmmArray array = new();
			OmmArray snapshot = new();
			for (int i = 0; i < items1.Count; i++ )
				array.AddAscii(items1[i]);
			
			for (int i = 0; i < items2.Count; i++ )
				snapshot.AddAscii(items2[i]);			

			batch.AddArray(EmaRdm.ENAME_BATCH_ITEM_LIST, array.Complete());
			
			batchSnapshot.AddArray(EmaRdm.ENAME_BATCH_ITEM_LIST, snapshot.Complete());
			
			consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Payload(batch.Complete()), appClient);
			consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Payload(batchSnapshot.Complete()).InterestAfterRefresh(false), appClient);
			
			ElementList view = new();
            OmmArray arrayView = new()
            {
                FixedWidth = 2
            };
            arrayView.AddInt(22);
			arrayView.AddInt(25);

			view.AddUInt(EmaRdm.ENAME_VIEW_TYPE, 1);
			view.AddArray(EmaRdm.ENAME_VIEW_DATA, arrayView.Complete());				
			
			for (int i = 0; i < items3.Count; i++ )
				consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name(items3[i]).Payload(view.Complete()), appClient);	
			DispatchThread thread1 = new(consumer);
			DispatchThread thread2 = new(consumer);
			DispatchThread thread3 = new(consumer);
			DispatchThread thread4 = new(consumer);
			DispatchThread thread5 = new(consumer);
			DispatchThread thread6 = new(consumer);
			DispatchThread thread7 = new(consumer);
			DispatchThread thread8 = new(consumer);
			
			thread1.Start();
			thread2.Start();
			thread3.Start();
			thread4.Start();
			thread5.Start();
			thread6.Start();
			thread7.Start();
			thread8.Start();

			Thread.Sleep(600000);
		}
        catch (Exception excp)
		{
			Console.WriteLine(excp.Message);
		}
		finally 
		{
			Console.WriteLine("Uninitializing Consumer");
			consumer?.Uninitialize();
		}
	}
}
