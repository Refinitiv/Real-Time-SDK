/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System;
using System.Threading;
using static LSEG.Ema.Access.DataType;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine("Received Refresh. Item Handle: " + @event.Handle + " Closure: " + @event.Closure);
		
		Decode(refreshMsg);

		// open a sub stream (a.k.A. nested message request)
		if (refreshMsg.State().StreamState == OmmState.StreamStates.OPEN &&
				refreshMsg.State().DataState == OmmState.DataStates.OK &&
				refreshMsg.DomainType() == 200)
		{
			RequestMsg reqMsg = new RequestMsg();
			reqMsg.Name(".DJI").PrivateStream(true).ServiceId(refreshMsg.ServiceId()).StreamId(1);
			@event.Consumer.Submit(new GenericMsg().Payload(reqMsg), @event.Handle);
		}
			
		Console.WriteLine();
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine("Received Update. Item Handle: " + @event.Handle + " Closure: " + @event.Closure);
		
		Decode(updateMsg);
		
		Console.WriteLine();
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine("Received Status. Item Handle: " + @event.Handle + " Closure: " + @event.Closure);
	
		Decode(statusMsg);
		
		Console.WriteLine();
	}
	
	public void OnGenericMsg(GenericMsg genericMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine("Received Generic. Item Handle: " + @event.Handle + " Closure: " + @event.Closure);
		
		Decode(genericMsg);
		
		Console.WriteLine();
	}

	void Decode(RefreshMsg refreshMsg)
	{
		Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

		Console.WriteLine("Item State: " + refreshMsg.State());

		Decode(refreshMsg.Attrib());
		
		Decode(refreshMsg.Payload());
	}

	void Decode(UpdateMsg updateMsg)
	{
		Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));

		Decode(updateMsg.Attrib());
		Decode(updateMsg.Payload());
	}

	void Decode(StatusMsg statusMsg)
	{
		Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

		if (statusMsg.HasState)
			Console.WriteLine("Item State: " + statusMsg.State());
	}
	
	void Decode(ComplexTypeData complexTypeData)
	{
		switch (complexTypeData.DataType)
		{
		case DataTypes.ELEMENT_LIST:
			Decode(complexTypeData.ElementList());
			break;
		case DataTypes.FIELD_LIST:
			Decode(complexTypeData.FieldList());
			break;
		default:
			break;
		}
	}

	void Decode(GenericMsg genMsg)
	{
		if (genMsg.HasServiceId)
			Console.WriteLine("ServiceId: " + genMsg.ServiceId());

		if (genMsg.HasPartNum)
			Console.WriteLine("PartNum:  " + genMsg.PartNum());

		if (genMsg.HasSeqNum)
			Console.WriteLine("SeqNum:   " + genMsg.SeqNum());

		switch (genMsg.Attrib().DataType)
		{
		case DataTypes.ELEMENT_LIST:
			Decode(genMsg.Attrib().ElementList());
			break;
		case DataTypes.FIELD_LIST:
			Decode(genMsg.Attrib().FieldList());
			break;
		default:
			break;
		}

		switch (genMsg.Payload().DataType)
		{
		case DataTypes.ELEMENT_LIST:
			Decode(genMsg.Payload().ElementList());
			break;
		case DataTypes.FIELD_LIST:
			Decode(genMsg.Payload().FieldList());
			break;
		case DataTypes.REFRESH_MSG:
			Decode(genMsg.Payload().RefreshMsg());
			break;
		case DataTypes.UPDATE_MSG:
			Decode(genMsg.Payload().UpdateMsg());
			break;
		case DataTypes.STATUS_MSG:
			Decode(genMsg.Payload().StatusMsg());
			break;
		default:
			break;
		}
	}

	void Decode(ElementList elementList)
	{
		foreach (ElementEntry elementEntry in elementList)
		{
			Console.WriteLine("Name: " + elementEntry.Name + " DataType: " + DataType.AsString(elementEntry.Load!.DataType) + " Value: ");

			if (Data.DataCode.BLANK == elementEntry.Code)
				Console.WriteLine(" blank");
			else
				switch (elementEntry.LoadType)
			{
				case DataTypes.REAL:
					Console.WriteLine(elementEntry.OmmRealValue().AsDouble());
					break;
				case DataTypes.DATE:
					Console.WriteLine(elementEntry.OmmDateValue().Day + " / " + elementEntry.OmmDateValue().Month + " / " + elementEntry.OmmDateValue().Year);
					break;
				case DataTypes.TIME:
					Console.WriteLine(elementEntry.OmmTimeValue().Hour + ":" + elementEntry.OmmTimeValue().Minute + ":" + elementEntry.OmmTimeValue().Second + ":" + elementEntry.OmmTimeValue().Millisecond);
					break;
				case DataTypes.INT:
					Console.WriteLine(elementEntry.IntValue());
					break;
				case DataTypes.UINT:
					Console.WriteLine(elementEntry.UIntValue());
					break;
				case DataTypes.ASCII:
					Console.WriteLine(elementEntry.OmmAsciiValue());
					break;
				case DataTypes.ENUM:
					Console.WriteLine(elementEntry.EnumValue());
					break;
				case DataTypes.RMTES :
					Console.WriteLine(elementEntry.OmmRmtesValue());
					break;
				case DataTypes.ERROR:
					Console.WriteLine(elementEntry.OmmErrorValue().ErrorCode + " (" + elementEntry.OmmErrorValue().ErrorCodeAsString() + ")");
					break;
				default:
					Console.WriteLine();
					break;
			}
		}
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
					Console.WriteLine(fieldEntry.OmmErrorValue().ErrorCode +" (" + fieldEntry.OmmErrorValue().ErrorCodeAsString() + ")");
					break;
				default :
					Console.WriteLine();
					break;
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
			consumer  = new(new OmmConsumerConfig().UserName("user"));
			consumer.RegisterClient(new RequestMsg().DomainType(200).ServiceName("DIRECT_FEED")
																			.Name("IBM.XYZ")
																			.PrivateStream(true), appClient, 1);

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
