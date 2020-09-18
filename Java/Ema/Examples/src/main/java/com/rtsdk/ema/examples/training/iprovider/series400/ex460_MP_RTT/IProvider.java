package com.rtsdk.ema.examples.training.iprovider.series400.ex460_MP_RTT;

import com.rtsdk.ema.access.*;
import com.rtsdk.ema.rdm.EmaRdm;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.rdm.DomainTypes;

import java.util.*;
import java.util.Map;

class AppClient implements OmmProviderClient
{
    private boolean enableRTT;
    /*
      holds the last latency value for each client that supports RTT requests
      clientLatencyMap.keySet() holds handles to all consumers that can receive RTT requests
    */
    public Map<Long, Long> clientLatencyMap = new HashMap<>();

    //holds all item requests for each client
    public Map<Long, List<Long>> clientItemHandlesMap = new HashMap<>();

    public AppClient(boolean rtt) {
        enableRTT = rtt;
    }

    public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent event)
    {
        switch (reqMsg.domainType())
        {
            case EmaRdm.MMT_LOGIN :
                processLoginRequest(reqMsg, event);
                break;
            case EmaRdm.MMT_MARKET_PRICE :
                processMarketPriceRequest(reqMsg, event);
                break;
            default :
                break;
        }
    }

    public void onRefreshMsg(RefreshMsg refreshMsg,	OmmProviderEvent event){}
    public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event){}

    public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent event){
        switch (genericMsg.domainType()) {
            case EmaRdm.MMT_LOGIN:
                processRTT(genericMsg, event);
                break;
            default:
        }
    }
    public void onPostMsg(PostMsg postMsg, OmmProviderEvent event){}
    public void onReissue(ReqMsg reqMsg, OmmProviderEvent event){}
    public void onClose(ReqMsg reqMsg, OmmProviderEvent event){
        switch (reqMsg.domainType())
        {
            case EmaRdm.MMT_LOGIN :
                clientLatencyMap.remove(event.handle());
                clientItemHandlesMap.remove(event.clientHandle());
                break;
            case EmaRdm.MMT_MARKET_PRICE :
                List<Long> list = clientItemHandlesMap.get(event.clientHandle());
                list.remove(event.handle());
                if (list.isEmpty()) {
                    clientItemHandlesMap.remove(event.clientHandle());
                }
                break;
            default :
                break;
        }
    }
    public void onAllMsg(Msg msg, OmmProviderEvent event){}

    void processRTT(GenericMsg genericMsg, OmmProviderEvent event) {
        if (clientLatencyMap.containsKey(event.handle())) {
            if (genericMsg.payload().dataType() == DataTypes.ELEMENT_LIST && genericMsg.domainType() == DomainTypes.LOGIN) {
                System.out.println("Received login RTT message from Consumer " + event.handle());
                ElementList data = genericMsg.payload().elementList();
                for ( ElementEntry elem : data) {
                    if (elem.name().equals(EmaRdm.ENAME_TICKS)) {
                        System.out.println("        RTT Tick value is: " + elem.uintValue());
                        long latency = System.nanoTime() - elem.uintValue();
                        clientLatencyMap.put(event.handle(), latency);
                        System.out.println("        Last RTT message latency is: " + latency);
                    }
                }
            }
        }
    }

    void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        if (enableRTT) {
            if (reqMsg.attrib().dataType() == DataTypes.ELEMENT_LIST) {
                ElementList reqAttributes = reqMsg.attrib().elementList();
                for ( ElementEntry reqAttrib : reqAttributes ) {
                    if (reqAttrib.name().equals(EmaRdm.ENAME_LATENCY)) {
                        clientLatencyMap.put(event.handle(), 0L);
                        System.out.println("Consumer with handle " + event.handle() + " supports gathering RTT statistics");
                    }
                }
            }
            ElementList elementList = EmaFactory.createElementList();
            elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LATENCY, EmaRdm.LOGIN_RTT_ELEMENT));
            event.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).
                            nameType(EmaRdm.USER_NAME).complete(true).solicited(true).
                            state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted").
                            attrib(elementList),
                    event.handle() );

        } else {
            event.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).
                            nameType(EmaRdm.USER_NAME).complete(true).solicited(true).
                            state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
                    event.handle() );
        }

    }

    void processMarketPriceRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        FieldList fieldList = EmaFactory.createFieldList();
        fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
        fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));

        event.provider().submit( EmaFactory.createRefreshMsg().name(reqMsg.name()).serviceId(reqMsg.serviceId()).solicited(true).
                        state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").
                        payload(fieldList).complete(true),
                event.handle() );

        if (clientItemHandlesMap.containsKey(event.clientHandle())) {
            clientItemHandlesMap.get(event.clientHandle()).add(event.handle());
        } else {
            LinkedList<Long> list = new LinkedList<>();
            list.add(event.handle());
            clientItemHandlesMap.put(event.clientHandle(), list);
        }
    }

    public void sendRTTRequests(OmmProvider provider) {
        if (!enableRTT) {
            System.out.println("This provider does not support RTT");
        } else {
            ElementList elementList = EmaFactory.createElementList();
            for (Long handle : clientLatencyMap.keySet()) {
                elementList.clear();
                Long latency = clientLatencyMap.get(handle);
                if (latency != 0) {
                    elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LATENCY, latency));
                }
                elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_TICKS, System.nanoTime()));
                provider.submit(EmaFactory.createGenericMsg().payload(elementList).domainType(DomainTypes.LOGIN).providerDriven(true).complete(true), handle);
            }
        }
    }

    public void sendUpdates(OmmProvider provider, FieldList fieldList) {
        for (Long cl_h : clientItemHandlesMap.keySet()) {
            List<Long> list = clientItemHandlesMap.get(cl_h);
            for (Long ih : list)  {
                provider.submit( EmaFactory.createUpdateMsg().payload( fieldList ), ih );
            }
        }
    }

    public boolean standBy() {
        return clientItemHandlesMap.size() == 0;
    }
}

public class IProvider
{
    static long DELTA = 1000;

    public static void main(String[] args)
    {

        long nextRequestTime = System.currentTimeMillis() + DELTA;
        OmmProvider provider = null;
        try
        {
            OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig();
            AppClient appClient = new AppClient(true);

            provider = EmaFactory.createOmmProvider(config.operationModel(OmmIProviderConfig.OperationModel.USER_DISPATCH).port("14002"), appClient);
            FieldList fieldList = EmaFactory.createFieldList();

            Random rnd = new Random();
            while (appClient.standBy()) {
                provider.dispatch(500);
                Thread.sleep(500);
            }

            for (int i = 0; i < 6000; i++)
            {
                provider.dispatch(50);

                if (System.currentTimeMillis() >= nextRequestTime) {

                    appClient.sendRTTRequests(provider);
                    fieldList.clear();
                    fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + rnd.nextInt(50), OmmReal.MagnitudeType.EXPONENT_NEG_2));
                    fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + rnd.nextInt(50), OmmReal.MagnitudeType.EXPONENT_0));

                    appClient.sendUpdates(provider, fieldList);

                    nextRequestTime = System.currentTimeMillis() + DELTA;
                }

                Thread.sleep(50);
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
