///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.examples.training.niprovider.series300.example310__MarketByOrder__Streaming;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.FilterEntry;
import com.thomsonreuters.ema.access.FilterList;
import com.thomsonreuters.ema.access.Map;
import com.thomsonreuters.ema.access.MapEntry;
import com.thomsonreuters.ema.access.OmmArray;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.OmmNiProviderConfig;
import com.thomsonreuters.ema.access.OmmProvider;
import com.thomsonreuters.ema.access.OmmReal;
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.rdm.EmaRdm;

public class NiProvider
{
    public static void main(String[] args)
    {
        OmmProvider provider = null;
        try
        {
            OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig();
            
            provider = EmaFactory.createOmmProvider(config.adminControlDirectory(OmmNiProviderConfig.AdminControl.USER_CONTROL).username("user"));
            
            long sourceDirectoryHandle = 1;
            long aaoHandle = 5;
            
            OmmArray capablities = EmaFactory.createOmmArray();
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_ORDER));
            
            OmmArray dictionaryUsed = EmaFactory.createOmmArray();
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFFld"));
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFEnum"));
            
            ElementList serviceInfoId = EmaFactory.createElementList();
            
            serviceInfoId.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "TEST_NI_PUB"));   
            serviceInfoId.add(EmaFactory.createElementEntry().array( EmaRdm.ENAME_CAPABILITIES, capablities));
            serviceInfoId.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));
            
            ElementList serviceStateId = EmaFactory.createElementList();
            serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
            
            FilterList filterList = EmaFactory.createFilterList();
            filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId));
            filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));
        
            Map map = EmaFactory.createMap();
            map.add( EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.ADD, filterList));
            
            provider.submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_DIRECTORY).clearCache(true).filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER)
                            .payload(map).complete(true), sourceDirectoryHandle);
            
            FieldList summary = EmaFactory.createFieldList();
            FieldList entryLoad = EmaFactory.createFieldList();
            
            summary.add(EmaFactory.createFieldEntry().enumValue(15, 840));
            summary.add(EmaFactory.createFieldEntry().enumValue(53, 1));
            summary.add(EmaFactory.createFieldEntry().enumValue(3423, 1));
            summary.add(EmaFactory.createFieldEntry().enumValue(1709, 2));
            
            map.clear();
            
            map.summaryData(summary);
            
            entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3427, 7.76, OmmReal.MagnitudeType.EXPONENT_NEG_2));
            entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3429, 9600));
            entryLoad.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
            entryLoad.add(EmaFactory.createFieldEntry().rmtes(212, ByteBuffer.wrap("Market Maker".getBytes())));
            
            map.add(EmaFactory.createMapEntry().keyAscii("100", MapEntry.MapAction.ADD, entryLoad));

            provider.submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER).serviceName("TEST_NI_PUB").name("AAO.V")
                            .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
                            .payload(map).complete(true), aaoHandle);
            
            Thread.sleep(1000);
            
            for( int i = 0; i < 60; i++ )
            {
                entryLoad.clear();
                
                entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3427, 7.76 + i * 0.1, OmmReal.MagnitudeType.EXPONENT_NEG_2));
                entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3429, 9600));
                entryLoad.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
                entryLoad.add(EmaFactory.createFieldEntry().rmtes(212, ByteBuffer.wrap("Market Maker".getBytes())));
                
                map.clear();
                
                map.add(EmaFactory.createMapEntry().keyAscii("100", MapEntry.MapAction.UPDATE, entryLoad));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER).serviceName("TEST_NI_PUB").name("AAO.V")
                                .payload(map), aaoHandle);
                
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
