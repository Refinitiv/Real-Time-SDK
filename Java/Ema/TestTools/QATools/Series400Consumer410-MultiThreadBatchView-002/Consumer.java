///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

//APIQA
package com.refinitiv.ema.examples.training.consumer.series400.ex410_MP_HorizontalScaling;
import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.Callable;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmArray;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerConfig.OperationModel;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmInvalidUsageException;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.rdm.EmaRdm;

public class Consumer {
	private EmaConsumerService emaConsumerService = new EmaConsumerServiceImpl();

	public static void main(String[] args) {
		
		System.out.println("Starting Consumer");
		Consumer demo = new Consumer();
		
		List<String> unifiedRequest = null;
		try {
			unifiedRequest = demo.populateUnifiedRequestList();
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		try {
			demo.emaConsumerService.fetchMarketData(unifiedRequest);
			System.out.println("Exiting Main");
		} catch (Exception e) {
			// TODO Auto-generated catch block
			System.out.println("####### Exception (In main): " + e.getMessage());
			e.printStackTrace();
		}
		
		
	}


	private List<String> populateUnifiedRequestList()
			throws Exception {
		List<String> unifiedList = new ArrayList<String>();
		
        BufferedReader inputStream = null;

        try {
        	inputStream =   new BufferedReader(new FileReader("tor_500.txt"));

        	String itemName;
            while ((itemName = inputStream.readLine()) != null) {
            	unifiedList.add(itemName.trim());
            }
        } catch (Exception e) {
        	
        } finally {
            if (inputStream != null) {
            	try {
            		inputStream.close();
            	} catch (Exception e1) {
            		
            	}
            }
        }

		return unifiedList;
	}
}

class EmaClient implements OmmConsumerClient{

	
	
	private CountDownLatch countDown;
	private OmmConsumer ommConsumer;
	
	public EmaClient(CountDownLatch countDown, OmmConsumer ommConsumer) {
		this.countDown = countDown;
		this.ommConsumer = ommConsumer;

	}

	@Override
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event) {
		String name = refreshMsg.name();
	
		System.out.println("onRefreshMsg, Name: " + name  + "  OMMConsumer: " + ommConsumer.consumerName());
			System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
			System.out.println("Item State: " + refreshMsg.state());
			
			ommConsumer.unregister(event.handle());//--unregister item individually
			countDown.countDown();	
	}

	@Override
	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) {
		String name = (statusMsg.hasName())?statusMsg.name(): "StatusMsg";
		System.out.println("onStatusMsg, Name: "+ name + "  OMMConsumer: " + ommConsumer.consumerName() + " State; " + statusMsg.state());
		boolean isErrorResponse = false;
		OmmState state = null;
		
			state = statusMsg.state();
			isErrorResponse = evaluateError(state);
	
			if(isErrorResponse){				
				countDown.countDown();
				ommConsumer.unregister(event.handle());//--unregister item individually
			}
		
	}

	@Override
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) {
		System.out.println("onUpdateMsg, Name: " + updateMsg.name());
	}
	
	@Override
	public void onAckMsg(AckMsg arg0, OmmConsumerEvent arg1) {
		System.out.println("onAckMsg: " + arg0);
	}

//	@Override
	public void onAllMsg(Msg arg0, OmmConsumerEvent arg1) {
		String name = (arg0.hasName())?arg0.name(): "AllMsg";
		System.out.println("onAllMsg: " + name);
		
	}

	@Override
	public void onGenericMsg(GenericMsg arg0, OmmConsumerEvent arg1) {
		System.out.println("inside onGenericMsg: " + arg0);
		
	}
	
	private boolean evaluateError(OmmState state) {
		boolean hasError = false;

		if (state != null) {
			
			switch (state.statusCode()) {
			case OmmState.StatusCode.NOT_AUTHORIZED:
			case OmmState.StatusCode.NOT_FOUND:
			case OmmState.StatusCode.TIMEOUT:
			case OmmState.StatusCode.SERVICE_DOWN:
				hasError = true;
			}
			
			switch (state.dataState()) {
			case OmmState.DataState.SUSPECT:
				hasError = true;
			}

		}
		return hasError;
	}

	
}

	
class EmaConsumer extends Thread {

	//private static final Logger LOGGER = LoggerFactory
	//		.getLogger(EmaConsumer.class);

	private OmmConsumer ommConsumer;
	private EmaClient emaClient;
	private CountDownLatch countDownLatch;
	private ReqMsg reqMsg;
	private EmaConsumerServiceImpl myCaller;

	
	public EmaConsumer(OmmConsumer ommConsumer, EmaClient emaClient,
			CountDownLatch countDownLatch, ReqMsg reqMsg, EmaConsumerServiceImpl myCaller) {
		this.ommConsumer = ommConsumer;
		this.emaClient = emaClient;
		this.countDownLatch = countDownLatch;
		this.reqMsg = reqMsg;
		this.myCaller = myCaller;
	}


	@Override
	public void run() {

		try {
			//User dispatch
			ommConsumer.registerClient(reqMsg, emaClient);
			while(countDownLatch.getCount() != 0){
				ommConsumer.dispatch(1000); //Let API do the call backs
			}
				

		} catch (Exception  excp ) {
			System.out.println("+++++Exception while executing dispatch, errorMessage:  " + excp.getMessage() );
			excp.printStackTrace();
		} finally {
			if (null != ommConsumer) {
				ommConsumer.uninitialize();
				ommConsumer = null;
				myCaller.incCount();
			}
		}
		return;
	}
	
	

}

interface EmaConsumerService {
	void fetchMarketData(List<String> unifiedRequest) throws Exception;
}


class EmaConsumerServiceImpl implements EmaConsumerService {
	
	private int threadCount = 0;
	
	@Override
	public void fetchMarketData(List<String> unifiedRequest) throws Exception {
	
		OmmConsumer ommConsumer = null;
		EmaConsumer emaConsumer;
				 
		try {		
			EmaConsumer consArray[] = null;
			for (int i = 0; i < 20; i++) {
				ommConsumer = fetchConsumerFromFactory();
				CountDownLatch countDown = new CountDownLatch(unifiedRequest.size());
				EmaClient emaClient = new EmaClient(countDown, ommConsumer);
				ReqMsg reqMsg = prepareRequestObject(unifiedRequest);
				emaConsumer = new EmaConsumer(ommConsumer, emaClient, countDown, reqMsg, this);
				emaConsumer.start();				
			}
			Thread.sleep (12000);
			while (threadCount < 20) 
				Thread.sleep (2000);
			
		} catch (InterruptedException | OmmInvalidUsageException excp) {
	//		LOGGER.error(excp.getMessage());
			System.out.println(excp.getMessage());
			excp.printStackTrace();
			
		} 
		

		return;
	}
	
	public synchronized void incCount() {
		threadCount++;
	}
	

	private OmmConsumer fetchConsumerFromFactory() {
		return EmaFactory.createOmmConsumer(EmaFactory
				.createOmmConsumerConfig().username("User")
				.applicationId("256").consumerName("Consumer_1")
				.operationModel(OperationModel.USER_DISPATCH));
	}
	

	private ReqMsg prepareRequestObject(List<String> unifiedRequest) {
		return EmaFactory
				.createReqMsg()
				.clear()
				.domainType(EmaRdm.MMT_MARKET_PRICE)
				.serviceName("DIRECT_FEED")
				.payload(
						getDynamicBatchRequestView("3,6,15,21,79,134,1059",
								unifiedRequest))
				.interestAfterRefresh(false);
		
	}

	private ElementList getDynamicBatchRequestView(String viewFieldList,
			List<String> unifiedRequest) {
		ElementList finalElements = EmaFactory.createElementList();
		// batch input
		OmmArray batchInputArray = createBatchArray(unifiedRequest);
		finalElements.add(EmaFactory.createElementEntry().array(
				EmaRdm.ENAME_BATCH_ITEM_LIST, batchInputArray));
		// View input
		OmmArray viewElement = EmaFactory.createOmmArray();
		List<String> viewFieldIdList = Arrays.asList(viewFieldList.trim()
				.split(","));
		//if (CollectionUtils.isNotEmpty(viewFieldIdList)) {
		if (viewFieldList != null && !viewFieldList.isEmpty()) {
			for (String field : viewFieldIdList) {
				viewElement.add(EmaFactory.createOmmArrayEntry().intValue(
						Integer.parseInt(field)));
			}
			finalElements.add(EmaFactory.createElementEntry().uintValue(
					EmaRdm.ENAME_VIEW_TYPE, 1));
			finalElements.add(EmaFactory.createElementEntry().array(
					EmaRdm.ENAME_VIEW_DATA, viewElement));
		}
		return finalElements;
	}

	private OmmArray createBatchArray(List<String> unifiedRequest) {
		OmmArray ommArray = EmaFactory.createOmmArray();
		//if (CollectionUtils.isNotEmpty(unifiedRequest)) {
		if (unifiedRequest != null && !unifiedRequest.isEmpty()) {
			unifiedRequest.stream().forEach(
					requestString -> {
						ommArray.add(EmaFactory.createOmmArrayEntry().ascii(
								requestString));
					});
		}
		return ommArray;
	}
}


class MarketDataError {
	 
	private String errorCode;
    private String errorMsg;
    
    public MarketDataError() {
	}

	public MarketDataError(String errorCode) {
        this.errorCode = errorCode;
    }

    public MarketDataError(String errorCode, String information) {
        this.errorCode = errorCode;
        this.errorMsg = information;
    }

    public String getErrorCode() {
        return errorCode;
    }

    public void setErrorCode(String errorCode) {
        this.errorCode = errorCode;
    }

    public String getInformation() {
        return errorMsg;
    }

    @Override
    public String toString() {
        return "MarketDataError [errorCode=" + errorCode
                + ", information=" + errorMsg + "]";
    }

}

class MarketDataRequest {

	private Set<String> requestRics;
	private Set<String> currencies;
	
	private boolean isDelayed = true;

	public boolean isDelayed() {
        return isDelayed;
    }

    public void setDelayed(boolean isDelayed) {
        this.isDelayed = isDelayed;
    }

    public Set<String> getRequestRics() {
		return requestRics;
	}

	public void setRequestRics(Set<String> requestRics) {
		this.requestRics = requestRics;
	}

	public Set<String> getCurrencies() {
		return currencies;
	}

	public void setCurrencies(Set<String> currencies) {
		this.currencies = currencies;
	}

}



