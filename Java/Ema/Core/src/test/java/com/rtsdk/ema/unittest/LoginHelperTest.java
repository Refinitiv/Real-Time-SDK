///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.unittest;

import static org.junit.Assert.assertEquals;

import java.nio.ByteBuffer;
import java.util.Iterator;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import com.rtsdk.ema.access.ElementEntry;
import com.rtsdk.ema.access.ElementList;
import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.JUnitTestConnect;
import com.rtsdk.ema.access.OmmInvalidUsageException;
import com.rtsdk.ema.access.OmmState.DataState;
import com.rtsdk.ema.access.OmmState.StatusCode;
import com.rtsdk.ema.access.OmmState.StreamState;
import com.rtsdk.ema.access.RefreshMsg;
import com.rtsdk.ema.access.ReqMsg;
import com.rtsdk.ema.access.StatusMsg;
import com.rtsdk.ema.domain.login.Login.LoginRefresh;
import com.rtsdk.ema.domain.login.Login.LoginReq;
import com.rtsdk.ema.domain.login.Login.LoginStatus;
import com.rtsdk.ema.rdm.EmaRdm;
import com.rtsdk.eta.codec.Codec;

public class LoginHelperTest {
	
	@Rule
	public final ExpectedException exception = ExpectedException.none();

	@Test
	public void encodeLoginRequestTest()
	{
		LoginReq loginReq = EmaFactory.Domain.createLoginReq();
		
		loginReq.name("UserName");
		
		assertEquals(true, loginReq.hasName());
		
		assertEquals("UserName", loginReq.name());
		
		loginReq.pause(true);
		
		assertEquals(true, loginReq.hasPause());
		
		assertEquals(true, loginReq.pause());
		
		loginReq.allowSuspectData(true);
		
		assertEquals(true, loginReq.hasAllowSuspectData());
		
		assertEquals(true, loginReq.allowSuspectData());
		
		loginReq.downloadConnectionConfig(true);
		
		assertEquals(true, loginReq.hasDownloadConnectionConfig());
		
		assertEquals(true, loginReq.downloadConnectionConfig());
		
		loginReq.applicationId("123");
		
		assertEquals(true, loginReq.hasApplicationId());
		
		assertEquals("123", loginReq.applicationId());
		
		loginReq.applicationName("application name test");
		
		assertEquals(true, loginReq.hasApplicationName());
		
		assertEquals("application name test", loginReq.applicationName());
		
		loginReq.applicationAuthorizationToken("application authentication token");
		
		assertEquals(true, loginReq.hasApplicationAuthorizationToken());
		
		assertEquals("application authentication token", loginReq.applicationAuthorizationToken());
		
		loginReq.instanceId("555");
		
		assertEquals(true, loginReq.hasInstanceId());
		
		assertEquals("555", loginReq.instanceId());
		
		loginReq.password("@password");
		
		assertEquals(true, loginReq.hasPassword());
		
		assertEquals("@password", loginReq.password());
		
		loginReq.position("127.0.0.1/net");
		
		assertEquals(true, loginReq.hasPosition());
		
		assertEquals("127.0.0.1/net", loginReq.position());
		
		loginReq.providePermissionExpressions(true);
		
		assertEquals(true, loginReq.hasProvidePermissionExpressions());
		
		assertEquals(true, loginReq.providePermissionExpressions());
		
		loginReq.providePermissionProfile(true);
		
		assertEquals(true, loginReq.hasProvidePermissionProfile());
		
		assertEquals(true, loginReq.providePermissionProfile());
		
		loginReq.role(EmaRdm.LOGIN_ROLE_PROV);
		
		assertEquals(true, loginReq.hasRole());
		
		assertEquals(EmaRdm.LOGIN_ROLE_PROV, loginReq.role());
		
		loginReq.singleOpen(true);
		
		assertEquals(true, loginReq.hasSingleOpen());
		
		assertEquals(true, loginReq.singleOpen());
		
		loginReq.supportProviderDictionaryDownload(true);
		
		assertEquals(true, loginReq.hasSupportProviderDictionaryDownload());
		
		assertEquals(true, loginReq.supportProviderDictionaryDownload());

		ByteBuffer authenticationExtended = ByteBuffer.allocate(22);
		
		authenticationExtended.put("authenticationExtended".getBytes());
		
		authenticationExtended.flip();
		
		loginReq.authenticationExtended(authenticationExtended);
		
		assertEquals(true, loginReq.hasAuthenticationExtended());
		
		assertEquals(authenticationExtended, loginReq.authenticationExtended());
		
		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory
						.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
        ReqMsg decReqMsg = JUnitTestConnect.createReqMsg();
        
        JUnitTestConnect.setRsslData(decReqMsg, loginReq.message(), Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

        assertEquals(true, decReqMsg.pause());
        
        com.rtsdk.ema.access.ElementList decodedEl = JUnitTestConnect.createElementList();
        JUnitTestConnect.setRsslData(decodedEl, decReqMsg.attrib().elementList(), Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

		Iterator<ElementEntry> iterator = decodedEl.iterator();
		
		ElementEntry elementEntry;
		String elementName = null;
		
		while(iterator.hasNext())
		{
			elementEntry = iterator.next();
			elementName = elementEntry.name();
			
			switch(elementName)
			{
			case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
			{
				assertEquals(1, elementEntry.uintValue());
			}
				break;
			case EmaRdm.ENAME_APP_ID:
			{
				assertEquals("123", elementEntry.ascii().ascii());
			}
				break;
			case EmaRdm.ENAME_APP_NAME:
			{
				assertEquals("application name test",elementEntry.ascii().ascii());
			}
				break;
			case EmaRdm.ENAME_APPAUTH_TOKEN:
			{
				assertEquals("application authentication token",elementEntry.ascii().ascii());
			}
				break;
			case EmaRdm.ENAME_DOWNLOAD_CON_CONFIG:
			{
				assertEquals(1, elementEntry.uintValue());
			}
				break;
			case EmaRdm.ENAME_INST_ID:
			{
				assertEquals("555",elementEntry.ascii().ascii());
			}
				break;
			case EmaRdm.ENAME_PASSWORD:
			{
				assertEquals("@password",elementEntry.ascii().ascii());
			}
				break;
			case EmaRdm.ENAME_POSITION:
			{
				assertEquals("127.0.0.1/net",elementEntry.ascii().ascii());
			}
				break;
			case EmaRdm.ENAME_PROV_PERM_EXP:
			{
				assertEquals(1, elementEntry.uintValue());
			}
				break;
			case EmaRdm.ENAME_PROV_PERM_PROF:
			{
				assertEquals(1, elementEntry.uintValue());
			}
				break;
			case EmaRdm.ENAME_ROLE:
			{
				assertEquals(EmaRdm.LOGIN_ROLE_PROV,elementEntry.uintValue());
			}
				break;
			case EmaRdm.ENAME_SINGLE_OPEN:
			{
				assertEquals(1,elementEntry.uintValue());
			}
				break;
			case EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD:
			{
				assertEquals(1, elementEntry.uintValue());
			}
				break;
			case EmaRdm.ENAME_AUTHN_EXTENDED:
			{
			    assertEquals(authenticationExtended, elementEntry.buffer().buffer());
			}
			    break;
			default:
				break;
			}
		}
	}
	
    @Test
    public void headerLoginReqTest()
    {
        LoginReq loginReq = EmaFactory.Domain.createLoginReq();
        
        loginReq.name("UserName");
        loginReq.nameType(EmaRdm.USER_AUTH_TOKEN);
        
        assertEquals(true, loginReq.hasName());
        
        assertEquals("UserName", loginReq.name());
        
        assertEquals(true, loginReq.hasNameType());
        
        assertEquals(EmaRdm.USER_AUTH_TOKEN, loginReq.nameType());
        
        ReqMsg reqMsg = loginReq.message();
        
        assertEquals("\0", reqMsg.name()); 
    }
    
    @Test
    public void clearLoginReqTest()
    {
        LoginReq loginReq = EmaFactory.Domain.createLoginReq();
        
        loginReq.allowSuspectData(false);
        loginReq.downloadConnectionConfig(true);
        loginReq.providePermissionProfile(false);
        loginReq.providePermissionExpressions(false);
        loginReq.singleOpen(false);
        loginReq.supportProviderDictionaryDownload(true);
        loginReq.role(EmaRdm.LOGIN_ROLE_PROV);
        loginReq.applicationId("AppId");
        loginReq.applicationName("AppName");
        loginReq.applicationAuthorizationToken("AppAuthToken");
        loginReq.instanceId("InstanceId");
        loginReq.password("12345");
        loginReq.position("Position");
        loginReq.authenticationExtended(ByteBuffer.wrap("AuthExtended".getBytes()));
        loginReq.name("Name");
        loginReq.nameType(EmaRdm.USER_EMAIL_ADDRESS);
        loginReq.pause(true);
        
        assertEquals(true, loginReq.hasAllowSuspectData());
        assertEquals(false, loginReq.allowSuspectData());
        assertEquals(true, loginReq.hasDownloadConnectionConfig());
        assertEquals(true, loginReq.downloadConnectionConfig());
        assertEquals(true, loginReq.hasProvidePermissionProfile());
        assertEquals(false, loginReq.providePermissionProfile());
        assertEquals(true, loginReq.hasProvidePermissionExpressions());
        assertEquals(false, loginReq.providePermissionExpressions());
        assertEquals(true, loginReq.hasSingleOpen());
        assertEquals(false, loginReq.singleOpen());
        assertEquals(true, loginReq.hasSupportProviderDictionaryDownload());
        assertEquals(true, loginReq.supportProviderDictionaryDownload());
        assertEquals(true, loginReq.hasRole());
        assertEquals(EmaRdm.LOGIN_ROLE_PROV, loginReq.role());
        assertEquals(true, loginReq.hasApplicationId());
        assertEquals("AppId", loginReq.applicationId());
        assertEquals(true, loginReq.hasApplicationName());
        assertEquals("AppName", loginReq.applicationName());
        assertEquals(true, loginReq.hasApplicationAuthorizationToken());
        assertEquals("AppAuthToken", loginReq.applicationAuthorizationToken());
        assertEquals(true, loginReq.hasInstanceId());
        assertEquals("InstanceId", loginReq.instanceId());
        assertEquals(true, loginReq.hasPassword());
        assertEquals("12345", loginReq.password());
        assertEquals(true, loginReq.hasPosition());
        assertEquals("Position", loginReq.position());
        assertEquals(true, loginReq.hasAuthenticationExtended());
        assertEquals(ByteBuffer.wrap("AuthExtended".getBytes()), loginReq.authenticationExtended());
        assertEquals(true, loginReq.hasName());
        assertEquals("Name", loginReq.name());
        assertEquals(true, loginReq.hasNameType());
        assertEquals(EmaRdm.USER_EMAIL_ADDRESS, loginReq.nameType());
        assertEquals(true, loginReq.hasPause());
        assertEquals(true, loginReq.pause());
        
        loginReq.clear();
        
        assertEquals(true, loginReq.hasAllowSuspectData());
        assertEquals(true, loginReq.allowSuspectData());
        assertEquals(false, loginReq.hasDownloadConnectionConfig());
        assertEquals(false, loginReq.downloadConnectionConfig());
        assertEquals(true, loginReq.hasProvidePermissionProfile());
        assertEquals(true, loginReq.providePermissionProfile());
        assertEquals(true, loginReq.hasProvidePermissionExpressions());
        assertEquals(true, loginReq.providePermissionExpressions());
        assertEquals(true, loginReq.hasSingleOpen());
        assertEquals(true, loginReq.singleOpen());
        assertEquals(false, loginReq.hasSupportProviderDictionaryDownload());
        assertEquals(false, loginReq.supportProviderDictionaryDownload());
        assertEquals(true, loginReq.hasRole());
        assertEquals(EmaRdm.LOGIN_ROLE_CONS, loginReq.role());
        assertEquals(true, loginReq.hasApplicationId());
        assertEquals(true, loginReq.hasApplicationName());
        assertEquals(false, loginReq.hasApplicationAuthorizationToken());
        assertEquals(false, loginReq.hasInstanceId());
        assertEquals(false, loginReq.hasPassword());
        assertEquals(true, loginReq.hasPosition());
        assertEquals(false, loginReq.hasAuthenticationExtended());
        assertEquals(true, loginReq.hasName());
        assertEquals(true, loginReq.hasNameType());
        assertEquals(EmaRdm.USER_NAME, loginReq.nameType());
        assertEquals(false, loginReq.hasPause());
        assertEquals(false, loginReq.pause());
    }
	
	@Test
	public void decodeLoginRequestTest()
	{
		ElementList encodedElementList = EmaFactory.createElementList();
		
		encodedElementList.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1));
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APP_ID, "123"));
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APP_NAME, "application name test"));
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APPAUTH_TOKEN, "application authentication token"));
		encodedElementList.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_DOWNLOAD_CON_CONFIG, 1));
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_INST_ID, "555"));
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_PASSWORD, "@password"));
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_POSITION, "127.0.0.1/net"));
		encodedElementList.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_PROV_PERM_EXP, 1));
		encodedElementList.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_PROV_PERM_PROF, 1));
		encodedElementList.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_ROLE, 1));
		encodedElementList.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SINGLE_OPEN, 1));
		encodedElementList.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD, 1));
        ByteBuffer authenticationExtended = ByteBuffer.allocate(22);
        authenticationExtended.put("authenticationExtended".getBytes());
        authenticationExtended.flip();
		encodedElementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_AUTHN_EXTENDED,  authenticationExtended));
		
		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

        ReqMsg encReqMsg = EmaFactory.createReqMsg();

        encReqMsg.domainType(EmaRdm.MMT_LOGIN);
        encReqMsg.attrib(encodedElementList);   
        
        ReqMsg decReqMsg = JUnitTestConnect.createReqMsg();

        JUnitTestConnect.setRsslData(decReqMsg, encReqMsg, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
        
        LoginReq loginReq = EmaFactory.Domain.createLoginReq();

        loginReq.message(decReqMsg);
		
		assertEquals(true, loginReq.hasAllowSuspectData());
		
		assertEquals(true, loginReq.allowSuspectData());
		
		assertEquals(true, loginReq.hasDownloadConnectionConfig());
		
		assertEquals(true, loginReq.downloadConnectionConfig());
		
		assertEquals(true, loginReq.hasApplicationId());
		
		assertEquals("123", loginReq.applicationId());
		
		assertEquals(true, loginReq.hasApplicationName());
		
		assertEquals("application name test", loginReq.applicationName());
		
		assertEquals(true, loginReq.hasApplicationAuthorizationToken());
		
		assertEquals("application authentication token", loginReq.applicationAuthorizationToken());
		
		assertEquals(true, loginReq.hasInstanceId());
		
		assertEquals("555", loginReq.instanceId());
		
		assertEquals(true, loginReq.hasPassword());
		
		assertEquals("@password", loginReq.password());
		
		assertEquals(true, loginReq.hasPosition());
		
		assertEquals("127.0.0.1/net", loginReq.position());
		
		assertEquals(true, loginReq.hasProvidePermissionExpressions());
		
		assertEquals(true, loginReq.providePermissionExpressions());
		
		assertEquals(true, loginReq.hasProvidePermissionProfile());
		
		assertEquals(true, loginReq.providePermissionProfile());
		
		assertEquals(true, loginReq.hasRole());
		
		assertEquals(EmaRdm.LOGIN_ROLE_PROV, loginReq.role());
		
		assertEquals(true, loginReq.hasSingleOpen());
		
		assertEquals(true, loginReq.singleOpen());
		
		assertEquals(true, loginReq.hasSupportProviderDictionaryDownload());
		
		assertEquals(true, loginReq.supportProviderDictionaryDownload());

		assertEquals(true, loginReq.hasAuthenticationExtended());
		
		assertEquals(authenticationExtended, loginReq.authenticationExtended());
	}
	
	@Test
	public void blankLoginRequestTest()
	{
		ElementList encodedElementList = EmaFactory.createElementList();
		
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APP_ID, ""));
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APP_NAME, ""));
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APPAUTH_TOKEN, ""));
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_INST_ID, ""));
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_PASSWORD, ""));
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_POSITION, ""));

        ByteBuffer authenticationExtended = ByteBuffer.allocate(0);
        //authenticationExtended.put("".getBytes());
        authenticationExtended.flip();
		encodedElementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_AUTHN_EXTENDED,  authenticationExtended));
		
		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

        ReqMsg encReqMsg = EmaFactory.createReqMsg();

        encReqMsg.domainType(EmaRdm.MMT_LOGIN);
        encReqMsg.attrib(encodedElementList);   
        
        ReqMsg decReqMsg = JUnitTestConnect.createReqMsg();

        JUnitTestConnect.setRsslData(decReqMsg, encReqMsg, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
        
        LoginReq loginReq = EmaFactory.Domain.createLoginReq();
        loginReq.applicationId("123");
        loginReq.applicationName("name");
        loginReq.applicationAuthorizationToken("token");
        loginReq.instanceId("id");
        loginReq.password("password");
        loginReq.position("123.456.789.012");
        authenticationExtended = ByteBuffer.allocate(5);
        authenticationExtended.put("tests".getBytes());
        authenticationExtended.flip();
        loginReq.authenticationExtended(authenticationExtended);

        loginReq.message(decReqMsg);
		
		assertEquals(false, loginReq.hasApplicationId());
		
		assertEquals(false, loginReq.hasApplicationName());

		assertEquals(false, loginReq.hasApplicationAuthorizationToken());
		
		assertEquals(false, loginReq.hasInstanceId());
		
		assertEquals(false, loginReq.hasPassword());
		
		assertEquals(false, loginReq.hasPosition());
		
		assertEquals(false, loginReq.hasAuthenticationExtended());
	}
	
	@Test
	public void errorHandlingLoginRequestTest()
	{
		LoginReq loginReq = EmaFactory.Domain.createLoginReq();
		
		exception.expect(OmmInvalidUsageException.class);
		exception.expectMessage("ApplicationAuthorizationToken element is not set");
		loginReq.applicationAuthorizationToken();
		
		exception.expect(OmmInvalidUsageException.class);
		exception.expectMessage("InstanceId element is not set");
		loginReq.instanceId();
		
		exception.expect(OmmInvalidUsageException.class);
		exception.expectMessage("Password element is not set");
		loginReq.password();
		
		exception.expect(OmmInvalidUsageException.class);
		exception.expectMessage("Position element is not set");
		loginReq.position();

        exception.expect(OmmInvalidUsageException.class);
        exception.expectMessage("authenticationExtended element is not set");
        loginReq.authenticationExtended();
	}
	
	@Test
	public void decodeLoginReqInvalidTypeTest()
	{
		ElementList encodedElementList = EmaFactory.createElementList();
		
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, "1"));

		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
        ReqMsg encReqMsg = EmaFactory.createReqMsg();

        encReqMsg.domainType(EmaRdm.MMT_LOGIN);
        encReqMsg.attrib(encodedElementList);   
        
        ReqMsg decReqMsg = JUnitTestConnect.createReqMsg();

        JUnitTestConnect.setRsslData(decReqMsg, encReqMsg, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

		exception.expect(OmmInvalidUsageException.class);
		exception.expectMessage("Decoding error for AllowSuspectData element. Attempt to uintValue() while actual entry data type is Ascii");

        LoginReq loginReq = EmaFactory.Domain.createLoginReq();

        loginReq.message(decReqMsg);
		
	}
	
	@Test
	public void encodeLoginRefreshTest()
	{
		LoginRefresh loginRefresh = EmaFactory.Domain.createLoginRefresh();
		
		loginRefresh.solicited(true);
        
        assertEquals(true, loginRefresh.hasSolicited());
        
        assertEquals(true, loginRefresh.solicited());
		
		loginRefresh.allowSuspectData(true);
		
		assertEquals(true, loginRefresh.hasAllowSuspectData());
		
		assertEquals(true, loginRefresh.allowSuspectData());
		
		loginRefresh.applicationId("123");
		
		assertEquals(true, loginRefresh.hasApplicationId());
		
		assertEquals("123", loginRefresh.applicationId());
		
		loginRefresh.applicationName("application name test");
		
		assertEquals(true, loginRefresh.hasApplicationName());
		
		assertEquals("application name test", loginRefresh.applicationName());
		
		loginRefresh.position("127.0.0.1/net");
		
		assertEquals(true, loginRefresh.hasPosition());
		
		assertEquals("127.0.0.1/net", loginRefresh.position());
		
		loginRefresh.providePermissionExpressions(true);
		
		assertEquals(true, loginRefresh.hasProvidePermissionExpressions());
		
		assertEquals(true, loginRefresh.providePermissionExpressions());
		
		loginRefresh.providePermissionProfile(true);
		
		assertEquals(true, loginRefresh.hasProvidePermissionProfile());
		
		assertEquals(true, loginRefresh.providePermissionProfile());
		
		loginRefresh.singleOpen(true);
		
		assertEquals(true, loginRefresh.hasSingleOpen());
		
		assertEquals(true, loginRefresh.singleOpen());
		
		loginRefresh.supportBatchRequests(EmaRdm.SUPPORT_BATCH_REQUEST | EmaRdm.SUPPORT_BATCH_REISSUE | EmaRdm.SUPPORT_BATCH_CLOSE);
		
		assertEquals(true, loginRefresh.hasSupportBatchRequests());
		
		assertEquals(EmaRdm.SUPPORT_BATCH_REQUEST | EmaRdm.SUPPORT_BATCH_REISSUE | EmaRdm.SUPPORT_BATCH_CLOSE, loginRefresh.supportBatchRequests());
		
		loginRefresh.supportEnhancedSymbolList(EmaRdm.SUPPORT_SYMBOL_LIST_DATA_STREAMS);
		
		assertEquals(true, loginRefresh.hasSupportEnhancedSymbolList());
		
		assertEquals(EmaRdm.SUPPORT_SYMBOL_LIST_DATA_STREAMS, loginRefresh.supportEnhancedSymbolList());
		
		loginRefresh.supportOMMPost(true);
		
		assertEquals(true, loginRefresh.hasSupportOMMPost());
		
		assertEquals(true, loginRefresh.supportOMMPost());
		
		loginRefresh.supportOptimizedPauseResume(true);
		
		assertEquals(true, loginRefresh.hasSupportOptimizedPauseResume());
		
		assertEquals(true, loginRefresh.supportOptimizedPauseResume());
		
		loginRefresh.supportProviderDictionaryDownload(true);
		
		assertEquals(true, loginRefresh.hasSupportProviderDictionaryDownload());
		
		assertEquals(true, loginRefresh.supportProviderDictionaryDownload());
		
		loginRefresh.supportViewRequests(true);
		
		assertEquals(true, loginRefresh.hasSupportViewRequests());
		
		assertEquals(true, loginRefresh.supportViewRequests());
		
		loginRefresh.supportStandby(true);
		
		assertEquals(true, loginRefresh.hasSupportStandby());
		
		assertEquals(true, loginRefresh.supportStandby());

        ByteBuffer authenticationExtended = ByteBuffer.allocate(22);
        
        authenticationExtended.put("authenticationExtended".getBytes());
        
        authenticationExtended.flip();
        
        loginRefresh.authenticationExtendedResp(authenticationExtended);
        
        assertEquals(true, loginRefresh.hasAuthenticationExtended());
        
        assertEquals(authenticationExtended, loginRefresh.authenticationExtended());

        loginRefresh.authenticationTTReissue(2);
        
        assertEquals(true, loginRefresh.hasAuthenticationTTReissue());
        
        assertEquals(2, loginRefresh.authenticationTTReissue());
        
        loginRefresh.authenticationErrorCode(3);
        
        assertEquals(true, loginRefresh.hasAuthenticationErrorCode());
        
        assertEquals(3, loginRefresh.authenticationErrorCode());
        
        loginRefresh.authenticationErrorText("authenticationErrorText");
        
        assertEquals(true, loginRefresh.hasAuthenticationErrorText());
        
        assertEquals("authenticationErrorText", loginRefresh.authenticationErrorText());
		
		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory
						.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

        RefreshMsg decRefreshMsg = JUnitTestConnect.createRefreshMsg();
        
        JUnitTestConnect.setRsslData(decRefreshMsg, loginRefresh.message(), Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

		com.rtsdk.ema.access.ElementList decodedEl = JUnitTestConnect.createElementList();
		JUnitTestConnect.setRsslData(decodedEl, decRefreshMsg.attrib().elementList(), Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		Iterator<ElementEntry> iterator = decodedEl.iterator();
		
		ElementEntry elementEntry;
		String elementName = null;
		
		while(iterator.hasNext())
		{
			elementEntry = iterator.next();
			elementName = elementEntry.name();
			
			switch(elementName)
			{
			case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
			{
				assertEquals(1, elementEntry.uintValue());
			}
				break;
			case EmaRdm.ENAME_APP_ID:
			{
				assertEquals("123", elementEntry.ascii().ascii());
			}
				break;
			case EmaRdm.ENAME_APP_NAME:
			{
				assertEquals("application name test",elementEntry.ascii().ascii());
			}
				break;
			case EmaRdm.ENAME_POSITION:
			{
				assertEquals("127.0.0.1/net",elementEntry.ascii().ascii());
			}
				break;
			case EmaRdm.ENAME_PROV_PERM_EXP:
			{
				assertEquals(1, elementEntry.uintValue());
			}
				break;
			case EmaRdm.ENAME_PROV_PERM_PROF:
			{
				assertEquals(1, elementEntry.uintValue());
			}
				break;
			case EmaRdm.ENAME_SINGLE_OPEN:
			{
				assertEquals(1, elementEntry.uintValue());
			}
				break;
			case EmaRdm.ENAME_SUPPORT_BATCH:
			{
				assertEquals(EmaRdm.SUPPORT_BATCH_REQUEST | EmaRdm.SUPPORT_BATCH_REISSUE | EmaRdm.SUPPORT_BATCH_CLOSE,elementEntry.uintValue());
			}
				break;
			case EmaRdm.ENAME_SUPPORT_POST:
			{
				assertEquals(1, elementEntry.uintValue());
			}
				break;
			case EmaRdm.ENAME_SUPPORT_OPR:
			{
				assertEquals(1, elementEntry.uintValue());
			}
				break;
			case EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD:
			{
				assertEquals(1, elementEntry.uintValue());
			}
				break;
			case EmaRdm.ENAME_SUPPORT_VIEW:
			{
				assertEquals(1, elementEntry.uintValue());
			}
				break;
			case EmaRdm.ENAME_SUPPORT_STANDBY:
			{
				assertEquals(1, elementEntry.uintValue());
			}
				break;
            case EmaRdm.ENAME_AUTHN_EXTENDED:
            {
                assertEquals(authenticationExtended, elementEntry.buffer().buffer());
            }
                break;   
            case EmaRdm.ENAME_AUTHN_TT_REISSUE:
            {
                assertEquals(2, elementEntry.uintValue());
            }
                break;   
            case EmaRdm.ENAME_AUTHN_ERRORCODE:
            {
                assertEquals(3, elementEntry.uintValue());
            }
                break;       
            case EmaRdm.ENAME_AUTHN_ERRORTEXT:
            {
                assertEquals("authenticationErrorText", elementEntry.ascii().ascii());
            }
                break;                            
			default:
				break;
			}
		}
	}
	
	@Test
	public void decodeLoginRefreshTest()
	{
		ElementList encodedElementList = EmaFactory.createElementList();
		
		encodedElementList.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1));
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APP_ID, "123"));
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APP_NAME, "application name test"));
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_POSITION, "127.0.0.1/net"));
		encodedElementList.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_PROV_PERM_EXP, 1));
		encodedElementList.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_PROV_PERM_PROF, 1));
		encodedElementList.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SINGLE_OPEN, 1));
		encodedElementList.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_BATCH, 7));
		encodedElementList.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_ENH_SYMBOL_LIST, 1));
		encodedElementList.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_POST, 1));
		encodedElementList.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_OPR, 1));
		encodedElementList.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD, 1));
		encodedElementList.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_VIEW, 1));
		encodedElementList.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_STANDBY, 1));
        ByteBuffer authenticationExtended = ByteBuffer.allocate(22);
        authenticationExtended.put("authenticationExtended".getBytes());
        authenticationExtended.flip();
        encodedElementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_AUTHN_EXTENDED_RESP,  authenticationExtended));
        encodedElementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_AUTHN_TT_REISSUE,  2));
        encodedElementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_AUTHN_ERRORCODE,  3));
        encodedElementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_AUTHN_ERRORTEXT,  "authenticationErrorText"));

        com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory
                .createDataDictionary();
        TestUtilities.eta_encodeDictionaryMsg(dictionary);
        
        RefreshMsg encRefreshMsg = EmaFactory.createRefreshMsg();

        encRefreshMsg.domainType(EmaRdm.MMT_LOGIN);
        encRefreshMsg.attrib(encodedElementList);   
        
        RefreshMsg decRefreshMsg = JUnitTestConnect.createRefreshMsg();

        JUnitTestConnect.setRsslData(decRefreshMsg, encRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
        
        LoginRefresh loginRefresh = EmaFactory.Domain.createLoginRefresh();

		loginRefresh.message(decRefreshMsg);
		
		assertEquals(true, loginRefresh.hasAllowSuspectData());
		
		assertEquals(true, loginRefresh.allowSuspectData());
		
		assertEquals(true, loginRefresh.hasApplicationId());
		
		assertEquals("123", loginRefresh.applicationId());
		
		assertEquals(true, loginRefresh.hasApplicationName());
		
		assertEquals("application name test", loginRefresh.applicationName());
		
		assertEquals(true, loginRefresh.hasPosition());
		
		assertEquals("127.0.0.1/net", loginRefresh.position());
		
		assertEquals(true, loginRefresh.hasProvidePermissionExpressions());
		
		assertEquals(true, loginRefresh.providePermissionExpressions());
		
		assertEquals(true, loginRefresh.hasProvidePermissionProfile());
		
		assertEquals(true, loginRefresh.providePermissionProfile());
		
		assertEquals(true, loginRefresh.hasSingleOpen());
		
		assertEquals(true, loginRefresh.singleOpen());
		
		assertEquals(true, loginRefresh.hasSupportBatchRequests());
		
		assertEquals(EmaRdm.SUPPORT_BATCH_REQUEST | EmaRdm.SUPPORT_BATCH_REISSUE | EmaRdm.SUPPORT_BATCH_CLOSE, loginRefresh.supportBatchRequests());
		
		assertEquals(true, loginRefresh.hasSupportEnhancedSymbolList());
		
		assertEquals(EmaRdm.SUPPORT_SYMBOL_LIST_DATA_STREAMS, loginRefresh.supportEnhancedSymbolList());
		
		assertEquals(true, loginRefresh.hasSupportOMMPost());
		
		assertEquals(true, loginRefresh.supportOMMPost());

		assertEquals(true, loginRefresh.hasSupportOptimizedPauseResume());
		
		assertEquals(true, loginRefresh.supportOptimizedPauseResume());
	
		assertEquals(true, loginRefresh.hasSupportProviderDictionaryDownload());
		
		assertEquals(true, loginRefresh.supportProviderDictionaryDownload());

		assertEquals(true, loginRefresh.hasSupportViewRequests());
		
		assertEquals(true, loginRefresh.supportViewRequests());
		
		assertEquals(true, loginRefresh.hasSupportStandby());
		
		assertEquals(true, loginRefresh.supportStandby());

        assertEquals(true, loginRefresh.hasAuthenticationExtended());
        
        assertEquals(authenticationExtended, loginRefresh.authenticationExtended());

        assertEquals(true, loginRefresh.hasAuthenticationTTReissue());
        
        assertEquals(2, loginRefresh.authenticationTTReissue());
        
        assertEquals(true, loginRefresh.hasAuthenticationErrorCode());
        
        assertEquals(3, loginRefresh.authenticationErrorCode());
        
        assertEquals(true, loginRefresh.hasAuthenticationErrorText());
        
        assertEquals("authenticationErrorText", loginRefresh.authenticationErrorText());
	}
	
	@Test
	public void blankLoginRefreshTest()
	{
		ElementList encodedElementList = EmaFactory.createElementList();
		
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APP_ID, ""));
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APP_NAME, ""));
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_POSITION, ""));
        ByteBuffer authenticationExtended = ByteBuffer.allocate(0);
        authenticationExtended.put("".getBytes());
        authenticationExtended.flip();
        encodedElementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_AUTHN_EXTENDED_RESP,  authenticationExtended));
        encodedElementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_AUTHN_ERRORTEXT,  ""));

        com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory
                .createDataDictionary();
        TestUtilities.eta_encodeDictionaryMsg(dictionary);
        
        RefreshMsg encRefreshMsg = EmaFactory.createRefreshMsg();

        encRefreshMsg.domainType(EmaRdm.MMT_LOGIN);
        encRefreshMsg.attrib(encodedElementList);   
        
        RefreshMsg decRefreshMsg = JUnitTestConnect.createRefreshMsg();

        JUnitTestConnect.setRsslData(decRefreshMsg, encRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
        
        LoginRefresh loginRefresh = EmaFactory.Domain.createLoginRefresh();
        
        loginRefresh.applicationId("123");
        loginRefresh.applicationName("name");
        loginRefresh.position("123.456.789.012");
        authenticationExtended = ByteBuffer.allocate(5);
        authenticationExtended.put("tests".getBytes());
        authenticationExtended.flip();
        loginRefresh.authenticationExtendedResp(authenticationExtended);
        loginRefresh.authenticationErrorText("error");

		loginRefresh.message(decRefreshMsg);

		
		assertEquals(false, loginRefresh.hasApplicationId());

		assertEquals(false, loginRefresh.hasApplicationName());
		
		assertEquals(false, loginRefresh.hasPosition());

        assertEquals(false, loginRefresh.hasAuthenticationExtended());
        
        assertEquals(false, loginRefresh.hasAuthenticationErrorText());
	}
	
	@Test
	public void headerLoginRefreshTest()
	{
	    LoginRefresh loginRefresh = EmaFactory.Domain.createLoginRefresh();
	    
        loginRefresh.state(StreamState.OPEN, DataState.OK, StatusCode.NONE, "headerLoginRefreshTest");
        loginRefresh.name("UserName");
        loginRefresh.nameType(EmaRdm.USER_NAME);
        loginRefresh.seqNum(5);
        loginRefresh.solicited(true);
        
        assertEquals(true, loginRefresh.hasState());
        
        assertEquals(StreamState.OPEN, loginRefresh.state().streamState());
        assertEquals(DataState.OK, loginRefresh.state().dataState());
        assertEquals(StatusCode.NONE, loginRefresh.state().statusCode());
        assertEquals("headerLoginRefreshTest", loginRefresh.state().statusText());
        
        assertEquals(true, loginRefresh.hasName());
        
        assertEquals("UserName", loginRefresh.name());
        
        assertEquals(true, loginRefresh.hasNameType());
        
        assertEquals(EmaRdm.USER_NAME, loginRefresh.nameType());
        
        assertEquals(true, loginRefresh.hasSeqNum());
        
        assertEquals(5, loginRefresh.seqNum());
        
        assertEquals(true, loginRefresh.hasSolicited());
        
        assertEquals(true, loginRefresh.solicited());
	}
	
    @Test
    public void clearLoginRefreshTest()
    {
        LoginRefresh loginRefresh = EmaFactory.Domain.createLoginRefresh();
        
        loginRefresh.allowSuspectData(false);
        loginRefresh.providePermissionProfile(false);
        loginRefresh.providePermissionExpressions(false);
        loginRefresh.singleOpen(false);
        loginRefresh.supportProviderDictionaryDownload(true);
        loginRefresh.supportBatchRequests(EmaRdm.SUPPORT_BATCH_REQUEST);
        loginRefresh.supportOptimizedPauseResume(true);
        loginRefresh.supportOMMPost(true);
        loginRefresh.supportViewRequests(true);
        loginRefresh.supportStandby(true);
        loginRefresh.supportEnhancedSymbolList(EmaRdm.SUPPORT_SYMBOL_LIST_DATA_STREAMS);
        loginRefresh.authenticationTTReissue(10);
        loginRefresh.authenticationErrorCode(10);
        loginRefresh.authenticationErrorText("ErrorText");
        loginRefresh.state(StreamState.OPEN, DataState.OK, StatusCode.NONE, "StatusText");
        loginRefresh.seqNum(10);
        loginRefresh.applicationId("AppId");
        loginRefresh.applicationName("AppName");
        loginRefresh.position("Position");
        loginRefresh.authenticationExtendedResp(ByteBuffer.wrap("AuthExtended".getBytes()));
        loginRefresh.name("Name");
        loginRefresh.nameType(EmaRdm.USER_EMAIL_ADDRESS);
        loginRefresh.solicited(false);
        
        assertEquals(true, loginRefresh.hasAllowSuspectData());
        assertEquals(false, loginRefresh.allowSuspectData());
        assertEquals(true, loginRefresh.hasProvidePermissionProfile());
        assertEquals(false, loginRefresh.providePermissionProfile());
        assertEquals(true, loginRefresh.hasProvidePermissionExpressions());
        assertEquals(false, loginRefresh.providePermissionExpressions());
        assertEquals(true, loginRefresh.hasSingleOpen());
        assertEquals(false, loginRefresh.singleOpen());
        assertEquals(true, loginRefresh.hasSupportProviderDictionaryDownload());
        assertEquals(true, loginRefresh.supportProviderDictionaryDownload());
        assertEquals(true, loginRefresh.hasSupportBatchRequests());
        assertEquals(EmaRdm.SUPPORT_BATCH_REQUEST, loginRefresh.supportBatchRequests());
        assertEquals(true, loginRefresh.hasSupportOptimizedPauseResume());
        assertEquals(true, loginRefresh.supportOptimizedPauseResume());
        assertEquals(true, loginRefresh.hasSupportOMMPost());
        assertEquals(true, loginRefresh.supportOMMPost());
        assertEquals(true, loginRefresh.hasSupportViewRequests());
        assertEquals(true, loginRefresh.supportViewRequests());
        assertEquals(true, loginRefresh.hasSupportStandby());
        assertEquals(true, loginRefresh.supportStandby());
        assertEquals(true, loginRefresh.hasSupportEnhancedSymbolList());
        assertEquals(EmaRdm.SUPPORT_SYMBOL_LIST_DATA_STREAMS, loginRefresh.supportEnhancedSymbolList());
        assertEquals(true, loginRefresh.hasAuthenticationTTReissue());
        assertEquals(10, loginRefresh.authenticationTTReissue());
        assertEquals(true, loginRefresh.hasAuthenticationErrorCode());
        assertEquals(10, loginRefresh.authenticationErrorCode());
        assertEquals(true, loginRefresh.hasAuthenticationErrorText());
        assertEquals("ErrorText", loginRefresh.authenticationErrorText());
        assertEquals(true, loginRefresh.hasState());
        assertEquals(StreamState.OPEN, loginRefresh.state().streamState());
        assertEquals(DataState.OK, loginRefresh.state().dataState());
        assertEquals(StatusCode.NONE, loginRefresh.state().statusCode());
        assertEquals("StatusText", loginRefresh.state().statusText());
        assertEquals(true, loginRefresh.hasSeqNum());
        assertEquals(10, loginRefresh.seqNum());
        assertEquals(true, loginRefresh.hasApplicationId());
        assertEquals("AppId", loginRefresh.applicationId());
        assertEquals(true, loginRefresh.hasApplicationName());
        assertEquals("AppName", loginRefresh.applicationName());
        assertEquals(true, loginRefresh.hasPosition());
        assertEquals("Position", loginRefresh.position());
        assertEquals(true, loginRefresh.hasAuthenticationExtended());
        assertEquals(ByteBuffer.wrap("AuthExtended".getBytes()), loginRefresh.authenticationExtended());
        assertEquals(true, loginRefresh.hasName());
        assertEquals("Name", loginRefresh.name());
        assertEquals(true, loginRefresh.hasNameType());
        assertEquals(EmaRdm.USER_EMAIL_ADDRESS, loginRefresh.nameType());
        assertEquals(true, loginRefresh.hasSolicited());
        assertEquals(false, loginRefresh.solicited());
        
        loginRefresh.clear();

        assertEquals(true, loginRefresh.hasAllowSuspectData());
        assertEquals(true, loginRefresh.allowSuspectData());
        assertEquals(true, loginRefresh.hasProvidePermissionProfile());
        assertEquals(true, loginRefresh.providePermissionProfile());
        assertEquals(true, loginRefresh.hasProvidePermissionExpressions());
        assertEquals(true, loginRefresh.providePermissionExpressions());
        assertEquals(false, loginRefresh.hasSupportBatchRequests());
        assertEquals(false, loginRefresh.hasSupportOptimizedPauseResume());
        assertEquals(false, loginRefresh.hasSupportOMMPost());
        assertEquals(false, loginRefresh.hasSupportViewRequests());
        assertEquals(false, loginRefresh.hasSupportStandby());
        assertEquals(false, loginRefresh.hasSupportEnhancedSymbolList());
        assertEquals(false, loginRefresh.hasAuthenticationTTReissue());
        assertEquals(false, loginRefresh.hasAuthenticationErrorCode());
        assertEquals(false, loginRefresh.hasAuthenticationErrorText());
        assertEquals(false, loginRefresh.hasState());
        assertEquals(false, loginRefresh.hasSeqNum());
        assertEquals(true, loginRefresh.hasSingleOpen());
        assertEquals(true, loginRefresh.singleOpen());
        assertEquals(false, loginRefresh.hasSupportProviderDictionaryDownload());
        assertEquals(false, loginRefresh.supportProviderDictionaryDownload());
        assertEquals(false, loginRefresh.hasApplicationId());
        assertEquals(false, loginRefresh.hasApplicationName());
        assertEquals(false, loginRefresh.hasPosition());
        assertEquals(false, loginRefresh.hasAuthenticationExtended());
        assertEquals(false, loginRefresh.hasName());
        assertEquals(true, loginRefresh.hasNameType());
        assertEquals(EmaRdm.USER_NAME, loginRefresh.nameType());
        assertEquals(true, loginRefresh.hasSolicited());
        assertEquals(true, loginRefresh.solicited());
    }
	
	@Test
	public void errorHandlingLoginRefreshTest()
	{
		LoginRefresh loginRefresh = EmaFactory.Domain.createLoginRefresh();
		
		exception.expect(OmmInvalidUsageException.class);
		exception.expectMessage("ApplicationId element is not set");
		loginRefresh.applicationId();
		
		exception.expect(OmmInvalidUsageException.class);
		exception.expectMessage("ApplicationName element is not set");
		loginRefresh.applicationName();
		
		exception.expect(OmmInvalidUsageException.class);
		exception.expectMessage("Position element is not set");
		loginRefresh.position();

        exception.expect(OmmInvalidUsageException.class);
        exception.expectMessage("AuthenticationExtended element is not set");
        loginRefresh.authenticationExtended();
	}
	
	@Test
	public void decodeLoginRefreshInvalidTypeTest()
	{
		ElementList encodedElementList = EmaFactory.createElementList();
		
		encodedElementList.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, "1"));

		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
        RefreshMsg encRefreshMsg = EmaFactory.createRefreshMsg();

        encRefreshMsg.domainType(EmaRdm.MMT_LOGIN);
        encRefreshMsg.attrib(encodedElementList);   
        
        RefreshMsg decRefreshMsg = JUnitTestConnect.createRefreshMsg();

        JUnitTestConnect.setRsslData(decRefreshMsg, encRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
        
        LoginRefresh loginRefresh = EmaFactory.Domain.createLoginRefresh();

		exception.expect(OmmInvalidUsageException.class);
		exception.expectMessage("Decoding error for AllowSuspectData element. Attempt to uintValue() while actual entry data type is Ascii");

        loginRefresh.message(decRefreshMsg);
	}
	
	   
    @Test
    public void encodeLoginStatusTest()
    {
        LoginStatus loginStatus = EmaFactory.Domain.createLoginStatus();

        loginStatus.authenticationErrorCode(3);
        
        assertEquals(true, loginStatus.hasAuthenticationErrorCode());
        
        assertEquals(3, loginStatus.authenticationErrorCode());
        
        loginStatus.authenticationErrorText("authenticationErrorText");
        
        assertEquals(true, loginStatus.hasAuthenticationErrorText());
        
        assertEquals("authenticationErrorText", loginStatus.authenticationErrorText());
        
        com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory
                        .createDataDictionary();
        TestUtilities.eta_encodeDictionaryMsg(dictionary);
        
        StatusMsg decStatusMsg = JUnitTestConnect.createStatusMsg();
        
        JUnitTestConnect.setRsslData(decStatusMsg, loginStatus.message(), Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

        com.rtsdk.ema.access.ElementList decodedEl = JUnitTestConnect.createElementList();
        JUnitTestConnect.setRsslData(decodedEl, decStatusMsg.attrib().elementList(), Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

        Iterator<ElementEntry> iterator = decodedEl.iterator();
        
        ElementEntry elementEntry;
        String elementName = null;
        
        while(iterator.hasNext())
        {
            elementEntry = iterator.next();
            elementName = elementEntry.name();
            
            switch(elementName)
            {
            case EmaRdm.ENAME_AUTHN_ERRORCODE:
            {
                assertEquals(3, elementEntry.uintValue());
            }
                break;       
            case EmaRdm.ENAME_AUTHN_ERRORTEXT:
            {
                assertEquals("authenticationErrorText", elementEntry.ascii().ascii());
            }
                break;                            
			default:
				break;
            }
        }
    }
    
    @Test
    public void decodeLoginStatusTest()
    {
        ElementList encodedElementList = EmaFactory.createElementList();

        encodedElementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_AUTHN_ERRORCODE,  3));
        encodedElementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_AUTHN_ERRORTEXT,  "authenticationErrorText"));

        com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory
                .createDataDictionary();
        TestUtilities.eta_encodeDictionaryMsg(dictionary);

        StatusMsg encStatusMsg = EmaFactory.createStatusMsg();

        encStatusMsg.domainType(EmaRdm.MMT_LOGIN);
        encStatusMsg.attrib(encodedElementList);   
        encStatusMsg.state(StreamState.OPEN, DataState.OK, StatusCode.NONE, "decodeLoginStatusTest");
        
        StatusMsg decStatusMsg = JUnitTestConnect.createStatusMsg();

        JUnitTestConnect.setRsslData(decStatusMsg, encStatusMsg, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

        LoginStatus loginStatus = EmaFactory.Domain.createLoginStatus();

        loginStatus.message(decStatusMsg);
        
        assertEquals(true, loginStatus.hasState());
        
        assertEquals(StreamState.OPEN, loginStatus.state().streamState());
        assertEquals(DataState.OK, loginStatus.state().dataState());
        assertEquals(StatusCode.NONE, loginStatus.state().statusCode());
        assertEquals("decodeLoginStatusTest", loginStatus.state().statusText());

        assertEquals(true, loginStatus.hasAuthenticationErrorCode());
        
        assertEquals(3, loginStatus.authenticationErrorCode());
        
        assertEquals(true, loginStatus.hasAuthenticationErrorText());
        
        assertEquals("authenticationErrorText", loginStatus.authenticationErrorText());
    }
    
    @Test
    public void blankLoginStatusTest()
    {
        ElementList encodedElementList = EmaFactory.createElementList();

        encodedElementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_AUTHN_ERRORTEXT,  ""));

        com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory
                .createDataDictionary();
        TestUtilities.eta_encodeDictionaryMsg(dictionary);

        StatusMsg encStatusMsg = EmaFactory.createStatusMsg();

        encStatusMsg.domainType(EmaRdm.MMT_LOGIN);
        encStatusMsg.attrib(encodedElementList);   
        encStatusMsg.state(StreamState.OPEN, DataState.OK, StatusCode.NONE, "decodeLoginStatusTest");
        
        StatusMsg decStatusMsg = JUnitTestConnect.createStatusMsg();

        JUnitTestConnect.setRsslData(decStatusMsg, encStatusMsg, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

        LoginStatus loginStatus = EmaFactory.Domain.createLoginStatus();
        loginStatus.authenticationErrorText("test");

        loginStatus.message(decStatusMsg);
        
        assertEquals(true, loginStatus.hasState());
        
        assertEquals(StreamState.OPEN, loginStatus.state().streamState());
        assertEquals(DataState.OK, loginStatus.state().dataState());
        assertEquals(StatusCode.NONE, loginStatus.state().statusCode());
        assertEquals("decodeLoginStatusTest", loginStatus.state().statusText());
        
        assertEquals(false, loginStatus.hasAuthenticationErrorText());        
    }
    
    
    @Test
    public void headerLoginStatusTest()
    {
        LoginStatus loginStatus = EmaFactory.Domain.createLoginStatus();
        
        loginStatus.state(StreamState.OPEN, DataState.OK, StatusCode.NONE, "headerLoginStatusTest");
        loginStatus.name("UserName");
        loginStatus.nameType(EmaRdm.USER_NAME);
        
        assertEquals(true, loginStatus.hasState());
        
        assertEquals(StreamState.OPEN, loginStatus.state().streamState());
        assertEquals(DataState.OK, loginStatus.state().dataState());
        assertEquals(StatusCode.NONE, loginStatus.state().statusCode());
        assertEquals("headerLoginStatusTest", loginStatus.state().statusText());
        
        assertEquals(true, loginStatus.hasName());
        
        assertEquals("UserName", loginStatus.name());
        
        assertEquals(true, loginStatus.hasNameType());
        
        assertEquals(EmaRdm.USER_NAME, loginStatus.nameType());
        
    }
    
    @Test
    public void clearLoginStatusTest()
    {
        LoginStatus loginStatus = EmaFactory.Domain.createLoginStatus();
        
        loginStatus.authenticationErrorCode(10);
        loginStatus.authenticationErrorText("ErrorText");
        loginStatus.state(StreamState.OPEN, DataState.OK, StatusCode.NONE, "StatusText");
        loginStatus.name("Name");
        loginStatus.nameType(EmaRdm.USER_EMAIL_ADDRESS);

        assertEquals(true, loginStatus.hasAuthenticationErrorCode());
        assertEquals(10, loginStatus.authenticationErrorCode());
        assertEquals(true, loginStatus.hasAuthenticationErrorText());
        assertEquals("ErrorText", loginStatus.authenticationErrorText());
        assertEquals(true, loginStatus.hasState());
        assertEquals(StreamState.OPEN, loginStatus.state().streamState());
        assertEquals(DataState.OK, loginStatus.state().dataState());
        assertEquals(StatusCode.NONE, loginStatus.state().statusCode());
        assertEquals("StatusText", loginStatus.state().statusText());
        assertEquals(true, loginStatus.hasName());
        assertEquals("Name", loginStatus.name());
        assertEquals(true, loginStatus.hasNameType());
        assertEquals(EmaRdm.USER_EMAIL_ADDRESS, loginStatus.nameType());
        
        loginStatus.clear();

        assertEquals(false, loginStatus.hasAuthenticationErrorCode());
        assertEquals(false, loginStatus.hasAuthenticationErrorText());
        assertEquals(false, loginStatus.hasState());
        assertEquals(false, loginStatus.hasName());
        assertEquals(true, loginStatus.hasNameType());
        assertEquals(EmaRdm.USER_NAME, loginStatus.nameType());
    }
    
    @Test
    public void errorHandlingLoginStatusTest()
    {
        LoginStatus loginStatus = EmaFactory.Domain.createLoginStatus();

        exception.expect(OmmInvalidUsageException.class);
        exception.expectMessage("AuthenticationErrorCode element is not set");
        loginStatus.authenticationErrorCode();
        
        exception.expect(OmmInvalidUsageException.class);
        exception.expectMessage("AuthenticationErrorText element is not set");
        loginStatus.authenticationErrorText();
    }
}
