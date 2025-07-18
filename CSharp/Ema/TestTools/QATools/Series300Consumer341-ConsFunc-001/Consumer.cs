/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using System;
using System.Threading;
using static LSEG.Ema.Access.DataType;
using static LSEG.Ema.Access.OmmReal;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
	private static int postId = 1;
	
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine("Received Refresh. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));
		
		Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

		Console.WriteLine("Item State: " + refreshMsg.State());

		if ( refreshMsg.DomainType() == EmaRdm.MMT_LOGIN && 
				refreshMsg.State().StreamState == OmmState.StreamStates.OPEN &&
				refreshMsg.State().DataState == OmmState.DataStates.OK )
			{
				PostMsg postMsg = new();
				RefreshMsg nestedRefreshMsg = new RefreshMsg();
				FieldList nestedFieldList = new FieldList();
				
				//FieldList is a collection in java
				nestedFieldList.AddReal(22, 34, MagnitudeTypes.EXPONENT_POS_1);
				nestedFieldList.AddReal(25, 35, MagnitudeTypes.EXPONENT_POS_1);
				nestedFieldList.AddTime(18, 11, 29, 30);
				nestedFieldList.AddEnumValue(37, 3);
				nestedFieldList.Complete();
				nestedRefreshMsg.Payload(nestedFieldList ).Complete(true);
				
				((OmmConsumer)@event!.Closure!).Submit( postMsg.PostId( postId++ ).ServiceName( "DIRECT_FEED" )
															.Name( "TRI.N" ).SolicitAck( true ).Complete(true)
															.Payload(nestedRefreshMsg), @event.Handle );
			}

		Decode( refreshMsg );
			
		Console.WriteLine();
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine("Received Update. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));
		
		Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));
		
		Decode( updateMsg );
		
		Console.WriteLine();
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine("Received Status. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));

		Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

		if (statusMsg.HasState)
			Console.WriteLine("Item State: " +statusMsg.State());
		
		Console.WriteLine();
	}
	
	public void OnAckMsg(AckMsg ackMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine("Received AckMsg. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));
		
		Decode( ackMsg );
		
		Console.WriteLine();
	}

	void Decode( AckMsg ackMsg )
	{
		if ( ackMsg.HasMsgKey )
			Console.WriteLine("Item Name: " + ( ackMsg.HasName ? ackMsg.Name() : "not set" ) +  "\nService Name: " 
					+ ( ackMsg.HasServiceName ? ackMsg.ServiceName() : "not set" ) );

		Console.WriteLine("Ack Id: "  + ackMsg.AckId());

		if ( ackMsg.HasNackCode )
			Console.WriteLine("Nack Code: " + ackMsg.NackCodeAsString());

		if ( ackMsg.HasText )
			Console.WriteLine("Text: " + ackMsg.Text());

		switch ( ackMsg.Attrib().DataType )
		{
		case DataTypes.ELEMENT_LIST:
			Decode( ackMsg.Attrib().ElementList() );
			break;
		case DataTypes.FIELD_LIST:
			Decode( ackMsg.Attrib().FieldList() );
			break;
		default:
			break;
		}

		switch ( ackMsg.Payload().DataType )
		{
		case DataTypes.ELEMENT_LIST:
			Decode( ackMsg.Payload().ElementList() );
			break;
		case DataTypes.FIELD_LIST:
			Decode( ackMsg.Payload().FieldList() );
			break;
		default:
			break;
		}
	}

	void Decode( Msg msg )
	{
		switch ( msg.Attrib().DataType )
		{
		case DataTypes.ELEMENT_LIST:
			Decode( msg.Attrib().ElementList() );
			break;
		case DataTypes.FIELD_LIST:
			Decode( msg.Attrib().FieldList() );
			break;
		default:
			break;
		}

		switch ( msg.Payload().DataType )
		{
		case  DataTypes.ELEMENT_LIST:
			Decode( msg.Payload().ElementList() );
			break;
		case DataTypes.FIELD_LIST:
			Decode(msg.Payload().FieldList() );
			break;
		default:
			break;
		}
	}
	
	void Decode(ElementList elementList)
	{
		foreach (ElementEntry elementEntry in elementList)
		{
			Console.Write(" Name = " + elementEntry.Name + " DataType: " + DataType.AsString(elementEntry.Load!.DataType) + " Value: ");

			if (Data.DataCode.BLANK == elementEntry.Code)
				Console.WriteLine(" blank");
			else
				switch (elementEntry.LoadType)
				{
				case DataTypes.REAL :
					Console.WriteLine(elementEntry.OmmRealValue().AsDouble());
					break;
				case DataTypes.DATE :
					Console.WriteLine(elementEntry.OmmDateValue().Day + " / " + elementEntry.OmmDateValue().Month + " / " + elementEntry.OmmDateValue().Year);
					break;
				case DataTypes.TIME :
					Console.WriteLine(elementEntry.OmmTimeValue().Hour + ":" + elementEntry.OmmTimeValue().Minute + ":" + elementEntry.OmmTimeValue().Second + ":" + elementEntry.OmmTimeValue().Millisecond);
					break;
				case DataTypes.INT :
					Console.WriteLine(elementEntry.IntValue());
					break;
				case DataTypes.UINT :
					Console.WriteLine(elementEntry.UIntValue());
					break;
				case DataTypes.ASCII :
					Console.WriteLine(elementEntry.OmmAsciiValue());
					break;
				case DataTypes.ENUM :
					Console.WriteLine(elementEntry.EnumValue());
					break;
				case DataTypes.RMTES :
					Console.WriteLine(elementEntry.OmmRmtesValue());
					break;
				case DataTypes.ERROR :
					Console.WriteLine(elementEntry.OmmErrorValue().ErrorCode +" (" + elementEntry.OmmErrorValue().ErrorCodeAsString() + ")");
					break;
				default :
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
	public static void Main(String[] args)
	{
		OmmConsumer? consumer = null;
		try
		{
			AppClient appClient = new AppClient();
			
			consumer = new(new OmmConsumerConfig().Host("localhost:14002").UserName("user"));
			
			RequestMsg reqMsg = new RequestMsg();

			//APIQA
			long loginHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_LOGIN), appClient, consumer);

			consumer.RegisterClient(reqMsg.Clear().ServiceName("DIRECT_FEED").Name("IBM.N"), appClient, consumer);

			Thread.Sleep(3000);
			int id = 2;

			PostMsg postMsg = new PostMsg();
			RefreshMsg nestedRefreshMsg = new RefreshMsg();
			FieldList nestedFieldList = new FieldList();
			//FieldList is a collection in java
			nestedFieldList.AddReal(22, 34, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
			nestedFieldList.AddReal(25, 35, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
			nestedFieldList.AddTime(18, 11, 29, 30);
			nestedFieldList.AddEnumValue(37, 3);

			nestedRefreshMsg.Payload(nestedFieldList).Complete(true);
			Console.WriteLine(postMsg);

			while (true)
			{
				consumer.Submit(postMsg.PostId(id++).ServiceName("DIRECT_FEED")
													.Name("TRI.N").SolicitAck(true).Complete(true)
													.Payload(nestedRefreshMsg), loginHandle);
				postMsg.Clear();

				Thread.Sleep(20000);
			}
			//END APIQA	
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
