package com.refinitiv.eta.valueadd.examples.provider;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.FilterEntryActions;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.shared.DirectoryRejectReason;
import com.refinitiv.eta.shared.DirectoryRequestInfo;
import com.refinitiv.eta.shared.DirectoryRequestInfoList;
import com.refinitiv.eta.rdm.Directory;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryClose;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorSubmitOptions;

/*
 * This is the source directory handler for the ETA Java Provider application.
 * <p>
 * Only one source directory stream per channel is allowed by this simple
 * provider.
 * <p>
 * It provides methods for processing source directory requests from consumers
 * and sending back the responses.
 * <p>
 * Methods for sending source directory request reject/close status messages,
 * initializing the source directory handler, setting the service name,
 * getting/setting the service id, checking if a request has minimal filter
 * flags, and closing source directory streams are also provided.
 */
class DirectoryHandler
{
    private static final int REJECT_MSG_SIZE = 1024;
    private static final int STATUS_MSG_SIZE = 1024;
    private static final int REFRESH_MSG_SIZE = 1024;

    private DirectoryClose _directoryClose = (DirectoryClose)DirectoryMsgFactory.createMsg();
    private DirectoryStatus _directoryStatus = (DirectoryStatus)DirectoryMsgFactory.createMsg();
    private DirectoryRefresh _directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
    private DirectoryRequest _directoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
    private EncodeIterator _encodeIter = CodecFactory.createEncodeIterator();
    private DirectoryRequestInfoList _directoryRequestInfoList;

    // service name of provider
    private String _serviceName;

    // service id associated with the service name of provider
    private int _serviceId = 1234;
    
    // openWindow
    private int _openWindow = 256;

    // vendor name
    private static final String vendor = "LSEG";

    // field dictionary name
    private static final String fieldDictionaryName = "RWFFld";

    // enumtype dictionary name
    private static final String enumTypeDictionaryName = "RWFEnum";

    // link name
    private static final String linkName = "ETA Provider Link";

    static final int OPEN_LIMIT = 10;

    private Service _service = DirectoryMsgFactory.createService();

    private ReactorSubmitOptions _submitOptions = ReactorFactory.createReactorSubmitOptions();

    DirectoryHandler()
    {
        _directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
        _directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);
        _directoryStatus.rdmMsgType(DirectoryMsgType.STATUS);
        _directoryClose.rdmMsgType(DirectoryMsgType.CLOSE);
        _directoryRequestInfoList = new DirectoryRequestInfoList();
    }

    /*
     * Initializes source directory information fields.
     */
    void init()
    {
        _directoryRequestInfoList.init();
    }

    /*
     * Sends directory close status message to a channel.
     */
    int sendCloseStatus(ReactorChannel chnl, ReactorErrorInfo errorInfo)
    {
        // proceed if source directory request info found
        DirectoryRequestInfo directoryReqInfo = findDirectoryReqInfo(chnl.channel());
        if (directoryReqInfo == null)
        {
            return CodecReturnCodes.SUCCESS;
        }

        // get a buffer for the source directory request
        TransportBuffer msgBuf = chnl.getBuffer(STATUS_MSG_SIZE, false, errorInfo);
        if (msgBuf == null)
        {
            return CodecReturnCodes.FAILURE;
        }
      
        // encode directory close
        _encodeIter.clear();
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            errorInfo.error().text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        _directoryStatus.streamId(directoryReqInfo.directoryRequest().streamId());
        
        _directoryStatus.applyHasState();
        _directoryStatus.state().streamState(StreamStates.CLOSED);
        _directoryStatus.state().dataState(DataStates.SUSPECT);
        _directoryStatus.state().text().data("Directory stream closed");
        ret = _directoryStatus.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            errorInfo.error().text("DirectoryStatus.encode failed");
            return ret;
        }

        // send close status
        return chnl.submit(msgBuf, _submitOptions, errorInfo);
    }

    /*
     * Sends the source directory request reject status message to a channel.
     */
    int sendRequestReject(ReactorChannel chnl, int streamId, DirectoryRejectReason reason, ReactorErrorInfo errorInfo)
    {
        // get a buffer for the login request reject status
        TransportBuffer msgBuf = chnl.getBuffer(REJECT_MSG_SIZE, false, errorInfo);
        if (msgBuf != null)
        {
            // encode login request reject status
            int ret = encodeRequestReject(chnl, streamId, reason, msgBuf, errorInfo);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            // send request reject status 
            return chnl.submit(msgBuf, _submitOptions, errorInfo);
        }
        else
        {
            errorInfo.error().text("Channel.getBuffer(): Failed " + errorInfo.error().text());
            return CodecReturnCodes.FAILURE;
        }
    }

    /*
     * Encodes the login request reject status. Returns success if encoding
     * succeeds or failure if encoding fails.
     */
    private int encodeRequestReject(ReactorChannel chnl, int streamId, DirectoryRejectReason reason, TransportBuffer msgBuf, ReactorErrorInfo errorInfo)
    {
        // clear encode iterator
        _encodeIter.clear();

        // set-up message
        _directoryStatus.streamId(streamId);
        _directoryStatus.applyHasState();
        _directoryStatus.state().streamState(StreamStates.CLOSED_RECOVER);
        _directoryStatus.state().dataState(DataStates.SUSPECT);
        switch (reason)
        {
            case MAX_SRCDIR_REQUESTS_REACHED:
                _directoryStatus.state().code(StateCodes.TOO_MANY_ITEMS);
                _directoryStatus.state().text().data("Source directory rejected for stream id " + streamId + " - max request count reached");
                break;
            case INCORRECT_FILTER_FLAGS:
                _directoryStatus.state().code(StateCodes.USAGE_ERROR);
                _directoryStatus.state().text().data("Source directory request rejected for stream id  " + streamId + " - request must minimally have INFO, STATE, and GROUP filter flags");
                break;
            case DIRECTORY_RDM_DECODER_FAILED:
                _directoryStatus.state().code(StateCodes.USAGE_ERROR);
                _directoryStatus.state().text().data("Source directory request rejected for stream id  " + streamId + " - decoding failure: " + errorInfo.error().text());
                break;
            default:
                break;
        }

        // encode message
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            errorInfo.error().text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        ret = _directoryStatus.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
            errorInfo.error().text("DirectoryStatus.encode() failed");

        return ret;
    }

    /*
     * Closes a dictionary stream.
     */
    void closeStream(int streamId)
    {
        // find original request information associated with chnl
        for (DirectoryRequestInfo sourceDirectoryReqInfo : _directoryRequestInfoList)
        {
            if (sourceDirectoryReqInfo.directoryRequest().streamId() == streamId && sourceDirectoryReqInfo.isInUse())
            {
                // clear original request information
                System.out.println("Closing source directory stream id '" + sourceDirectoryReqInfo.directoryRequest().streamId() + "' with service name: " + _serviceName);
                sourceDirectoryReqInfo.clear();
                break;
            }
        }
    }

    /*
     * Closes all open dictionary streams for a channel.
     */
    void closeStream(ReactorChannel chnl)
    {
        //find original request information associated with chnl
        DirectoryRequestInfo dirReqInfo = findDirectoryReqInfo(chnl.channel());
        if(dirReqInfo != null)
        {
            // clear original request information 
            System.out.println("Closing source directory stream id '" + dirReqInfo.directoryRequest().streamId() + "' with service name: " + _serviceName);
            dirReqInfo.clear();
        }
    }

    /*
     * Finds dictionary request information for a channel.
     */    
    private DirectoryRequestInfo findDirectoryReqInfo(Channel chnl)
    {
        //find original request information associated with chnl
        for (DirectoryRequestInfo sourceDirectoryReqInfo : _directoryRequestInfoList)
        {
            if (sourceDirectoryReqInfo.channel() == chnl && sourceDirectoryReqInfo.isInUse())
            {
                return sourceDirectoryReqInfo;
            }
        }
        
        return null;
    }
    
    /*
     * Retrieves DirectoryRequest to use with a consumer that is
     * requesting the source directory.
     */
    DirectoryRequest getDirectoryRequest(ReactorChannel chnl, DirectoryRequest directoryRequest)
    {
    	return _directoryRequestInfoList.get(chnl.channel(), directoryRequest);
    }
    
    /*
     * Sends directory refresh message to a channel.
     */
    int sendRefresh(ReactorChannel chnl, DirectoryRequest srcDirReqInfo, ReactorErrorInfo errorInfo)
    {
        // get a buffer for the source directory request
        TransportBuffer msgBuf = chnl.getBuffer(REFRESH_MSG_SIZE, false, errorInfo);
        if (msgBuf == null)
        {
            return CodecReturnCodes.FAILURE;
        }

        // encode source directory request
        _directoryRefresh.clear();
        _directoryRefresh.streamId(srcDirReqInfo.streamId());

        // clear cache
        _directoryRefresh.applyClearCache();
        _directoryRefresh.applySolicited();

        // state information for response message
        _directoryRefresh.state().clear();
        _directoryRefresh.state().streamState(StreamStates.OPEN);
        _directoryRefresh.state().dataState(DataStates.OK);
        _directoryRefresh.state().code(StateCodes.NONE);
        _directoryRefresh.state().text().data("Source Directory Refresh Completed");

        // attribInfo information for response message
        _directoryRefresh.filter(srcDirReqInfo.filter());

        // populate the Service
        _service.clear();
        _service.action(MapEntryActions.ADD);

        // set the service Id (map key)
        _service.serviceId(serviceId());

        if ((srcDirReqInfo.filter() & Directory.ServiceFilterFlags.INFO) != 0)
        {
            _service.applyHasInfo();
            _service.info().action(FilterEntryActions.SET);

            // vendor
            _service.info().applyHasVendor();
            _service.info().vendor().data(vendor);

            // service name - required
            _service.info().serviceName().data(_serviceName);


            // Qos Range is not supported
            _service.info().applyHasSupportsQosRange();
            _service.info().supportsQosRange(0);

            // capabilities - required
            _service.info().capabilitiesList().add((long)DomainTypes.MARKET_PRICE);
            _service.info().capabilitiesList().add((long)DomainTypes.MARKET_BY_ORDER);
            _service.info().capabilitiesList().add((long)DomainTypes.MARKET_BY_PRICE);
            _service.info().capabilitiesList().add((long)DomainTypes.DICTIONARY);
            _service.info().capabilitiesList().add((long)DomainTypes.SYMBOL_LIST);
            _service.info().capabilitiesList().add((long)DomainTypes.SYSTEM);

            // qos
            _service.info().applyHasQos();
            Qos qos = CodecFactory.createQos();
            qos.rate(QosRates.TICK_BY_TICK);
            qos.timeliness(QosTimeliness.REALTIME);
            _service.info().qosList().add(qos);
         

            // dictionary used
            _service.info().applyHasDictionariesUsed();
            _service.info().dictionariesUsedList().add(fieldDictionaryName);
            _service.info().dictionariesUsedList().add(enumTypeDictionaryName);
           

            // dictionary provided
            _service.info().applyHasDictionariesProvided();
            _service.info().dictionariesProvidedList().add(fieldDictionaryName);
            _service.info().dictionariesProvidedList().add(enumTypeDictionaryName);
          

            // isSource = Service is provided directly from original publisher
            _service.info().applyHasIsSource();
            _service.info().isSource(1);
 
            // itemList - Name of SymbolList that includes all of the items that
            // he publisher currently provides.
            _service.info().applyHasItemList();
            _service.info().itemList().data("_ETA_ITEM_LIST");
 
            // accepting customer status = no
            _service.info().applyHasAcceptingConsumerStatus();
            _service.info().acceptingConsumerStatus(0);

            // supports out of band snapshots = no
            _service.info().applyHasSupportsOutOfBandSnapshots();
            _service.info().supportsOutOfBandSnapshots(0);
        }

        if ((srcDirReqInfo.filter() & Directory.ServiceFilterFlags.STATE) != 0)
        {
            _service.applyHasState();
            _service.state().action(FilterEntryActions.SET);

            // service state
            _service.state().serviceState(1);

            // accepting requests
            _service.state().applyHasAcceptingRequests();
            _service.state().acceptingRequests(1);
        }

        if ((srcDirReqInfo.filter() & Directory.ServiceFilterFlags.LOAD) != 0)
        {
            _service.applyHasLoad();
            _service.load().action(FilterEntryActions.SET);

            // open limit
            _service.load().applyHasOpenLimit();
            _service.load().openLimit(OPEN_LIMIT);
            // open Window
            _service.load().applyHasOpenWindow();
            _service.load().openWindow(_openWindow);
        }

        if ((srcDirReqInfo.filter() & Directory.ServiceFilterFlags.LINK) != 0)
        {
            _service.applyHasLink();
            _service.link().action(FilterEntryActions.SET);

            Service.ServiceLink serviceLink = new Service.ServiceLink();

            // link name - Map Entry Key 
            serviceLink.name().data(linkName);

            // link type
            serviceLink.applyHasType();
            serviceLink.type(Directory.LinkTypes.INTERACTIVE);

            // link text
            serviceLink.applyHasText();
            serviceLink.text().data("Link state is up");
            _service.link().linkList().add(serviceLink);
        }

        _directoryRefresh.serviceList().add(_service);

        // encode directory request
        _encodeIter.clear();
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            errorInfo.error().text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }
        
        ret = _directoryRefresh.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            errorInfo.error().text("DirectoryRefresh.encode() failed");
            return ret;
        }

        // send source directory request
        return chnl.submit(msgBuf, _submitOptions, errorInfo);
    }

    /*
     * Gets the service name requested by the application.
     */
    String serviceName()
    {
        return _serviceName;
    }

    /*
     * Sets the service name requested by the application.
     */
    void serviceName(String serviceName)
    {
        this._serviceName = serviceName;
    }

    /*
     * Gets the service id requested by the application.
     */
    int serviceId()
    {
        return _serviceId;
    }

    /*
     * Sets the service id requested by the application.
     */
    void serviceId(int serviceId)
    {
        this._serviceId = serviceId;
    }
    
    int openWindow()
    {
        return _openWindow;
    }

    /*
     * Sets the openWindow.
     */
    void openWindow(int openWindow)
    {
        this._openWindow = openWindow;
    }
}
