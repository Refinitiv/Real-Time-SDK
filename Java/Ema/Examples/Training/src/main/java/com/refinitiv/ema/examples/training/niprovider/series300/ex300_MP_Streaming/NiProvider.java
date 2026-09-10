/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024,2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.niprovider.series300.ex300_MP_Streaming;


import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.FilterEntry;
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

import java.util.Arrays;


public class NiProvider {

	public static void main(String[] args)
	{
		OmmProvider provider = null;
		try
		{
			OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig();
			
			provider = EmaFactory.createOmmProvider(config.adminControlDirectory(OmmNiProviderConfig.AdminControl.USER_CONTROL)
					.username("user"));			
			
			long sourceDirectoryHandle = 1;

            final DirectoryRefresh directoryRefresh =  EmaFactory.Domain.createDirectoryRefresh();
            final DirectoryService directoryService = EmaFactory.Domain.createDirectoryService();
            final DirectoryServiceState directoryServiceState = EmaFactory.Domain.createDirectoryServiceState();
            final DirectoryServiceInfo directoryServiceInfo = EmaFactory.Domain.createDirectoryServiceInfo();

            directoryServiceInfo.serviceName("NI_PUB");
            directoryServiceInfo.capabilitiesList(Arrays.asList((long)EmaRdm.MMT_MARKET_PRICE, (long)EmaRdm.MMT_MARKET_BY_PRICE));
            directoryServiceInfo.dictionariesUsedList(Arrays.asList("RWFFld", "RWFEnum"));
            directoryServiceInfo.action(FilterEntry.FilterAction.SET);

            directoryServiceState.serviceState(EmaRdm.SERVICE_UP);
            directoryServiceState.action(FilterEntry.FilterAction.SET);

            directoryService.serviceId(2);
            directoryService.action(MapEntry.MapAction.ADD);
            directoryService.state(directoryServiceState);
            directoryService.info(directoryServiceInfo);

            directoryRefresh.serviceList().add(directoryService);
            directoryRefresh.filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER);

            provider.submit(directoryRefresh.message(), sourceDirectoryHandle);

			long itemHandle = 5;
			
			FieldList fieldList = EmaFactory.createFieldList();
			
			fieldList.add( EmaFactory.createFieldEntry().real(22, 14400, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 14700, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			provider.submit( EmaFactory.createRefreshMsg().serviceId(2).name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(1000);
			
			for( int i = 0; i < 60; i++ )
			{
				fieldList.clear();
				fieldList.add(EmaFactory.createFieldEntry().real(22, 14400 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
				
				provider.submit( EmaFactory.createUpdateMsg().serviceId(2).name("IBM.N").payload( fieldList ), itemHandle );
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
