package com.rtsdk.ema.examples.training.niprovider.series300.example340__Login__Streaming;

import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.FieldList;
import com.rtsdk.ema.access.GenericMsg;
import com.rtsdk.ema.access.Msg;
import com.rtsdk.ema.access.OmmException;
import com.rtsdk.ema.access.OmmNiProviderConfig;
import com.rtsdk.ema.access.OmmProvider;
import com.rtsdk.ema.access.OmmProviderClient;
import com.rtsdk.ema.access.OmmProviderEvent;
import com.rtsdk.ema.access.OmmReal;
import com.rtsdk.ema.access.OmmState;
import com.rtsdk.ema.access.PostMsg;
import com.rtsdk.ema.access.RefreshMsg;
import com.rtsdk.ema.access.ReqMsg;
import com.rtsdk.ema.access.StatusMsg;
import com.rtsdk.ema.rdm.EmaRdm;

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
