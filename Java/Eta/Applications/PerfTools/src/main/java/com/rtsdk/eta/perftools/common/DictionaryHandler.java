package com.rtsdk.eta.perftools.common;

import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DataDictionary;
import com.rtsdk.eta.codec.DataStates;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.Msg;
import com.rtsdk.eta.codec.MsgClasses;
import com.rtsdk.eta.codec.State;
import com.rtsdk.eta.codec.StatusMsg;
import com.rtsdk.eta.codec.StreamStates;
import com.rtsdk.eta.rdm.Dictionary;
import com.rtsdk.eta.transport.Channel;
import com.rtsdk.eta.transport.Error;
import com.rtsdk.eta.transport.TransportBuffer;
import com.rtsdk.eta.transport.TransportFactory;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryClose;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequest;

/**
 * This is the dictionary handler for the UPA consumer application. It provides
 * methods for loading the field/enumType dictionaries from a file and sending
 * requests for those dictionaries to a provider. Methods for processing the
 * dictionary response and closing a dictionary stream are also provided.
 */
public class DictionaryHandler
{
    public final int FIELD_DICTIONARY_TYPE = 1;
    public final int ENUM_DICTIONARY_TYPE = 2;

    public final int FIELD_DICTIONARY_STREAM_ID = 3;
    public final int ENUM_TYPE_DICTIONARY_STREAM_ID = 4;

    public static final int MAX_MSG_SIZE = 1024;
    public static int TRANSPORT_BUFFER_SIZE_REQUEST = MAX_MSG_SIZE;
    public static int TRANSPORT_BUFFER_SIZE_CLOSE = MAX_MSG_SIZE;

    private static final String FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
    private static final String FIELD_DICTIONARY_DOWNLOAD_NAME = "RWFFld";
    private static final String ENUM_TABLE_FILE_NAME = "enumtype.def";
    private static final String ENUM_TABLE_DOWNLOAD_NAME = "RWFEnum";

    private DataDictionary _dictionary;
    private boolean _fieldDictionaryLoaded = false;
    private boolean _fieldDictionaryLoadedFromFile = false;
    private boolean _enumTypeDictionaryLoaded = false;
    private boolean _enumTypeDictionaryLoadedFromFile = false;
    private State[] _states; // first=field dictionary state, second=enum
                            // dictionary state
    private int _serviceId;
    private int _fieldDictionaryStreamId = -1;
    private int _enumDictionaryStreamId = -1;

    private DictionaryRequest _dictionaryRequest;
    private DictionaryClose _dictionaryClose;
    private DictionaryRefresh _dictionaryRefresh;
    private EncodeIterator _encIter;

    /**
     * Instantiates a new dictionary handler.
     */
    public DictionaryHandler()
    {
        _dictionary = CodecFactory.createDataDictionary();
        _states = new State[2];
        _states[0] = CodecFactory.createState();
        _states[1] = CodecFactory.createState();
        _dictionaryRequest = (DictionaryRequest)DictionaryMsgFactory.createMsg();
        _dictionaryClose = (DictionaryClose)DictionaryMsgFactory.createMsg();
        _dictionaryRefresh = (DictionaryRefresh)DictionaryMsgFactory.createMsg();
        _dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
        _dictionaryRequest.rdmMsgType(DictionaryMsgType.REQUEST);
        _dictionaryClose.rdmMsgType(DictionaryMsgType.CLOSE);
        _encIter = CodecFactory.createEncodeIterator();
    }

    /**
     * Load dictionary from file if possible. Otherwise we will try to download
     * it.
     */
    public void loadDictionary()
    {
        com.rtsdk.eta.transport.Error error = TransportFactory.createError();

        _dictionary.clear();
        if (_dictionary.loadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, error) < 0)
        {
            System.out.println("Unable to load field dictionary.  Will attempt to download from provider.\n\tText: "
                    + error.text());
        }
        else
        {
            _fieldDictionaryLoaded = true;
            _fieldDictionaryLoadedFromFile = true;
        }

        if (_dictionary.loadEnumTypeDictionary(ENUM_TABLE_FILE_NAME, error) < 0)
        {
            System.out.println("Unable to load enum dictionary.  Will attempt to download from provider.\n\tText: "
                        + error.text());
        }
        else
        {
            _enumTypeDictionaryLoaded = true;
            _enumTypeDictionaryLoadedFromFile = true;
        }
    }

    /**
     * Returns whether or not field dictionary has been loaded.
     *
     * @return true, if is field dictionary loaded
     */
    public boolean isFieldDictionaryLoaded()
    {
        return _fieldDictionaryLoaded;
    }

    /**
     * Returns whether or not field dictionary has been loaded from a file.
     *
     * @return true, if is field dictionary loaded from file
     */
    public boolean isFieldDictionaryLoadedFromFile()
    {
        return _fieldDictionaryLoadedFromFile;
    }

    /**
     * Returns whether or not the enumeration types dictionary has been loaded.
     *
     * @return true, if is enum type dictionary loaded
     */
    public boolean isEnumTypeDictionaryLoaded()
    {
        return _enumTypeDictionaryLoaded;
    }

    /**
     * Returns whether or not the enumeration types dictionary has been loaded
     * from a file.
     *
     * @return true, if is enum type dictionary loaded from file
     */
    public boolean isEnumTypeDictionaryLoadedFromFile()
    {
        return _enumTypeDictionaryLoadedFromFile;
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
     * Sets the id of a service to request dictionary from.
     *
     * @param serviceId the service id
     */
    public void serviceId(int serviceId)
    {
        _serviceId = serviceId;
    }

    /**
     * Sends both field and enumType dictionary requests to a channel. This
     * consists of getting a message buffer, encoding the dictionary request,
     * and sending the dictionary request to the server.
     *
     * @param channel the channel
     * @param dictionaryType the dictionary type
     * @param error the error
     * @return the request
     */
    public TransportBuffer getRequest(Channel channel, int dictionaryType, Error error)
    {
    	/* initialize state management array */
    	/* these will be updated as refresh and status messages are received */
    	_states[0].dataState(DataStates.NO_CHANGE);
    	_states[0].streamState(StreamStates.UNSPECIFIED);

    	/* get a buffer for the dictionary request */
    	TransportBuffer msgBuf = channel.getBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, error);

    	if (msgBuf != null)
    	{
    		/* encode dictionary request */
    		_dictionaryRequest.clear();
    		if (dictionaryType == FIELD_DICTIONARY_TYPE)
    		{
    			_dictionaryRequest.dictionaryName().data(FIELD_DICTIONARY_DOWNLOAD_NAME);
    			_dictionaryRequest.streamId(FIELD_DICTIONARY_STREAM_ID);
    		}
    		else
    		{
    			_dictionaryRequest.dictionaryName().data(ENUM_TABLE_DOWNLOAD_NAME);
    			_dictionaryRequest.streamId(ENUM_DICTIONARY_TYPE);
    		}
    		_dictionaryRequest.serviceId(_serviceId);

    		_encIter.clear();
    		_encIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());

    		int ret = _dictionaryRequest.encode(_encIter);
    		if (ret != CodecReturnCodes.SUCCESS)
    		{
    			error.text("encodeDictionaryRequest(): Failed <code: " + error.errorId() + ">");
    			return null;
    		}
    	}

    	return msgBuf;
    }

    /**
     * Processes a dictionary response. This consists of calling decode() on rdm
     * dictionary messages to decode the dictionary and setting dictionary
     * download states (fieldDictionaryLoaded and enumTypeDictionaryLoaded)
     * after complete dictionaries are received.
     *
     * @param chnl - The channel of the response
     * @param msg - The partially decoded message
     * @param dIter - The decode iterator
     * @param error the error
     * @return success if decoding succeeds, failure if it fails.
     */
    public int processResponse(Channel chnl, Msg msg, DecodeIterator dIter, Error error)
    {
        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
                return handleDictRefresh(msg, dIter, error);

            case MsgClasses.STATUS:
                System.out.println("Received StatusMsg for dictionary");
                StatusMsg statusMsg = (StatusMsg)msg;
                if (statusMsg.checkHasState())
                {
                    System.out.println("   " + statusMsg.state());
                }
                break;

            default:
                System.out.println("Received Unhandled Dictionary MsgClass: " + msg.msgClass());
                break;
        }

        return CodecReturnCodes.SUCCESS;
    }

    /**
     * Handle dictionary refresh (complete or parts).
     * 
     * Dictionary type is extracted from the first part with dictionary info.
     * Stream Id for dictionaries are then saved, which compared later to find
     * type of dictionary part received
     * 
     * Updates dictionary download states after full dictionary is received.
     */
    private int handleDictRefresh(Msg msg, DecodeIterator dIter, com.rtsdk.eta.transport.Error error)
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
                    _fieldDictionaryStreamId = _dictionaryRefresh.streamId();
                    break;
                case Dictionary.Types.ENUM_TABLES:
                    _enumDictionaryStreamId = _dictionaryRefresh.streamId();
                    break;
                default:
                    error.text("Received unexpected dictionary message on stream " +
                            msg.streamId());
                    return CodecReturnCodes.FAILURE;
            }
        }

        System.out.println("Received Dictionary Refresh");
        System.out.println(_dictionaryRefresh.toString());

        if (_dictionaryRefresh.streamId() == _fieldDictionaryStreamId)
        {
            ret = _dictionary.decodeFieldDictionary(dIter, Dictionary.VerbosityValues.VERBOSE, error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            if (_dictionaryRefresh.checkRefreshComplete())
            {
                _fieldDictionaryLoaded = true;
                if (!isEnumTypeDictionaryLoaded())
                    System.out.println("Field Dictionary complete, waiting for Enum Table...");
            }
        }
        else if (_dictionaryRefresh.streamId() == _enumDictionaryStreamId)
        {
            ret = _dictionary.decodeEnumTypeDictionary(dIter, Dictionary.VerbosityValues.VERBOSE, error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            if (_dictionaryRefresh.checkRefreshComplete())
            {
                _enumTypeDictionaryLoaded = true;
                if (!isFieldDictionaryLoaded())
                    System.out.println("Enumerated Types Dictionary complete, waiting for Field Dictionary...");
            }
        }
        else
        {
            error.text("Received unexpected dictionary message on stream " +
                    msg.streamId());
            return CodecReturnCodes.FAILURE;
        }

        return CodecReturnCodes.SUCCESS;
    }

}
