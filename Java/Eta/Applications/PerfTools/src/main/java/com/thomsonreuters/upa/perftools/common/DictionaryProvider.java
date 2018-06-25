package com.thomsonreuters.upa.perftools.common;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.Dictionary;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelInfo;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.transport.WritePriorities;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryStatus;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorSubmitOptions;

/**
 * Provides sending of the upajprovperf's dictionary, if requested.
 */
public class DictionaryProvider
{
	
	/**
	 * The Enum DictionaryRejectReason.
	 */
	public enum DictionaryRejectReason
	{
	    UNKNOWN_DICTIONARY_NAME,
	    MAX_DICTIONARY_REQUESTS_REACHED;
	}

    private static final int        MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE = 128000;
    private static final int        MAX_DICTIONARY_STATUS_MSG_SIZE = 1024;
    
    private static final String     FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
    private static final String     ENUM_TABLE_FILE_NAME = "enumtype.def";

    // field dictionary name
    private static final Buffer fieldDictionaryDownloadName;

    // enumtype dictionary name
    private static final Buffer enumTypeDictionaryDownloadName;
    
    static
    {
        fieldDictionaryDownloadName = CodecFactory.createBuffer();
        fieldDictionaryDownloadName.data("RWFFld");
        enumTypeDictionaryDownloadName = CodecFactory.createBuffer();
        enumTypeDictionaryDownloadName.data( "RWFEnum");
    }
    
    private DataDictionary          _dictionary;   
    
    private EncodeIterator          _encodeIter;

    private DictionaryRefresh       _dictionaryRefresh;
    private DictionaryRequest       _dictionaryRequest;
    private DictionaryStatus        _dictionaryStatus;
    private ChannelInfo             _chnlInfo;

    private ReactorErrorInfo      _errorInfo; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    private ReactorSubmitOptions  _reactorSubmitOptions; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    private ReactorChannelInfo    _reactorChnlInfo; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    
    /**
     * Instantiates a new dictionary provider.
     */
    public DictionaryProvider()
    {
        _encodeIter = CodecFactory.createEncodeIterator();
        _dictionary = CodecFactory.createDataDictionary();
        _dictionaryRefresh = (DictionaryRefresh)DictionaryMsgFactory.createMsg();
        _dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
        
        _dictionaryRequest = (DictionaryRequest)DictionaryMsgFactory.createMsg();
        _dictionaryRequest.rdmMsgType(DictionaryMsgType.REQUEST);
        
        _dictionaryStatus = (DictionaryStatus)DictionaryMsgFactory.createMsg();
        _dictionaryStatus.rdmMsgType(DictionaryMsgType.STATUS);
        _chnlInfo = TransportFactory.createChannelInfo();
        
        _errorInfo = ReactorFactory.createReactorErrorInfo();
        _reactorSubmitOptions = ReactorFactory.createReactorSubmitOptions();
        _reactorSubmitOptions.clear();
        _reactorSubmitOptions.writeArgs().priority(WritePriorities.HIGH);
        _reactorChnlInfo = ReactorFactory.createReactorChannelInfo();
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
            return false;
        }

        if (_dictionary.loadEnumTypeDictionary(ENUM_TABLE_FILE_NAME, error) < 0)
            return false;

        return true;
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
     * Processes a dictionary request.
     * 
     * @param channelHandler - Channel handler for the directory messages
     * @param clientChannelInfo - Information about client connected
     * @param msg - The partially decoded message
     * @param dIter - The decode iterator
     * @param error - Error information in case of failure
     * @return {@link PerfToolsReturnCodes#SUCCESS} for successful request
     *         processing, &lt; {@link PerfToolsReturnCodes#SUCCESS} when request
     */
    public int processMsg(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, Msg msg, DecodeIterator dIter, Error error)
    {
        switch (msg.msgClass())
        {
            case MsgClasses.REQUEST:
                int ret = _dictionaryRequest.decode(dIter, msg);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    error.text("DictionaryRequest.decode() failed with return code: " + CodecReturnCodes.toString(ret));
                    error.errorId(ret);
                    return PerfToolsReturnCodes.FAILURE;
                }

                System.out.println("Received Dictionary Request for DictionaryName: " + _dictionaryRequest.dictionaryName());
                if (fieldDictionaryDownloadName.equals(_dictionaryRequest.dictionaryName()))
                {
                    return sendFieldDictionaryResponse(channelHandler, clientChannelInfo, error);
                }
                else if (enumTypeDictionaryDownloadName.equals(_dictionaryRequest.dictionaryName()))
                {
                    return sendEnumTypeDictionaryResponse(channelHandler, clientChannelInfo, error);
                }
                else
                {
                    return sendRequestReject(channelHandler, clientChannelInfo, msg.streamId(), DictionaryRejectReason.UNKNOWN_DICTIONARY_NAME, error);
                }

            case MsgClasses.CLOSE:
                System.out.println("Received Dictionary Close for streamId " + msg.streamId());
                break;

            default:
                error.text("Received unhandled Source Directory msg type: " + msg.msgClass());
                error.errorId(PerfToolsReturnCodes.FAILURE);
                return PerfToolsReturnCodes.FAILURE;
        }

        return PerfToolsReturnCodes.SUCCESS;
    }

    /*
     * Sends the dictionary request reject status message for a channel. 
     * chnl - The channel to send request reject status message to 
     * streamId - The stream id of the request 
     * reason - The reason for the reject
     */
    private int sendRequestReject(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, int streamId, DictionaryRejectReason reason, Error error)
    {
        Channel channel = clientChannelInfo.channel;
        
        // get a buffer for the dictionary request reject status 
        TransportBuffer msgBuf = channel.getBuffer(MAX_DICTIONARY_STATUS_MSG_SIZE, false, error);
        if (msgBuf == null)
            return PerfToolsReturnCodes.FAILURE;
        
        // encode dictionary request reject status
        int ret = encodeDictionaryRequestReject(channel, streamId, reason, msgBuf, error);
        if (ret != CodecReturnCodes.SUCCESS)
            return PerfToolsReturnCodes.FAILURE;

        // send request reject status
        return channelHandler.writeChannel(clientChannelInfo, msgBuf, 0, error);

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

        // clear encode iterator 
        _encodeIter.clear();
        
        // encode message
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
        if (ret < CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIter.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            error.errorId(ret);
            return PerfToolsReturnCodes.FAILURE;
        }
        ret = _dictionaryStatus.encode(_encodeIter);

        if (ret < CodecReturnCodes.SUCCESS)
        {
            error.text("DictionaryStatus.encode() failed with return code: " + CodecReturnCodes.toString(ret));
            error.errorId(ret);
            return PerfToolsReturnCodes.FAILURE;
        }

        return PerfToolsReturnCodes.SUCCESS;
    }

    /**
     * Send request reject reactor.
     *
     * @param clientChannelInfo the client channel info
     * @param streamId the stream id
     * @param reason the reason
     * @param error the error
     * @return the int
     */
    /*
     * Sends the dictionary request reject status message for a reactor channel. 
     * chnl - The channel to send request reject status message to 
     * streamId - The stream id of the request 
     * reason - The reason for the reject
     */
    public int sendRequestRejectReactor(ClientChannelInfo clientChannelInfo, int streamId, DictionaryRejectReason reason, Error error)
    {
        ReactorChannel reactorChannel = clientChannelInfo.reactorChannel;
        
        // get a buffer for the dictionary request reject status 
        TransportBuffer msgBuf = reactorChannel.getBuffer(MAX_DICTIONARY_STATUS_MSG_SIZE, false, _errorInfo);
        if (msgBuf == null)
            return PerfToolsReturnCodes.FAILURE;
        
        // encode dictionary request reject status
        int ret = encodeDictionaryRequestReject(reactorChannel.channel(), streamId, reason, msgBuf, error);
        if (ret != CodecReturnCodes.SUCCESS)
            return PerfToolsReturnCodes.FAILURE;

        // send request reject status
        return reactorChannel.submit(msgBuf, _reactorSubmitOptions, _errorInfo);
    }

    /*
     * Sends a field dictionary response to a channel. This consists of getting
     * a message buffer, encoding the dictionary response, and sending the
     * dictionary response to the server. Returns success if send dictionary
     * response succeeds or failure if send response fails. 
     * channelHandler - The client channel handler.
     * clientChannelInfo - Client channel information to send dictionary response to.
     */
    private int sendFieldDictionaryResponse(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, Error error)
    {
        // set-up message
        _dictionaryRefresh.clear();
        _dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
        _dictionaryRefresh.streamId(_dictionaryRequest.streamId());
        _dictionaryRefresh.dictionaryType(Dictionary.Types.FIELD_DEFINITIONS);
        _dictionaryRefresh.dictionary(_dictionary);
        _dictionaryRefresh.state().streamState(StreamStates.OPEN);
        _dictionaryRefresh.state().dataState(DataStates.OK);
        _dictionaryRefresh.state().code(StateCodes.NONE);
        _dictionaryRefresh.verbosity(_dictionaryRequest.verbosity());
        _dictionaryRefresh.serviceId(_dictionaryRequest.serviceId());
        _dictionaryRefresh.dictionaryName().data(_dictionaryRequest.dictionaryName().data(), _dictionaryRequest.dictionaryName().position(), _dictionaryRequest.dictionaryName().length());
        _dictionaryRefresh.applySolicited();
        
        Channel channel = clientChannelInfo.channel;
        
        while (true)
        {
            int ret = channel.info(_chnlInfo, error);
            if (ret != TransportReturnCodes.SUCCESS)
            {
                return PerfToolsReturnCodes.FAILURE;
            }
            
            // get a buffer for the dictionary response
            TransportBuffer msgBuf = channel.getBuffer(_chnlInfo.maxFragmentSize(), false, error);
            if (msgBuf == null)
            {
                return PerfToolsReturnCodes.FAILURE;
            }

            _encodeIter.clear();
            ret = _encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
            if (ret != CodecReturnCodes.SUCCESS)
            {
                error.text("EncodeIter.setBufferAndRWFVersion() failed with return code: " + ret);
                error.errorId(ret);
                return PerfToolsReturnCodes.FAILURE;
            }
            
            _dictionaryRefresh.state().text().data("Field Dictionary Refresh (starting fid " + _dictionaryRefresh.startFid() + ")");
            ret = _dictionaryRefresh.encode(_encodeIter);
            if (ret < CodecReturnCodes.SUCCESS)
            {
                error.text("DictionaryRefresh.encode() failed");
                error.errorId(ret);
                return PerfToolsReturnCodes.FAILURE;
            }

            // send dictionary response
            if(channelHandler.writeChannel(clientChannelInfo, msgBuf, 0, error) < TransportReturnCodes.SUCCESS)
                return PerfToolsReturnCodes.FAILURE;

            // break out of loop when all dictionary responses sent
            if (ret == TransportReturnCodes.SUCCESS)
            {
                break;
            }
        }

        return PerfToolsReturnCodes.SUCCESS;
    }

    /**
     * Send field dictionary response reactor.
     *
     * @param clientChannelInfo the client channel info
     * @param error the error
     * @return the int
     */
    /*
     * Sends a field dictionary response to a reactor channel. This consists of getting
     * a message buffer, encoding the dictionary response, and sending the
     * dictionary response to the server. Returns success if send dictionary
     * response succeeds or failure if send response fails. 
     * clientChannelInfo - Client channel information to send dictionary response to.
     */
    public int sendFieldDictionaryResponseReactor(ClientChannelInfo clientChannelInfo, Error error)
    {
        // set-up message
        _dictionaryRefresh.clear();
        _dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
        _dictionaryRefresh.streamId(_dictionaryRequest.streamId());
        _dictionaryRefresh.dictionaryType(Dictionary.Types.FIELD_DEFINITIONS);
        _dictionaryRefresh.dictionary(_dictionary);
        _dictionaryRefresh.state().streamState(StreamStates.OPEN);
        _dictionaryRefresh.state().dataState(DataStates.OK);
        _dictionaryRefresh.state().code(StateCodes.NONE);
        _dictionaryRefresh.verbosity(_dictionaryRequest.verbosity());
        _dictionaryRefresh.serviceId(_dictionaryRequest.serviceId());
        _dictionaryRefresh.dictionaryName().data(_dictionaryRequest.dictionaryName().data(), _dictionaryRequest.dictionaryName().position(), _dictionaryRequest.dictionaryName().length());
        _dictionaryRefresh.applySolicited();
        
        ReactorChannel reactorChannel = clientChannelInfo.reactorChannel;
        
        while (true)
        {
            int ret = reactorChannel.info(_reactorChnlInfo, _errorInfo);
            if (ret != TransportReturnCodes.SUCCESS)
            {
                return PerfToolsReturnCodes.FAILURE;
            }
            
            // get a buffer for the dictionary response
            TransportBuffer msgBuf = reactorChannel.getBuffer(_reactorChnlInfo.channelInfo().maxFragmentSize(), false, _errorInfo);
            if (msgBuf == null)
            {
                return PerfToolsReturnCodes.FAILURE;
            }

            _encodeIter.clear();
            ret = _encodeIter.setBufferAndRWFVersion(msgBuf, reactorChannel.majorVersion(), reactorChannel.minorVersion());
            if (ret != CodecReturnCodes.SUCCESS)
            {
                error.text("EncodeIter.setBufferAndRWFVersion() failed with return code: " + ret);
                error.errorId(ret);
                return PerfToolsReturnCodes.FAILURE;
            }
            
            _dictionaryRefresh.state().text().data("Field Dictionary Refresh (starting fid " + _dictionaryRefresh.startFid() + ")");
            ret = _dictionaryRefresh.encode(_encodeIter);
            if (ret < CodecReturnCodes.SUCCESS)
            {
                error.text("DictionaryRefresh.encode() failed");
                error.errorId(ret);
                return PerfToolsReturnCodes.FAILURE;
            }

            // send dictionary response
            if(reactorChannel.submit(msgBuf, _reactorSubmitOptions, _errorInfo) < ReactorReturnCodes.SUCCESS)
                return PerfToolsReturnCodes.FAILURE;

            // break out of loop when all dictionary responses sent
            if (ret == ReactorReturnCodes.SUCCESS)
            {
                break;
            }
        }

        return PerfToolsReturnCodes.SUCCESS;
    }

    /*
     * Sends a enum dictionary response to a channel. This consists of getting
     * a message buffer, encoding the dictionary response, and sending the
     * dictionary response to the server. Returns success if send dictionary
     * response succeeds or failure if send response fails. 
     * channelHandler - The client channel handler.
     * clientChannelInfo - Client channel information to send dictionary response to.
     */
    private int sendEnumTypeDictionaryResponse(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, Error error)
    {
        Channel channel = clientChannelInfo.channel;
        
        // get a buffer for the dictionary response
        TransportBuffer msgBuf = channel.getBuffer(MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE, false, error);
        if (msgBuf == null)
            return PerfToolsReturnCodes.FAILURE;

        //encode dictionary refresh - enum type
        _dictionaryRefresh.clear();
        _dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
        _dictionaryRefresh.streamId(_dictionaryRequest.streamId());
        _dictionaryRefresh.applySolicited();
        _dictionaryRefresh.dictionaryType(Dictionary.Types.ENUM_TABLES);
        _dictionaryRefresh.dictionary(_dictionary);
        _dictionaryRefresh.verbosity(_dictionaryRequest.verbosity());
        _dictionaryRefresh.dictionaryName().data(_dictionaryRequest.dictionaryName().data(), _dictionaryRequest.dictionaryName().position(), _dictionaryRequest.dictionaryName().length());
        _dictionaryRefresh.applyRefreshComplete();

        _encodeIter.clear();
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIter.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            error.errorId(ret);
            return PerfToolsReturnCodes.FAILURE;
        }

        // encode message
        ret = _dictionaryRefresh.encode(_encodeIter);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            error.text("DictionaryRefresh.encode() failed");
            error.errorId(ret);
            return PerfToolsReturnCodes.FAILURE;
        }

        // send dictionary response
        ret = channelHandler.writeChannel(clientChannelInfo, msgBuf, 0, error);
        if (ret < TransportReturnCodes.SUCCESS)
            return PerfToolsReturnCodes.FAILURE;

        return PerfToolsReturnCodes.SUCCESS;
    }

    /**
     * Send enum type dictionary response reactor.
     *
     * @param clientChannelInfo the client channel info
     * @param error the error
     * @return the int
     */
    /*
     * Sends a enum dictionary response to a reactor channel. This consists of getting
     * a message buffer, encoding the dictionary response, and sending the
     * dictionary response to the server. Returns success if send dictionary
     * response succeeds or failure if send response fails. 
     * clientChannelInfo - Client channel information to send dictionary response to.
     */
    public int sendEnumTypeDictionaryResponseReactor(ClientChannelInfo clientChannelInfo, Error error)
    {
        ReactorChannel reactorChannel = clientChannelInfo.reactorChannel;
        
        // get a buffer for the dictionary response
        TransportBuffer msgBuf = reactorChannel.getBuffer(MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE, false, _errorInfo);
        if (msgBuf == null)
            return PerfToolsReturnCodes.FAILURE;

        //encode dictionary refresh - enum type
        _dictionaryRefresh.clear();
        _dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
        _dictionaryRefresh.streamId(_dictionaryRequest.streamId());
        _dictionaryRefresh.applySolicited();
        _dictionaryRefresh.dictionaryType(Dictionary.Types.ENUM_TABLES);
        _dictionaryRefresh.dictionary(_dictionary);
        _dictionaryRefresh.verbosity(_dictionaryRequest.verbosity());
        _dictionaryRefresh.dictionaryName().data(_dictionaryRequest.dictionaryName().data(), _dictionaryRequest.dictionaryName().position(), _dictionaryRequest.dictionaryName().length());
        _dictionaryRefresh.applyRefreshComplete();

        _encodeIter.clear();
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, reactorChannel.majorVersion(), reactorChannel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIter.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            error.errorId(ret);
            return PerfToolsReturnCodes.FAILURE;
        }

        // encode message
        ret = _dictionaryRefresh.encode(_encodeIter);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            error.text("DictionaryRefresh.encode() failed");
            error.errorId(ret);
            return PerfToolsReturnCodes.FAILURE;
        }

        // send dictionary response
        ret = reactorChannel.submit(msgBuf, _reactorSubmitOptions, _errorInfo);
        if (ret < TransportReturnCodes.SUCCESS)
            return PerfToolsReturnCodes.FAILURE;

        return PerfToolsReturnCodes.SUCCESS;
    }

    /**
     * Field dictionary download name.
     *
     * @return the buffer
     */
    /* Returns fieldDictionaryDownloadName. */
    public Buffer fieldDictionaryDownloadName()
    {
        return fieldDictionaryDownloadName;
    }
    
    /**
     * Enum type dictionary download name.
     *
     * @return the buffer
     */
    /* Returns enumTypeDictionaryDownloadName. */
    public Buffer enumTypeDictionaryDownloadName()
    {
        return enumTypeDictionaryDownloadName;
    }

    /**
     * Dictionary request.
     *
     * @return the dictionary request
     */
    /* Returns dictionaryRequest. */
    public DictionaryRequest dictionaryRequest()
    {
        return _dictionaryRequest;
    }
}