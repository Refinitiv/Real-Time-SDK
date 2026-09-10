/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024,2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.niprovider.series300.ex310_MBO_Streaming;

import java.nio.ByteBuffer;
import java.util.Arrays;

import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.FilterEntry;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmNiProviderConfig;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.domain.directory.DirectoryRefresh;
import com.refinitiv.ema.domain.directory.DirectoryService;
import com.refinitiv.ema.domain.directory.DirectoryServiceInfo;
import com.refinitiv.ema.domain.directory.DirectoryServiceState;
import com.refinitiv.ema.rdm.EmaRdm;

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

            final DirectoryRefresh directoryRefresh =  EmaFactory.Domain.createDirectoryRefresh();
            final DirectoryService directoryService = EmaFactory.Domain.createDirectoryService();
            final DirectoryServiceState directoryServiceState = EmaFactory.Domain.createDirectoryServiceState();
            final DirectoryServiceInfo directoryServiceInfo = EmaFactory.Domain.createDirectoryServiceInfo();

            directoryServiceInfo.serviceName("TEST_NI_PUB");
            directoryServiceInfo.capabilitiesList(Arrays.asList((long)EmaRdm.MMT_MARKET_PRICE, (long)EmaRdm.MMT_MARKET_BY_ORDER));
            directoryServiceInfo.dictionariesUsedList(Arrays.asList("RWFFld", "RWFEnum"));
            directoryServiceInfo.action(FilterEntry.FilterAction.SET);

            directoryServiceState.serviceState(EmaRdm.SERVICE_UP);
            directoryServiceState.action(FilterEntry.FilterAction.SET);

            directoryService.serviceId(1);
            directoryService.action(MapEntry.MapAction.ADD);
            directoryService.state(directoryServiceState);
            directoryService.info(directoryServiceInfo);

            directoryRefresh.serviceList().add(directoryService);
            directoryRefresh.filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER);
            directoryRefresh.clearCache(true);
            directoryRefresh.complete(true);

            provider.submit(directoryRefresh.message(), sourceDirectoryHandle);

            FieldList summary = EmaFactory.createFieldList();
            FieldList entryLoad = EmaFactory.createFieldList();
            
            summary.add(EmaFactory.createFieldEntry().enumValue(15, 840));
            summary.add(EmaFactory.createFieldEntry().enumValue(53, 1));
            summary.add(EmaFactory.createFieldEntry().enumValue(3423, 1));
            summary.add(EmaFactory.createFieldEntry().enumValue(1709, 2));
            
            Map map = EmaFactory.createMap();
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
