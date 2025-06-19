/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System.Threading;
using System;
using LSEG.Ema.Rdm;
using static LSEG.Ema.Access.DataType;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
	private DataDictionary dataDictionary = new();
	private bool fldDictComplete = false;
	private bool enumTypeComplete = false;
	
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine("Received Refresh. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));

		Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));
		//APIQA
		Console.WriteLine("Service Id: " + (refreshMsg.HasServiceId ? refreshMsg.ServiceId() : 0));

		Console.WriteLine("Item State: " + refreshMsg.State());
		Console.WriteLine("REFRESH COMPLETE FLAG: " + refreshMsg.Complete());
		//END APIQA
		Decode(refreshMsg, refreshMsg.Complete());

		Console.WriteLine();
	}

	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine("Received Update. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));

		Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));

		Decode(updateMsg, false);

		Console.WriteLine();
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine("Received Status. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));

		Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

		if (statusMsg.HasState)
			Console.WriteLine("Item State: " + statusMsg.State());

		Console.WriteLine();
	}

	void Decode(Msg msg, bool complete)
	{
		switch (msg.Payload().DataType)
		{
		case DataTypes.SERIES:
			
			if ( msg.Name().Equals("RWFFld") )
			{
				//APIQA
				dataDictionary.DecodeFieldDictionary(msg.Payload().Series(), EmaRdm.DICTIONARY_MINIMAL);
				//END APIQA
				if ( complete )
				{
					fldDictComplete = true;
				}
			}
			else if ( msg.Name().Equals("RWFEnum") )
			{
				//APIQA
				dataDictionary.DecodeEnumTypeDictionary(msg.Payload().Series(), EmaRdm.DICTIONARY_MINIMAL);
				//END APIQA
				if ( complete )
				{
					enumTypeComplete = true;
				}
			}
		
			if ( fldDictComplete && enumTypeComplete )
			{
				Console.WriteLine(dataDictionary);
			}
		
			break;
		case DataTypes.FIELD_LIST:
			Decode(msg.Payload().FieldList());
			break;
		default:
			break;
		}
	}
	
	void Decode(FieldList fieldList)
	{
		foreach (FieldEntry fieldEntry in fieldList)
		{
			Console.Write("Fid: " + fieldEntry.FieldId + " Name = " + fieldEntry.Name + " DataType: "
					+ DataType.AsString(fieldEntry.Load!.DataType) + " Value: ");

			if (Data.DataCode.BLANK == fieldEntry.Code)
				Console.WriteLine(" blank");
			else
				switch (fieldEntry.LoadType)
				{
				case DataTypes.REAL:
					Console.WriteLine(fieldEntry.OmmRealValue().AsDouble());
					break;
				case DataTypes.DATE:
					Console.WriteLine(fieldEntry.OmmDateValue().Day + " / " + fieldEntry.OmmDateValue().Month + " / "
							+ fieldEntry.OmmDateValue().Year);
					break;
				case DataTypes.TIME:
					Console.WriteLine(fieldEntry.OmmTimeValue().Hour + ":" + fieldEntry.OmmTimeValue().Minute + ":"
							+ fieldEntry.OmmTimeValue().Second + ":" + fieldEntry.OmmTimeValue().Millisecond);
					break;
				case DataTypes.INT:
					Console.WriteLine(fieldEntry.IntValue());
					break;
				case DataTypes.UINT:
					Console.WriteLine(fieldEntry.UIntValue());
					break;
				case DataTypes.ASCII:
					Console.WriteLine(fieldEntry.OmmAsciiValue());
					break;
				case DataTypes.ENUM:
					Console.WriteLine(fieldEntry.HasEnumDisplay ? fieldEntry.EnumDisplay() : fieldEntry.EnumValue());
					break;
				case DataTypes.RMTES:
					Console.WriteLine(fieldEntry.OmmRmtesValue());
					break;
				case DataTypes.ERROR:
					Console.WriteLine(
							fieldEntry.OmmErrorValue().ErrorCode + " (" + fieldEntry.OmmErrorValue().ErrorCodeAsString() + ")");
					break;
				default:
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

			consumer = new(new OmmConsumerConfig().Host("localhost:14002").UserName("user"));

			RequestMsg reqMsg = new();
			//APIQA
			consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DICTIONARY).Name("RWFFld").
					 Filter(EmaRdm.DICTIONARY_MINIMAL).ServiceName("DIRECT_FEED"), appClient);

			consumer.RegisterClient(reqMsg.Clear().DomainType(EmaRdm.MMT_DICTIONARY).Name("RWFEnum").
                    Filter(EmaRdm.DICTIONARY_MINIMAL).ServiceName("DIRECT_FEED"), appClient);
			//END APIQA
			consumer.RegisterClient(reqMsg.Clear().ServiceName("DIRECT_FEED").Name("IBM.N"), appClient);

			Thread.Sleep(60000); // API calls OnRefreshMsg(), OnUpdateMsg() and
                                 // OnStatusMsg()
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
