package com.refinitiv.eta.valueadd.examples.provider;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.shared.DictionaryRejectReason;
import com.refinitiv.eta.shared.DictionaryRequestInfo;
import com.refinitiv.eta.shared.DictionaryRequestInfoList;
import com.refinitiv.eta.rdm.Dictionary;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRefreshFlags;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryStatus;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorSubmitOptions;

/*
 * This is the dictionary handler for the UPA - Java Interactive Provider
 * application. Only two dictionary streams (field and enum type) per channel
 * are allowed by this simple provider.
 * <p>
 * It provides methods for processing dictionary requests from consumers and
 * sending back the responses.
 * <p>
 * Methods for sending dictionary request reject/close status messages,
 * initializing the dictionary provider, loading dictionaries from files,
 * getting the loaded dictionary, and closing dictionary streams are also
 * provided.
 */
class DictionaryHandler
{
    private static final int MAX_FIELD_DICTIONARY_MSG_SIZE = 8192;
    private static final int MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE = 12800;
    private static final int MAX_DICTIONARY_STATUS_MSG_SIZE = 1024;
    
    private static final String FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
    private static final String ENUM_TABLE_FILE_NAME = "enumtype.def";

    // field dictionary name
    static final Buffer fieldDictionaryDownloadName;

    // enumtype dictionary name
    static final Buffer enumTypeDictionaryDownloadName;

    private ReactorSubmitOptions _submitOptions = ReactorFactory.createReactorSubmitOptions();

    static
    {
        fieldDictionaryDownloadName = CodecFactory.createBuffer();
        fieldDictionaryDownloadName.data("RWFFld");
        enumTypeDictionaryDownloadName = CodecFactory.createBuffer();
        enumTypeDictionaryDownloadName.data( "RWFEnum");
    }

    private DataDictionary _dictionary;
    private DictionaryRequestInfoList _dictionaryRequestInfoList;
    
    private EncodeIterator _encodeIter = CodecFactory.createEncodeIterator();
    private DictionaryRefresh _dictionaryRefresh = (DictionaryRefresh)DictionaryMsgFactory.createMsg();
    private DictionaryStatus _dictionaryStatus = (DictionaryStatus)DictionaryMsgFactory.createMsg();
    private DictionaryRequest _dictionaryRequest = (DictionaryRequest)DictionaryMsgFactory.createMsg();
    
    DictionaryHandler()
    {
        _dictionary = CodecFactory.createDataDictionary();
        _dictionaryRequestInfoList = new DictionaryRequestInfoList();
        _dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
        _dictionaryStatus.rdmMsgType(DictionaryMsgType.STATUS);
        _dictionaryRequest.rdmMsgType(DictionaryMsgType.REQUEST);
   }

    /*
     * Initializes dictionary information fields.
     */
    void init()
    {
        _dictionaryRequestInfoList.init();
    }

    /*
     * Returns the data dictionary.
     */
    DataDictionary dictionary()
    {
        return _dictionary;
    }

    /*
     * Calls the method to delete the dictionary, freeing all memory
     * associated with it.
     */
    void freeDictionary()
    {
        _dictionary.clear();
    }

    /*
     * Loads dictionary files
     */
    boolean loadDictionary(Error error)
    {
        _dictionary.clear();
        if (_dictionary.loadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, error) < 0)
        {
            return false;
        }

        if (_dictionary.loadEnumTypeDictionary(ENUM_TABLE_FILE_NAME, error) < 0)
            return false;

        return true;
    }

    /*
     * Closes a dictionary stream.
     */
    void closeStream(int streamId)
    {
        // find original request information associated with streamId
        for (DictionaryRequestInfo dictionraryReqInfo : _dictionaryRequestInfoList)
        {
            if (dictionraryReqInfo.dictionaryRequest().streamId() == streamId && dictionraryReqInfo.isInUse())
            {
                // clear original request information
                System.out.println("Closing dictionary stream id " + dictionraryReqInfo.dictionaryRequest().streamId() +
                                   " with dictionary name: " + dictionraryReqInfo.dictionaryRequest().dictionaryName());
                dictionraryReqInfo.clear();
                break;
            }
        }
    }

    /*
     * Closes all open dictionary streams for a channel.
     */
    void closeStream(ReactorChannel chnl)
    {
        // find original request information associated with chnl
        for (DictionaryRequestInfo dictionraryReqInfo : _dictionaryRequestInfoList)
        {
            if (dictionraryReqInfo.channel() == chnl.channel() && dictionraryReqInfo.isInUse())
            {
                // clear original request information
                System.out.println("Closing dictionary stream id  " + dictionraryReqInfo.dictionaryRequest().streamId() +
                                   " with dictionary name: " + dictionraryReqInfo.dictionaryRequest().dictionaryName());
                dictionraryReqInfo.clear();
            }
        }
    }

    /*
     * 
     * Sends the dictionary close status message(s) for a channel. This consists
     * of finding all request information for this channel and sending the close
     * status messages to the channel.
     */
    int sendCloseStatusMsgs(ReactorChannel chnl, ReactorErrorInfo errorInfo)
    {
        int ret = 0;
        for (DictionaryRequestInfo dictionraryReqInfo : _dictionaryRequestInfoList)
        {
            if (dictionraryReqInfo.isInUse() && dictionraryReqInfo.channel() == chnl.channel())
            {
                ret = sendCloseStatusMsg(chnl, dictionraryReqInfo.dictionaryRequest().streamId(), errorInfo);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }

        return CodecReturnCodes.SUCCESS;
    }

    /*
     * Sends the dictionary close status message for a channel. Returns success
     * if send dictionary close status succeeds or failure if it fails.
     */
    int sendCloseStatusMsg(ReactorChannel chnl, int streamId, ReactorErrorInfo errorInfo)
    {
        // get a buffer for the dictionary close status 
        TransportBuffer msgBuf = chnl.getBuffer(MAX_DICTIONARY_STATUS_MSG_SIZE, false, errorInfo);

        if (msgBuf != null)
        {
            // encode directory close
            _encodeIter.clear();
            int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
            if (ret != CodecReturnCodes.SUCCESS)
            {
                errorInfo.error().text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
                return ret;
            }
            
            // encode dictionary close status
            _dictionaryStatus.clear();
            _dictionaryStatus.streamId(streamId);
            _dictionaryStatus.applyHasState();
            _dictionaryStatus.state().streamState(StreamStates.CLOSED);
            _dictionaryStatus.state().dataState(DataStates.SUSPECT);
            _dictionaryStatus.state().text().data("Dictionary stream closed");
            ret = _dictionaryStatus.encode(_encodeIter);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("encodeDictionaryCloseStatus() failed with return code: " + ret);
                return ret;
            }

            // send close status
            return chnl.submit(msgBuf, _submitOptions, errorInfo);
        }
        else
        {
            return CodecReturnCodes.FAILURE;
        }
    }

    /*
     * Sends the dictionary request reject status message for a channel. 
     */
    int sendRequestReject(ReactorChannel chnl, int streamId, DictionaryRejectReason reason, ReactorErrorInfo errorInfo)
    {
        // get a buffer for the dictionary request reject status
        TransportBuffer msgBuf = chnl.getBuffer(MAX_DICTIONARY_STATUS_MSG_SIZE, false, errorInfo);

        if (msgBuf != null)
        {
            // encode dictionary request reject status 
            int ret = encodeDictionaryRequestReject(chnl, streamId, reason, msgBuf, errorInfo);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            // send request reject status 
            return chnl.submit(msgBuf, _submitOptions, errorInfo);
        }
        else
        {
            return CodecReturnCodes.FAILURE;
        }
    }

    /*
     * Encodes the dictionary request reject status. Returns success if encoding
     * succeeds or failure if encoding fails. 
     */
    private int encodeDictionaryRequestReject(ReactorChannel chnl, int streamId, DictionaryRejectReason reason, TransportBuffer msgBuf, ReactorErrorInfo errorInfo)
    {
        // clear encode iterator
        _encodeIter.clear();

        // set-up message
        _dictionaryStatus.clear();
        _dictionaryStatus.streamId(streamId);
        _dictionaryStatus.applyHasState();
        _dictionaryStatus.state().dataState(DataStates.SUSPECT);

        switch (reason)
        {
            case UNKNOWN_DICTIONARY_NAME:
                _dictionaryStatus.state().code(StateCodes.NOT_FOUND);
                _dictionaryStatus.state().streamState(StreamStates.CLOSED);
                _dictionaryStatus.state().text().data("Dictionary request rejected for stream id " + streamId + " - dictionary name unknown");
                break;
            case MAX_DICTIONARY_REQUESTS_REACHED:
                _dictionaryStatus.state().code(StateCodes.TOO_MANY_ITEMS);
                _dictionaryStatus.state().streamState(StreamStates.CLOSED_RECOVER);
                _dictionaryStatus.state().text().data("Dictionary request rejected for stream id " + streamId + " -  max request count reached");
                break;
            case DICTIONARY_RDM_DECODER_FAILED:
                _dictionaryStatus.state().code(StateCodes.USAGE_ERROR);
                _dictionaryStatus.state().text().data("Dictionary request rejected for stream id " + streamId + " - decoding failure: " + errorInfo.error().text());
                break;
            default:
                break;
        }

        // encode message
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        ret = _dictionaryStatus.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("DictionaryStatus.encode() failed");
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }


    /*
     * Sends a field dictionary response to a channel. This consists of getting
     * a message buffer, encoding the dictionary response, and sending the
     * dictionary response to the server. Returns success if send dictionary
     * response succeeds or failure if send response fails. 
     */
    int sendFieldDictionaryResponse(ReactorChannel chnl, DictionaryRequest dictionaryRequest, ReactorErrorInfo errorInfo)
    {
        _dictionaryRefresh.clear();

        _dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
        _dictionaryRefresh.streamId(dictionaryRequest.streamId());
        _dictionaryRefresh.dictionaryType(Dictionary.Types.FIELD_DEFINITIONS);
        _dictionaryRefresh.dictionary(_dictionary);
        _dictionaryRefresh.state().streamState(StreamStates.OPEN);
        _dictionaryRefresh.state().dataState(DataStates.OK);
        _dictionaryRefresh.state().code(StateCodes.NONE);
        _dictionaryRefresh.verbosity(dictionaryRequest.verbosity());
        _dictionaryRefresh.serviceId(dictionaryRequest.serviceId());
        _dictionaryRefresh.dictionaryName().data(dictionaryRequest.dictionaryName().data(),
                                                 dictionaryRequest.dictionaryName().position(), dictionaryRequest.dictionaryName().length());
        _dictionaryRefresh.applySolicited();
        
        boolean firstMultiPart = true;
        
        while (true)
        {
            // get a buffer for the dictionary response
            TransportBuffer msgBuf = chnl.getBuffer(MAX_FIELD_DICTIONARY_MSG_SIZE, false, errorInfo);
            if(msgBuf == null)
                return CodecReturnCodes.FAILURE;

            _dictionaryRefresh.state().text().data("Field Dictionary Refresh (starting fid " + _dictionaryRefresh.startFid() + ")");

            // clear encode iterator
            _encodeIter.clear();
            int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
            if (ret != CodecReturnCodes.SUCCESS)
            {
                errorInfo.error().text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
                return ret;
            }

            if (firstMultiPart)
            {
            	_dictionaryRefresh.applyClearCache();
            	firstMultiPart = false;
            }
            else
            	_dictionaryRefresh.flags( DictionaryRefreshFlags.SOLICITED );               
            
            // encode message
            ret = _dictionaryRefresh.encode(_encodeIter);
            if (ret < CodecReturnCodes.SUCCESS)
            {
                errorInfo.error().text("DictionaryRefresh.encode() failed");
                return ret;
            }

            // send dictionary response
            if (chnl.submit(msgBuf, _submitOptions, errorInfo) != TransportReturnCodes.SUCCESS)
                return CodecReturnCodes.FAILURE;

            // break out of loop when all dictionary responses sent
            if (ret == CodecReturnCodes.SUCCESS)
            {
                break;
            }

            // sleep between dictionary responses
            try
            {
                Thread.sleep(1);
            }
            catch (InterruptedException e)
            {
            }
        }
          
        return CodecReturnCodes.SUCCESS;
    }

    /*
     * Sends an enum type dictionary response to a channel. This consists of
     * getting a message buffer, encoding the dictionary response, and sending
     * the dictionary response to the server. Returns success if send dictionary
     * response succeeds or failure if send response fails. 
     */
    int sendEnumTypeDictionaryResponse(ReactorChannel chnl, DictionaryRequest dictionaryRequest, ReactorErrorInfo errorInfo)
    {
        _dictionaryRefresh.clear();
        
        _dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
        _dictionaryRefresh.streamId(dictionaryRequest.streamId());
        _dictionaryRefresh.dictionaryType(Dictionary.Types.ENUM_TABLES);
        _dictionaryRefresh.dictionary(_dictionary);
        _dictionaryRefresh.serviceId(dictionaryRequest.serviceId());
        _dictionaryRefresh.verbosity(dictionaryRequest.verbosity());
        _dictionaryRefresh.applySolicited();

        _dictionaryRefresh.state().streamState(StreamStates.OPEN);
        _dictionaryRefresh.state().dataState(DataStates.OK);
        _dictionaryRefresh.state().code(StateCodes.NONE);

        // dictionaryName
        _dictionaryRefresh.dictionaryName().data(dictionaryRequest.dictionaryName().data(),
                                                 dictionaryRequest.dictionaryName().position(), dictionaryRequest.dictionaryName().length());

        boolean firstMultiPart = true;
        
        while (true)
        {
            // get a buffer for the dictionary response
            TransportBuffer msgBuf = chnl.getBuffer(MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE, false, errorInfo);
            if (msgBuf == null)
                return CodecReturnCodes.FAILURE;        	
        	
        	_dictionaryRefresh.state().text().data("Enum Type Dictionary Refresh (starting enum " + _dictionaryRefresh.startEnumTableCount() + ")");

        	// clear encode iterator
	        _encodeIter.clear();
	        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
	        if (ret < CodecReturnCodes.SUCCESS)
	        {
	            errorInfo.error().text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
	            return ret;
	        }
	
            if (firstMultiPart)
            {
            	_dictionaryRefresh.applyClearCache();
            	firstMultiPart = false;
            }
            else
            	_dictionaryRefresh.flags( DictionaryRefreshFlags.SOLICITED );   	        
	        
	        // encode message
	        ret = _dictionaryRefresh.encode(_encodeIter);
	        if (ret < CodecReturnCodes.SUCCESS)
	        {
	            errorInfo.error().text("DictionaryRefresh.encode() failed");
	            return ret;
	        }
	
	        // send dictionary response
	        if (chnl.submit(msgBuf, _submitOptions, errorInfo) != TransportReturnCodes.SUCCESS)
	            return CodecReturnCodes.FAILURE;
	        
            // break out of loop when all dictionary responses sent
            if (ret == CodecReturnCodes.SUCCESS)
            {
                break;
            }

            // sleep between dictionary responses
            try
            {
                Thread.sleep(1);
            }
            catch (InterruptedException e)
            {
            }	        

        }

        return CodecReturnCodes.SUCCESS;
    }
    
    /*
     * Gets dictionary request information for the channel.
     */
    DictionaryRequestInfo getDictionaryRequestInfo(ReactorChannel reactorChannel, DictionaryRequest dictionaryRequest)
	{
		return _dictionaryRequestInfoList.get(reactorChannel.channel(), dictionaryRequest);
	}
}
