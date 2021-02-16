package com.refinitiv.ema.examples.training.iprovider.series100.ex180_MP_Websocket;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient, OmmProviderErrorClient {

    long itemHandle;

    @Override
    public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent providerEvent) {
    }

    @Override
    public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent providerEvent) {
    }

    @Override
    public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent providerEvent) {
    }

    @Override
    public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {
    }

    @Override
    public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent) {
        switch (reqMsg.domainType()) {
            case EmaRdm.MMT_LOGIN:
                processLoginRequest(reqMsg, providerEvent);
                break;
            case EmaRdm.MMT_MARKET_PRICE:
                processMarketPriceRequest(reqMsg, providerEvent);
                break;
            default:
                processInvalidItemRequest(reqMsg, providerEvent);
                break;
        }
    }

    @Override
    public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {

    }

    @Override
    public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {
    }

    @Override
    public void onAllMsg(Msg msg, OmmProviderEvent providerEvent) {
    }

    @Override
    public void onInvalidHandle(long handle, String text) {
    }

    @Override
    public void onInvalidUsage(String text) {
    }

    @Override
    public void onInvalidUsage(String text, int errorCode) {
        System.out.printf("\nonInvalidUsage callback function\n" +
                "Error text: %s\n" +
                "Error code: %d\n", text, errorCode
        );
        itemHandle = 0;
    }

    @Override
    public void onJsonConverterError(ProviderSessionInfo providerSessionInfo, int errorCode, String text) {
        System.out.printf("\nonJsonConverter callback function\n" +
                "Error text: %s\n" +
                "Error code: %d\n" +
                "Closing the client channel\n", text, errorCode
        );
        providerSessionInfo.getProvider().closeChannel(providerSessionInfo.getClientHandle());
    }

    void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event) {
        event.provider().submit(
                EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN)
                        .attrib(EmaFactory.createElementList())
                        .name(reqMsg.name())
                        .nameType(EmaRdm.USER_NAME)
                        .complete(true)
                        .solicited(true)
                        .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
                event.handle());
    }

    void processMarketPriceRequest(ReqMsg reqMsg, OmmProviderEvent event) {
        if (itemHandle != 0) {
            processInvalidItemRequest(reqMsg, event);
            return;
        }

        FieldList fieldEntries = EmaFactory.createFieldList();
        fieldEntries.add(EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldEntries.add(EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldEntries.add(EmaFactory.createFieldEntry().real(30, 9, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldEntries.add(EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        event.provider().submit(
                EmaFactory.createRefreshMsg()
                        .name(reqMsg.name())
                        .serviceName(reqMsg.serviceName())
                        .solicited(true)
                        .complete(true)
                        .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed")
                        .payload(fieldEntries),
                event.handle()
        );

        itemHandle = event.handle();
    }

    void processInvalidItemRequest(ReqMsg reqMsg, OmmProviderEvent event) {
        event.provider().submit(
                EmaFactory.createStatusMsg()
                        .name(reqMsg.name())
                        .serviceName(reqMsg.serviceName())
                        .domainType(reqMsg.domainType())
                        .state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NOT_FOUND, "Item not found"),
                event.handle());
    }
}

public class IProvider {
    public static void main(String... args) {
        OmmProvider provider = null;
        try {
            AppClient appClient = new AppClient();

            provider = EmaFactory.createOmmProvider(
                    EmaFactory.createOmmIProviderConfig()
                            .providerName("Provider_3")
                            .operationModel(OmmIProviderConfig.OperationModel.USER_DISPATCH),
                    appClient, appClient);

            //Prepare data
            FieldList fieldEntries = EmaFactory.createFieldList();


            int count = 0;
            long startTime = System.currentTimeMillis();

            while (startTime + 60000 > System.currentTimeMillis()) {
                provider.dispatch(1000000);
                if (appClient.itemHandle != 0) {
                    fieldEntries.clear();
                    fieldEntries.add(EmaFactory.createFieldEntry().real(22, 3391 + count, OmmReal.MagnitudeType.EXPONENT_NEG_2));
                    fieldEntries.add(EmaFactory.createFieldEntry().real(30, 10 + count, OmmReal.MagnitudeType.EXPONENT_0));
                    provider.submit(EmaFactory.createUpdateMsg().payload(fieldEntries), appClient.itemHandle);
                    count++;
                }
            }
        } catch (OmmException e) {
            System.out.println(e.getMessage());
        } finally {
            if (provider != null) {
                provider.uninitialize();
            }
        }
    }
}
