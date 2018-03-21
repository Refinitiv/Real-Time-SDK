///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright Thomson Reuters 2016. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.thomsonreuters.ema.examples.training.consumer.series400.example440__System__TunnelStream;

import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.AckMsg;
import com.thomsonreuters.ema.access.ClassOfService;
import com.thomsonreuters.ema.access.CosAuthentication;
import com.thomsonreuters.ema.access.CosDataIntegrity;
import com.thomsonreuters.ema.access.CosFlowControl;
import com.thomsonreuters.ema.access.CosGuarantee;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.TunnelStreamRequest;
import com.thomsonreuters.ema.access.UpdateMsg;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.OmmConsumer;
import com.thomsonreuters.ema.access.OmmConsumerClient;
import com.thomsonreuters.ema.access.OmmConsumerEvent;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.OmmReal;
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.access.PostMsg;
import com.thomsonreuters.ema.rdm.EmaRdm;

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
		if (refreshMsg.domainType() == EmaRdm.MMT_SYSTEM)
		{
		    if (refreshMsg.state().streamState() == OmmState.StreamState.OPEN &&
		    		refreshMsg.state().dataState() == OmmState.DataState.OK)
		    {
		        // client login to private stream accepted - ready to contribute
		        FieldList fieldList = EmaFactory.createFieldList();
		        fieldList.add(EmaFactory.createFieldEntry()
		                .realFromDouble(22, 123, OmmReal.MagnitudeType.EXPONENT_NEG_8));
		        fieldList.add(EmaFactory.createFieldEntry()
		                .realFromDouble(25, 456, OmmReal.MagnitudeType.EXPONENT_NEG_8));

		        UpdateMsg updateMsg = EmaFactory.createUpdateMsg()
		                .serviceName("DIRECT_FEED").name("TEST")
		                .streamId(refreshMsg.streamId())
		                .payload(fieldList);

		        PostMsg postMsg = EmaFactory.createPostMsg()
		                .domainType(EmaRdm.MMT_MARKET_PRICE)
		                .postId(123)
		                .serviceName("DIRECT_FEED").name("TEST")
		                .solicitAck(true)
		                .complete(true)
		                .payload(updateMsg);
		        // THIS CALL CHANGES MESSAGE in ItemCallbackClient.processTunnelStreamRefreshMsg()
		        // APIQA 
		        _ommConsumer.submit(postMsg, event.handle());
		    }
		}
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

			_ommConsumer.registerClient(EmaFactory.createReqMsg().name("TUNNEL_IBM").serviceId(1).domainType(EmaRdm.MMT_SYSTEM), this, 1, _tunnelStreamHandle);
			/*_ommConsumer.registerClient(EmaFactory.createReqMsg().name("TUNNEL_IBM").serviceId(1), this, 1,
					_tunnelStreamHandle);*/
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
