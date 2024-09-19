///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series400.ex440_System_TunStrm;

import com.refinitiv.ema.access.Msg;
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
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.rdm.EmaRdm;

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

			appClient.setTunnelStreamHandle(consumer.registerClient(tsr, appClient));

			Thread.sleep(60000); // API calls onRefreshMsg(), onUpdateMsg() and
									// onStatusMsg()
		} catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
		} finally
		{
			if (consumer != null)
				consumer.uninitialize();
		}
	}
}
