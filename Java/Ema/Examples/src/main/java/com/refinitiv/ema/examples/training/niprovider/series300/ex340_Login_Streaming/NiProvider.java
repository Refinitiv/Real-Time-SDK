/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.niprovider.series300.ex340_Login_Streaming;

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
import com.refinitiv.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
    public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent providerEvent)
    {
        System.out.println("Received Refresh. Handle: " + providerEvent.handle() + " Closure: " + providerEvent.closure());
        System.out.println(refreshMsg.toString());
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent providerEvent)
    {
        System.out.println("Received Status. Handle: " + providerEvent.handle() + " Closure: " + providerEvent.closure());
        System.out.println(statusMsg.toString());
    }

    public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent providerEvent) {}
    public void onAllMsg(Msg msg, OmmProviderEvent providerEvent) {}
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {}
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
}
public class NiProvider
{
    public static void main(String[] args)
    {
        OmmProvider provider = null;
        try
        {
            AppClient appClient = new AppClient();
            OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig();
            
            provider = EmaFactory.createOmmProvider(config.username("user"));
            
            provider.registerClient(EmaFactory.createReqMsg().domainType(EmaRdm.MMT_LOGIN), appClient);
            
            Thread.sleep(1000);
            
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
