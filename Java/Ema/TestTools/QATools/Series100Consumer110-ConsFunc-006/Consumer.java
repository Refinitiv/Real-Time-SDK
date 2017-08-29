///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright Thomson Reuters 2016. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

package com.thomsonreuters.ema.examples.training.consumer.series100.example110__MarketPrice__FileConfig;

import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.AckMsg;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.OmmArray;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.UpdateMsg;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.OmmConsumer;
import com.thomsonreuters.ema.access.OmmConsumerClient;
import com.thomsonreuters.ema.access.OmmConsumerEvent;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.rdm.EmaRdm;

class AppClient implements OmmConsumerClient
{
    ReqMsg req = EmaFactory.createReqMsg();
    ElementList eleList = EmaFactory.createElementList();
    OmmArray array = EmaFactory.createOmmArray();

    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
    {
    }

    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
    {
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event)
    {
    }

    void iter(OmmConsumer consumer, AppClient client)
    {

        // repeat send ten same items in batch
        req.clear();
        eleList.clear();
        array.clear();
        array.add(EmaFactory.createOmmArrayEntry().ascii("IBM.N"));
        array.add(EmaFactory.createOmmArrayEntry().ascii("MSFT.O"));
        array.add(EmaFactory.createOmmArrayEntry().ascii("GOOG.O"));
        array.add(EmaFactory.createOmmArrayEntry().ascii("TRI.N"));
        array.add(EmaFactory.createOmmArrayEntry().ascii("GAZP.MM"));
        array.add(EmaFactory.createOmmArrayEntry().ascii(".09IY"));
        array.add(EmaFactory.createOmmArrayEntry().ascii(".TRXFLDESP"));
        array.add(EmaFactory.createOmmArrayEntry().ascii("A3M.MC"));
        array.add(EmaFactory.createOmmArrayEntry().ascii("ABE.MC"));
        array.add(EmaFactory.createOmmArrayEntry().ascii("ACS.MC"));
        eleList.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_BATCH_ITEM_LIST, array));

        long handle = consumer.registerClient(req.serviceName("DIRECT_FEED").interestAfterRefresh(false).payload(eleList), client, 1);
    }

    void iter1(OmmConsumer consumer, AppClient client, int count)
    {

        // repeat send ten different items in batch
        req.clear();
        eleList.clear();
        array.clear();
        array.add(EmaFactory.createOmmArrayEntry().ascii("IBM.N" + count));
        array.add(EmaFactory.createOmmArrayEntry().ascii("MSFT.O" + count));
        array.add(EmaFactory.createOmmArrayEntry().ascii("GOOG.O" + count));
        array.add(EmaFactory.createOmmArrayEntry().ascii("TRI.N" + count));
        array.add(EmaFactory.createOmmArrayEntry().ascii("GAZP.MM" + count));
        array.add(EmaFactory.createOmmArrayEntry().ascii(".09IY" + count));
        array.add(EmaFactory.createOmmArrayEntry().ascii(".TRXFLDESP" + count));
        array.add(EmaFactory.createOmmArrayEntry().ascii("A3M.MC" + count));
        array.add(EmaFactory.createOmmArrayEntry().ascii("ABE.MC" + count));
        array.add(EmaFactory.createOmmArrayEntry().ascii("ACS.MC" + count));
        eleList.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_BATCH_ITEM_LIST, array));

        long handle = consumer.registerClient(req.serviceName("DIRECT_FEED").interestAfterRefresh(false).payload(eleList), client, 1);
    }

    void iter2(OmmConsumer consumer, AppClient client, int count)
    {

        // repeat send 10 diff item as single item request
        consumer.registerClient(req.clear().serviceName("DIRECT_FEED").name("IBM.N" + count).interestAfterRefresh(false), client, 1);
        consumer.registerClient(req.clear().serviceName("DIRECT_FEED").name("MSFT.O" + count).interestAfterRefresh(false), client, 1);
        consumer.registerClient(req.clear().serviceName("DIRECT_FEED").name("GOOG.O" + count).interestAfterRefresh(false), client, 1);
        consumer.registerClient(req.clear().serviceName("DIRECT_FEED").name("TRI.N" + count).interestAfterRefresh(false), client, 1);
        consumer.registerClient(req.clear().serviceName("DIRECT_FEED").name("GAZP.MM" + count).interestAfterRefresh(false), client, 1);
        consumer.registerClient(req.clear().serviceName("DIRECT_FEED").name(".09IY" + count).interestAfterRefresh(false), client, 1);
        consumer.registerClient(req.clear().serviceName("DIRECT_FEED").name(".TRXFLDESP" + count).interestAfterRefresh(false), client, 1);
        consumer.registerClient(req.clear().serviceName("DIRECT_FEED").name("A3M.MC" + count).interestAfterRefresh(false), client, 1);
        consumer.registerClient(req.clear().serviceName("DIRECT_FEED").name("ABE.MC" + count).interestAfterRefresh(false), client, 1);
        consumer.registerClient(req.clear().serviceName("DIRECT_FEED").name("ACS.MC" + count).interestAfterRefresh(false), client, 1);

    }

    public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent)
    {
    }

    public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent)
    {
    }

    public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent)
    {
    }

}

public class Consumer
{
    public static void main(String[] args)
    {
        OmmConsumer consumer = null;
        try
        {
            AppClient appClient = new AppClient();

            consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().username("rmds"));

            long startTime = System.currentTimeMillis();
            int count = 0;
            while (true)
            {
                count++;
                appClient.iter1(consumer, appClient, count);
                Thread.sleep(500);
                System.out.println("*** cnt= " + count);
            }

        }
        catch (InterruptedException | OmmException excp)
        {
            System.out.println(excp.getMessage());
        }
        finally
        {
            if (consumer != null)
                consumer.uninitialize();
        }
    }
}
//END APIQA
