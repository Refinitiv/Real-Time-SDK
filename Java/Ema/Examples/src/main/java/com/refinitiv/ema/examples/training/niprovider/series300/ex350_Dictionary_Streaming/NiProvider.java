///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.niprovider.series300.ex350_Dictionary_Streaming;

import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmNiProviderConfig;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmProviderClient;
import com.refinitiv.ema.access.OmmProviderEvent;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
	public DataDictionary dataDictionary = EmaFactory.createDataDictionary();
	public boolean fldDictComplete = false;
	public boolean enumTypeComplete = false;
	private boolean dumpDictionary = false;
	
	AppClient(String[] args)
	{
		int idx = 0;
		
		while ( idx < args.length )
		{
			if ( args[idx].compareTo("-dumpDictionary") == 0 )
			{
				dumpDictionary = true;
			}
			
			++idx;
		}
	}

	
    public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent providerEvent)
    {
    	System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));
		
		System.out.println("Item State: " + refreshMsg.state());
        
        decode(refreshMsg, refreshMsg.complete());
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent providerEvent)
    {
    	System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

		if (statusMsg.hasState())
			System.out.println("Item State: " +statusMsg.state());
		
		System.out.println();
    }

    public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent providerEvent) {}
    public void onAllMsg(Msg msg, OmmProviderEvent providerEvent) {}
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {}
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	
	void decode(Msg msg, boolean complete)
	{
		switch (msg.payload().dataType())
		{
		case DataTypes.SERIES:
			
			if ( msg.name().equals("RWFFld") )
			{
				dataDictionary.decodeFieldDictionary(msg.payload().series(), EmaRdm.DICTIONARY_NORMAL);
				
				if ( complete )
				{
					fldDictComplete = true;
				}
			}
			else if ( msg.name().equals("RWFEnum") )
			{
				dataDictionary.decodeEnumTypeDictionary(msg.payload().series(), EmaRdm.DICTIONARY_NORMAL);
				
				if ( complete )
				{
					enumTypeComplete = true;
				}
			}
		
			if ( fldDictComplete && enumTypeComplete )
			{
				System.out.println();
				System.out.println("Dictionary download complete");
				System.out.println("Dictionary Id : " + dataDictionary.dictionaryId());
				System.out.println("Dictionary field version : " + dataDictionary.fieldVersion());
				System.out.println("Number of dictionary entries : " + dataDictionary.entries().size());
				
				if ( dumpDictionary )
					System.out.println(dataDictionary);
			}
		
			break;
		default:
			break;
		}
	}
}
public class NiProvider
{
    public static void main(String[] args)
    {
        OmmProvider provider = null;
        try
        {
            AppClient appClient = new AppClient(args);
            OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig();
            
            provider = EmaFactory.createOmmProvider(config.username("user"));
			
            provider.registerClient(EmaFactory.createReqMsg().name("RWFFld").filter(EmaRdm.DICTIONARY_NORMAL)
					.serviceName("TEST_NI_PUB").domainType(EmaRdm.MMT_DICTIONARY), appClient);
			
            provider.registerClient(EmaFactory.createReqMsg().name("RWFEnum").filter(EmaRdm.DICTIONARY_NORMAL)
					.serviceName("TEST_NI_PUB").domainType(EmaRdm.MMT_DICTIONARY), appClient);
            
            long triHandle = 6;
            
            FieldList fieldList = EmaFactory.createFieldList();
            
            fieldList.add( EmaFactory.createFieldEntry().real(22, 4100, OmmReal.MagnitudeType.EXPONENT_NEG_2));
            fieldList.add( EmaFactory.createFieldEntry().real(25, 4200, OmmReal.MagnitudeType.EXPONENT_NEG_2));
            fieldList.add( EmaFactory.createFieldEntry().real(30, 20,  OmmReal.MagnitudeType.EXPONENT_0));
            fieldList.add( EmaFactory.createFieldEntry().real(31, 40, OmmReal.MagnitudeType.EXPONENT_0));
            
            provider.submit(EmaFactory.createRefreshMsg().serviceName("TEST_NI_PUB").name("TRI.N")
                    .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
                    .payload(fieldList).complete(true), triHandle);
            
            Thread.sleep(1000);
            
            for( int i = 0; i < 60; i++ )
            {
                fieldList.clear();
                fieldList.add(EmaFactory.createFieldEntry().real(22, 4100 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
                fieldList.add(EmaFactory.createFieldEntry().real(30, 21 + i, OmmReal.MagnitudeType.EXPONENT_0));
                
                provider.submit(EmaFactory.createUpdateMsg().serviceName("TEST_NI_PUB").name("TRI.N").payload( fieldList ), triHandle);
                Thread.sleep(1000);
            }
        } 
        catch (InterruptedException | OmmException excp)
        {
            System.out.println(excp.getMessage());
        }
        finally 
        {
            if (provider != null) provider.uninitialize();
        }
    }
}
