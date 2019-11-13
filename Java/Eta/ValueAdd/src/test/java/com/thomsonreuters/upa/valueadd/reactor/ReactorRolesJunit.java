///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.valueadd.reactor;

import static org.junit.Assert.*;

import org.junit.Test;

import com.thomsonreuters.upa.rdm.Dictionary;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;

public class ReactorRolesJunit
{
    class ConsumerCallbackHandler implements ConsumerCallback, ReactorAuthTokenEventCallback
    {
    	int _reactorChannelEventCallbackCount;
    	int _defaultMsgCallbackCount;
    	int _rdmLoginMsgCallbackCount;
    	int _rdmDirectoryMsgCallbackCount;
    	int _rdmDictionaryMsgCallbackCount;
    	int _authTokenEventCallbackCount;
    	    	
		@Override
		public int reactorChannelEventCallback(ReactorChannelEvent event)
		{
			_reactorChannelEventCallbackCount++;
			return ReactorCallbackReturnCodes.SUCCESS;
		}

		@Override
		public int defaultMsgCallback(ReactorMsgEvent event)
		{
			_defaultMsgCallbackCount++;
			return ReactorCallbackReturnCodes.SUCCESS;
		}

		@Override
		public int rdmLoginMsgCallback(RDMLoginMsgEvent event)
		{
			_rdmLoginMsgCallbackCount++;
			return ReactorCallbackReturnCodes.SUCCESS;
		}

		@Override
		public int rdmDirectoryMsgCallback(RDMDirectoryMsgEvent event)
		{
			_rdmDirectoryMsgCallbackCount++;
			return ReactorCallbackReturnCodes.SUCCESS;
		}

		@Override
		public int rdmDictionaryMsgCallback(RDMDictionaryMsgEvent event)
		{
			_rdmDictionaryMsgCallbackCount++;
			return ReactorCallbackReturnCodes.SUCCESS;
		}
		
		@Override
		public int reactorAuthTokenEventCallback(ReactorAuthTokenEvent event)
		{
			_authTokenEventCallbackCount++;
			return ReactorCallbackReturnCodes.SUCCESS;
		}		
    }

    class ProviderCallbackHandler implements ProviderCallback
    {
    	int _reactorChannelEventCallbackCount;
    	int _defaultMsgCallbackCount;
    	int _rdmLoginMsgCallbackCount;
    	int _rdmDirectoryMsgCallbackCount;
    	int _rdmDictionaryMsgCallbackCount;

    	@Override
		public int reactorChannelEventCallback(ReactorChannelEvent event)
    	{
    		_reactorChannelEventCallbackCount++;
			return ReactorCallbackReturnCodes.SUCCESS;
		}

		@Override
		public int defaultMsgCallback(ReactorMsgEvent event)
		{
			_defaultMsgCallbackCount++;
			return ReactorCallbackReturnCodes.SUCCESS;
		}

		@Override
		public int rdmLoginMsgCallback(RDMLoginMsgEvent event)
		{
			_rdmLoginMsgCallbackCount++;
			return ReactorCallbackReturnCodes.SUCCESS;
		}

		@Override
		public int rdmDirectoryMsgCallback(RDMDirectoryMsgEvent event)
		{
			_rdmDirectoryMsgCallbackCount++;
			return ReactorCallbackReturnCodes.SUCCESS;
		}

		@Override
		public int rdmDictionaryMsgCallback(RDMDictionaryMsgEvent event)
		{
			_rdmDictionaryMsgCallbackCount++;
			return ReactorCallbackReturnCodes.SUCCESS;
		}    	
    }
    
    class NIProviderCallbackHandler implements NIProviderCallback, ReactorAuthTokenEventCallback
    {
    	int _reactorChannelEventCallbackCount;
    	int _defaultMsgCallbackCount;
    	int _rdmLoginMsgCallbackCount;
    	int _authTokenEventCallbackCount;

    	@Override
		public int reactorChannelEventCallback(ReactorChannelEvent event) {
    		_reactorChannelEventCallbackCount++;
			return ReactorCallbackReturnCodes.SUCCESS;
		}

		@Override
		public int defaultMsgCallback(ReactorMsgEvent event) {
			_defaultMsgCallbackCount++;
		    return ReactorCallbackReturnCodes.SUCCESS;
		}

		@Override
		public int rdmLoginMsgCallback(RDMLoginMsgEvent event) {
			_rdmLoginMsgCallbackCount++;
			return ReactorCallbackReturnCodes.SUCCESS;
		}
		
	    @Override
	    public int reactorAuthTokenEventCallback(ReactorAuthTokenEvent event)
	    {   _authTokenEventCallbackCount++;
	        return ReactorCallbackReturnCodes.SUCCESS;
	    }		
    }
    
    @Test
    public void createReactorRolesDefaultTest()
    {
        ReactorRole reactorRole = new ReactorRole();
        assertEquals(0, reactorRole.type());

        ConsumerRole consumerRole = ReactorFactory.createConsumerRole();
        ProviderRole providerRole = ReactorFactory.createProviderRole();
        NIProviderRole niProviderRole = ReactorFactory.createNIProviderRole();

        assertNotNull(consumerRole);
        assertNotNull(providerRole);
        assertNotNull(niProviderRole);

        assertEquals(ReactorRoleTypes.CONSUMER, consumerRole.type());
        assertEquals(ReactorRoleTypes.PROVIDER, providerRole.type());
        assertEquals(ReactorRoleTypes.NIPROVIDER, niProviderRole.type());

        assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
        consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
        assertEquals(DictionaryDownloadModes.FIRST_AVAILABLE, consumerRole.dictionaryDownloadMode());
        consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
        assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());

        consumerRole.initDefaultRDMLoginRequest();
        assertNotNull(consumerRole.rdmLoginRequest());
        verifyLoginRequest(consumerRole.rdmLoginRequest(), Login.RoleTypes.CONS, ConsumerRole.LOGIN_STREAM_ID);

        consumerRole.initDefaultRDMDirectoryRequest();
        assertNotNull(consumerRole.rdmDirectoryRequest());
        verifyDirectoryRequest(consumerRole.rdmDirectoryRequest(), ConsumerRole.DIRECTORY_STREAM_ID);
 
        consumerRole.initDefaultRDMFieldDictionaryRequest();
        assertNotNull(consumerRole.rdmFieldDictionaryRequest());
        verifyDictionaryRequest(consumerRole.rdmFieldDictionaryRequest(), ConsumerRole.FIELD_DICTIONARY_STREAM_ID, "RWFFld");

        consumerRole.initDefaultRDMEnumDictionaryRequest();
        assertNotNull(consumerRole.rdmEnumDictionaryRequest());
        verifyDictionaryRequest(consumerRole.rdmEnumDictionaryRequest(), ConsumerRole.ENUM_DICTIONARY_STREAM_ID, "RWFEnum");

        niProviderRole.initDefaultRDMLoginRequest();
        assertNotNull(niProviderRole.rdmLoginRequest());
        verifyLoginRequest(niProviderRole.rdmLoginRequest(), Login.RoleTypes.PROV, NIProviderRole.LOGIN_STREAM_ID);
        
        niProviderRole.initDefaultRDMDirectoryRefresh("DIRECT_FEED", 1);
        assertNotNull(niProviderRole.rdmDirectoryRefresh());
        verifyDirectoryRefresh(niProviderRole.rdmDirectoryRefresh(), NIProviderRole.DIRECTORY_STREAM_ID, "DIRECT_FEED");
        
        // test callbacks in ConsumerRole, ProviderRole and NIProviderRole
        ConsumerCallbackHandler consumerCallbackHandler = new ConsumerCallbackHandler();
        ProviderCallbackHandler providerCallbackHandler = new ProviderCallbackHandler();
        NIProviderCallbackHandler niproviderCallbackHandler = new NIProviderCallbackHandler();
        ReactorChannelEvent reactorChannelEvent =  ReactorFactory.createReactorChannelEvent();
        ReactorMsgEvent reactorMsgEvent = ReactorFactory.createReactorMsgEvent();
        RDMLoginMsgEvent rdmLoginMsgEvent = ReactorFactory.createRDMLoginMsgEvent();
        RDMDirectoryMsgEvent rdmDirectoryMsgEvent = ReactorFactory.createRDMDirectoryMsgEvent();
        RDMDictionaryMsgEvent rdmDictionaryMsgEvent = ReactorFactory.createRDMDictionaryMsgEvent();
                        
        // ConsumerRole callback test
        consumerRole.channelEventCallback(consumerCallbackHandler);
        assertEquals(consumerCallbackHandler, consumerRole.channelEventCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, consumerRole.channelEventCallback().reactorChannelEventCallback(reactorChannelEvent));
        assertEquals(1, consumerCallbackHandler._reactorChannelEventCallbackCount);
        
        consumerRole.defaultMsgCallback(consumerCallbackHandler);
        assertEquals(consumerCallbackHandler, consumerRole.defaultMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, consumerRole.defaultMsgCallback().defaultMsgCallback(reactorMsgEvent));
        assertEquals(1, consumerCallbackHandler._defaultMsgCallbackCount);
        
        consumerRole.loginMsgCallback(consumerCallbackHandler);
        assertEquals(consumerCallbackHandler, consumerRole.loginMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, consumerRole.loginMsgCallback().rdmLoginMsgCallback(rdmLoginMsgEvent));
        assertEquals(1, consumerCallbackHandler._rdmLoginMsgCallbackCount);
        
        consumerRole.directoryMsgCallback(consumerCallbackHandler);
        assertEquals(consumerCallbackHandler, consumerRole.directoryMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, consumerRole.directoryMsgCallback().rdmDirectoryMsgCallback(rdmDirectoryMsgEvent));
        assertEquals(1, consumerCallbackHandler._rdmDirectoryMsgCallbackCount);
        
        consumerRole.dictionaryMsgCallback(consumerCallbackHandler);
        assertEquals(consumerCallbackHandler, consumerRole.dictionaryMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, consumerRole.dictionaryMsgCallback().rdmDictionaryMsgCallback(rdmDictionaryMsgEvent));
        assertEquals(1, consumerCallbackHandler._rdmDictionaryMsgCallbackCount);
        
        consumerRole.watchlistOptions().enableWatchlist(true);
        consumerRole.watchlistOptions().itemCountHint(4);
        consumerRole.watchlistOptions().maxOutstandingPosts(5);
        consumerRole.watchlistOptions().obeyOpenWindow(true);
        consumerRole.watchlistOptions().channelOpenCallback(consumerCallbackHandler);
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, consumerRole.watchlistOptions().channelOpenCallback().reactorChannelEventCallback(reactorChannelEvent));
        assertEquals(2, consumerCallbackHandler._reactorChannelEventCallbackCount);     
        assertTrue(consumerRole.watchlistOptions().enableWatchlist());
        assertEquals(4, consumerRole.watchlistOptions().itemCountHint());
        assertEquals(5, consumerRole.watchlistOptions().maxOutstandingPosts());
        assertTrue(consumerRole.watchlistOptions().obeyOpenWindow());
        
        reactorMsgEvent.streamInfo().serviceName("DIRECT_FEED");
        reactorMsgEvent.streamInfo().userSpecObject(new String(" JUNIT TEST "));
                
        // ProviderRole callback test
        providerRole.channelEventCallback(providerCallbackHandler);
        assertEquals(providerCallbackHandler, providerRole.channelEventCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, providerRole.channelEventCallback().reactorChannelEventCallback(reactorChannelEvent));
        assertEquals(1, providerCallbackHandler._reactorChannelEventCallbackCount);
        
        providerRole.defaultMsgCallback(providerCallbackHandler);
        assertEquals(providerCallbackHandler, providerRole.defaultMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, providerRole.defaultMsgCallback().defaultMsgCallback(reactorMsgEvent));
        assertEquals(1, providerCallbackHandler._defaultMsgCallbackCount);
        
        providerRole.loginMsgCallback(providerCallbackHandler);
        assertEquals(providerCallbackHandler, providerRole.loginMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, providerRole.loginMsgCallback().rdmLoginMsgCallback(rdmLoginMsgEvent));
        assertEquals(1, providerCallbackHandler._rdmLoginMsgCallbackCount);
        
        providerRole.directoryMsgCallback(providerCallbackHandler);
        assertEquals(providerCallbackHandler, providerRole.directoryMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, providerRole.directoryMsgCallback().rdmDirectoryMsgCallback(rdmDirectoryMsgEvent));
        assertEquals(1, providerCallbackHandler._rdmDirectoryMsgCallbackCount);
        
        providerRole.dictionaryMsgCallback(providerCallbackHandler);
        assertEquals(providerCallbackHandler, providerRole.dictionaryMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, providerRole.dictionaryMsgCallback().rdmDictionaryMsgCallback(rdmDictionaryMsgEvent));
        assertEquals(1, providerCallbackHandler._rdmDictionaryMsgCallbackCount);
        
        // NIProviderRole callback test
        niProviderRole.channelEventCallback(niproviderCallbackHandler);
        assertEquals(niproviderCallbackHandler, niProviderRole.channelEventCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, niProviderRole.channelEventCallback().reactorChannelEventCallback(reactorChannelEvent));
        assertEquals(1, niproviderCallbackHandler._reactorChannelEventCallbackCount);
        
        niProviderRole.defaultMsgCallback(niproviderCallbackHandler);
        assertEquals(niproviderCallbackHandler, niProviderRole.defaultMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, niProviderRole.defaultMsgCallback().defaultMsgCallback(reactorMsgEvent));
        assertEquals(1, niproviderCallbackHandler._defaultMsgCallbackCount);
        
        niProviderRole.loginMsgCallback(niproviderCallbackHandler);
        assertEquals(niproviderCallbackHandler, niProviderRole.loginMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, niProviderRole.loginMsgCallback().rdmLoginMsgCallback(rdmLoginMsgEvent));
        assertEquals(1, niproviderCallbackHandler._rdmLoginMsgCallbackCount);
    }

	@Test
    public void createReactorRolesUserProvidedTest()
    {
        ReactorRole reactorRole = new ReactorRole();
        assertEquals(0, reactorRole.type());

        ConsumerRole consumerRole = ReactorFactory.createConsumerRole();
        ProviderRole providerRole = ReactorFactory.createProviderRole();
        NIProviderRole niProviderRole = ReactorFactory.createNIProviderRole();

        assertNotNull(consumerRole);
        assertNotNull(providerRole);
        assertNotNull(niProviderRole);

        assertEquals(ReactorRoleTypes.CONSUMER, consumerRole.type());
        assertEquals(ReactorRoleTypes.PROVIDER, providerRole.type());
        assertEquals(ReactorRoleTypes.NIPROVIDER, niProviderRole.type());

        assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
        consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
        assertEquals(DictionaryDownloadModes.FIRST_AVAILABLE, consumerRole.dictionaryDownloadMode());
        consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
        assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());

        // verify user provided login request and stream id
        LoginRequest loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
        loginRequest.rdmMsgType(LoginMsgType.REQUEST);
        loginRequest.initDefaultRequest(11);                
        consumerRole.rdmLoginRequest(loginRequest);
        consumerRole.initDefaultRDMLoginRequest();
        assertNotNull(consumerRole.rdmLoginRequest());
        verifyLoginRequest(consumerRole.rdmLoginRequest(), Login.RoleTypes.CONS, 11);
        // verify user provided stream id equals 0 is overridden to default value
        loginRequest.initDefaultRequest(0);
        consumerRole.rdmLoginRequest(loginRequest);
        consumerRole.initDefaultRDMLoginRequest();
        assertNotNull(consumerRole.rdmLoginRequest());
        verifyLoginRequest(consumerRole.rdmLoginRequest(), Login.RoleTypes.CONS, ConsumerRole.LOGIN_STREAM_ID);

        // verify user provided directory request and stream id
        DirectoryRequest directoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
        directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
        directoryRequest.streamId(22);
        consumerRole.rdmDirectoryRequest(directoryRequest);
        consumerRole.initDefaultRDMDirectoryRequest();
        assertNotNull(consumerRole.rdmDirectoryRequest());
        verifyDirectoryRequest(consumerRole.rdmDirectoryRequest(), 22);
        // verify user provided stream id equals 0 is overridden to default value
        directoryRequest.streamId(0);
        consumerRole.rdmDirectoryRequest(directoryRequest);
        consumerRole.initDefaultRDMDirectoryRequest();
        assertNotNull(consumerRole.rdmDirectoryRequest());
        verifyDirectoryRequest(consumerRole.rdmDirectoryRequest(), ConsumerRole.DIRECTORY_STREAM_ID);
 
        // verify user provided field dictionary request and stream id
        DictionaryRequest dictionaryRequest = (DictionaryRequest)DictionaryMsgFactory.createMsg();
        dictionaryRequest.rdmMsgType(DictionaryMsgType.REQUEST);
        dictionaryRequest.streamId(33);
        consumerRole.rdmFieldDictionaryRequest(dictionaryRequest);
        consumerRole.initDefaultRDMFieldDictionaryRequest();
        assertNotNull(consumerRole.rdmFieldDictionaryRequest());
        verifyDictionaryRequest(consumerRole.rdmFieldDictionaryRequest(), 33, "RWFFld");
        // verify user provided stream id equals 0 is overridden to default value
        dictionaryRequest.streamId(0);
        consumerRole.rdmFieldDictionaryRequest(dictionaryRequest);
        consumerRole.initDefaultRDMFieldDictionaryRequest();
        assertNotNull(consumerRole.rdmFieldDictionaryRequest());
        verifyDictionaryRequest(consumerRole.rdmFieldDictionaryRequest(), ConsumerRole.FIELD_DICTIONARY_STREAM_ID, "RWFFld");
        // verify user provided stream id that collides with directory id is overridden to one plus directory id
        dictionaryRequest.streamId(ConsumerRole.DIRECTORY_STREAM_ID);
        consumerRole.rdmFieldDictionaryRequest(dictionaryRequest);
        consumerRole.initDefaultRDMFieldDictionaryRequest();
        assertNotNull(consumerRole.rdmFieldDictionaryRequest());
        verifyDictionaryRequest(consumerRole.rdmFieldDictionaryRequest(), ConsumerRole.DIRECTORY_STREAM_ID + 1, "RWFFld");

        // verify user provided enum type dictionary request and stream id
        dictionaryRequest = (DictionaryRequest)DictionaryMsgFactory.createMsg();
        dictionaryRequest.rdmMsgType(DictionaryMsgType.REQUEST);
        dictionaryRequest.streamId(44);
        consumerRole.rdmEnumDictionaryRequest(dictionaryRequest);
        consumerRole.initDefaultRDMEnumDictionaryRequest();
        assertNotNull(consumerRole.rdmEnumDictionaryRequest());
        verifyDictionaryRequest(consumerRole.rdmEnumDictionaryRequest(), 44, "RWFEnum");
        // verify user provided stream id equals 0 is overridden to default value
        dictionaryRequest.streamId(0);
        consumerRole.rdmEnumDictionaryRequest(dictionaryRequest);
        consumerRole.initDefaultRDMEnumDictionaryRequest();
        assertNotNull(consumerRole.rdmEnumDictionaryRequest());
        verifyDictionaryRequest(consumerRole.rdmEnumDictionaryRequest(), ConsumerRole.ENUM_DICTIONARY_STREAM_ID, "RWFEnum");
        // verify user provided stream id that collides with field dictionary id is overridden to one plus field dictionary id
        dictionaryRequest.streamId(ConsumerRole.FIELD_DICTIONARY_STREAM_ID);
        consumerRole.rdmEnumDictionaryRequest(dictionaryRequest);
        consumerRole.initDefaultRDMEnumDictionaryRequest();
        assertNotNull(consumerRole.rdmEnumDictionaryRequest());
        verifyDictionaryRequest(consumerRole.rdmEnumDictionaryRequest(), ConsumerRole.FIELD_DICTIONARY_STREAM_ID + 1, "RWFEnum");

        // verify user provided login request and stream id
        loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
        loginRequest.rdmMsgType(LoginMsgType.REQUEST);
        loginRequest.initDefaultRequest(55);
        niProviderRole.rdmLoginRequest(loginRequest);
        niProviderRole.initDefaultRDMLoginRequest();
        assertNotNull(niProviderRole.rdmLoginRequest());
        verifyLoginRequest(niProviderRole.rdmLoginRequest(), Login.RoleTypes.PROV, 55);
        // verify user provided stream id equals 0 is overridden to default value
        loginRequest.initDefaultRequest(0);
        niProviderRole.rdmLoginRequest(loginRequest);
        niProviderRole.initDefaultRDMLoginRequest();
        assertNotNull(niProviderRole.rdmLoginRequest());
        verifyLoginRequest(niProviderRole.rdmLoginRequest(), Login.RoleTypes.PROV, NIProviderRole.LOGIN_STREAM_ID);
        
        // verify user provided directory refresh and stream id
        DirectoryRefresh directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
        directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);
        directoryRefresh.streamId(66);
        niProviderRole.rdmDirectoryRefresh(directoryRefresh);
        niProviderRole.initDefaultRDMDirectoryRefresh("DIRECT_FEED", 1);
        assertNotNull(niProviderRole.rdmDirectoryRefresh());
        verifyDirectoryRefresh(niProviderRole.rdmDirectoryRefresh(), 66, "DIRECT_FEED");
        // verify user provided stream id equals 0 is overridden to default value
        directoryRefresh.streamId(0);
        niProviderRole.rdmDirectoryRefresh(directoryRefresh);
        niProviderRole.initDefaultRDMDirectoryRefresh("DIRECT_FEED", 1);
        assertNotNull(niProviderRole.rdmDirectoryRefresh());
        verifyDirectoryRefresh(niProviderRole.rdmDirectoryRefresh(), NIProviderRole.DIRECTORY_STREAM_ID, "DIRECT_FEED");
        
        // test callbacks in ConsumerRole, ProviderRole and NIProviderRole
        ConsumerCallbackHandler consumerCallbackHandler = new ConsumerCallbackHandler();
        ProviderCallbackHandler providerCallbackHandler = new ProviderCallbackHandler();
        NIProviderCallbackHandler niproviderCallbackHandler = new NIProviderCallbackHandler();
        ReactorChannelEvent reactorChannelEvent =  ReactorFactory.createReactorChannelEvent();
        ReactorMsgEvent reactorMsgEvent = ReactorFactory.createReactorMsgEvent();
        RDMLoginMsgEvent rdmLoginMsgEvent = ReactorFactory.createRDMLoginMsgEvent();
        RDMDirectoryMsgEvent rdmDirectoryMsgEvent = ReactorFactory.createRDMDirectoryMsgEvent();
        RDMDictionaryMsgEvent rdmDictionaryMsgEvent = ReactorFactory.createRDMDictionaryMsgEvent();
        
        // ConsumerRole callback test
        consumerRole.channelEventCallback(consumerCallbackHandler);
        assertEquals(consumerCallbackHandler, consumerRole.channelEventCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, consumerRole.channelEventCallback().reactorChannelEventCallback(reactorChannelEvent));
        assertEquals(1, consumerCallbackHandler._reactorChannelEventCallbackCount);
        
        consumerRole.defaultMsgCallback(consumerCallbackHandler);
        assertEquals(consumerCallbackHandler, consumerRole.defaultMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, consumerRole.defaultMsgCallback().defaultMsgCallback(reactorMsgEvent));
        assertEquals(1, consumerCallbackHandler._defaultMsgCallbackCount);
        
        consumerRole.loginMsgCallback(consumerCallbackHandler);
        assertEquals(consumerCallbackHandler, consumerRole.loginMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, consumerRole.loginMsgCallback().rdmLoginMsgCallback(rdmLoginMsgEvent));
        assertEquals(1, consumerCallbackHandler._rdmLoginMsgCallbackCount);
        
        consumerRole.directoryMsgCallback(consumerCallbackHandler);
        assertEquals(consumerCallbackHandler, consumerRole.directoryMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, consumerRole.directoryMsgCallback().rdmDirectoryMsgCallback(rdmDirectoryMsgEvent));
        assertEquals(1, consumerCallbackHandler._rdmDirectoryMsgCallbackCount);
        
        consumerRole.dictionaryMsgCallback(consumerCallbackHandler);
        assertEquals(consumerCallbackHandler, consumerRole.dictionaryMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, consumerRole.dictionaryMsgCallback().rdmDictionaryMsgCallback(rdmDictionaryMsgEvent));
        assertEquals(1, consumerCallbackHandler._rdmDictionaryMsgCallbackCount);
        
        // ProviderRole callback test
        providerRole.channelEventCallback(providerCallbackHandler);
        assertEquals(providerCallbackHandler, providerRole.channelEventCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, providerRole.channelEventCallback().reactorChannelEventCallback(reactorChannelEvent));
        assertEquals(1, providerCallbackHandler._reactorChannelEventCallbackCount);
        
        providerRole.defaultMsgCallback(providerCallbackHandler);
        assertEquals(providerCallbackHandler, providerRole.defaultMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, providerRole.defaultMsgCallback().defaultMsgCallback(reactorMsgEvent));
        assertEquals(1, providerCallbackHandler._defaultMsgCallbackCount);
        
        providerRole.loginMsgCallback(providerCallbackHandler);
        assertEquals(providerCallbackHandler, providerRole.loginMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, providerRole.loginMsgCallback().rdmLoginMsgCallback(rdmLoginMsgEvent));
        assertEquals(1, providerCallbackHandler._rdmLoginMsgCallbackCount);
        
        providerRole.directoryMsgCallback(providerCallbackHandler);
        assertEquals(providerCallbackHandler, providerRole.directoryMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, providerRole.directoryMsgCallback().rdmDirectoryMsgCallback(rdmDirectoryMsgEvent));
        assertEquals(1, providerCallbackHandler._rdmDirectoryMsgCallbackCount);
        
        providerRole.dictionaryMsgCallback(providerCallbackHandler);
        assertEquals(providerCallbackHandler, providerRole.dictionaryMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, providerRole.dictionaryMsgCallback().rdmDictionaryMsgCallback(rdmDictionaryMsgEvent));
        assertEquals(1, providerCallbackHandler._rdmDictionaryMsgCallbackCount);
        
        // NIProviderRole callback test
        niProviderRole.channelEventCallback(niproviderCallbackHandler);
        assertEquals(niproviderCallbackHandler, niProviderRole.channelEventCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, niProviderRole.channelEventCallback().reactorChannelEventCallback(reactorChannelEvent));
        assertEquals(1, niproviderCallbackHandler._reactorChannelEventCallbackCount);
        
        niProviderRole.defaultMsgCallback(niproviderCallbackHandler);
        assertEquals(niproviderCallbackHandler, niProviderRole.defaultMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, niProviderRole.defaultMsgCallback().defaultMsgCallback(reactorMsgEvent));
        assertEquals(1, niproviderCallbackHandler._defaultMsgCallbackCount);
        
        niProviderRole.loginMsgCallback(niproviderCallbackHandler);
        assertEquals(niproviderCallbackHandler, niProviderRole.loginMsgCallback());
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, niProviderRole.loginMsgCallback().rdmLoginMsgCallback(rdmLoginMsgEvent));
        assertEquals(1, niproviderCallbackHandler._rdmLoginMsgCallbackCount);
    }
    
    private void verifyLoginRequest(LoginRequest loginRequest, int role, int streamId)
    {
        assertEquals(LoginMsgType.REQUEST, loginRequest.rdmMsgType());
        assertEquals(true, loginRequest.checkHasRole());
        assertEquals(role, loginRequest.role());
        assertEquals(streamId, loginRequest.streamId());
    }

    private void verifyDirectoryRequest(DirectoryRequest directoryRequest, int streamId)
    {
        assertEquals(DirectoryMsgType.REQUEST, directoryRequest.rdmMsgType());
        assertEquals(true, directoryRequest.checkStreaming());
        assertEquals(ConsumerRole.FILTER_TO_REQUEST, directoryRequest.filter());
        assertEquals(streamId, directoryRequest.streamId());
    }
    
    private void verifyDictionaryRequest(DictionaryRequest dictionaryRequest, int streamId, String name)
    {
        assertEquals(DictionaryMsgType.REQUEST, dictionaryRequest.rdmMsgType());
        assertEquals(true, dictionaryRequest.checkStreaming());
        assertEquals(Dictionary.VerbosityValues.NORMAL, dictionaryRequest.verbosity());
        assertEquals(streamId, dictionaryRequest.streamId());
        assertTrue(dictionaryRequest.dictionaryName().toString().equals(name));		
	}
    
	private void verifyDirectoryRefresh(DirectoryRefresh directoryRefresh, int streamId, String serviceName)
	{
        assertEquals(DirectoryMsgType.REFRESH, directoryRefresh.rdmMsgType());
        assertEquals(true, directoryRefresh.checkClearCache());
        assertEquals(false, directoryRefresh.checkSolicited());
        assertEquals(NIProviderRole.FILTER_TO_REFRESH, directoryRefresh.filter());
        assertEquals(streamId, directoryRefresh.streamId());
        assertEquals(1, directoryRefresh.serviceList().size());
        assertTrue(directoryRefresh.serviceList().get(0).info().serviceName().toString().equals(serviceName));
	}
}
