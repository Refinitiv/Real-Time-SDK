///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.niprovider.series200.ex210_MBO_Streaming;

import java.nio.ByteBuffer;

import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmNiProviderConfig;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.rdm.EmaRdm;

public class NiProvider
{
    public static void main(String[] args)
    {
        OmmProvider provider = null;
        try
        {
            OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig();
            Map map = EmaFactory.createMap();
            FieldList summary = EmaFactory.createFieldList();
            FieldList entryLoad = EmaFactory.createFieldList();
            long aaoHandle = 5;
            long aggHandle = 6;
                        
            provider = EmaFactory.createOmmProvider(config.username("user"));
            
            summary.add(EmaFactory.createFieldEntry().enumValue(15, 840));
            summary.add(EmaFactory.createFieldEntry().enumValue(53, 1));
            summary.add(EmaFactory.createFieldEntry().enumValue(3423, 1));
            summary.add(EmaFactory.createFieldEntry().enumValue(1709, 2));
            
            map.summaryData(summary);
            
            entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3427, 7.76, OmmReal.MagnitudeType.EXPONENT_NEG_2));
            entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3429, 9600));
            entryLoad.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
            entryLoad.add(EmaFactory.createFieldEntry().rmtes(212, ByteBuffer.wrap("Market Maker".getBytes())));
            
            map.add(EmaFactory.createMapEntry().keyAscii("100", MapEntry.MapAction.ADD, entryLoad));

            provider.submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER).serviceName("TEST_NI_PUB").name("AAO.V")
                            .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
                            .payload(map).complete(true), aaoHandle);

            summary.clear();
            
            summary.add(EmaFactory.createFieldEntry().enumValue(15, 840));
            summary.add(EmaFactory.createFieldEntry().enumValue(53, 1));
            summary.add(EmaFactory.createFieldEntry().enumValue(3423, 1));
            summary.add(EmaFactory.createFieldEntry().enumValue(1709, 2));
            
            map.clear();
            
            map.summaryData(summary);

            entryLoad.clear();
            
            entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3427, 9.92, OmmReal.MagnitudeType.EXPONENT_NEG_2));
            entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3429, 1200));
            entryLoad.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
            entryLoad.add(EmaFactory.createFieldEntry().rmtes(212, ByteBuffer.wrap("Market Maker".getBytes())));
            
            map.add(EmaFactory.createMapEntry().keyAscii("222", MapEntry.MapAction.ADD, entryLoad));
            
            provider.submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER).serviceName("TEST_NI_PUB").name("AGG.V")
                            .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
                            .payload(map).complete(true), aggHandle);

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
                
                entryLoad.clear();
                
                entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3427, 9.92 + i * 0.1, OmmReal.MagnitudeType.EXPONENT_NEG_2));
                entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3429, 1200));
                entryLoad.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
                entryLoad.add(EmaFactory.createFieldEntry().rmtes(212, ByteBuffer.wrap("Market Maker".getBytes())));
                
                map.clear();
                
                map.add(EmaFactory.createMapEntry().keyAscii("222", MapEntry.MapAction.UPDATE, entryLoad));
                
                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER).serviceName("TEST_NI_PUB").name("AGG.V")
                                .payload(map), aggHandle);                
                
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
