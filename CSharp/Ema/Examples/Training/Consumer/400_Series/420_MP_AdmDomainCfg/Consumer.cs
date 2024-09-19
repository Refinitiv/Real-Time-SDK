/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using System;
using static LSEG.Ema.Access.DataType;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent _)
	{
		if (refreshMsg.HasMsgKey)
			Console.WriteLine("Item Name: " + refreshMsg.Name() + " Service Name: " + refreshMsg.ServiceName());
		
		Console.WriteLine("Item State: " + refreshMsg.State());

		if (DataType.DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
			Decode(refreshMsg.Payload().FieldList());
		
		Console.WriteLine();
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent _) 
	{
		if (updateMsg.HasMsgKey)
			Console.WriteLine("Item Name: " + updateMsg.Name() + " Service Name: " + updateMsg.ServiceName());
		
		if (DataType.DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
			Decode(updateMsg.Payload().FieldList());
		
		Console.WriteLine();
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent _) 
	{
		if (statusMsg.HasMsgKey)
			Console.WriteLine("Item Name: " + statusMsg.Name() + " Service Name: " + statusMsg.ServiceName());

		if (statusMsg.HasState)
			Console.WriteLine("Item State: " +statusMsg.State());
		
		Console.WriteLine();
	}

	void Decode(FieldList fieldList)
	{
		var iter = fieldList.GetEnumerator();
		FieldEntry fieldEntry;
		while (iter.MoveNext())
		{
			fieldEntry = iter.Current;
			Console.Write("Fid: " + fieldEntry.FieldId + " Name = " + fieldEntry.Name + " DataType: " + DataType.AsString(fieldEntry.Load!.DataType) + " Value: ");

			if (Data.DataCode.BLANK == fieldEntry.Code)
				Console.WriteLine(" blank");
			else
				switch (fieldEntry.LoadType)
				{
				case DataTypes.REAL:
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
				case DataTypes.RMTES:
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
	public static void Main()
	{
		OmmConsumer? consumer = null;
		try
		{
			AppClient appClient = new();
			
			RequestMsg reqMsg = new();
			ElementList elementList= new();
			elementList.AddAscii(EmaRdm.ENAME_APP_ID, "127");
			elementList.AddAscii(EmaRdm.ENAME_POSITION, "127.0.0.1/net");
			elementList.AddUInt(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1);
			elementList.Complete();
			
            consumer = new(new OmmConsumerConfig().OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH)
					.AddAdminMsg(reqMsg.DomainType(EmaRdm.MMT_LOGIN).Name("user").NameType(EmaRdm.USER_NAME).Attrib(elementList))
					.AddAdminMsg(reqMsg.Clear().DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_INFO_FILTER  | EmaRdm.SERVICE_STATE_FILTER | EmaRdm.SERVICE_GROUP_FILTER))
					.AddAdminMsg(reqMsg.Clear().DomainType(EmaRdm.MMT_DICTIONARY).Filter(EmaRdm.DICTIONARY_VERBOSE).Name("RWFFld").ServiceId(1))
					.AddAdminMsg(reqMsg.Clear().DomainType(EmaRdm.MMT_DICTIONARY).Filter(EmaRdm.DICTIONARY_VERBOSE).Name("RWFEnum").ServiceId(1)));
		
			consumer.RegisterClient(reqMsg.Clear().ServiceName("DIRECT_FEED").Name("IBM.N"), appClient, null);

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


