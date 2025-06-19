/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

package com.refinitiv.ema.examples.training.consumer.series400.ex440_System_TunStrm;

import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.ClassOfService;
import com.refinitiv.ema.access.CosAuthentication;
import com.refinitiv.ema.access.CosDataIntegrity;
import com.refinitiv.ema.access.CosFlowControl;
import com.refinitiv.ema.access.CosGuarantee;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.TunnelStreamRequest;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmState;

class AppClient implements OmmConsumerClient
{
    private OmmConsumer _ommConsumer;
    private long _tunnelStreamHandle;
    private boolean _subItemOpen;

    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
    {
        System.out.println("Handle: " + event.handle());
        System.out.println("Parent Handle: " + event.parentHandle());
        System.out.println("Closure: " + event.closure());

        System.out.println(refreshMsg);

        System.out.println();
    }

    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
    {
        System.out.println("Handle: " + event.handle());
        System.out.println("Parent Handle: " + event.parentHandle());
        System.out.println("Closure: " + event.closure());

        System.out.println(updateMsg);

        System.out.println();
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event)
    {
        System.out.println("Handle: " + event.handle());
        System.out.println("Parent Handle: " + event.parentHandle());
        System.out.println("Closure: " + event.closure());

        System.out.println(statusMsg);

        if (!_subItemOpen && event.handle() == _tunnelStreamHandle && statusMsg.hasState() && statusMsg.state().streamState() == OmmState.StreamState.OPEN)
        {
            _subItemOpen = true;

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            _ommConsumer.registerClient(reqMsg.name("TUNNEL_IBM").serviceId(1), this, 1, _tunnelStreamHandle);
            _ommConsumer.registerClient(reqMsg.clear().name("TUNNEL_TRI").serviceId(1), this, 1, _tunnelStreamHandle);
            _ommConsumer.registerClient(reqMsg.clear().name("TUNNEL_A").serviceId(1), this, 1, _tunnelStreamHandle);
            _ommConsumer.registerClient(reqMsg.clear().name("TUNNEL_B").serviceId(1), this, 1, _tunnelStreamHandle);
            _ommConsumer.registerClient(reqMsg.clear().name("TUNNEL_C").serviceId(1), this, 1, _tunnelStreamHandle);
            _ommConsumer.registerClient(reqMsg.clear().name("TUNNEL_D").serviceId(1), this, 1, _tunnelStreamHandle);
        }

        System.out.println();
    }

    public void setOmmConsumer(OmmConsumer ommConsumer)
    {
        _ommConsumer = ommConsumer;
    }

    public void setTunnelStreamHandle(long tunnelStreamHandle)
    {
        _tunnelStreamHandle = tunnelStreamHandle;
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
        if (args.length != 1 && args.length != 2)
        {
            System.out.println("Error: Invalid number of arguments");
            System.out.println("Usage: -m 1-6(testcase 1 or 2 or 3 or 4 ...) ");
            return;
        }

        OmmConsumer consumer = null;
        try
        {
            AppClient appClient = new AppClient();

            consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().username("user"));
            appClient.setOmmConsumer(consumer);

            CosAuthentication cosAuthentication = EmaFactory.createCosAuthentication();
            CosDataIntegrity cosDataIntegrity = EmaFactory.createCosDataIntegrity();
            CosFlowControl cosFlowControl = EmaFactory.createCosFlowControl();
            CosGuarantee cosGuarantee = EmaFactory.createCosGuarantee();
            cosAuthentication.type(CosAuthentication.CosAuthenticationType.OMM_LOGIN);

            int temp = Integer.valueOf(args[1]).intValue();

            if (args[0].equals("-m") || args[0].equals("-M"))
            {
                cosDataIntegrity.type(CosDataIntegrity.CosDataIntegrityType.RELIABLE);
                cosFlowControl.type(CosFlowControl.CosFlowControlType.BIDIRECTIONAL).recvWindowSize(120);
                cosGuarantee.type(CosGuarantee.CosGuaranteeType.NONE);

                if (temp == 1)
                {
                    cosDataIntegrity.type(CosDataIntegrity.CosDataIntegrityType.RELIABLE);
                    cosFlowControl.type(CosFlowControl.CosFlowControlType.BIDIRECTIONAL).recvWindowSize(120);
                    cosGuarantee.type(CosGuarantee.CosGuaranteeType.NONE);
                }
                else if (temp == 2)
                {
                    cosDataIntegrity.type(CosDataIntegrity.CosDataIntegrityType.RELIABLE);
                    cosFlowControl.type(CosFlowControl.CosFlowControlType.NONE).recvWindowSize(1200);
                    cosGuarantee.type(CosGuarantee.CosGuaranteeType.NONE);
                }
                else if (temp == 3)
                {
                    cosDataIntegrity.type(CosDataIntegrity.CosDataIntegrityType.BEST_EFFORT);
                    cosFlowControl.type(CosFlowControl.CosFlowControlType.BIDIRECTIONAL).recvWindowSize(120);
                    cosGuarantee.type(CosGuarantee.CosGuaranteeType.NONE);
                }
                else if (temp == 4)
                {
                    cosDataIntegrity.type(CosDataIntegrity.CosDataIntegrityType.RELIABLE);
                    cosFlowControl.type(CosFlowControl.CosFlowControlType.NONE).recvWindowSize(1200);
                    cosGuarantee.type(CosGuarantee.CosGuaranteeType.NONE);
                }
                else if (temp == 5)
                {
                    cosDataIntegrity.type(CosDataIntegrity.CosDataIntegrityType.RELIABLE);
                    cosFlowControl.type(CosFlowControl.CosFlowControlType.BIDIRECTIONAL).recvWindowSize(20);
                    cosGuarantee.type(CosGuarantee.CosGuaranteeType.NONE);
                }
                else if (temp == 6)
                {
                    cosDataIntegrity.type(CosDataIntegrity.CosDataIntegrityType.RELIABLE);
                    cosFlowControl.type(CosFlowControl.CosFlowControlType.BIDIRECTIONAL).recvWindowSize(120);
                    cosGuarantee.type(CosGuarantee.CosGuaranteeType.PERSISTENT_QUEUE);
                }
                else
                    return;
            }

            ClassOfService cos = EmaFactory.createClassOfService();
            cos.authentication(cosAuthentication).dataIntegrity(cosDataIntegrity).flowControl(cosFlowControl).guarantee(cosGuarantee);

            ElementList elementList = EmaFactory.createElementList();
            elementList.add(EmaFactory.createElementEntry().uintValue("SingleOpen", 1));
            elementList.add(EmaFactory.createElementEntry().uintValue("AllowSuspectData", 1));
            elementList.add(EmaFactory.createElementEntry().ascii("ApplicationName", "EMA"));

            TunnelStreamRequest tsr = EmaFactory.createTunnelStreamRequest().classOfService(cos).domainType(EmaRdm.MMT_SYSTEM).name("TUNNEL").serviceName("DIRECT_FEED");
            tsr.loginReqMsg(EmaFactory.createReqMsg().domainType(EmaRdm.MMT_LOGIN).name("patel").attrib(elementList));

            long tunnelSTreamandle = consumer.registerClient(tsr, appClient);
            appClient.setTunnelStreamHandle(tunnelSTreamandle);

            if (temp == 1)
            {
                Thread.sleep(10000);
                consumer.unregister(tunnelSTreamandle);
            }

            Thread.sleep(600000); // API calls onRefreshMsg(), onUpdateMsg() and
            // onStatusMsg()
        }
        catch (InterruptedException | OmmException excp)
        {
            System.out.println(excp.toString());
        }
        finally
        {
            if (consumer != null)
                consumer.uninitialize();
        }
    }
}

//END APIQA
