package com.refinitiv.eta.examples.common;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.rdm.Dictionary;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.shared.DictionaryRejectReason;
import com.refinitiv.eta.shared.DictionaryRequestInfo;
import com.refinitiv.eta.shared.DictionaryRequestInfoList;
import com.refinitiv.eta.shared.ProviderSession;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryClose;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRefreshFlags;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryStatus;

/**
 * This is the dictionary handler for both the UPA - Java Interactive Provider
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
public class ProviderDictionaryHandler
{
    private static final int MAX_FIELD_DICTIONARY_MSG_SIZE = 8192;
    private static final int MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE = 12800;
    private static final int MAX_DICTIONARY_STATUS_MSG_SIZE = 1024;
    
    private static final String FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
    private static final String ENUM_TYPE_FILE_NAME = "enumtype.def";

    private static final Buffer FIELD_DICTIONARY_DOWNLOAD_NAME;
    private static final Buffer ENUM_TYPE_DOWNLOAD_NAME;

    private static final int FIELD_DICTIONARY_STREAM_ID = -1;
    private static final int ENUM_TYPE_DICTIONARY_STREAM_ID = -2;
    
	boolean _bFieldDictionaryLoadedFromFile = false;
	boolean _bEnumDictionaryLoadedFromFile = false;
	
	boolean _bFieldDictionaryDownloaded = false;
	boolean _bEnumDictionaryDownloaded = false;
		
    private ProviderSession _providerSession = null;
    private DataDictionary _dictionary;
    private DictionaryRequestInfoList _dictionaryRequestInfoList;
    
    private EncodeIterator _encodeIter = CodecFactory.createEncodeIterator();
    private DictionaryRefresh _dictionaryRefresh = (DictionaryRefresh)DictionaryMsgFactory.createMsg();
    private DictionaryStatus _dictionaryStatus = (DictionaryStatus)DictionaryMsgFactory.createMsg();
    private DictionaryRequest _dictionaryRequest = (DictionaryRequest)DictionaryMsgFactory.createMsg();
    private DictionaryClose _dictionaryClose;
    
    private State[] states; // first=field dictionary state
							// second=enum dictionary state

    static
    {
    	FIELD_DICTIONARY_DOWNLOAD_NAME = CodecFactory.createBuffer();
    	FIELD_DICTIONARY_DOWNLOAD_NAME.data("RWFFld");
        
        ENUM_TYPE_DOWNLOAD_NAME = CodecFactory.createBuffer();
        ENUM_TYPE_DOWNLOAD_NAME.data( "RWFEnum");
    }
    
    /**
     * Instantiates a new provider dictionary handler.
     *
     * @param providerSesssion the provider sesssion
     */
    public ProviderDictionaryHandler(ProviderSession providerSesssion)
    {
        _dictionary = CodecFactory.createDataDictionary();
        _providerSession = providerSesssion;
        _dictionaryRequestInfoList = new DictionaryRequestInfoList();
        _dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
        _dictionaryStatus.rdmMsgType(DictionaryMsgType.STATUS);
        _dictionaryRequest.rdmMsgType(DictionaryMsgType.REQUEST);
    
        states = new State[2];
        states[0] = CodecFactory.createState();
        states[1] = CodecFactory.createState();
   }

    /**
     * Initializes dictionary information fields.
     */
    public void init()
    {
        _dictionaryRequestInfoList.init();
    }

    /**
     * Returns the data dictionary.
     *
     * @return the data dictionary
     */
    public DataDictionary dictionary()
    {
        return _dictionary;
    }

    /**
     * Calls the method to delete the dictionary, freeing all memory
     * associated with it.
     */
    public void freeDictionary()
    {
        _dictionary.clear();
    }

    /**
     * Loads dictionary files.
     *
     * @param error the error
     * @return true, if successful
     */
    public boolean loadDictionary(Error error)
    {
        _dictionary.clear();
        if (_dictionary.loadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, error) < 0)
        {
        	System.out.format("Unable to load field dictionary: %s.Error Text: %s\n",FIELD_DICTIONARY_FILE_NAME,error.text() );
            return false;
        }
        else
        {
        	_bFieldDictionaryLoadedFromFile = true;
        }

        if (_dictionary.loadEnumTypeDictionary(ENUM_TYPE_FILE_NAME, error) < 0)
        {
        	System.out.format("Unable to load enum dictionary: %s.Error Text: %s\n",ENUM_TYPE_FILE_NAME,error.text() );
            return false;
        }
        else
        {
        	_bEnumDictionaryLoadedFromFile = true;
        }
        
        return true;
    }

    /**
     * Indicates if the dictionary has been loaded.
     *
     * @return true, if is dictionary ready
     */
    public boolean isDictionaryReady() 
	{
		if( _bFieldDictionaryLoadedFromFile == true && _bEnumDictionaryLoadedFromFile == true )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
    
    /**
     * Send dictionary requests.
     *
     * @param chnl the chnl
     * @param error the error
     * @param serviceId the service id
     * @return the int
     */
    public int sendDictionaryRequests(Channel chnl, Error error, int serviceId)
	{
		if( _dictionaryRequest == null )
		{
			_dictionaryRequest = (DictionaryRequest)DictionaryMsgFactory.createMsg();
			_dictionaryRequest.rdmMsgType(DictionaryMsgType.REQUEST);
		}
		
		int sendStatus = requestDictionary(chnl,error,serviceId,FIELD_DICTIONARY_STREAM_ID,FIELD_DICTIONARY_DOWNLOAD_NAME.toString());
		if( sendStatus == CodecReturnCodes.SUCCESS)
		{
			sendStatus = requestDictionary(chnl,error,serviceId,ENUM_TYPE_DICTIONARY_STREAM_ID,ENUM_TYPE_DOWNLOAD_NAME.toString());
		}
		
		return sendStatus;
	}
	
	int requestDictionary(Channel chnl, Error error, int serviceId,int streamId,String dictName)
	{
		/* get a buffer for the dictionary request */
		TransportBuffer msgBuf = chnl.getBuffer(ChannelSession.MAX_MSG_SIZE, false, error);
		if (msgBuf == null)
		{
			return CodecReturnCodes.FAILURE;
		}

		/* encode dictionary request */
		_dictionaryRequest.clear();

		_dictionaryRequest.dictionaryName().data(dictName);
		_dictionaryRequest.streamId(streamId);
		_dictionaryRequest.serviceId(serviceId);
		_dictionaryRequest.verbosity(Dictionary.VerbosityValues.NORMAL);

		_encodeIter.clear();

		int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
		if (ret != CodecReturnCodes.SUCCESS)
		{
			error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
			return ret;
		}

		ret = _dictionaryRequest.encode(_encodeIter);
		if (ret != CodecReturnCodes.SUCCESS)
		{
			error.text("encodeDictionaryRequest(): Failed <code: " + error.errorId() + ">");
			return ret;
		}

		System.out.println(_dictionaryRequest.toString());

		//send request
		return _providerSession.write(chnl, msgBuf, error);
	}


    /**
     * Processes a dictionary request. This consists of calling
     * decodeDictionaryRequest() to decode the request and calling
     * sendDictionaryResonse() to send the dictionary response. Returns success
     * if dictionary request processing succeeds or failure if processing fails.
     *
     * @param chnl - The channel of the request msg - The partially decoded
     *            message
     * @param msg the msg
     * @param dIter - The decode iterator
     * @param error the error
     * @return {@link CodecReturnCodes}
     */
    public int processRequest(Channel chnl, Msg msg, DecodeIterator dIter, Error error)
    {
    	// decode dictionary request
    	_dictionaryRequest.clear();
    	int ret = _dictionaryRequest.decode(dIter, msg);
    	if (ret != CodecReturnCodes.SUCCESS)
    	{
    		error.text("decodeDictionaryRequest() failed with return code: " + ret);
    		return ret;
    	}

    	DictionaryRequestInfo dictionaryRequestInfo = _dictionaryRequestInfoList.get(chnl, _dictionaryRequest);
    	if (dictionaryRequestInfo == null)
    	{
    		return sendRequestReject(chnl, msg.streamId(), DictionaryRejectReason.MAX_DICTIONARY_REQUESTS_REACHED, error);
    	}

    	System.out.println("Received Dictionary Request for DictionaryName: " + dictionaryRequestInfo.dictionaryRequest.dictionaryName());
    	if (FIELD_DICTIONARY_DOWNLOAD_NAME.equals(dictionaryRequestInfo.dictionaryRequest.dictionaryName()))
    	{
    		// send field dictionary refresh message 
    		ret = sendFieldDictionaryResponse(chnl, dictionaryRequestInfo, error);
    		if (ret != CodecReturnCodes.SUCCESS)
    		{
    			return ret;
    		}
    	}
    	else if (ENUM_TYPE_DOWNLOAD_NAME.equals(dictionaryRequestInfo.dictionaryRequest.dictionaryName()))
    	{
    		// send enum dictionary refresh message
    		ret = sendEnumTypeDictionaryResponse(chnl, dictionaryRequestInfo, error);
    		if (ret != CodecReturnCodes.SUCCESS)
    		{
    			return ret;
    		}
    	}
    	else
    	{
    		ret = sendRequestReject(chnl, msg.streamId(), DictionaryRejectReason.UNKNOWN_DICTIONARY_NAME, error);
    		return ret;
    	}

        return CodecReturnCodes.SUCCESS;
    }

	/**
	 * Process message.
	 *
	 * @param chnl the chnl
	 * @param msg the msg
	 * @param dIter the d iter
	 * @param error the error
	 * @return the int
	 */
	public int processMessage(Channel chnl,Msg msg, DecodeIterator dIter, Error error)
	{
        switch (msg.msgClass())
        {
        	case MsgClasses.REQUEST:             
        		return processRequest(chnl,msg, dIter, error);

        	case MsgClasses.REFRESH:             
                return processRefresh(msg, dIter, error);
                
            case MsgClasses.CLOSE:
                System.out.println("Received Dictionary Close for StreamId " + msg.streamId());

                // close dictionary stream
                closeStream(msg.streamId());
                return CodecReturnCodes.SUCCESS;

            case MsgClasses.STATUS:
                System.out.println("Received StatusMsg for dictionary");
                StatusMsg statusMsg = (StatusMsg)msg;
                if (statusMsg.checkHasState())
                {
                    System.out.println("   " + statusMsg.state());
                    State state = statusMsg.state();
                    if (msg.streamId() == FIELD_DICTIONARY_STREAM_ID)
                    {
                    	states[0].dataState(state.dataState());
                    	states[0].streamState(state.streamState());
                    }
                    else if (msg.streamId() == ENUM_TYPE_DICTIONARY_STREAM_ID)
                    {
                    	states[1].dataState(state.dataState());
                    	states[1].streamState(state.streamState());
                    }
                }                
                return CodecReturnCodes.SUCCESS;

            default:
                System.out.println("Received Unhandled Dictionary MsgClass: " + msg.msgClass());
                return CodecReturnCodes.FAILURE;
        }
	}

	int processRefresh(Msg msg, DecodeIterator dIter, com.refinitiv.eta.transport.Error error)
	{
		int ret = _dictionaryRefresh.decode(dIter, msg);
		if (ret != CodecReturnCodes.SUCCESS)
		{
			error.text("Error decoding dictionary refresh: <" + CodecReturnCodes.toString(ret) + ">");
			return ret;
		}

		if (_dictionaryRefresh.checkHasInfo())
		{
			switch (_dictionaryRefresh.dictionaryType())
			{
			case Dictionary.Types.FIELD_DEFINITIONS:
				break;
			case Dictionary.Types.ENUM_TABLES:
				break;
			default:
				error.text("Received unexpected dictionary message on stream " +msg.streamId());
				return CodecReturnCodes.FAILURE;
			}
		}

		if ( _dictionaryRefresh.streamId() == FIELD_DICTIONARY_STREAM_ID )
		{
			System.out.println("Received Dictionary Refresh for field dictionary");
			
			RefreshMsg refreshMsg = (RefreshMsg)msg;

			states[0].dataState(refreshMsg.state().dataState());
			states[0].streamState(refreshMsg.state().streamState()); 			
			
			ret = _dictionary.decodeFieldDictionary(dIter, Dictionary.VerbosityValues.VERBOSE, error);
			if (ret != CodecReturnCodes.SUCCESS)
			{
				return ret;
			}

			if (_dictionaryRefresh.checkRefreshComplete())
			{
				_bFieldDictionaryDownloaded = true;              
				if ( ! _bEnumDictionaryDownloaded )
					System.out.println("Field Dictionary complete, waiting for Enum Table...");
			}
		}
		else if ( _dictionaryRefresh.streamId() == ENUM_TYPE_DICTIONARY_STREAM_ID )
		{
			System.out.println("Received Dictionary Refresh for enum type");
			
			RefreshMsg refreshMsg = (RefreshMsg)msg;

			states[1].dataState(refreshMsg.state().dataState());
			states[1].streamState(refreshMsg.state().streamState());        	
			
			ret = _dictionary.decodeEnumTypeDictionary(dIter, Dictionary.VerbosityValues.VERBOSE, error);
			if (ret != CodecReturnCodes.SUCCESS)
			{
				return ret;
			}

			if (_dictionaryRefresh.checkRefreshComplete())
			{
				_bEnumDictionaryDownloaded = true;
				if ( ! _bFieldDictionaryDownloaded )
					System.out.println("Enumerated Types Dictionary complete, waiting for Field Dictionary...");
			}
		}
		else
		{
			error.text("Received unexpected dictionary message on stream " +
					msg.streamId());
			return CodecReturnCodes.FAILURE;
		}

		if ( _bFieldDictionaryDownloaded && _bEnumDictionaryDownloaded )
			System.out.println("Dictionary Download complete for both field and enum type dictionaries");

		return CodecReturnCodes.SUCCESS;
	}

    /*
     * Closes a dictionary stream. streamId - The stream id to close the
     * dictionary for
     */
    private void closeStream(int streamId)
    {
        // find original request information associated with streamId
        for (DictionaryRequestInfo dictionraryReqInfo : _dictionaryRequestInfoList)
        {
            if (dictionraryReqInfo.dictionaryRequest.streamId() == streamId && dictionraryReqInfo.isInUse)
            {
                // clear original request information
                System.out.println("Closing dictionary stream id " + dictionraryReqInfo.dictionaryRequest.streamId() + " with dictionary name: " + dictionraryReqInfo.dictionaryRequest.dictionaryName());
                dictionraryReqInfo.clear();
                break;
            }
        }
    }

    /**
     * Closes all open dictionary streams for a channel.
     *  
     * @param chnl  - The channel to close the dictionary streams for
     */
    public void closeRequests(Channel chnl)
    {
        // find original request information associated with chnl
        for (DictionaryRequestInfo dictionraryReqInfo : _dictionaryRequestInfoList)
        {
            if (dictionraryReqInfo.channel == chnl && dictionraryReqInfo.isInUse)
            {
                // clear original request information
                System.out.println("Closing dictionary stream id  " + dictionraryReqInfo.dictionaryRequest.streamId() + " with dictionary name: " + dictionraryReqInfo.dictionaryRequest.dictionaryName());
                dictionraryReqInfo.clear();
            }
        }
    }

    /**
     * Sends the dictionary close status message(s) for a channel. This consists
     * of finding all request information for this channel and sending the close
     * status messages to the channel.
     *
     * @param chnl - The channel to send close status message(s) to
     * @param error the error
     * @return {@link CodecReturnCodes}
     */
    public int sendCloseStatusMsgs(Channel chnl, Error error)
    {
        int ret = 0;
        for (DictionaryRequestInfo dictionraryReqInfo : _dictionaryRequestInfoList)
        {
            if (dictionraryReqInfo.isInUse && dictionraryReqInfo.channel == chnl)
            {
                ret = sendCloseStatusMsg(chnl, dictionraryReqInfo.dictionaryRequest.streamId(), error);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }

        return CodecReturnCodes.SUCCESS;
    }

    /**
     * Sends the dictionary close status message for a channel. Returns success
     * if send dictionary close status succeeds or failure if it fails.
     *
     * @param chnl - The channel to send close status message to
     * @param streamId - The stream id of the close status
     * @param error the error
     * @return {@link CodecReturnCodes}
     */
    public int sendCloseStatusMsg(Channel chnl, int streamId, Error error)
    {
        // get a buffer for the dictionary close status 
        TransportBuffer msgBuf = chnl.getBuffer(MAX_DICTIONARY_STATUS_MSG_SIZE, false, error);

        if (msgBuf != null)
        {
            // encode directory close
            _encodeIter.clear();
            int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
            if (ret != CodecReturnCodes.SUCCESS)
            {
                error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
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
            return _providerSession.write(chnl, msgBuf, error);
        }
        else
        {
            return CodecReturnCodes.FAILURE;
        }
    }

    /*
     * Sends the dictionary request reject status message for a channel. 
     * chnl - The channel to send request reject status message to 
     * streamId - The stream id of the request 
     * reason - The reason for the reject
     */
    private int sendRequestReject(Channel chnl, int streamId, DictionaryRejectReason reason, Error error)
    {
        // get a buffer for the dictionary request reject status
        TransportBuffer msgBuf = chnl.getBuffer(MAX_DICTIONARY_STATUS_MSG_SIZE, false, error);

        if (msgBuf != null)
        {
            // encode dictionary request reject status 
            int ret = encodeDictionaryRequestReject(chnl, streamId, reason, msgBuf, error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            // send request reject status 
            return _providerSession.write(chnl, msgBuf, error);
        }
        else
        {
            return CodecReturnCodes.FAILURE;
        }
    }

    /*
     * Encodes the dictionary request reject status. Returns success if encoding
     * succeeds or failure if encoding fails. 
     * chnl - The channel to send request reject status message to 
     * streamId - The stream id of the request 
     * reason - The reason for the reject 
     * msgBuf - The message buffer to encode the dictionary request reject into
     */
    private int encodeDictionaryRequestReject(Channel chnl, int streamId, DictionaryRejectReason reason, TransportBuffer msgBuf, Error error)
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
     * chnl - The channel to send a dictionary response to.
     * dictionaryReqInfo - The dictionary request information.
     */
    private int sendFieldDictionaryResponse(Channel chnl, DictionaryRequestInfo dictionaryReqInfo, Error error)
    {
        _dictionaryRefresh.clear();

        _dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
        _dictionaryRefresh.streamId(dictionaryReqInfo.dictionaryRequest.streamId());
        _dictionaryRefresh.dictionaryType(Dictionary.Types.FIELD_DEFINITIONS);
        _dictionaryRefresh.dictionary(_dictionary);
        _dictionaryRefresh.state().streamState(StreamStates.OPEN);
        _dictionaryRefresh.state().dataState(DataStates.OK);
        _dictionaryRefresh.state().code(StateCodes.NONE);
        _dictionaryRefresh.verbosity(dictionaryReqInfo.dictionaryRequest.verbosity());
        _dictionaryRefresh.serviceId(dictionaryReqInfo.dictionaryRequest.serviceId());
        _dictionaryRefresh.dictionaryName().data(dictionaryReqInfo.dictionaryRequest.dictionaryName().data());
        _dictionaryRefresh.applySolicited();
        
        boolean firstMultiPart = true;
        
        while (true)
        {
            // get a buffer for the dictionary response
            TransportBuffer msgBuf = chnl.getBuffer(MAX_FIELD_DICTIONARY_MSG_SIZE, false, error);
            if(msgBuf == null)
                return CodecReturnCodes.FAILURE;

            _dictionaryRefresh.state().text().data("Field Dictionary Refresh (starting fid " + _dictionaryRefresh.startFid() + ")");

            // clear encode iterator
            _encodeIter.clear();
            int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
            if (ret != CodecReturnCodes.SUCCESS)
            {
                error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
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
                error.text("DictionaryRefresh.encode() failed");
                return ret;
            }

            // send dictionary response
            if (_providerSession.write(chnl, msgBuf, error) != TransportReturnCodes.SUCCESS)
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
     * chnl - The channel to send a dictionary response to 
     * dictionaryReqInfo - The dictionary request information
     */
    private int sendEnumTypeDictionaryResponse(Channel chnl, DictionaryRequestInfo dictionaryReqInfo, Error error)
    {
        _dictionaryRefresh.clear();
        
        _dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
        _dictionaryRefresh.streamId(dictionaryReqInfo.dictionaryRequest.streamId());
        _dictionaryRefresh.dictionaryType(Dictionary.Types.ENUM_TABLES);
        _dictionaryRefresh.dictionary(_dictionary);
        _dictionaryRefresh.serviceId(dictionaryReqInfo.dictionaryRequest.serviceId());
        _dictionaryRefresh.verbosity(dictionaryReqInfo.dictionaryRequest.verbosity());
        _dictionaryRefresh.applySolicited();

        _dictionaryRefresh.state().streamState(StreamStates.OPEN);
        _dictionaryRefresh.state().dataState(DataStates.OK);
        _dictionaryRefresh.state().code(StateCodes.NONE);
        _dictionaryRefresh.state().text().data("Enum Type Dictionary Refresh");

        boolean firstMultiPart = true;        
        
        // dictionaryName
        _dictionaryRefresh.dictionaryName().data(dictionaryReqInfo.dictionaryRequest.dictionaryName().data());
        
        while (true)
        {
            // get a buffer for the dictionary response
            TransportBuffer msgBuf = chnl.getBuffer(MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE, false, error);
            if (msgBuf == null)
                return CodecReturnCodes.FAILURE;        	
        	
            _dictionaryRefresh.state().text().data("Enum Type Dictionary Refresh (starting enum " + _dictionaryRefresh.startEnumTableCount() + ")");
            
	        // clear encode iterator
	        _encodeIter.clear();
	        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
	        if (ret < CodecReturnCodes.SUCCESS)
	        {
	            error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
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
	            error.text("DictionaryRefresh.encode() failed");
	            return ret;
	        }
	
	        // send dictionary response
	        if (_providerSession.write(chnl, msgBuf, error) != TransportReturnCodes.SUCCESS)
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
    

    /**
     * Close the dictionary stream if there is one.
     * 
     * @return true if stream is closed
	 *
     * @param chnl - The channel to send a dictionary close to
     * @param streamId - The stream id of the dictionary stream to close
     * @param error - The error 
     */
    private boolean closeDictionary(Channel chnl, int streamId, Error error)
    {
		if( _dictionaryClose == null )
		{
			_dictionaryClose = (DictionaryClose)DictionaryMsgFactory.createMsg();
			_dictionaryClose.rdmMsgType(DictionaryMsgType.CLOSE);
		}
		
        /* get a buffer for the dictionary close */
        TransportBuffer msgBuf = chnl.getBuffer(ChannelSession.MAX_MSG_SIZE, false, error);
        if (msgBuf == null)
        {
            return false;
        }

        _encodeIter.clear();
        _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());

        /* encode dictionary close */
        _dictionaryClose.clear();
        _dictionaryClose.streamId(streamId);
        
        int ret = _dictionaryClose.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("encodeDictionaryClose(): Failed <code: " + CodecReturnCodes.toString(ret) + ">");
            return false;
        }

        /* send close */
        ret = _providerSession.write(chnl, msgBuf, error);
        if (ret != TransportReturnCodes.SUCCESS)
        {
            return false;
        }

        _dictionary.clear();
        
        return true;
    }

    /**
     * Close stream.
     *
     * @param clientChannel the client channel
     * @param error the error
     */
    public void closeStream(Channel clientChannel, Error error)
    {
    	/* close dictionary stream */
    	if( _bFieldDictionaryDownloaded )
    	{
    		/*
    		 * we only want to close a stream if it was not already closed (e.g.
    		 * rejected by provider, closed via refresh or status)
    		 */
    		if (states[0].dataState() == DataStates.OK && states[0].streamState() == StreamStates.NON_STREAMING )
    		{
    			if( closeDictionary(clientChannel,FIELD_DICTIONARY_STREAM_ID, error) == true)
    			{
    				_bFieldDictionaryDownloaded = false;
    				states[0].clear();
    			}
    		}
    	}

    	if( _bEnumDictionaryDownloaded )
    	{
    		/*
    		 * we only want to close a stream if it was not already closed (e.g.
    		 * rejected by provider, closed via refresh or status)
    		 */
    		if (states[1].dataState() == DataStates.OK && states[1].streamState() == StreamStates.NON_STREAMING )
    		{
    			if( closeDictionary(clientChannel,ENUM_TYPE_DICTIONARY_STREAM_ID, error) == true )
    			{
    				_bEnumDictionaryDownloaded = false;
    				states[1].clear();
    			}
    		}
    	}
    }
}
