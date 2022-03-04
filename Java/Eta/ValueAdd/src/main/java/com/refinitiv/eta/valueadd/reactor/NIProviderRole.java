/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.FilterEntryActions;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.rdm.Dictionary;
import com.refinitiv.eta.rdm.Directory;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryClose;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryClose;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginClose;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;

/**
 * Class representing the role of an OMM Non-Interactive Provider.
 * 
 * @see ReactorRole
 * @see ReactorRoleTypes
 */
public class NIProviderRole extends ReactorRole
{
    static final int LOGIN_STREAM_ID = 1;
    static final int DIRECTORY_STREAM_ID = -1;
    static final int FIELD_DICTIONARY_STREAM_ID = -2;
    static final int ENUM_DICTIONARY_STREAM_ID = -3;
    static final int OPEN_LIMIT = 5;
    static final String VENDOR = "Refinitiv";
    static final String LINK_NAME = "NI_PUB";
    static final String FIELD_DICTIONARY_NAME = "RWFFld";
    static final String ENUM_TYPE_DICTIONARY_NAME = "RWFEnum";

    static final long FILTER_TO_REFRESH = Directory.ServiceFilterFlags.INFO
            | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.LOAD
            | Directory.ServiceFilterFlags.LINK;

    LoginRequest _loginRequest = null;
    LoginClose _loginClose = null;
    DirectoryRefresh _directoryRefresh = null;
    DirectoryClose _directoryClose = null;
    RDMLoginMsgCallback _loginMsgCallback = null;
	Buffer _stateText = CodecFactory.createBuffer();

	/**
     * Dictionary download parameters
     */
    RDMDictionaryMsgCallback _dictionaryMsgCallback = null;
    DictionaryRequest _fieldDictionaryRequest = null;
    DictionaryClose _fieldDictionaryClose = null;
    DictionaryRequest _enumDictionaryRequest = null;
    DictionaryClose _enumDictionaryClose = null;
    int _dictionaryDownloadMode = DictionaryDownloadModes.NONE;
    final Buffer _fieldDictionaryName = CodecFactory.createBuffer();
    final Buffer _enumTypeDictionaryName = CodecFactory.createBuffer();
    boolean _receivedFieldDictionaryResp = false;
    boolean _receivedEnumDictionaryResp = false;

    private Service _service = DirectoryMsgFactory.createService();
    
    /**
     * Instantiates a new NI provider role.
     */
    public NIProviderRole()
    {
        _type = ReactorRoleTypes.NIPROVIDER;
        _stateText.data("Source Directory Refresh Completed");
        _fieldDictionaryName.data("RWFFld");
        _enumTypeDictionaryName.data("RWFEnum");
    }
    
    /**
     * The {@link LoginRequest} to be sent during the connection establishment
     * process. This can be populated with a user's specific information or
     * invoke {@link #initDefaultRDMLoginRequest()} to populate with default
     * information. If this parameter is left empty no login will be sent to
     * the system; useful for systems that do not require a login.
     *
     * @param loginRequest the login request
     */
    public void rdmLoginRequest(LoginRequest loginRequest)
    {
        copyLoginRequest(loginRequest);
    }
    
    /**
     * The {@link LoginRequest} to be sent during the connection establishment
     * process. This can be populated with a user's specific information or
     * invoke {@link #initDefaultRDMLoginRequest()} to populate with default
     * information. If this parameter is left empty no login will be sent to
     * the system; useful for systems that do not require a login.
     * 
     * @return the rdmLoginRequest
     */
    public LoginRequest rdmLoginRequest()
    {
        return _loginRequest;
    }
    
    /**
     * Initializes the RDM LoginRequest with default information. If the
     * rdmLoginRequest has already been defined (due to a previous call to
     * {@link #rdmLoginRequest(LoginRequest)}) the rdmLoginRequest object will
     * be reused.
     */
    public void initDefaultRDMLoginRequest()
    {
    	String userName = "";
    	int streamId;
    	
        if (_loginRequest == null)
        {
        	streamId = LOGIN_STREAM_ID;
            _loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
        }
        else
        {
        	streamId = (_loginRequest.streamId() == 0 ? LOGIN_STREAM_ID : _loginRequest.streamId());
        	userName = _loginRequest.userName().toString();
            _loginRequest.clear();
        }

        _loginRequest.rdmMsgType(LoginMsgType.REQUEST);
        _loginRequest.initDefaultRequest(streamId);
        _loginRequest.applyHasAttrib();
        if (!userName.equals(""))
        {
        	_loginRequest.userName().data(userName);
        }
        _loginRequest.applyHasRole();
        _loginRequest.role(Login.RoleTypes.PROV);
    }
    
    /**
     * Rdm login close.
     *
     * @return the login close
     */
    /*
     * The LoginClose to be sent to close the Login stream.
     * This corresponds to the LoginRequest sent during the
     * connection establishment process.
     */
    LoginClose rdmLoginClose()
    {
    	if (_loginRequest == null)
    		return null;
    	
    	if (_loginClose == null)
    	{
    		_loginClose = (LoginClose)LoginMsgFactory.createMsg();
    		_loginClose.rdmMsgType(LoginMsgType.CLOSE);
    	}
    	
    	_loginClose.streamId(_loginRequest.streamId());
    	
        return _loginClose;
    }
    
    /**
     *  A Directory Refresh to be sent during the setup of a Non-Interactive Provider
     * session. This can be populated with a user's specific information or
     * invoke {@link #initDefaultRDMDirectoryRefresh(String, int)} to populate with default
     * information. Requires LoginRequest to be set.
     *
     * @param directoryRefresh the directory refresh
     */
    public void rdmDirectoryRefresh(DirectoryRefresh directoryRefresh)
    {
        copyDirectoryRefresh(directoryRefresh);
    }
    
    /** A Directory Refresh to be sent during the setup of a Non-Interactive Provider
     * session. This can be populated with a user's specific information or
     * invoke {@link #initDefaultRDMDirectoryRefresh(String, int)} to populate with default
     * information. Requires LoginRequest to be set.
     * 
     * @return the rdmDirectoryRefresh
     */
    public DirectoryRefresh rdmDirectoryRefresh()
    {
        return _directoryRefresh;
    }
    
    /**
     * Initializes the RDM DirectoryRefresh with default information. If the
     * rdmDirectoryRefresh has already been defined (due to a previous call to
     * {@link #rdmDirectoryRefresh(DirectoryRefresh)}) the rdmDirectoryRefresh
     * object will be reused.
     * 
     * @param serviceName the serviceName for this source directory refresh
     * @param serviceId the serviceId for this source directory refresh
     */
    public void initDefaultRDMDirectoryRefresh(String serviceName, int serviceId)
    {
    	int streamId;
    	
        if (_directoryRefresh == null)
        {
        	streamId = DIRECTORY_STREAM_ID;
            _directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
        }
        else
        {
        	streamId = (_directoryRefresh.streamId() == 0 ? DIRECTORY_STREAM_ID : _directoryRefresh.streamId());
            _directoryRefresh.clear();
        }

        _directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);
        
        // stream id
        _directoryRefresh.streamId(streamId);
        
        // state information
        _directoryRefresh.state().streamState(StreamStates.OPEN);
        _directoryRefresh.state().dataState(DataStates.OK);
        _directoryRefresh.state().code(StateCodes.NONE);
        _directoryRefresh.state().text(_stateText);
        
        //clear cache
        _directoryRefresh.applyClearCache();

        //attribInfo information
        _directoryRefresh.filter(FILTER_TO_REFRESH);

        //_service
        _service.clear();
        _service.action(MapEntryActions.ADD);

        //set the _service Id (map key)
        _service.serviceId(serviceId);

        if ((FILTER_TO_REFRESH & Directory.ServiceFilterFlags.INFO) != 0)
        {
            _service.applyHasInfo();
            _service.info().action(FilterEntryActions.SET);

            //vendor 
            _service.info().applyHasVendor();
            _service.info().vendor().data(VENDOR);

            //_service name - required
            _service.info().serviceName().data(serviceName);

            //Qos Range is not supported
            _service.info().applyHasSupportsQosRange();
            _service.info().supportsQosRange(0);
          
            //capabilities - required
            _service.info().capabilitiesList().add((long)DomainTypes.MARKET_PRICE);
            _service.info().capabilitiesList().add((long)DomainTypes.MARKET_BY_ORDER);

            //qos
            _service.info().applyHasQos();
            Qos qos = CodecFactory.createQos();
            qos.rate(QosRates.TICK_BY_TICK);
            qos.timeliness(QosTimeliness.REALTIME);
            _service.info().qosList().add(qos);

            //dictionary used
            _service.info().applyHasDictionariesUsed();
            _service.info().dictionariesUsedList().add(FIELD_DICTIONARY_NAME);
            _service.info().dictionariesUsedList().add(ENUM_TYPE_DICTIONARY_NAME);

            //isSource = Service is provided directly from original publisher
            _service.info().applyHasIsSource();
            _service.info().isSource(1);
            
            /*
             * itemList - Name of SymbolList that includes all of the items that
             * the publisher currently provides. Blank for this example
             */
            _service.info().applyHasItemList();
            _service.info().itemList().data("");

            _service.info().applyHasAcceptingConsumerStatus();
            //accepting customer status = no
            _service.info().acceptingConsumerStatus(0);

            _service.info().applyHasSupportsOutOfBandSnapshots();
            //supports out of band snapshots = no
            _service.info().supportsOutOfBandSnapshots(0);
        }

        if ((FILTER_TO_REFRESH & Directory.ServiceFilterFlags.STATE) != 0)
        {
            _service.applyHasState();
            _service.state().action(FilterEntryActions.SET);

            //_service state
            _service.state().serviceState(1);

            //accepting requests
            _service.state().applyHasAcceptingRequests();
            _service.state().acceptingRequests(1);

            //status
            _service.state().applyHasStatus();
            _service.state().status().dataState(DataStates.OK);
            _service.state().status().streamState(StreamStates.OPEN);
            _service.state().status().code(StateCodes.NONE);
            _service.state().status().text().data("OK");
        }

        if ((FILTER_TO_REFRESH & Directory.ServiceFilterFlags.LOAD) != 0)
        {
            _service.applyHasLoad();
            _service.load().action(FilterEntryActions.SET);

            //open limit
            _service.load().applyHasOpenLimit();
            _service.load().openLimit(OPEN_LIMIT);

            //load factor
            _service.load().applyHasLoadFactor();
            _service.load().loadFactor(1);
        }

        if ((FILTER_TO_REFRESH & Directory.ServiceFilterFlags.LINK) != 0)
        {
            _service.applyHasLink();
            _service.link().action(FilterEntryActions.SET);

            Service.ServiceLink serviceLink = new Service.ServiceLink();

            //link name - Map Entry Key
            serviceLink.name().data(LINK_NAME);

            //link type
            serviceLink.applyHasType();
            serviceLink.type(Directory.LinkTypes.INTERACTIVE);

            //link state
            serviceLink.linkState(Directory.LinkStates.UP);

            //link code
            serviceLink.applyHasCode();
            serviceLink.linkCode(Directory.LinkCodes.OK);

            //link text
            serviceLink.applyHasText();
            serviceLink.text().data("Link state is up");

            _service.link().linkList().add(serviceLink);
        }

        _directoryRefresh.serviceList().add(_service);
    }
    
    /**
     * Rdm directory close.
     *
     * @return the directory close
     */
    /*
     * The DirectoryClose to be sent to close the Directory stream.
     * This corresponds to the DirectoryRefresh sent during the
     * connection establishment process.
     */
    DirectoryClose rdmDirectoryClose()
    {
        if (_directoryRefresh == null)
            return null;
        
        if (_directoryClose == null)
        {
            _directoryClose = (DirectoryClose)DirectoryMsgFactory.createMsg();
            _directoryClose.rdmMsgType(DirectoryMsgType.CLOSE);
        }
        
        _directoryClose.streamId(_directoryRefresh.streamId());
        
        return _directoryClose;
    }

    /**
     *  A Field Dictionary Request to be sent during the setup of a Consumer-Provider
     * session. Requires DirectoryRequest to be set.
     *
     * @param fieldDictionaryRequest the field dictionary request
     */
    void rdmFieldDictionaryRequest(DictionaryRequest fieldDictionaryRequest) {
        _fieldDictionaryRequest = fieldDictionaryRequest;
    }

    /**
     * A Field Dictionary Request to be sent during the setup of a Consumer-Provider
     * session. Requires DirectoryRequest to be set.
     *
     * @return the dictionary request
     */
    DictionaryRequest rdmFieldDictionaryRequest() {
        return _fieldDictionaryRequest;
    }

    /**
     * Initializes the RDM Field DictionaryRequest with default information. If the
     * rdmFieldDictionaryRequest has already been defined, the rdmFieldDictionaryRequest
     * object will be reused.
     */
    void initDefaultRDMFieldDictionaryRequest() {
        int streamId;

        if (_fieldDictionaryRequest == null) {
            streamId = FIELD_DICTIONARY_STREAM_ID;
            _fieldDictionaryRequest = (DictionaryRequest) DictionaryMsgFactory.createMsg();
        } else {
            streamId = (_fieldDictionaryRequest.streamId() == 0 ? FIELD_DICTIONARY_STREAM_ID : _fieldDictionaryRequest.streamId());
            _fieldDictionaryRequest.clear();
        }

        // make sure stream id isn't already being used
        while (streamId == _loginRequest.streamId() ||
                streamId == _directoryRefresh.streamId()) {
            streamId--;
        }
        _fieldDictionaryRequest.rdmMsgType(DictionaryMsgType.REQUEST);
        _fieldDictionaryRequest.streamId(streamId);
        _fieldDictionaryRequest.applyStreaming();
        _fieldDictionaryRequest.verbosity(Dictionary.VerbosityValues.NORMAL);
        _fieldDictionaryRequest.dictionaryName(_fieldDictionaryName);

        return;
    }

    /**
     * The DictionaryClose to be sent to close the Field Dictionary stream.
     * This corresponds to the Field DictionaryRequest sent during the
     * connection establishment process.
     *
     * @return the dictionary close
     */
    DictionaryClose rdmFieldDictionaryClose() {
        if (_fieldDictionaryRequest == null)
            return null;

        if (_fieldDictionaryClose == null) {
            _fieldDictionaryClose = (DictionaryClose) DictionaryMsgFactory.createMsg();
            _fieldDictionaryClose.rdmMsgType(DictionaryMsgType.CLOSE);
        }

        _fieldDictionaryClose.streamId(_fieldDictionaryRequest.streamId());

        return _fieldDictionaryClose;
    }

    /**
     * A EnumType Dictionary Request to be sent during the setup of a Consumer-Provider
     * session. Requires Field DictionaryRequest to be set.
     *
     * @param enumDictionaryRequest the enum dictionary request
     */
    void rdmEnumDictionaryRequest(DictionaryRequest enumDictionaryRequest) {
        _enumDictionaryRequest = enumDictionaryRequest;
    }

    /**
     * A EnumType Dictionary Request to be sent during the setup of a Consumer-Provider
     * session. Requires Field DictionaryRequest to be set.
     *
     * @return the dictionary request
     */
    DictionaryRequest rdmEnumDictionaryRequest() {
        return _enumDictionaryRequest;
    }

    /**
     * Initializes the RDM EnumType DictionaryRequest with default information. If the
     * rdmEnumDictionaryRequest has already been defined, the rdmEnumDictionaryRequest
     * object will be reused.
     */
    void initDefaultRDMEnumDictionaryRequest() {
        int streamId;

        if (_enumDictionaryRequest == null) {
            streamId = ENUM_DICTIONARY_STREAM_ID;
            _enumDictionaryRequest = (DictionaryRequest) DictionaryMsgFactory.createMsg();
        } else {
            streamId = (_enumDictionaryRequest.streamId() == 0 ? ENUM_DICTIONARY_STREAM_ID : _enumDictionaryRequest.streamId());
            _enumDictionaryRequest.clear();
        }

        // make sure stream id isn't already being used
        while (streamId == _loginRequest.streamId() ||
                streamId == _directoryRefresh.streamId() ||
                streamId == _fieldDictionaryRequest.streamId()) {
            streamId--;
        }
        _enumDictionaryRequest.rdmMsgType(DictionaryMsgType.REQUEST);
        _enumDictionaryRequest.streamId(streamId);
        _enumDictionaryRequest.applyStreaming();
        _enumDictionaryRequest.verbosity(Dictionary.VerbosityValues.NORMAL);
        _enumDictionaryRequest.dictionaryName(_enumTypeDictionaryName);

        return;
    }

    /**
     * The DictionaryClose to be sent to close the EnumType Dictionary stream.
     * This corresponds to the EnumType DictionaryRequest sent during the
     * connection establishment process.
     *
     * @return the dictionary close
     */
    DictionaryClose rdmEnumDictionaryClose() {
        if (_enumDictionaryRequest == null)
            return null;

        if (_enumDictionaryClose == null) {
            _enumDictionaryClose = (DictionaryClose) DictionaryMsgFactory.createMsg();
            _enumDictionaryClose.rdmMsgType(DictionaryMsgType.CLOSE);
        }

        _enumDictionaryClose.streamId(_enumDictionaryRequest.streamId());

        return _enumDictionaryClose;
    }

    /**
     *  A callback function for processing RDMLoginMsgEvents received. If not present,
     * the received message will be passed to the defaultMsgCallback.
     *
     * @param callback the callback
     * @see RDMLoginMsgCallback
     * @see RDMLoginMsgEvent
     */
    public void loginMsgCallback(RDMLoginMsgCallback callback)
    {
        _loginMsgCallback = callback;
    }

    /** A callback function for processing RDMLoginMsgEvents received. If not present,
     * the received message will be passed to the defaultMsgCallback.
     * 
     * @return the loginMsgCallback
     */
    public RDMLoginMsgCallback loginMsgCallback()
    {
        return _loginMsgCallback;
    }

    public void dictionaryMsgCallback(RDMDictionaryMsgCallback dictionaryMsgCallback) {
        this._dictionaryMsgCallback = dictionaryMsgCallback;
    }

    public RDMDictionaryMsgCallback dictionaryMsgCallback() {
        return _dictionaryMsgCallback;
    }

    public int dictionaryDownloadMode() {
        return _dictionaryDownloadMode;
    }

    public void dictionaryDownloadMode(int dictionaryDownloadMode) {
        _dictionaryDownloadMode = dictionaryDownloadMode;
    }

    public Buffer fieldDictionaryName() {
        return _fieldDictionaryName;
    }

    public Buffer enumTypeDictionaryName() {
        return _enumTypeDictionaryName;
    }

    public boolean receivedFieldDictionaryResp() {
        return _receivedFieldDictionaryResp;
    }

    public void receivedFieldDictionaryResp(boolean receivedFieldDictionaryResp) {
        _receivedFieldDictionaryResp = receivedFieldDictionaryResp;
    }

    public boolean receivedEnumDictionaryResp() {
        return _receivedEnumDictionaryResp;
    }

    public void receivedEnumDictionaryResp(boolean receivedEnumDictionaryResp) {
        _receivedEnumDictionaryResp = receivedEnumDictionaryResp;
    }

    /*
     * Performs a deep copy from a specified NIProviderRole into this NIProviderRole.
     * Only public facing attributes are copied.
     */
    void copy(NIProviderRole role)
    {
        super.copy(role);
        _loginMsgCallback = role.loginMsgCallback();
        _dictionaryMsgCallback = role.dictionaryMsgCallback();
        copyLoginRequest(role.rdmLoginRequest());
        copyDirectoryRefresh(role.rdmDirectoryRefresh());
        _dictionaryDownloadMode = role.dictionaryDownloadMode();
    }
    
    /*
     * Performs a deep copy from a specified LoginRequest into the LoginRequest associated with this NIProviderRole.
     */
    void copyLoginRequest(LoginRequest loginRequest)
    {
        if (loginRequest != null)
        {
            if (_loginRequest == null)
            {
                _loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
                _loginRequest.rdmMsgType(LoginMsgType.REQUEST);
            }
            loginRequest.copy(_loginRequest);
        }
    }
    
    /*
     * Performs a deep copy from a specified DirectoryRefresh into the DirectoryRefresh associated with this NIProviderRole.
     */
    void copyDirectoryRefresh(DirectoryRefresh directoryRefresh)
    {
        if (directoryRefresh != null)
        {
            if (_directoryRefresh == null)
            {
                _directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
                _directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);
            }
            directoryRefresh.copy(_directoryRefresh);
        }
    }
}
