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
					{
						OmmTime ommTime = fieldEntry.OmmTimeValue();
						Console.WriteLine($"{ommTime.Hour}:{ommTime.Minute}:{ommTime.Second}:{ommTime.Millisecond}:{ommTime.Microsecond}:{ommTime.Nanosecond}");
						break;
					}
				case DataTypes.DATETIME :
					Console.WriteLine(fieldEntry.OmmDateTimeValue().Day + " / " + fieldEntry.OmmDateTimeValue().Month + " / " +
						fieldEntry.OmmDateTimeValue().Year + "." + fieldEntry.OmmDateTimeValue().Hour + ":" + 
						fieldEntry.OmmDateTimeValue().Minute + ":" + fieldEntry.OmmDateTimeValue().Second + ":" + 
						fieldEntry.OmmDateTimeValue().Millisecond + ":" + fieldEntry.OmmDateTimeValue().Microsecond+ ":" + 
						fieldEntry.OmmDateTimeValue().Nanosecond);
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

public class Consumer 
{
	static Map CreateProgramaticConfig()
	{
		Map innerMap = new();
		Map configMap = new();
		ElementList elementList = new();
		ElementList innerElementList = new();
		
		elementList.AddAscii("DefaultConsumer", "Consumer_1" );
		
		innerElementList.AddAscii( "Channel", "Channel_1" );
		innerElementList.AddAscii( "Dictionary", "Dictionary_1" );
		innerElementList.AddUInt( "ItemCountHint", 5000 );
		innerElementList.AddUInt( "ServiceCountHint", 5000 );
		innerElementList.AddUInt( "ObeyOpenWindow", 0 );
		innerElementList.AddUInt( "PostAckTimeout", 5000 );
		innerElementList.AddUInt( "RequestTimeout", 5000 );
		innerElementList.AddUInt( "MaxOutstandingPosts", 5000 );
		innerElementList.AddInt( "DispatchTimeoutApiThread", 1 );
		innerElementList.AddUInt( "HandleException", 0 );
		innerElementList.AddUInt( "MaxDispatchCountApiThread", 500 );
		innerElementList.AddUInt( "MaxDispatchCountUserThread", 500 );
		innerElementList.AddInt( "PipePort", 4001 );
		innerElementList.AddInt( "ReconnectAttemptLimit", 10 );
		innerElementList.AddInt( "ReconnectMinDelay", 2000 );
		innerElementList.AddInt( "ReconnectMaxDelay", 6000 );
		innerElementList.AddUInt( "XmlTraceToStdout", 0 );
		innerElementList.AddUInt( "MsgKeyInUpdates", 1 );

		innerMap.AddKeyAscii( "Consumer_1", MapAction.ADD, innerElementList.Complete());
		innerElementList.Clear();
		
		elementList.AddMap( "ConsumerList", innerMap.Complete());
		innerMap.Clear();

		configMap.AddKeyAscii( "ConsumerGroup", MapAction.ADD, elementList.Complete());
		elementList.Clear();

		innerElementList.AddEnum( "ChannelType", EmaConfig.ConnectionTypeEnum.SOCKET);
		innerElementList.AddEnum( "CompressionType", EmaConfig.CompressionTypeEnum.ZLIB);
		innerElementList.AddUInt( "GuaranteedOutputBuffers", 5000);
		innerElementList.AddUInt( "ConnectionPingTimeout", 50000);
		innerElementList.AddAscii( "Host", "localhost");
		innerElementList.AddAscii("Port", "14002");
		innerElementList.AddUInt( "TcpNodelay", 0);

		innerMap.AddKeyAscii( "Channel_1", MapAction.ADD, innerElementList.Complete());
		innerElementList.Clear();
				
		elementList.AddMap( "ChannelList", innerMap.Complete());
		innerMap.Clear();

		configMap.AddKeyAscii("ChannelGroup", MapAction.ADD, elementList.Complete());
		elementList.Clear();

		innerElementList.AddEnum( "DictionaryType", EmaConfig.DictionaryTypeEnum.FILE);
		innerElementList.AddAscii( "RdmFieldDictionaryFileName", "./RDMFieldDictionary");
		innerElementList.AddAscii( "EnumTypeDefFileName", "./enumtype.def" );
		innerMap.AddKeyAscii( "Dictionary_1", MapAction.ADD, innerElementList.Complete());
		innerElementList.Clear();
		
		elementList.AddMap( "DictionaryList", innerMap.Complete());
		innerMap.Clear();
		
		configMap.AddKeyAscii( "DictionaryGroup", MapAction.ADD, elementList.Complete());
		elementList.Clear();
		
		return configMap.Complete();
	}
	
	public static void Main()
	{
		OmmConsumer? consumer = null;
		try
		{
			AppClient appClient = new();
			
			consumer  = new(new OmmConsumerConfig().Config( CreateProgramaticConfig())); // use programmatic configuration parameters
			
			consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name("IBM.N"), appClient);
			
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


