///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.rtsdk.ema.examples.training.consumer.series400.example440__System__TunnelStream;

import com.rtsdk.ema.access.Msg;
import com.rtsdk.ema.access.AckMsg;
import com.rtsdk.ema.access.ClassOfService;
import com.rtsdk.ema.access.CosAuthentication;
import com.rtsdk.ema.access.CosDataIntegrity;
import com.rtsdk.ema.access.CosFlowControl;
import com.rtsdk.ema.access.CosGuarantee;
import com.rtsdk.ema.access.GenericMsg;
import com.rtsdk.ema.access.RefreshMsg;
//APIQA
import com.rtsdk.ema.access.ReqMsg;
import com.rtsdk.ema.access.StatusMsg;
import com.rtsdk.ema.access.TunnelStreamRequest;
import com.rtsdk.ema.access.UpdateMsg;
import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.OmmConsumer;
import com.rtsdk.ema.access.OmmConsumerClient;
import com.rtsdk.ema.access.OmmConsumerEvent;
import com.rtsdk.ema.access.OmmException;
import com.rtsdk.ema.access.OmmState;
import com.rtsdk.ema.rdm.EmaRdm;

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

		if (!_subItemOpen && event.handle() == _tunnelStreamHandle && statusMsg.hasState()
				&& statusMsg.state().streamState() == OmmState.StreamState.OPEN)
		{
			_subItemOpen = true;

			_ommConsumer.registerClient(EmaFactory.createReqMsg().name("TUNNEL_IBM").serviceId(1), this, 1,
					_tunnelStreamHandle);
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
		OmmConsumer consumer = null;
		try
		{
			AppClient appClient = new AppClient();

			consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().username("user"));

			appClient.setOmmConsumer(consumer);

			ClassOfService cos = EmaFactory.createClassOfService().
					authentication(EmaFactory.createCosAuthentication().type(CosAuthentication.CosAuthenticationType.OMM_LOGIN))
					.dataIntegrity(EmaFactory.createCosDataIntegrity().type(CosDataIntegrity.CosDataIntegrityType.RELIABLE))
					.flowControl(EmaFactory.createCosFlowControl().type(CosFlowControl.CosFlowControlType.BIDIRECTIONAL).recvWindowSize(1200))
					.guarantee(EmaFactory.createCosGuarantee().type(CosGuarantee.CosGuaranteeType.NONE));

			TunnelStreamRequest tsr = EmaFactory.createTunnelStreamRequest().classOfService(cos)
					.domainType(EmaRdm.MMT_SYSTEM).name("TUNNEL").serviceName("DIRECT_FEED");

			long tunnelStreamHandle = consumer.registerClient(tsr, appClient);
			appClient.setTunnelStreamHandle(tunnelStreamHandle);
		    //APIQA	
			ReqMsg reqMsg = EmaFactory.createReqMsg();

			reqMsg.domainType(EmaRdm.MMT_SYSTEM)
			        .name("TUNNEL")
			        .privateStream(true)
			        .streamId(5); // stream id here causes NullPointerException

		    //END APIQA	
			consumer.registerClient(reqMsg, appClient, null, tunnelStreamHandle);

			Thread.sleep(60000); // API calls onRefreshMsg(), onUpdateMsg() and
									// onStatusMsg()
		}
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
		} finally
		{
			if (consumer != null)
				consumer.uninitialize();
		}
	}
}
