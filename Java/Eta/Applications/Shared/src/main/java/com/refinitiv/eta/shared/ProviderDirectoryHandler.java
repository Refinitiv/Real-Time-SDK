package com.refinitiv.eta.shared;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.FilterEntryActions;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.rdm.Directory;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryClose;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;

/**
 * This is the source directory handler for the UPA Java Provider application.
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
public class ProviderDirectoryHandler
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
    
    private boolean _enableGenericProvider; // used for generic provider

    // service name of provider
    private String _serviceName;

    // service id associated with the service name of provider
    private int _serviceId = 1234;

    // vendor name
    private static final String vendor = "Refinitiv";

    // field dictionary name
    private static final String fieldDictionaryName = "RWFFld";

    // enumtype dictionary name
    private static final String enumTypeDictionaryName = "RWFEnum";

    // link name
    private static final String linkName = "UPA Provider Link";

    public static final int OPEN_LIMIT = 10;

    private ProviderSession _providerSession;

    private Service _service = DirectoryMsgFactory.createService();
    
    public static final int GENERIC_DOMAIN = 200; // used for generic provider

    public ProviderDirectoryHandler(ProviderSession providerSession)
    {
        _directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
        _directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);
        _directoryStatus.rdmMsgType(DirectoryMsgType.STATUS);
        _directoryClose.rdmMsgType(DirectoryMsgType.CLOSE);
        _directoryRequestInfoList = new DirectoryRequestInfoList();
        _providerSession = providerSession;
    }
    
    /**
     * Enables the source directory handler for a generic provider.
     * A generic provider does not use a dictionary and is only capable
     * of providing the user-defined generic domain. 
     */
    public void enableGenericProvider()
    {
        _enableGenericProvider = true;
    }

    /**
     * Initializes source directory information fields.
     */
    public void init()
    {
        _directoryRequestInfoList.init();
    }

    /**
     * Processes a source directory request. This consists of calling
     * directoryRequest.decode() to decode the request and calling
     * sendSourceDirectoryResponse() to send the source directory response.
     * 
     * @param chnl - The channel of the response
     * @param msg - The partially decoded message
     * @param dIter - The decode iterator
     * @param error - Error information in case of any encoding or socket
     *            writing error
     * @return {@link CodecReturnCodes}
     */
    public int processRequest(Channel chnl, Msg msg, DecodeIterator dIter, Error error)
    {
        switch (msg.msgClass())
        {
            case MsgClasses.REQUEST:
                // get key
                MsgKey msgKey = msg.msgKey();
                if (!keyHasMinFilterFlags(msgKey))
                {
                    return sendRequestReject(chnl, msg.streamId(), DirectoryRejectReason.INCORRECT_FILTER_FLAGS, error);
                }
                _directoryRequest.clear();
                if (_directoryRequest.decode(dIter, msg) != CodecReturnCodes.SUCCESS)
                {
                    return sendRequestReject(chnl, msg.streamId(), DirectoryRejectReason.INCORRECT_FILTER_FLAGS, error);
                }
                DirectoryRequest pdirectoryRequest = _directoryRequestInfoList.get(chnl, _directoryRequest);
                if (pdirectoryRequest == null)
                {
                    return sendRequestReject(chnl, msg.streamId(), DirectoryRejectReason.MAX_SRCDIR_REQUESTS_REACHED, error);
                }

                System.out.println("Received Source Directory Request");

                // send source directory response
                return sendRefresh(chnl, pdirectoryRequest, error);
            case MsgClasses.CLOSE:
            {
                System.out.println("Received Directory Close for StreamId " + msg.streamId());

                // close directory stream
                closeStream(msg.streamId());
                return CodecReturnCodes.SUCCESS;
            }
            default:
                error.text("Received unhandled Source Directory msg type: " + msg.msgClass());
                return CodecReturnCodes.FAILURE;
        }
    }

    /**
     * Sends directory close status message to a channel.
     * 
     * @param chnl - The channel to send close status message to
     * @param error - Error information in case of encoding or socket writing
     *            failure.
     * @return {@link CodecReturnCodes}
     */
    public int sendCloseStatus(Channel chnl, Error error)
    {
        // proceed if source directory request info found
        DirectoryRequestInfo directoryReqInfo = findDirectoryReqInfo(chnl);
        if (directoryReqInfo == null)
        {
            return CodecReturnCodes.SUCCESS;
        }

        // get a buffer for the source directory request
        TransportBuffer msgBuf = chnl.getBuffer(STATUS_MSG_SIZE, false, error);
        if (msgBuf == null)
        {
            return CodecReturnCodes.FAILURE;
        }
      
        // encode directory close
        _encodeIter.clear();
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        _directoryStatus.streamId(directoryReqInfo.directoryRequest.streamId());
        
        _directoryStatus.applyHasState();
        _directoryStatus.state().streamState(StreamStates.CLOSED);
        _directoryStatus.state().dataState(DataStates.SUSPECT);
        _directoryStatus.state().text().data("Directory stream closed");
        ret = _directoryStatus.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("DirectoryStatus.encode failed");
            return ret;
        }

        // send close status
        return _providerSession.write(chnl, msgBuf, error);
    }

    /*
     * Sends the source directory request reject status message to a channel.
     */
    private int sendRequestReject(Channel chnl, int streamId, DirectoryRejectReason reason, Error error)
    {
        // get a buffer for the login request reject status
        TransportBuffer msgBuf = chnl.getBuffer(REJECT_MSG_SIZE, false, error);
        if (msgBuf != null)
        {
            // encode login request reject status
            int ret = encodeRequestReject(chnl, streamId, reason, msgBuf, error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            // send request reject status 
            return _providerSession.write(chnl, msgBuf, error);
        }
        else
        {
            error.text("Channel.getBuffer(): Failed " + error.text());
            return CodecReturnCodes.FAILURE;
        }
    }

    /*
     * Encodes the login request reject status. Returns success if encoding
     * succeeds or failure if encoding fails.
     */
    private int encodeRequestReject(Channel chnl, int streamId, DirectoryRejectReason reason, TransportBuffer msgBuf, Error error)
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
                _directoryStatus.state().text().data("Source directory rejected for stream id " + streamId + "- max request count reached");
                break;
            case INCORRECT_FILTER_FLAGS:
                _directoryStatus.state().code(StateCodes.USAGE_ERROR);
                _directoryStatus.state().text().data("Source directory request rejected for stream id  " + streamId + "- request must minimally have INFO, STATE, and GROUP filter flags");
                break;
            case DIRECTORY_RDM_DECODER_FAILED:
                _directoryStatus.state().code(StateCodes.USAGE_ERROR);
                _directoryStatus.state().text().data("Source directory request rejected for stream id  " + streamId + "- decoding failure");
                break;
            default:
                break;
        }

        // encode message
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        ret = _directoryStatus.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
            error.text("DirectoryStatus.encode() failed");

        return ret;
    }

    /**
     * Closes the source directory stream for a channel.
     * 
     * @param chnl - The channel to close the source directory stream for
     */
    public void closeRequest(Channel chnl)
    {
        //find original request information associated with chnl
        DirectoryRequestInfo dirReqInfo = findDirectoryReqInfo(chnl);
        if(dirReqInfo != null)
        {
            // clear original request information 
            System.out.println("Closing source directory stream id '" + dirReqInfo.directoryRequest.streamId() + "' with service name: " + _serviceName);
            dirReqInfo.clear();
        }
    }

    private DirectoryRequestInfo findDirectoryReqInfo(Channel chnl)
    {
        //find original request information associated with chnl
        for (DirectoryRequestInfo sourceDirectoryReqInfo : _directoryRequestInfoList)
        {
            if (sourceDirectoryReqInfo.chnl == chnl && sourceDirectoryReqInfo.isInUse)
            {
                return sourceDirectoryReqInfo;
            }
        }
        
        return null;
    }
    private void closeStream(int streamId)
    {
        // find original request information associated with chnl
        for (DirectoryRequestInfo sourceDirectoryReqInfo : _directoryRequestInfoList)
        {
            if (sourceDirectoryReqInfo.directoryRequest.streamId() == streamId && sourceDirectoryReqInfo.isInUse)
            {
                // clear original request information
                System.out.println("Closing source directory stream id '" + _directoryRequest.streamId() + "' with service name: " + _serviceName);
                sourceDirectoryReqInfo.clear();
                break;
            }
        }
    }

    /**
     * Sends directory refresh message to a channel.
     * 
     * @param chnl - The channel to send a source directory response to
     * @param srcDirReqInfo - The source directory request information
     * @param error - Error information populated in case of encoding or socket
     *            writing
     * @return {@link CodecReturnCodes}
     */
    public int sendRefresh(Channel chnl, DirectoryRequest srcDirReqInfo, Error error)
    {
        // get a buffer for the source directory request
        TransportBuffer msgBuf = chnl.getBuffer(REFRESH_MSG_SIZE, false, error);
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
            if (!_enableGenericProvider)
            {
                _service.info().capabilitiesList().add((long)DomainTypes.MARKET_PRICE);
                _service.info().capabilitiesList().add((long)DomainTypes.MARKET_BY_ORDER);
                _service.info().capabilitiesList().add((long)DomainTypes.MARKET_BY_PRICE);
                _service.info().capabilitiesList().add((long)DomainTypes.DICTIONARY);
                _service.info().capabilitiesList().add((long)DomainTypes.SYMBOL_LIST);
            }
            else // generic provider only supports the user-defined generic domain
            {
                _service.info().capabilitiesList().add((long)GENERIC_DOMAIN);
            }

            // qos
            _service.info().applyHasQos();
            Qos qos = CodecFactory.createQos();
            qos.rate(QosRates.TICK_BY_TICK);
            qos.timeliness(QosTimeliness.REALTIME);
            _service.info().qosList().add(qos);
         
            // dictionary used
            if (!_enableGenericProvider)
            {
                _service.info().applyHasDictionariesUsed();
                _service.info().dictionariesUsedList().add(fieldDictionaryName);
                _service.info().dictionariesUsedList().add(enumTypeDictionaryName);
            }

            // dictionary provided
            if (!_enableGenericProvider)
            {
                _service.info().applyHasDictionariesProvided();
                _service.info().dictionariesProvidedList().add(fieldDictionaryName);
                _service.info().dictionariesProvidedList().add(enumTypeDictionaryName);
            }
          
            // isSource = Service is provided directly from original publisher
            _service.info().applyHasIsSource();
            _service.info().isSource(1);
 
            // itemList - Name of SymbolList that includes all of the items that
            // he publisher currently provides.
            if (!_enableGenericProvider)
            {
                _service.info().applyHasItemList();
                _service.info().itemList().data("_UPA_ITEM_LIST");
            }
 
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
            error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }
        
        ret = _directoryRefresh.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("DirectoryRefresh.encode() failed");
            return ret;
        }

        // send source directory request
        return _providerSession.write(chnl, msgBuf, error);
    }

    /*
     * Does key have minimal filter flags. Request key must minimally have INFO,
     * STATE, and GROUP filter flags. key - The message key
     */
    private boolean keyHasMinFilterFlags(MsgKey key)
    {
        return key.checkHasFilter() &&
                (key.filter() & Directory.ServiceFilterFlags.INFO) != 0 &&
                (key.filter() & Directory.ServiceFilterFlags.STATE) != 0 &&
                (key.filter() & Directory.ServiceFilterFlags.GROUP) != 0;
    }

    /**
     * Gets the service name requested by the application.
     * 
     * @return servicename - The service name requested by the application
     */
    public String serviceName()
    {
        return _serviceName;
    }

    /**
     * Sets the service name requested by the application.
     * 
     * @param serviceName - The service name requested by the application
     */
    public void serviceName(String serviceName)
    {
        this._serviceName = serviceName;
    }

    /**
     * Gets the service id requested by the application.
     * 
     * @return serviceid - The service id requested by the application
     */
    public int serviceId()
    {
        return _serviceId;
    }

    /**
     * Sets the service id requested by the application.
     * 
     * @param serviceId - The service id requested by the application
     */
    public void serviceId(int serviceId)
    {
        this._serviceId = serviceId;
    }
}
