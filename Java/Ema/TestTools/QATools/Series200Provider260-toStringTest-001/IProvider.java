///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.iprovider.series200.ex260_Custom_NestedMsg;

import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.FilterEntry;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.FilterList;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.Vector;
import com.refinitiv.ema.access.VectorEntry;
import com.refinitiv.ema.access.Series;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmIProviderConfig;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmProviderClient;
import com.refinitiv.ema.access.OmmProviderEvent;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.unittest.TestUtilities;

class AppClient implements OmmProviderClient
{
	public long itemHandle = 0;
	public static final int APP_DOMAIN = 200;
	
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent)
	{
		switch(reqMsg.domainType())
		{
		case EmaRdm.MMT_LOGIN:
			processLoginRequest(reqMsg,providerEvent);
			break;
		case APP_DOMAIN:
			processCustomDomainRequest(reqMsg,providerEvent);
			break;
		default:
			processInvalidItemRequest(reqMsg,providerEvent);
			break;
		}
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent providerEvent)
	{
		System.out.println("Received Generic. Item Handle: " + providerEvent.handle() + " Closure: " + providerEvent.closure() );
	}
	
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent providerEvent) {}
	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent providerEvent) {}
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {}
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onAllMsg(Msg msg, OmmProviderEvent providerEvent) {}	
	
	void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		event.provider().submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).
				nameType(EmaRdm.USER_NAME).complete(true).solicited(true).
				state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
				event.handle());
	}
	
	void processCustomDomainRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		if(itemHandle != 0)
		{
			processInvalidItemRequest( reqMsg, event );
			return;
		}
		// API QA RTSDK-1643
		DataDictionary dictionary = EmaFactory.createDataDictionary();
		dictionary.loadFieldDictionary("./RDMFieldDictionary");
		dictionary.loadEnumTypeDictionary("./enumtype.def");
		
		FieldList fieldList = EmaFactory.createFieldList();
		fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
		fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
		
		ElementList elementList = EmaFactory.createElementList();
		elementList.add( EmaFactory.createElementEntry().ascii("ATest_addAscii", "Test_addAscii") );
		elementList.add( EmaFactory.createElementEntry().intValue("QATest_addInt", 50000000) );
		elementList.add( EmaFactory.createElementEntry().uintValue("QATest_addUInt", 1) );
		System.out.println("Error when ElementList.toString(): " + elementList.toString());
		System.out.println("Success when ElementList.toString(dictionary): \n" + elementList.toString(dictionary));
		
		Map map = EmaFactory.createMap();
		map.keyFieldId(1).totalCountHint(0).summaryData(fieldList);
		System.out.println("Error when Map.toString(): " + map.toString());
		System.out.println("Success when Map.toString(dictionary): \n" + map.toString(dictionary));
		
		FilterList filterList = EmaFactory.createFilterList();
		filterList.add(EmaFactory.createFilterEntry().fieldList(3, FilterEntry.FilterAction.SET, fieldList));
		System.out.println("Error when FilterList.toString(): " + filterList.toString());
		System.out.println("Success when FilterList.toString(dictionary): \n" + filterList.toString(dictionary));
		
		Vector vector = EmaFactory.createVector();
		VectorEntry ve = EmaFactory.createVectorEntry().fieldList(1, VectorEntry.VectorAction.SET, fieldList);
		vector.add(ve);
		System.out.println("Error when Vector.toString(): " + vector.toString());
		System.out.println("Success when Vector.toString(dictionary): \n" + vector.toString(dictionary));
		
		Series series = EmaFactory.createSeries();
		series.totalCountHint(0).summaryData(elementList);
		System.out.println("Error when Series.toString(): " + series.toString());
		System.out.println("Success when Series.toString(dictionary): \n" + series.toString(dictionary));
		
		ReqMsg requestMsg = EmaFactory.createReqMsg();
		requestMsg.serviceName("DIRECT_FEED").name("IBM.N");
		System.out.println("Error when ReqMsg.toString(): " + requestMsg.toString());
		System.out.println("Success when ReqMsg.toString(dictionary): \n" + requestMsg.toString(dictionary));
		
		RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
		refreshMsg.domainType(reqMsg.domainType()).name(reqMsg.name()).serviceId(reqMsg.serviceId()).
			state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").
			solicited(true).payload(fieldList);
		System.out.println("Error when RefreshMsg.toString(): " + refreshMsg.toString());
		System.out.println("Success when RefreshMsg.toString(dictionary): \n" + refreshMsg.toString(dictionary));
		
		GenericMsg genericMsg = EmaFactory.createGenericMsg();
		genericMsg.domainType(reqMsg.domainType()).name("genericMsg").payload(refreshMsg);
		System.out.println("Error when GenericMsg.toString(): " + genericMsg.toString());
		System.out.println("Success when GenericMsg.toString(dictionary): \n" + genericMsg.toString(dictionary));
		
		StatusMsg statusMsg = EmaFactory.createStatusMsg();
		statusMsg.name(reqMsg.name()).serviceName(reqMsg.serviceName()).domainType(reqMsg.domainType()).
			state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NOT_FOUND, "Item not found");
		System.out.println("Error when StatusMsg.toString(): " + statusMsg.toString());
		System.out.println("Success when StatusMsg.toString(dictionary): \n" + statusMsg.toString(dictionary));
		
		UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
		updateMsg.payload(fieldList);
		System.out.println("Error when UpdateMsg.toString(): " + updateMsg.toString());
		System.out.println("Success when UpdateMsg.toString(dictionary): \n" + updateMsg.toString(dictionary));
		
		PostMsg postMsg = EmaFactory.createPostMsg();
		postMsg.postId(1).serviceId(1).name(reqMsg.name()).solicitAck(true).payload(elementList);
		System.out.println("Error when PostMsg.toString(): " + postMsg.toString());
		System.out.println("Success when PostMsg.toString(dictionary): \n" + postMsg.toString(dictionary));
		
		AckMsg ackMsg = EmaFactory.createAckMsg();
		ackMsg.domainType(reqMsg.domainType());
		ackMsg.ackId(1).seqNum(1);
		System.out.println("Error when AckMsg.toString(): " + ackMsg.toString());
		System.out.println("Success when AckMsg.toString(dictionary): \n" + ackMsg.toString(dictionary));
		//End API QA RTSDK-1643
		
		event.provider().submit(EmaFactory.createRefreshMsg().domainType(reqMsg.domainType()).serviceId(reqMsg.serviceId()).name(reqMsg.name()).
				state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").solicited(true).
				privateStream(reqMsg.privateStream()).complete(true), event.handle());
		
		event.provider().submit( EmaFactory.createGenericMsg().name("genericMsg").domainType(reqMsg.domainType()).
				payload(EmaFactory.createRefreshMsg().name(reqMsg.name()).serviceName(reqMsg.serviceName()).
				state( OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "NestedMsg").
				payload(fieldList)), event.handle());
		
		itemHandle = event.handle();
	}
	
	void processInvalidItemRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		event.provider().submit(EmaFactory.createStatusMsg().name(reqMsg.name()).serviceName(reqMsg.serviceName()).
				domainType(reqMsg.domainType()).
				state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NOT_FOUND, "Item not found"),
				event.handle());
	}
	
}

public class IProvider
{

	public static void main(String[] args)
	{
		OmmProvider provider = null;
		try
		{
			AppClient appClient = new AppClient();
			FieldList fieldList = EmaFactory.createFieldList();
			GenericMsg genericMsg = EmaFactory.createGenericMsg();
			UpdateMsg updateMsg = EmaFactory.createUpdateMsg();

			provider = EmaFactory.createOmmProvider(EmaFactory.createOmmIProviderConfig().
					operationModel(OmmIProviderConfig.OperationModel.USER_DISPATCH), appClient );
			
			while(appClient.itemHandle == 0)
			{
				provider.dispatch(1000);
				Thread.sleep(1000);
			}
			
			for(int i = 0; i < 60; i++)
			{
			   provider.dispatch(1000);
		
			   fieldList.clear();
			   fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			   fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
			
			   provider.submit(genericMsg.clear().domainType(AppClient.APP_DOMAIN).name("genericMsg").payload(
					updateMsg.clear().payload(fieldList)), appClient.itemHandle );
				
			   Thread.sleep(1000);
			}
		}
		catch (OmmException | InterruptedException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally 
		{
			if (provider != null) provider.uninitialize();
		}
	}
}
