/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.consumer.series200.ex270_SymbolList;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.eta.rdm.SymbolList;

import java.util.ArrayList;
import java.util.List;


class AppClient implements OmmConsumerClient
{
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Stream ID: " + refreshMsg.streamId());
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

		System.out.println("Item State: " + refreshMsg.state());

		if (DataType.DataTypes.MAP == refreshMsg.payload().dataType())
			decode(refreshMsg.payload().map());
		else if (DataType.DataTypes.FIELD_LIST == refreshMsg.payload().dataType())
			decode(refreshMsg.payload().fieldList(), false);

		System.out.println();
	}

	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
	{
		System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
		System.out.println("Stream ID: " + updateMsg.streamId());
		System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));

		if (DataType.DataTypes.MAP == updateMsg.payload().dataType())
			decode(updateMsg.payload().map());
		else if (DataType.DataTypes.FIELD_LIST == updateMsg.payload().dataType())
			decode(updateMsg.payload().fieldList(), false);

		System.out.println();
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event)
	{
		System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
		System.out.println("Stream ID: " + statusMsg.streamId());
		System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

		if (statusMsg.hasState())
			System.out.println("Item State: " +statusMsg.state());

		System.out.println();
	}

	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}

	void decode(FieldList fieldList, boolean newLine)
	{
		for(FieldEntry fieldEntry : fieldList)
		{

			System.out.print(fieldEntry.name() + "\t");

			if (Data.DataCode.BLANK == fieldEntry.code())
				System.out.println(" blank");
			else
				switch (fieldEntry.loadType())
				{
					case DataTypes.REAL :
						System.out.println(fieldEntry.real().asDouble());
						break;
					case DataTypes.DATE :
						System.out.println(fieldEntry.date().day() + " / " + fieldEntry.date().month() + " / " + fieldEntry.date().year());
						break;
					case DataTypes.TIME :
						System.out.println(fieldEntry.time().hour() + ":" + fieldEntry.time().minute() + ":" + fieldEntry.time().second() + ":" + fieldEntry.time().millisecond());
						break;
					case DataTypes.INT :
						System.out.println(fieldEntry.intValue());
						break;
					case DataTypes.UINT :
						System.out.println(fieldEntry.uintValue());
						break;
					case DataTypes.ASCII :
						System.out.println(fieldEntry.ascii());
						break;
					case DataTypes.RMTES :
						System.out.println(fieldEntry.rmtes());
						break;
					case DataTypes.ENUM :
						System.out.println(fieldEntry.hasEnumDisplay() ? fieldEntry.enumDisplay() : fieldEntry.enumValue());
						break;
					case DataTypes.ERROR :
						System.out.println("(" + fieldEntry.error().errorCodeAsString() + ")");
						break;
					default :
						System.out.println();
						break;
				}

			if (newLine)
				System.out.println();
		}
	}

	void decode(Map map)
	{
		if (DataTypes.FIELD_LIST == map.summaryData().dataType())
		{
			System.out.println("Summary :");
			decode(map.summaryData().fieldList(), true);
			System.out.println();
		}

		boolean firstEntry = true;

		for(MapEntry mapEntry : map)
		{
			if (firstEntry)
			{
				firstEntry = false;
				System.out.println("Name\tAction");
				System.out.println();
			}

			switch (mapEntry.key().dataType())
			{
				case DataTypes.BUFFER :
					System.out.println(mapEntry.key().buffer() + "\t" + mapEntry.mapActionAsString());
					break;
				case DataTypes.ASCII :
					System.out.println(mapEntry.key().ascii() + "\t" + mapEntry.mapActionAsString());
					break;
				case DataTypes.RMTES :
					System.out.println(mapEntry.key().rmtes() + "\t" + mapEntry.mapActionAsString());
					break;
				default:
					break;
			}

			if (DataTypes.FIELD_LIST == mapEntry.loadType())
			{
				System.out.println("\t");
				decode(mapEntry.fieldList(), false);
			}
		}
	}
}

public class Consumer
{
	static void printHelp()
	{
		System.out.println("\nOptions:\n" +
				"\t-?                           Shows this usage\n" +
				"\t-slItem                      Specifies Symbol List item name to be requested\r\n" +
				"\t-enhancedSymbolListRequestOn In case specified, Enhanced Symbol List Request feature will be turned on\r\n" +
				"\t-snapshots                   Items from Symbol List will be requested as non-streaming if this parameter is specified (works only together with -enhancedSymbolListRequestOn)\r\n" +
				"\t-mp                          Specifies the list of ordinary Market Price items to be requested (default is empty list)\r\n" +
				"\t-mpPrivate                   If mentioned, ordinary Market Price items are requested as private (if not mentioned items are not private)\r\n" +
				"\t-snapshotMpRequests          If mentioned, ordinary Market Price items are requested as non-streaming (if not mentioned items are streaming)\r\n" +
				"\t-mbp                         Specifies the list of ordinary Market By Price items to be requested (default is empty list)\r\n" +
				"\t-mbpPrivate                  If mentioned, ordinary Market By Price items are requested as private (if not mentioned items are not private)\r\n" +
				"\t-snapshotMbpRequests         If mentioned, ordinary Market By Price items are requested as non-streaming (if not mentioned items are streaming)\r\n" +
				"\t-s                           Service name (default is ELEKTRON_DD)\r\n" +
				"\t-consumerName                The desired Consumer configuration entry (default is Consumer_1)\r\n" +
				"Sample command line arguments: \"-slItem .BV.N -enhancedSymbolListRequestOn -snapshots -mp TRI.N,IBM.N -mpPrivate -snapshotMpRequests -mbp ABC.N,DEF.N -mbpPrivate -snapshotMbpRequests -s DIRECT_FEED\"");
	}

	public static void main(String[] args)
	{
		OmmConsumer consumer = null;

		int argsCount = 0;
		boolean enhancedSymbolListRequestOn = false;
		String itemName = ".AV.N";
		boolean snapshots = false;
		String[] mpRequests = null;
		boolean privateMpRequests = false;
		boolean snapshotMpRequests = false;
		String serviceName = "ELEKTRON_DD";
		String reqString = null;
		String reqMbpString = null;
		String[] mbpRequests = null;
		boolean privateMbpRequests = false;
		boolean snapshotMbpRequests = false;
		String reqMboString = null;
		String[] mboRequests = null;
		boolean privateMboRequests = false;
		boolean snapshotMboRequests = false;
		String consumerName = "Consumer_1";

		while (argsCount < args.length)
		{
			if (args[argsCount].equals("-?"))
			{
				printHelp();
				return;
			}
			if ("-slItem".equals(args[argsCount]))
			{
				itemName = argsCount < (args.length-1) ? args[++argsCount] : ".AV.N";
				++argsCount;
			}
			else if ("-enhancedSymbolListRequestOn".equals(args[argsCount]))
			{
				enhancedSymbolListRequestOn = true;
				++argsCount;
			}
			else if ("-snapshots".equals(args[argsCount]))
			{
				snapshots = true;
				++argsCount;
			}
			else if ("-mp".equals(args[argsCount]))
			{
				reqString = argsCount < (args.length-1) ? args[++argsCount] : "TRI.N";
				mpRequests = reqString.split(",");
				++argsCount;
			}
			else if ("-mpPrivate".equals(args[argsCount]))
			{
				privateMpRequests = true;
				++argsCount;
			}
			else if ("-snapshotMpRequests".equals(args[argsCount]))
			{
				snapshotMpRequests = true;
				++argsCount;
			}
			else if ("-mbp".equals(args[argsCount]))
			{
				reqMbpString = argsCount < (args.length-1) ? args[++argsCount] : "TRI.N";
				mbpRequests = reqMbpString.split(",");
				++argsCount;
			}
			else if ("-mbpPrivate".equals(args[argsCount]))
			{
				privateMbpRequests = true;
				++argsCount;
			}
			else if ("-snapshotMbpRequests".equals(args[argsCount]))
			{
				snapshotMbpRequests = true;
				++argsCount;
			}
			else if ("-mbo".equals(args[argsCount]))
			{
				reqMboString = argsCount < (args.length-1) ? args[++argsCount] : "TRI.N";
				mboRequests = reqMboString.split(",");
				++argsCount;
			}
			else if ("-mboPrivate".equals(args[argsCount]))
			{
				privateMboRequests = true;
				++argsCount;
			}
			else if ("-snapshotMboRequests".equals(args[argsCount]))
			{
				snapshotMboRequests = true;
				++argsCount;
			}
			else if ("-s".equals(args[argsCount]))
			{
				serviceName = args[++argsCount];
				++argsCount;
			}
			else if ("-consumerName".equals(args[argsCount]))
			{
				consumerName = argsCount < (args.length-1) ? args[++argsCount] : "Consumer_1";
				++argsCount;
			}
			else // unrecognized command line argument
			{
				System.out.println("Unrecognized parameter: " + args[argsCount]);
				printHelp();
				return;
			}
		}

		System.out.println("App settings: \n\t-slItem " + itemName
				+ ", \n\tEnhanced SymbolL List Feature on: " + enhancedSymbolListRequestOn
				+ ", \n\t-snapshots: " + snapshots
				+ ", \n\t-mp: " + reqString
				+ ", \n\t-mpPrivate: " + privateMpRequests
				+ ", \n\t-snapshotMpRequests: " + snapshotMpRequests
				+ ", \n\t-mbp: " + reqMbpString
				+ ", \n\t-mbpPrivate: " + privateMbpRequests
				+ ", \n\t-snapshotMbpRequests: " + snapshotMbpRequests
				+ ", \n\t-mbo: " + reqMboString
				+ ", \n\t-mboPrivate: " + privateMboRequests
				+ ", \n\t-snapshotMboRequests: " + snapshotMboRequests
				+ ", \n\t-serviceName: " + serviceName
				+ ", \n\t-consumerName: " + consumerName);

		try
		{
			AppClient appClient = new AppClient();

			consumer = EmaFactory.createOmmConsumer(EmaFactory
					.createOmmConsumerConfig().consumerName(consumerName)
					.username("user"));

			if (mpRequests != null && mpRequests.length > 0)
			{
				for (int i = 0; i < mpRequests.length; i++)
				{
					ReqMsg requestMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_MARKET_PRICE).serviceName(serviceName).name(mpRequests[i]);

					if (snapshotMpRequests) requestMsg.interestAfterRefresh(false);
					if (privateMpRequests) requestMsg.privateStream(true);

					consumer.registerClient(requestMsg, appClient, 0);
				}
			}

			if (mbpRequests != null && mbpRequests.length > 0)
			{
				for (int i = 0; i < mbpRequests.length; i++)
				{
					ReqMsg requestMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_MARKET_BY_PRICE).serviceName(serviceName).name(mbpRequests[i]);

					if (snapshotMbpRequests) requestMsg.interestAfterRefresh(false);
					if (privateMbpRequests) requestMsg.privateStream(true);

					consumer.registerClient(requestMsg, appClient, 0);
				}
			}

			if (mboRequests != null && mboRequests.length > 0)
			{
				for (int i = 0; i < mboRequests.length; i++)
				{
					ReqMsg requestMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER).serviceName(serviceName).name(mboRequests[i]);

					if (snapshotMboRequests) requestMsg.interestAfterRefresh(false);
					if (privateMboRequests) requestMsg.privateStream(true);

					consumer.registerClient(requestMsg, appClient, 0);
				}
			}

			ReqMsg request = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName(serviceName).name(itemName);

			if (enhancedSymbolListRequestOn)
			{
				ElementList payload = EmaFactory.createElementList();
				ElementEntry entry = EmaFactory.createElementEntry();

				ElementList eePayload = EmaFactory.createElementList();
				ElementEntry eeEntry = EmaFactory.createElementEntry();

				eeEntry.uintValue(":DataStreams", snapshots
						? SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_SNAPSHOTS
						: SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);

				eePayload.add(eeEntry);

				entry.elementList(":SymbolListBehaviors", eePayload);
				payload.add(entry);

				request.payload(payload);
			}

			consumer.registerClient(request, appClient, 0);

			Thread.sleep(60000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
		}
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally
		{
			if (consumer != null) consumer.uninitialize();
		}
	}
}


