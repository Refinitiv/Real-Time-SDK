/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
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
		Console.WriteLine("Received Refresh. Item Handle: " + @event.Handle + " Closure: " + @event.Closure);

		Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

		Console.WriteLine("Item State: " + refreshMsg.State());
		Decode(refreshMsg, refreshMsg.Complete());

		Console.WriteLine();
	}

	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine("Received Update. Item Handle: " + @event.Handle + " Closure: " + @event.Closure);

		Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));

		Decode(updateMsg, false);

		Console.WriteLine();
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine("Received Status. Item Handle: " + @event.Handle + " Closure: " + @event.Closure);

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
				dataDictionary.DecodeFieldDictionary(msg.Payload().Series(), EmaRdm.DICTIONARY_NORMAL);

				if ( complete )
				{
					fldDictComplete = true;
				}
			}
			else if ( msg.Name().Equals("RWFEnum") )
			{
				dataDictionary.DecodeEnumTypeDictionary(msg.Payload().Series(), EmaRdm.DICTIONARY_NORMAL);

				if ( complete )
				{
					enumTypeComplete = true;
				}
			}
		
			if ( fldDictComplete && enumTypeComplete )
			{
					Console.WriteLine(dataDictionary);
					//API QA
					DictionaryEntry entry22 = dataDictionary.Entry(22);
					DictionaryEntry entry25 = dataDictionary.Entry(25);
					Console.WriteLine("QA Prints entry22 : " + entry22.Acronym);
					Console.WriteLine("QA Prints entry25 : " + entry25.Acronym);
					if (entry22 == entry25)
						Console.WriteLine("Test 1 : QA Prints check entry1 and entry2 are equal ");
					DictionaryEntry entryBID = dataDictionary.Entry("BID");
					DictionaryEntry entryASK = dataDictionary.Entry("ASK");
					Console.WriteLine("QA Prints entryBID fid : " + entryBID.FieldId);
					Console.WriteLine("QA Prints entryASK fid : " + entryASK.FieldId);
					if (entryBID == entryASK)
						Console.WriteLine("Test 2 : QA Prints check entryBID and entryASK are equal ");

					DictionaryEntry entry30 = new();
					dataDictionary.Entry(30, entry30);
					DictionaryEntry entry31 = new();
					dataDictionary.Entry(31, entry31);
					Console.WriteLine("QA Prints entry30 fname : " + entry30.Acronym);
					Console.WriteLine("QA Prints entry31 fname : " + entry31.Acronym);
					if (entry30 == entry31)
						Console.WriteLine("Test 3 : QA Prints check entry30 and entry31 are equal ");
					else
						Console.WriteLine("Test 3 : QA Prints check entry30 and entry31 are NOT equal ");

					DictionaryEntry entryBIDSIZE = new();
					dataDictionary.Entry("BIDSIZE", entryBIDSIZE);
					DictionaryEntry entryASKSIZE = new();
					dataDictionary.Entry("ASKSIZE", entryASKSIZE);
					Console.WriteLine("QA Prints entryBIDSIZE fid : " + entryBIDSIZE.FieldId);
					Console.WriteLine("QA Prints entryASKSIZE fid : " + entryASKSIZE.FieldId);
					if (entryBIDSIZE == entryASKSIZE)
						Console.WriteLine("Test 4 : QA Prints check entryBIDSIZE and entryASKSIZE are equal ");
				else
						Console.WriteLine("Test 4 : QA Prints check entryBIDSIZE and entryASKSIZE are NOT equal ");

					try
					{
						Console.WriteLine("Test 5 Error Case : Trying to use entry owned by API (entry22)...");
						dataDictionary.Entry(22, entry22);
					}
					catch (OmmException ex)
					{
						Console.WriteLine("QA Prints Exception Type : " + ex.Type);
						Console.WriteLine("QA Prints Exception Message : " + ex.Message);
					}
					try
					{
						Console.WriteLine("Test 6 Error Case : Trying to use entry which is null (entryNull)...");
						DictionaryEntry ?entryNull = null;
						dataDictionary.Entry(25, entryNull);
					}
					catch (OmmException ex)
					{
						//ex.printStackTrace();
						Console.WriteLine("QA Prints Exception Type : " + ex.Type);
						Console.WriteLine("QA Prints Exception Message : " + ex.Message);
					}
					try
					{
						Console.WriteLine("Test 7 Error Case : Trying to use entry owned by API (entry25)...");
						dataDictionary.Entry("BID", entry22);
					}
					catch (OmmException ex)
					{
						Console.WriteLine("QA Prints Exception Type : " + ex.Type);
						Console.WriteLine("QA Prints Exception Message : " + ex.Message);
					}
					try
					{
						Console.WriteLine("Test 8 Error Case : Trying to use entry which is null (entryNull)...");
						DictionaryEntry ?entryNull = null;
						dataDictionary.Entry("ASK", entryNull);
					}
					catch (OmmException ex)
					{
						//ex.printStackTrace();
						Console.WriteLine("QA Prints Exception Type : " + ex.Type);
						Console.WriteLine("QA Prints Exception Message : " + ex.Message);
					}
					//End APIQA
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
	
			consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DICTIONARY).Name("RWFFld").
					 Filter(EmaRdm.DICTIONARY_NORMAL), appClient);

			consumer.RegisterClient(reqMsg.Clear().DomainType(EmaRdm.MMT_DICTIONARY).Name("RWFEnum").
                    Filter(EmaRdm.DICTIONARY_NORMAL), appClient);

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
