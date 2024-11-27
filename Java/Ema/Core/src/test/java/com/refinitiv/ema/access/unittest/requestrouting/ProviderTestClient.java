///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|              Copyright (C) 2024 LSEG. All rights reserved.                --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access.unittest.requestrouting;

import static org.junit.Assert.assertTrue;

import java.nio.ByteBuffer;
import java.util.ArrayDeque;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.locks.ReentrantLock;

import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.ema.access.*;
import com.refinitiv.ema.access.DataType.DataTypes;

class RequestAttributes
{
	long handle;
	
	String name;
	
	int serviceId;
}

public class ProviderTestClient extends TimerTask implements OmmProviderClient {
	
	private ProviderTestOptions _providerTestOptions;
	private OmmProvider _ommProvider;
	private long _loginHandle;
	private String _loginUserName;
	private Timer _timer;
	private ElementList _refreshAttributes;
	
	private ArrayDeque<Msg> _messageQueue = new ArrayDeque<Msg>();
	
	private ReentrantLock _accessLock = new java.util.concurrent.locks.ReentrantLock();
	
	private HashMap<String, RequestAttributes> _itemNameToHandleMap = new HashMap<>(10);
	
	private int _serviceId = -1; /* This is service Id from the request message. -1 indicates that the service Id is not set */
	
	private static DataDictionary DataDictionary;
	
	private int fragmentationSize = 1280000;
	private Series series = EmaFactory.createSeries();
	private RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
	
	private int currentValue;
	private boolean result;
	
	private long warmStandbyMode = -1;
	
	public ProviderTestClient(ProviderTestOptions testOptions)
	{
		_providerTestOptions = testOptions;
		
		_timer = new Timer(true);
	}
	
	public long loginHandle()
	{
		return _loginHandle;
	}
	
	public Long retriveItemHandle(String itemName)
	{
		return _itemNameToHandleMap.get(itemName).handle;
	}
	
	public int queueSize()
	{
		_accessLock.lock();
		try
		{
			return _messageQueue.size();
		}
		finally
		{
			_accessLock.unlock();
		}
	}
	
	public Msg popMessage()
	{
		_accessLock.lock();
		try
		{
			return _messageQueue.removeFirst();
		}
		finally
		{
			_accessLock.unlock();
		}
	}

	@Override
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent providerEvent) {

	}

	@Override
	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent providerEvent) {
		
	}

	@Override
	public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent providerEvent) {
		
		_accessLock.lock();
		
		try
		{
			GenericMsg cloneMsg = EmaFactory.createGenericMsg(genericMsg);
			
			System.out.println(cloneMsg);
			
			_messageQueue.add(cloneMsg);
			
			if(_providerTestOptions.sendGenericMessage)
			{
				GenericMsg submitMsg = EmaFactory.createGenericMsg().name(genericMsg.name()).domainType(genericMsg.domainType()).complete(genericMsg.complete());
				
				if(genericMsg.hasServiceId()) // Service ID is set from the request message.
				{
					if(_serviceId != -1 && genericMsg.serviceId() == _serviceId)
					{
						submitMsg.serviceId(genericMsg.serviceId());
					}
				}
				
			    providerEvent.provider().submit( submitMsg, providerEvent.handle() );
			}
			else if (_providerTestOptions.supportStandby && genericMsg.name().equals(EmaRdm.ENAME_CONS_CONN_STATUS) && genericMsg.domainType() == DomainTypes.LOGIN)
			{		
				Map map = genericMsg.payload().map();
				Iterator<MapEntry> mapIt = map.iterator();
				assertTrue(mapIt.hasNext());
				MapEntry mapEntry = mapIt.next();
				
				long recvWarmStandbyMode = warmStandbyMode;
				
				if(mapEntry.key().ascii().toString().equals(EmaRdm.ENAME_WARMSTANDBY_INFO) )
				{
					ElementList elementList = mapEntry.elementList();
					Iterator<ElementEntry> elementListIt = elementList.iterator();
					assertTrue(elementListIt.hasNext());
					ElementEntry elementEntry = elementListIt.next();
					if(elementEntry.name().equals(EmaRdm.ENAME_WARMSTANDBY_MODE))
					{
						recvWarmStandbyMode = elementEntry.uintValue();
					}
					
					/* Standby server is changed to active server */
					if(warmStandbyMode == 1 && recvWarmStandbyMode == 0)
					{
						/* Send unsolicited refresh message for the requested items */ 
						Iterator<RequestAttributes>  it = _itemNameToHandleMap.values().iterator();
						while(it.hasNext())
						{
							RequestAttributes reqAttributes = it.next();
							FieldList fieldList = EmaFactory.createFieldList();
							fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
							fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
							fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
							fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
							
							RefreshMsg refreshMsg = EmaFactory.createRefreshMsg().name(reqAttributes.name).serviceId(reqAttributes.serviceId).solicited(false).clearCache(true).
									state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Unsolicited Refresh Completed").
									payload(fieldList).complete(true);
							
							if(_providerTestOptions.itemGroupId != null)
							{
								refreshMsg.itemGroup(_providerTestOptions.itemGroupId);
							}
							
							providerEvent.provider().submit( refreshMsg, reqAttributes.handle );
						}
						
					}
					
					warmStandbyMode = recvWarmStandbyMode;
				}
			}
			else if (_providerTestOptions.supportStandby && genericMsg.name().equals(EmaRdm.ENAME_CONS_STATUS) && genericMsg.domainType() == DomainTypes.SOURCE)
			{		
				Map map = genericMsg.payload().map();
				Iterator<MapEntry> mapIt = map.iterator();
				assertTrue(mapIt.hasNext());
				MapEntry mapEntry = mapIt.next();
				
				long recvWarmStandbyMode = warmStandbyMode;
				long serviceId = mapEntry.key().uintValue();
				
				//if(mapEntry.key().ascii().toString().equals(EmaRdm.ENAME_WARMSTANDBY_INFO) )
				//{
					ElementList elementList = mapEntry.elementList();
					Iterator<ElementEntry> elementListIt = elementList.iterator();
					assertTrue(elementListIt.hasNext());
					ElementEntry elementEntry = elementListIt.next();
					if(elementEntry.name().equals(EmaRdm.ENAME_WARMSTANDBY_MODE))
					{
						recvWarmStandbyMode = elementEntry.uintValue();
					}
					
					/* Standby server is changed to active server */
					if(warmStandbyMode == 1 && recvWarmStandbyMode == 0)
					{
						/* Send unsolicited refresh message for the requested items */ 
						Iterator<RequestAttributes>  it = _itemNameToHandleMap.values().iterator();
						while(it.hasNext())
						{
							RequestAttributes reqAttributes = it.next();
							FieldList fieldList = EmaFactory.createFieldList();
							fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
							fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
							fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
							fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
							
							RefreshMsg refreshMsg = EmaFactory.createRefreshMsg().name(reqAttributes.name).serviceId(reqAttributes.serviceId).solicited(false).clearCache(true).
									state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Unsolicited Refresh Completed").
									payload(fieldList).complete(true);
							
							if(_providerTestOptions.itemGroupId != null)
							{
								refreshMsg.itemGroup(_providerTestOptions.itemGroupId);
							}
							
							providerEvent.provider().submit( refreshMsg, reqAttributes.handle );
						}
						
					}
					
					warmStandbyMode = recvWarmStandbyMode;
				//}
			}
		}
		finally
		{
			_accessLock.unlock();
		}
		
	}

	@Override
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {
		
		_accessLock.lock();
		
		try
		{
			PostMsg cloneMsg = EmaFactory.createPostMsg(postMsg);
			
			System.out.println(cloneMsg);
			
			_messageQueue.add(cloneMsg);
			
			if(postMsg.solicitAck())
			{
				if(postMsg.solicitAck()) {
					AckMsg ackMsg = EmaFactory.createAckMsg();
					
					if(postMsg.hasSeqNum()){
						ackMsg.seqNum(postMsg.seqNum());
					}
					if(postMsg.hasName()){
						ackMsg.name(postMsg.name());
					}
					if(postMsg.hasServiceId()){
						ackMsg.serviceId(postMsg.serviceId());
					}
					
					ackMsg.ackId(postMsg.postId()).domainType(postMsg.domainType());
					
					providerEvent.provider().submit(ackMsg, providerEvent.handle());
				}
			}
		}
		finally
		{
			_accessLock.unlock();
		}
		
	}

	@Override
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent) {
		
		switch (reqMsg.domainType())
		{
			case EmaRdm.MMT_LOGIN :
				
				_ommProvider = providerEvent.provider();
				_loginUserName = reqMsg.name();
				_loginHandle = providerEvent.handle();
				
				_refreshAttributes = EmaFactory.createElementList();
				
				if (reqMsg.attrib().dataType() == DataTypes.ELEMENT_LIST )
				{
					ElementList reqAttributes = reqMsg.attrib().elementList();
					
					if(_providerTestOptions.supportOMMPosting)
					{
						_refreshAttributes.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_POST, 1) );
					}
					
					if(_providerTestOptions.supportStandby)
					{
						_refreshAttributes.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_STANDBY, 1) );
						_refreshAttributes.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_WARMSTANDBY_MODE, 1) );
					}
					
					for( ElementEntry reqAttrib : reqAttributes )
					{
						String name = reqAttrib.name();
												
						if ( name.equals(EmaRdm.ENAME_ALLOW_SUSPECT_DATA) ||
								name.equals(EmaRdm.ENAME_SINGLE_OPEN) )
						{
							_refreshAttributes.add( EmaFactory.createElementEntry().uintValue(name, reqAttrib.uintValue()) );
						}
						else if ( name.equals(EmaRdm.ENAME_APP_ID) || 
							name.equals(EmaRdm.ENAME_POSITION ) )
						{
							_refreshAttributes.add( EmaFactory.createElementEntry().ascii(name, reqAttrib.ascii().toString()) );
						}
					}
				}
				
				if(_providerTestOptions.sendLoginResponseInMiliSecond == 0)
				{
					if(_providerTestOptions.acceptLoginRequest)
					{
						if(_providerTestOptions.sendRefreshAttrib)
						{
							providerEvent.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(_loginUserName).
									nameType(EmaRdm.USER_NAME).complete(true).solicited(true).attrib(_refreshAttributes).
									state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
									providerEvent.handle() );
						}
						else
						{
							providerEvent.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(_loginUserName).
									nameType(EmaRdm.USER_NAME).complete(true).solicited(true).
									state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
									providerEvent.handle() );
						}
					}
					else
					{
						providerEvent.provider().submit( EmaFactory.createStatusMsg().domainType(EmaRdm.MMT_LOGIN).name(_loginUserName).nameType(EmaRdm.USER_NAME).
								state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NOT_AUTHORIZED, "Login denied"),
								providerEvent.handle() );
					}
				}
				else
				{
					_timer.schedule(this, _providerTestOptions.sendLoginResponseInMiliSecond);
					
					
				}
					
			break;
			
			case EmaRdm.MMT_MARKET_PRICE:
			case 55:
				
				_accessLock.lock();
				
				try
				{
					ReqMsg cloneMsg = EmaFactory.createReqMsg(reqMsg);
				
					System.out.println(cloneMsg);
					
					_messageQueue.add(cloneMsg);
					
					if(reqMsg.hasServiceId())
					{
						_serviceId = reqMsg.serviceId();
					}
					
					
					if(_providerTestOptions.sendItemResponse == false)
					{
						System.out.println("Skip sending item response for this request");
						break;
					}
					else if (_providerTestOptions.closeItemRequest)
					{
						System.out.println("Closes this item request");
						
						providerEvent.provider().submit( EmaFactory.createStatusMsg().domainType(reqMsg.domainType()).name(reqMsg.name()).
								state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NOT_AUTHORIZED, "Unauthorized access to the item."),
								providerEvent.handle() );
						break;
					}
					
					FieldList fieldList = EmaFactory.createFieldList();
					fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
					fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
					fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
					fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
					
										
					RefreshMsg refreshMsg = EmaFactory.createRefreshMsg().name(reqMsg.name()).serviceId(reqMsg.serviceId()).solicited(true).domainType(reqMsg.domainType()).
							state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").
							complete(true);
					
					if(_providerTestOptions.supportStandby) 
					{
						/* Send payload only on the active server */
						if( warmStandbyMode == 0)
						{
							refreshMsg.payload(fieldList);
						}
						else
						{
							/* Send blank payload for the standby server */
						}
					}
					else
					{
						refreshMsg.payload(fieldList);
					}
					
					if(_providerTestOptions.itemGroupId != null)
					{
						refreshMsg.itemGroup(_providerTestOptions.itemGroupId);
					}
					
					providerEvent.provider().submit( refreshMsg, providerEvent.handle() );
					
					if(_providerTestOptions.sendUpdateMessage)
					{
						fieldList.clear();
						fieldList.add( EmaFactory.createFieldEntry().real(22, 3991, OmmReal.MagnitudeType.EXPONENT_NEG_2));
						fieldList.add( EmaFactory.createFieldEntry().real(25, 3995, OmmReal.MagnitudeType.EXPONENT_NEG_2));
						
						UpdateMsg updateMsg = EmaFactory.createUpdateMsg().name(reqMsg.name()).serviceId(reqMsg.serviceId()).updateTypeNum(EmaRdm.INSTRUMENT_UPDATE_TRADE).domainType(reqMsg.domainType())
								.payload(fieldList);
						
						providerEvent.provider().submit( updateMsg, providerEvent.handle() );
					}
					
					RequestAttributes reqAttributes = new RequestAttributes();
					reqAttributes.name = reqMsg.name();
					reqAttributes.handle = providerEvent.handle();
					reqAttributes.serviceId = reqMsg.serviceId();
					
					_itemNameToHandleMap.put(reqMsg.name(), reqAttributes);

				}
				finally
				{
					_accessLock.unlock();
				}
				
			break;
			
			case EmaRdm.MMT_DIRECTORY:
				
				if(_providerTestOptions.sourceDirectoryPayload != null)
				{
					RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
					providerEvent.provider().submit( refreshMsg.domainType(EmaRdm.MMT_DIRECTORY).clearCache(true).
														filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
														payload(_providerTestOptions.sourceDirectoryPayload).solicited(true).complete(true), providerEvent.handle());
				}
				
			break;
			
			case EmaRdm.MMT_DICTIONARY:
				
				_accessLock.lock();
				
				try
				{
					ReqMsg cloneMsg = EmaFactory.createReqMsg(reqMsg);
					
					System.out.println(cloneMsg);
					
					_messageQueue.add(cloneMsg);
				
					processDictionaryRequest(reqMsg, providerEvent);
				}
				finally
				{
					_accessLock.unlock();
				}
				
			break;
			
			case EmaRdm.MMT_SYMBOL_LIST:
				
				_accessLock.lock();
				
				try
				{
					ReqMsg cloneMsg = EmaFactory.createReqMsg(reqMsg);
				
					System.out.println(cloneMsg);
					
					_messageQueue.add(cloneMsg);
				
					FieldList summaryData = EmaFactory.createFieldList();
					summaryData.add(EmaFactory.createFieldEntry().uintValue(1, 74));
					summaryData.add(EmaFactory.createFieldEntry().rmtes(3, ByteBuffer.wrap("TOP 25 BY VOLUME".getBytes())));
					summaryData.add(EmaFactory.createFieldEntry().realFromDouble(77, 1864.0));
					summaryData.add(EmaFactory.createFieldEntry().enumValue(1709, 559));
					summaryData.add(EmaFactory.createFieldEntry().time(3798, 15, 45, 47, 0, 0, 0));
					summaryData.add(EmaFactory.createFieldEntry().time(14269, 15, 45, 47, 451, 675, 0));
					
					Map map = EmaFactory.createMap();
					
					map.totalCountHint(3);
					map.summaryData(summaryData);
					
					FieldList mapEntryValue = EmaFactory.createFieldList();
					mapEntryValue.add(EmaFactory.createFieldEntry().uintValue(6453, 1));
					map.add(EmaFactory.createMapEntry().keyBuffer(ByteBuffer.wrap("itemA".getBytes()), MapEntry.MapAction.ADD, mapEntryValue));
					
					mapEntryValue.add(EmaFactory.createFieldEntry().uintValue(6453, 2));
					map.add(EmaFactory.createMapEntry().keyBuffer(ByteBuffer.wrap("itemB".getBytes()), MapEntry.MapAction.ADD, mapEntryValue));
					
					mapEntryValue.add(EmaFactory.createFieldEntry().uintValue(6453, 3));
					map.add(EmaFactory.createMapEntry().keyBuffer(ByteBuffer.wrap("itemC".getBytes()), MapEntry.MapAction.ADD, mapEntryValue));
					
					RefreshMsg refreshMsg = EmaFactory.createRefreshMsg().name(reqMsg.name()).serviceId(reqMsg.serviceId()).solicited(true).
							state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").
							payload(map).complete(true);
					
					providerEvent.provider().submit( refreshMsg, providerEvent.handle() );
				}
				finally
				{
					_accessLock.unlock();
				}
				
			break;
			
		}
	}

	@Override
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {
		
		switch (reqMsg.domainType())
		{
		case EmaRdm.MMT_DICTIONARY:
			
			_accessLock.lock();
			
			try
			{
				ReqMsg cloneMsg = EmaFactory.createReqMsg(reqMsg);
				
				System.out.println(cloneMsg);
				
				_messageQueue.add(cloneMsg);
			
				processDictionaryRequest(reqMsg, providerEvent);
			}
			finally
			{
				_accessLock.unlock();
			}
			
		break;
		}
		
	}

	@Override
	public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {
		
		ReqMsg cloneMsg = EmaFactory.createReqMsg(reqMsg);
		
		System.out.println("onClose(): " + reqMsg);
		
		_messageQueue.add(cloneMsg);
	}

	@Override
	public void onAllMsg(Msg msg, OmmProviderEvent providerEvent) {
		
	}
	
	public void forceLogout()
	{
		_ommProvider.submit( EmaFactory.createStatusMsg().domainType(EmaRdm.MMT_LOGIN).name(_loginUserName).nameType(EmaRdm.USER_NAME).
				state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NOT_AUTHORIZED, "Force logout"),
				_loginHandle );
	}
	
	void processDictionaryRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		if(DataDictionary == null)
		{
			DataDictionary = EmaFactory.createDataDictionary();
			
			DataDictionary.loadFieldDictionary( "../../etc/RDMFieldDictionary" );
			DataDictionary.loadEnumTypeDictionary( "../../etc/enumtype.def" );
		}
		
		result = false;
		refreshMsg.clear().clearCache( true );

		if ( reqMsg.name().equals( "RWFFld" ) )
		{
			currentValue = DataDictionary.minFid();

			while ( !result )
			{
				currentValue = DataDictionary.encodeFieldDictionary( series, currentValue, reqMsg.filter(), fragmentationSize );
				
				result = currentValue == DataDictionary.maxFid() ? true : false;

				event.provider().submit( refreshMsg.name( reqMsg.name() ).serviceName( reqMsg.serviceName() ).
						domainType( EmaRdm.MMT_DICTIONARY ).filter( reqMsg.filter() ).payload( series ).complete( result ).
						solicited( true ), event.handle() );
				
				refreshMsg.clear();
			}
		}
		else if ( reqMsg.name().equals( "RWFEnum" ) )
		{
			currentValue = 0;

			while ( !result )
			{
				currentValue = DataDictionary.encodeEnumTypeDictionary( series, currentValue, reqMsg.filter(), fragmentationSize );
				
				result =  currentValue == DataDictionary.enumTables().size() ? true : false;
					
				event.provider().submit( refreshMsg.name( reqMsg.name() ).serviceName( reqMsg.serviceName() ).
						domainType( EmaRdm.MMT_DICTIONARY ).filter( reqMsg.filter() ).payload( series ).complete( result ).
						solicited( true ), event.handle() );
				
				refreshMsg.clear();
			}
		}
	}

	@Override
	public void run() {
				
		if(_providerTestOptions.acceptLoginRequest)
		{
			if(_providerTestOptions.sendRefreshAttrib)
			{
				_ommProvider.submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(_loginUserName).
						nameType(EmaRdm.USER_NAME).complete(true).solicited(true).attrib(_refreshAttributes).
						state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
						_loginHandle );
			}
			else
			{
				_ommProvider.submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(_loginUserName).
						nameType(EmaRdm.USER_NAME).complete(true).solicited(true).
						state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
						_loginHandle );
			}
		}
		else
		{
			_ommProvider.submit( EmaFactory.createStatusMsg().domainType(EmaRdm.MMT_LOGIN).name(_loginUserName).nameType(EmaRdm.USER_NAME).
					state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NOT_AUTHORIZED, "Login denied"),
					_loginHandle );
		}
		
	}

}
