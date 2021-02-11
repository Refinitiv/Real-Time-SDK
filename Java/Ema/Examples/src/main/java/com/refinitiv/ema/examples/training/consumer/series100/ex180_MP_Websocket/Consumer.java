package com.refinitiv.ema.examples.training.consumer.series100.ex180_MP_Websocket;

import com.refinitiv.ema.access.*;

class AppClient implements OmmConsumerClient {

    @Override
    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent consumerEvent) {
        System.out.println(refreshMsg);
    }

    @Override
    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent consumerEvent) {
        System.out.println(updateMsg);
    }

    @Override
    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent consumerEvent) {
        System.out.println(statusMsg);
    }

    @Override
    public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent) {
    }

    @Override
    public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent) {
    }

    @Override
    public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent) {
    }
}

public class Consumer {
    public static void main(String[] args) {
        OmmConsumer consumer = null;
        try {
            AppClient appClient = new AppClient();

            consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig()
                    .consumerName("Consumer_6")
                    .operationModel(OmmConsumerConfig.OperationModel.USER_DISPATCH));
            ReqMsg reqMsg = EmaFactory.createReqMsg();
            consumer.registerClient(reqMsg.serviceName("DIRECT_FEED").name("IBM.N"), appClient);
            long startTime = System.currentTimeMillis();
            while (startTime + 60000 > System.currentTimeMillis()) {
                consumer.dispatch(10);
            }
        } catch (OmmException excp) {
            System.out.println(excp.getMessage());
        } finally {
            if (consumer != null) consumer.uninitialize();
        }
    }
}
