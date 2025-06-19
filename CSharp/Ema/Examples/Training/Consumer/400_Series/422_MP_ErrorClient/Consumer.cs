/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System;
using System.Threading;
using static LSEG.Ema.Access.DataType;
using static LSEG.Ema.Access.OmmConsumerConfig;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent _)
	{
		Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));
		
		Console.WriteLine("Item State: " + refreshMsg.State());
		
		if (DataType.DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
			Decode(refreshMsg.Payload().FieldList());
		
		Console.WriteLine();
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent _) 
	{
		Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));
		
		if (DataType.DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
			Decode(updateMsg.Payload().FieldList());
		
		Console.WriteLine();
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent _) 
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
			Console.Write("Fid: " + fieldEntry.FieldId + " Name = " + fieldEntry.Name + " DataType: " + DataType.AsString(fieldEntry.Load!.DataType) + " Value: ");

			if (Data.DataCode.BLANK == fieldEntry.Code)
				Console.WriteLine(" blank");
			else
				switch (fieldEntry.LoadType)
				{
				case DataTypes.REAL :
					Console.WriteLine(fieldEntry.OmmRealValue().AsDouble());
					break;
				case DataTypes.DATE :
					Console.WriteLine(fieldEntry.OmmDateValue().Day + " / " + fieldEntry.OmmDateValue().Month + " / " + fieldEntry.OmmDateValue().Year);
					break;
				case DataTypes.TIME :
					Console.WriteLine(fieldEntry.OmmTimeValue().Hour + ":" + fieldEntry.OmmTimeValue().Minute + ":" + fieldEntry.OmmTimeValue().Second + ":" + fieldEntry.OmmTimeValue().Millisecond);
					break;
				case DataTypes.INT :
					Console.WriteLine(fieldEntry.IntValue());
					break;
				case DataTypes.UINT :
					Console.WriteLine(fieldEntry.UIntValue());
					break;
				case DataTypes.ASCII :
					Console.WriteLine(fieldEntry.OmmAsciiValue());
					break;
				case DataTypes.ENUM :
					Console.WriteLine(fieldEntry.HasEnumDisplay ? fieldEntry.EnumDisplay() : fieldEntry.EnumValue());
					break;
				case DataTypes.RMTES :
					Console.WriteLine(fieldEntry.OmmRmtesValue());
					break;
				case DataTypes.ERROR :
					Console.WriteLine("(" + fieldEntry.OmmErrorValue().ErrorCodeAsString() + ")");
					break;
				default :
					Console.WriteLine();
					break;
				}
		}
	}
}

class AppErrorClient : IOmmConsumerErrorClient
{
	public void OnInvalidHandle(long handle, String text)
	{
		Console.WriteLine("onInvalidHandle callback function" + "\nInvalid handle: " + handle + "\nError text: " + text); 
	}

	public void OnInvalidUsage(String text, int errorCode) {
		Console.WriteLine("onInvalidUsage callback function" + "\nError text: " + text +" , Error code: " + errorCode); 
	}

	public void OnDispatchError(string text, int errorCode) {
		Console.WriteLine("OnDispatchError callback function" + "\nError text: " + text + " , Error code: " + errorCode);
	}
}

public class Consumer 
{
	public static void Main()
	{
		OmmConsumer? consumer = null;
		AppClient appClient = new();
		AppErrorClient appErrorClient = new();

		try
		{
			consumer = new(new OmmConsumerConfig().OperationModel(OperationModelMode.USER_DISPATCH)
										.UserName("user"), appErrorClient);

			long invalidHandle = 0;

			consumer.Reissue(new RequestMsg(), invalidHandle);
			consumer.Submit(new GenericMsg(), invalidHandle);
			consumer.Submit(new PostMsg(), invalidHandle);
			consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name("IBM.N"), appClient);
			var startTime = DateTime.Now;
			var duration = TimeSpan.FromMilliseconds(60000);
			while (DateTime.Now < startTime + duration)
				consumer.Dispatch(10);      // calls to OnRefreshMsg(), OnUpdateMsg(), or OnStatusMsg() execute on this thread
		}
		catch (OmmException excp)
		{
			Console.WriteLine(excp);
		}
		finally
		{
			consumer?.Uninitialize();
		}
	}
}
