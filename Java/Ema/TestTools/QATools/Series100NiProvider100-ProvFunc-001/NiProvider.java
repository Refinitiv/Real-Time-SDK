///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.examples.training.niprovider.series100.example100__MarketPrice__Streaming;

import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.OmmNiProviderConfig;
import com.thomsonreuters.ema.access.OmmProvider;
import com.thomsonreuters.ema.access.OmmReal;
import com.thomsonreuters.ema.access.OmmState;

public class NiProvider {

	public static void main(String[] args)
	{
		OmmProvider provider = null;
		try
		{
		    //APIQA
			// NIProvider only publishes for 2 seconds
            for (int i = 0; i < 1000000; i++)
            {
				OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig();
				
				System.out.println("!!! createOmmProvider() " + i + " !!!");
				provider = EmaFactory.createOmmProvider(config.host("localhost:14003").username("user"));
				
				long itemHandle = 5;
				
				FieldList fieldList = EmaFactory.createFieldList();
				
				fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
				fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
				
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), itemHandle);
				
				Thread.sleep(1000);
				
				for( int j = 0; j < 2; j++ )
				{
					fieldList.clear();
					fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + j, OmmReal.MagnitudeType.EXPONENT_NEG_2));
					fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + j, OmmReal.MagnitudeType.EXPONENT_0));
					
					provider.submit( EmaFactory.createUpdateMsg().serviceName("NI_PUB").name("IBM.N").payload( fieldList ), itemHandle );
					Thread.sleep(1000);
				}
				System.out.println("!!! provider.uninitialize() " + i + " !!!");
				provider.uninitialize();
            }
			//END APIQA
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
