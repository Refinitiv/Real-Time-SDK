///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2024 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.niprovider.series400.ex405_MP_MsgPacking;

import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmNiProviderConfig;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PackedMsg;
import com.refinitiv.ema.access.UpdateMsg;

public class NiProvider {

	public static void main(String[] args)
	{
		OmmProvider provider = null;
		try
		{
			OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig();
			
			provider = EmaFactory.createOmmProvider(config.username("user"));
			
			long itemHandle = 5;
			
			PackedMsg packedMsg = EmaFactory.createPackedMsg(provider);
			packedMsg.initBuffer();
			
			FieldList fieldList = EmaFactory.createFieldList();
			UpdateMsg msg;
			
			boolean sentRefreshMsg = false;	// Keeps track if we packed and sent our first refresh

			// Once connected, run application for 60 seconds, submitting 60 packed messages total.
			for( int i = 0; i < 60; i++ )
			{
				// Each message packs 10 messages before submitting the full packed message.
				for( int j = 0; j < 10; j++ )
				{
					if (!sentRefreshMsg)
					{
						fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
						fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
						fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
						fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));

						packedMsg.addMsg( EmaFactory.createRefreshMsg().serviceName("NI_PUB").name("IBM.N")
								.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
								.payload(fieldList).complete(true), itemHandle);

						sentRefreshMsg = true;
					}
					
					fieldList.clear();
					fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + j, OmmReal.MagnitudeType.EXPONENT_NEG_2));
					fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + j, OmmReal.MagnitudeType.EXPONENT_0));
					
					msg = EmaFactory.createUpdateMsg().serviceName("NI_PUB").name("IBM.N").payload( fieldList );
					
					packedMsg.addMsg(msg, itemHandle);
				}
				if (packedMsg.packedMsgCount() > 0)
				{
					provider.submit(packedMsg);
					packedMsg.initBuffer();	 	// Re-initialize buffer for next set of packed messages.
				}
				
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
