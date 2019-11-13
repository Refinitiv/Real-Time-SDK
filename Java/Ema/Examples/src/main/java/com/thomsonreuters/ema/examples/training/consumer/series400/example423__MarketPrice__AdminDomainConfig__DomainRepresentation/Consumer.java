///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.thomsonreuters.ema.examples.training.consumer.series400.example423__MarketPrice__AdminDomainConfig__DomainRepresentation;

import java.util.Iterator;

import com.thomsonreuters.ema.access.AckMsg;
import com.thomsonreuters.ema.access.Data;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldEntry;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.OmmConsumer;
import com.thomsonreuters.ema.access.OmmConsumerClient;
import com.thomsonreuters.ema.access.OmmConsumerConfig;
import com.thomsonreuters.ema.access.OmmConsumerEvent;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.UpdateMsg;
import com.thomsonreuters.ema.domain.login.Login.LoginReq;
import com.thomsonreuters.ema.rdm.EmaRdm;


class AppClient implements OmmConsumerClient
{
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		if (refreshMsg.hasMsgKey())
			System.out.println("Item Name: " + refreshMsg.name() + " Service Name: " + refreshMsg.serviceName());
		
		System.out.println("Item State: " + refreshMsg.state());

		if (DataType.DataTypes.FIELD_LIST == refreshMsg.payload().dataType())
			decode(refreshMsg.payload().fieldList());
		
		System.out.println();
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		if (updateMsg.hasMsgKey())
			System.out.println("Item Name: " + updateMsg.name() + " Service Name: " + updateMsg.serviceName());
		
		if (DataType.DataTypes.FIELD_LIST == updateMsg.payload().dataType())
			decode(updateMsg.payload().fieldList());
		
		System.out.println();
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		if (statusMsg.hasMsgKey())
			System.out.println("Item Name: " + statusMsg.name() + " Service Name: " + statusMsg.serviceName());

		if (statusMsg.hasState())
			System.out.println("Item State: " +statusMsg.state());
		
		System.out.println();
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}

	void decode(FieldList fieldList)
	{
		Iterator<FieldEntry> iter = fieldList.iterator();
		FieldEntry fieldEntry;
		while (iter.hasNext())
		{
			fieldEntry = iter.next();
			System.out.print("Fid: " + fieldEntry.fieldId() + " Name = " + fieldEntry.name() + " DataType: " + DataType.asString(fieldEntry.load().dataType()) + " Value: ");

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
				case DataTypes.ENUM :
					System.out.println(fieldEntry.hasEnumDisplay() ? fieldEntry.enumDisplay() : fieldEntry.enumValue());
					break;
				case DataTypes.RMTES:
				    System.out.println(fieldEntry.rmtes());
				    break;
				case DataTypes.ERROR :
					System.out.println("(" + fieldEntry.error().errorCodeAsString() + ")");
					break;
				default :
					System.out.println();
					break;
				}
		}
	}
}

public class Consumer 
{
	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		try
		{
			AppClient appClient = new AppClient();
			
			LoginReq loginReq = EmaFactory.Domain.createLoginReq();
			ReqMsg reqMsg = EmaFactory.createReqMsg();

			consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().operationModel(OmmConsumerConfig.OperationModel.USER_DISPATCH)
					.addAdminMsg(loginReq.name("user").nameType(EmaRdm.USER_NAME).applicationId("127").position("127.0.0.1/net").allowSuspectData(true).message())
					.addAdminMsg(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(EmaRdm.SERVICE_INFO_FILTER  | EmaRdm.SERVICE_STATE_FILTER | EmaRdm.SERVICE_GROUP_FILTER))
					.addAdminMsg(reqMsg.clear().domainType(EmaRdm.MMT_DICTIONARY).filter(EmaRdm.DICTIONARY_VERBOSE).name("RWFFld").serviceId(1))
					.addAdminMsg(reqMsg.clear().domainType(EmaRdm.MMT_DICTIONARY).filter(EmaRdm.DICTIONARY_VERBOSE).name("RWFEnum").serviceId(1)));
		
			consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("IBM.N"), appClient, null);
			
			long startTime = System.currentTimeMillis();
			while (startTime + 60000 > System.currentTimeMillis())
				consumer.dispatch(10);		// calls to onRefreshMsg(), onUpdateMsg(), or onStatusMsg() execute on this thread
		}
		catch (OmmException excp)
		{
			System.out.println(excp);
		}
		finally 
		{
			if (consumer != null) consumer.uninitialize();
		}
	}
}


