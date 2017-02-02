package com.thomsonreuters.upa.examples.common;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.Dictionary;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryClose;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryRequest;

/**
 * This is the dictionary handler for the UPA consumer application. It provides
 * methods for loading the field/enumType dictionaries from a file and sending
 * requests for those dictionaries to a provider. Methods for processing the
 * dictionary response and closing a dictionary stream are also provided.
 */
public class DictionaryHandler
{
    public final int FIELD_DICTIONARY_STREAM_ID = 3;
    public final int ENUM_TYPE_DICTIONARY_STREAM_ID = 4;

    public static int TRANSPORT_BUFFER_SIZE_REQUEST = ChannelSession.MAX_MSG_SIZE;
    public static int TRANSPORT_BUFFER_SIZE_CLOSE = ChannelSession.MAX_MSG_SIZE;

    private static final String FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
    protected static final String FIELD_DICTIONARY_DOWNLOAD_NAME = "RWFFld";
    private static final String ENUM_TABLE_FILE_NAME = "enumtype.def";
    protected static final String ENUM_TABLE_DOWNLOAD_NAME = "RWFEnum";

    protected DataDictionary dictionary;
// APIQA: ETA-2660
    protected DataDictionary dictionaryFromNetwork;
// END APIQA
    protected boolean fieldDictionaryLoaded = false;
    private boolean fieldDictionaryLoadedFromFile = false;
    protected boolean enumTypeDictionaryLoaded = false;
    private boolean enumTypeDictionaryLoadedFromFile = false;
    protected State[] states; // first=field dictionary state, second=enum
                              // dictionary state
    protected int serviceId;
    protected int fieldDictionaryStreamId = -1;
    protected int enumDictionaryStreamId = -1;

    private DictionaryRequest dictionaryRequest;
    protected DictionaryClose dictionaryClose;
    protected DictionaryRefresh dictionaryRefresh;
    protected EncodeIterator encIter;

    public DictionaryHandler()
    {
        dictionary = CodecFactory.createDataDictionary();
// APIQA: ETA-2660
        dictionaryFromNetwork = CodecFactory.createDataDictionary();
// END APIQA
        states = new State[2];
        states[0] = CodecFactory.createState();
        states[1] = CodecFactory.createState();
        dictionaryRequest = (DictionaryRequest)DictionaryMsgFactory.createMsg();
        dictionaryClose = (DictionaryClose)DictionaryMsgFactory.createMsg();
        dictionaryRefresh = (DictionaryRefresh)DictionaryMsgFactory.createMsg();
        dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
        dictionaryRequest.rdmMsgType(DictionaryMsgType.REQUEST);
        dictionaryClose.rdmMsgType(DictionaryMsgType.CLOSE);
        encIter = CodecFactory.createEncodeIterator();
    }

    /**
     * Load dictionary from file if possible. Otherwise we will try to download it.
     */
    public void loadDictionary()
    {
        com.thomsonreuters.upa.transport.Error error = TransportFactory.createError();

        dictionary.clear();
// APIQA: ETA-2660
        dictionaryFromNetwork.clear();
// END APIQA
        if (dictionary.loadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, error) < 0)
        {
            System.out.println("Unable to load field dictionary.  Will attempt to download from provider.\n\tText: "
                    + error.text());
        }
        else
        {
// APIQA: ETA-2660
            fieldDictionaryLoaded = false;
            fieldDictionaryLoadedFromFile = false;
// END APIQA
        }

        if (dictionary.loadEnumTypeDictionary(ENUM_TABLE_FILE_NAME, error) < 0)
        {
            System.out.println("Unable to load enum dictionary.  Will attempt to download from provider.\n\tText: "
                        + error.text());
        }
        else
        {
// APIQA: ETA-2660
            enumTypeDictionaryLoaded = false;
            enumTypeDictionaryLoadedFromFile = false;
// END APIQA
        }
    }

    /**
     * Returns whether or not field dictionary has been loaded
     */
    public boolean isFieldDictionaryLoaded()
    {
        return fieldDictionaryLoaded;
    }

    /**
     * Returns whether or not field dictionary has been loaded from a file.
     */
    public boolean isFieldDictionaryLoadedFromFile()
    {
        return fieldDictionaryLoadedFromFile;
    }

    /**
     * Returns whether or not the enumeration types dictionary has been loaded
     */
    public boolean isEnumTypeDictionaryLoaded()
    {
        return enumTypeDictionaryLoaded;
    }

    /**
     * Returns whether or not the enumeration types dictionary has been loaded
     * from a file.
     */
    public boolean isEnumTypeDictionaryLoadedFromFile()
    {
        return enumTypeDictionaryLoadedFromFile;
    }

    /**
     * Returns the data dictionary.
     */
    public DataDictionary dictionary()
    {
        return dictionary;
    }

    /**
     * Sets the id of a service to request dictionary from.
     * 
     * @param serviceId
     */
    public void serviceId(int serviceId)
    {
        this.serviceId = serviceId;
    }

    /**
     * Sends both field and enumType dictionary requests to a channel. This
     * consists of getting a message buffer, encoding the dictionary request,
     * and sending the dictionary request to the server.
     * 
     * @param chnl - The channel to send a dictionary requests to
     */
    public int sendRequests(ChannelSession chnl, Error error)
    {
        if (!isFieldDictionaryLoaded())
        {
            /* initialize state management array */
            /* these will be updated as refresh and status messages are received */
            states[0].dataState(DataStates.NO_CHANGE);
            states[0].streamState(StreamStates.UNSPECIFIED);

            int ret = sendRequest(chnl, FIELD_DICTIONARY_DOWNLOAD_NAME, FIELD_DICTIONARY_STREAM_ID,
                                  serviceId, error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }

        if (!isEnumTypeDictionaryLoaded())
        {
            states[1].dataState(DataStates.NO_CHANGE);
            states[1].streamState(StreamStates.UNSPECIFIED);

            int ret = sendRequest(chnl, ENUM_TABLE_DOWNLOAD_NAME, ENUM_TYPE_DICTIONARY_STREAM_ID,
                                  serviceId, error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }

        return CodecReturnCodes.SUCCESS;
    }

    protected int sendRequest(ChannelSession chnl, String dictionaryName, int streamId, long serviceId, Error error)
    {
        TransportBuffer msgBuf = null;

        /* get a buffer for the dictionary request */
        msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, error);

        if (msgBuf != null)
        {
            /* encode dictionary request */
            dictionaryRequest.clear();
            dictionaryRequest.dictionaryName().data(dictionaryName);
            dictionaryRequest.streamId(streamId);
            dictionaryRequest.serviceId((int)serviceId);

            encIter.clear();
            encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());

            int ret = dictionaryRequest.encode(encIter);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                error.text("encodeDictionaryRequest(): Failed <code: " + error.errorId() + ">");
                return ret;
            }

            System.out.println(dictionaryRequest.toString());
            
            //send request
            return chnl.write(msgBuf, error);
        }
        else
        {
            return CodecReturnCodes.FAILURE;
        }
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
     * 
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
                    State state = statusMsg.state();
                    if (msg.streamId() == fieldDictionaryStreamId)
                    {
                    	states[0].dataState(state.dataState());
                    	states[0].streamState(state.streamState());
                    }
                    else if (msg.streamId() == enumDictionaryStreamId)
                    {
                    	states[1].dataState(state.dataState());
                    	states[1].streamState(state.streamState());
                    }
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
     * Updates dictionary download states after full dictionary is recieved.
     */
    private int handleDictRefresh(Msg msg, DecodeIterator dIter, com.thomsonreuters.upa.transport.Error error)
    {
        int ret = dictionaryRefresh.decode(dIter, (RefreshMsg)msg);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("Error decoding dictionary refresh: <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        if (dictionaryRefresh.checkHasInfo())
        {
            switch (dictionaryRefresh.dictionaryType())
            {
                case Dictionary.Types.FIELD_DEFINITIONS:
                    fieldDictionaryStreamId = dictionaryRefresh.streamId();
                    break;
                case Dictionary.Types.ENUM_TABLES:
                    enumDictionaryStreamId = dictionaryRefresh.streamId();
                    break;
                default:
                    error.text("Received unexpected dictionary message on stream " +
                            msg.streamId());
                    return CodecReturnCodes.FAILURE;
            }
        }

        System.out.println("Received Dictionary Refresh");
        System.out.println(dictionaryRefresh.toString());

        if (dictionaryRefresh.streamId() == fieldDictionaryStreamId)
        {
            RefreshMsg refreshMsg = (RefreshMsg)msg;
            states[0].dataState(refreshMsg.state().dataState());
            states[0].streamState(refreshMsg.state().streamState());        	
// APIQA: ETA-2660              
            ret = dictionaryFromNetwork.decodeFieldDictionary(dIter, Dictionary.VerbosityValues.VERBOSE, error);
// END APIQA
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            if (dictionaryRefresh.checkRefreshComplete())
            {
                fieldDictionaryLoaded = true;              
                if (!isEnumTypeDictionaryLoaded())
                    System.out.println("Field Dictionary complete, waiting for Enum Table...");
            }
        }
        else if (dictionaryRefresh.streamId() == enumDictionaryStreamId)
        {
            RefreshMsg refreshMsg = (RefreshMsg)msg;
            states[1].dataState(refreshMsg.state().dataState());
            states[1].streamState(refreshMsg.state().streamState());               
// APIQA: ETA-2660               
             ret = dictionaryFromNetwork.decodeEnumTypeDictionary(dIter, Dictionary.VerbosityValues.VERBOSE, error);
// END APIQA
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            if (dictionaryRefresh.checkRefreshComplete())
            {
            	enumTypeDictionaryLoaded = true;
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
// APIQA: ETA-2660 This code compares the downloaded dictionary with the 
// dictionary from the file dictionary
        if (isFieldDictionaryLoaded() && isEnumTypeDictionaryLoaded())
        {
            if (dictionary.enumTableCount() == dictionaryFromNetwork.enumTableCount() &&
                dictionary.numberOfEntries() == dictionaryFromNetwork.numberOfEntries())
                System.out.println("Dictionary from file and from network are the same");
            else
                System.out.println("ERROR: Dictionary from file is different than from the network");
        }
// END APIQA  
        return CodecReturnCodes.SUCCESS;
    }

    /**
     * Close the dictionary stream if there is one.
     * 
     * @param chnl - The channel to send a dictionary close to
     * @param streamId - The stream id of the dictionary stream to close
     */
    private int closeStream(ChannelSession chnl, int streamId, Error error)
    {
        /* get a buffer for the dictionary close */
        TransportBuffer msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, error);
        if (msgBuf == null)
        {
            return CodecReturnCodes.FAILURE;
        }

        /* encode dictionary close */
        dictionaryClose.clear();
        dictionaryClose.streamId(streamId);
        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());
        int ret = dictionaryClose.encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("encodeDictionaryClose(): Failed <code: " + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        /* send close */
        ret = chnl.write(msgBuf, error);
        if (ret != TransportReturnCodes.SUCCESS)
        {
            return ret;
        }

        dictionary.clear();
        if (streamId == FIELD_DICTIONARY_STREAM_ID)
            fieldDictionaryLoaded = false;
        if (streamId == ENUM_TYPE_DICTIONARY_STREAM_ID)
            enumTypeDictionaryLoaded = false;

        return TransportReturnCodes.SUCCESS;
    }

    public int closeStreams(ChannelSession channel, Error error)
    {
        /* close dictionary stream */
        if (!isFieldDictionaryLoadedFromFile())
        {
            State fieldDictStreamState = states[0];
            /*
             * we only want to close a stream if it was not already closed (e.g.
             * rejected by provider, closed via refresh or status)
             */
            if (fieldDictStreamState.isFinal())
                return TransportReturnCodes.SUCCESS;
            int ret = closeStream(channel,
                                  FIELD_DICTIONARY_STREAM_ID, error);
            if (ret != TransportReturnCodes.SUCCESS)
                return ret;
        }
        if (!isEnumTypeDictionaryLoadedFromFile())
        {
            State enumDictStreamState = states[1];
            /*
             * we only want to close a stream if it was not already closed (e.g.
             * rejected by provider, closed via refresh or status)
             */
            if (enumDictStreamState.isFinal())
                return TransportReturnCodes.SUCCESS;
            int ret = closeStream(channel,
                                  ENUM_TYPE_DICTIONARY_STREAM_ID, error);
            if (ret != TransportReturnCodes.SUCCESS)
                return ret;
        }

        return TransportReturnCodes.SUCCESS;
    }
}
