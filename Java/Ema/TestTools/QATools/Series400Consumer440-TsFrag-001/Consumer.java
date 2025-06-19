/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

package com.refinitiv.ema.examples.training.consumer.series400.ex440_System_TunStrm;

import com.refinitiv.ema.access.Msg;
// APIQA:
import java.nio.ByteBuffer;
// END APIQA:
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.ClassOfService;
import com.refinitiv.ema.access.CosAuthentication;
import com.refinitiv.ema.access.CosDataIntegrity;
import com.refinitiv.ema.access.CosFlowControl;
import com.refinitiv.ema.access.CosGuarantee;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.TunnelStreamRequest;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;
// APIQA:
import com.refinitiv.ema.access.OmmOpaque;
// END APIQA:
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.rdm.EmaRdm;

class AppClient implements OmmConsumerClient
{
    private OmmConsumer _ommConsumer;
    private long _tunnelStreamHandle;
    private boolean _subItemOpen;
    // APIQA: subitem handle
    public long _subItemHandle;

    // END APIQA:

    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
    {
        System.out.println("Handle: " + event.handle());
        System.out.println("Parent Handle: " + event.parentHandle());
        System.out.println("Closure: " + event.closure());

        System.out.println(refreshMsg);

        System.out.println();
        // APIQA: save subitem handle
        _subItemHandle = event.handle();
        // END APIQA
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

            // APIQA: register subitem on SYSTEM domain
            _ommConsumer.registerClient(EmaFactory.createReqMsg().name("TUNNEL_IBM").serviceId(1).domainType(EmaRdm.MMT_SYSTEM), this, 1, _tunnelStreamHandle);
            // END APIQA:
            // _ommConsumer.registerClient(EmaFactory.createReqMsg().name("TUNNEL_IBM").serviceId(1),
            // this, 1,
            // _tunnelStreamHandle);
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
        // APIQA:
        System.out.println("Handle: " + consumerEvent.handle());
        System.out.println("Parent Handle: " + consumerEvent.parentHandle());
        System.out.println("Closure: " + consumerEvent.closure());

        System.out.println(genericMsg);
        // END APIQA:
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
    // APIQA: buffer size, fill size and runtime
    private static int _tunnelBufSize;
    private static int _tunnelFillSize;
    private static int _runTime = 60;

    // get command line arguments
    public static boolean init(String[] argv)
    {
        int count = argv.length;
        int idx = 0;

        while (idx < count)
        {
            if (0 == argv[idx].compareTo("-bufSize"))
            {
                if (++idx <= count)
                {
                    _tunnelBufSize = Integer.valueOf(argv[idx]);
                }
                ++idx;
            }
            else if (0 == argv[idx].compareTo("-fillSize"))
            {
                if (++idx <= count)
                {
                    _tunnelFillSize = Integer.valueOf(argv[idx]);
                }
                ++idx;
            }
            else if (0 == argv[idx].compareTo("-runtime"))
            {
                if (++idx <= count)
                {
                    _runTime = Integer.valueOf(argv[idx]);
                }
                ++idx;
            }
            else
            {
                System.out.println("Unrecognized command line option.");
                return false;
            }
        }

        return true;
    }

    // END APIQA:

    public static void main(String[] args)
    {
        // APIQA: get command line arguments
        if (!init(args))
            return;
        // END APIQA:
        OmmConsumer consumer = null;
        try
        {
            AppClient appClient = new AppClient();

            consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().username("user"));

            appClient.setOmmConsumer(consumer);

            ClassOfService cos = EmaFactory.createClassOfService().authentication(EmaFactory.createCosAuthentication().type(CosAuthentication.CosAuthenticationType.OMM_LOGIN))
                    .dataIntegrity(EmaFactory.createCosDataIntegrity().type(CosDataIntegrity.CosDataIntegrityType.RELIABLE))
                    .flowControl(EmaFactory.createCosFlowControl().type(CosFlowControl.CosFlowControlType.BIDIRECTIONAL).recvWindowSize(1200))
                    .guarantee(EmaFactory.createCosGuarantee().type(CosGuarantee.CosGuaranteeType.NONE));

            TunnelStreamRequest tsr = EmaFactory.createTunnelStreamRequest().classOfService(cos).domainType(EmaRdm.MMT_SYSTEM).name("TUNNEL").serviceName("DIRECT_FEED");

            appClient.setTunnelStreamHandle(consumer.registerClient(tsr, appClient));

            // APIQA: Create a buffer of _tunnelBufSize and fill buffer up to
            // _tunnelFillSize with values 0 to 255 repeatedly
            for (int i = 0; i < _runTime; i++)
            {
                if (appClient._subItemHandle != 0)
                {
                    OmmOpaque opaque = EmaFactory.createOmmOpaque();
                    if (_tunnelBufSize > 614400)
                    {
                        _tunnelBufSize = 614400;
                    }
                    ByteBuffer byteBuffer = ByteBuffer.allocate(_tunnelBufSize);
                    if (_tunnelFillSize == 0 || _tunnelFillSize > _tunnelBufSize)
                    {
                        _tunnelFillSize = _tunnelBufSize;
                    }
                    else if (_tunnelFillSize < 10)
                    {
                        _tunnelFillSize = 10;
                    }
                    for (int j = 0, b = 0; j < _tunnelFillSize - 10; j++)
                    {
                        if (b == 256)
                        {
                            b = 0;
                        }
                        byteBuffer.put((byte)b++);
                    }
                    byteBuffer.flip();
                    opaque.buffer(byteBuffer);
                    // send buffer nested in a generic message
                    try
                    {
                        consumer.submit(EmaFactory.createGenericMsg().domainType(EmaRdm.MMT_SYSTEM).payload(opaque), appClient._subItemHandle);
                    }
                    catch (Exception e)
                    {
                        System.out.println("consumer.submit() GenericMsg Exception: " + e.getLocalizedMessage());
                    }
                }
                Thread.sleep(1000);
            }
            // END APIQA:
            // Thread.sleep(60000); // API calls onRefreshMsg(), onUpdateMsg()
            // and
            // onStatusMsg()
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
