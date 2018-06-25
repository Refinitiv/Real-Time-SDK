package com.thomsonreuters.upa.examples.edfexamples.common;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.GlobalFieldSetDefDb;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.examples.common.ChannelSession;
import com.thomsonreuters.upa.examples.common.DictionaryHandler;
import com.thomsonreuters.upa.rdm.Dictionary;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportReturnCodes;

/**
 * This is the dictionary handler for the UPA EDF Consumer application. It provides
 * methods for loading the field/enumType dictionaries from a file and sending
 * requests for those dictionaries and the GlobalSetDef dictionary to the Ref Data Server. 
 * Methods for processing the dictionary response and closing a dictionary stream 
 * are also provided.
 */
public class EDFDictionaryHandler extends DictionaryHandler
{
    public final int GLOBAL_DEFS_DICTIONARY_STREAM_ID = 5;
    private static String GLOBAL_DEF_DOWNLOAD_NAME = "EDF_BATS";
    
    private GlobalFieldSetDefDb fieldSetDef;
    private boolean globalDefsDictionaryLoaded = false;
    private boolean globalDefsDictionaryLoadedFromFile = false;
    private int globalDefsDictionaryStreamId = -1;

    /**
     * Instantiates a new EDF dictionary handler.
     */
    public EDFDictionaryHandler()
    {
        super();
    }
    
    
    /**
     * Load dictionary from file if possible. Otherwise we will try to download it.
     */
    public void loadDictionary()
    {
        super.loadDictionary();
        
        /*We do not load Field Set Def dictionary because we get the Global Set Defs from Reference Data Server*/
    }

    /**
     * Returns whether or not the global defs dictionary has been loaded.
     *
     * @return true, if is global defs dictionary loaded
     */
    public boolean isGlobalDefsDictionaryLoaded()
    {
        return globalDefsDictionaryLoaded;
    }

    /**
     * Returns whether or not the global defs dictionary has been loaded
     * from a file.
     *
     * @return true, if is global defs dictionary loaded from file
     */
    public boolean isGlobalDefsDictionaryLoadedFromFile()
    {
        return globalDefsDictionaryLoadedFromFile;
    }

    /**
     * Returns the field Set Definition database.
     *
     * @return the global field set def db
     */
    public GlobalFieldSetDefDb fieldSetDefDb()
    {
        return fieldSetDef;
    }

    /**
     * Sends both field and enumType dictionary requests to a channel. This
     * consists of getting a message buffer, encoding the dictionary request,
     * and sending the dictionary request to the server.
     *
     * @param chnl - The channel to send a dictionary requests to
     * @param dictionaryName the dictionary name
     * @param error the error
     * @return the int
     */
    public int sendRequests(ChannelSession chnl, String dictionaryName, Error error)
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
        
        if (!isGlobalDefsDictionaryLoaded())
        {
            states[1].dataState(DataStates.NO_CHANGE);
            states[1].streamState(StreamStates.UNSPECIFIED);

            if (dictionaryName != null)
                GLOBAL_DEF_DOWNLOAD_NAME = dictionaryName;
            
            int ret = sendRequest(chnl, GLOBAL_DEF_DOWNLOAD_NAME, GLOBAL_DEFS_DICTIONARY_STREAM_ID,
                                  serviceId, error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }

        return CodecReturnCodes.SUCCESS;
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
                    else if (msg.streamId() == globalDefsDictionaryStreamId)
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
        int ret = dictionaryRefresh.decode(dIter, msg);
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
                case Dictionary.Types.FIELD_SET_DEFINITION:
                    //fieldDictionaryStreamId = dictionaryRefresh.streamId();
                    globalDefsDictionaryStreamId = dictionaryRefresh.streamId();
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
            ret = dictionary.decodeFieldDictionary(dIter, Dictionary.VerbosityValues.VERBOSE, error);
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
        	ret = dictionary.decodeEnumTypeDictionary(dIter, Dictionary.VerbosityValues.VERBOSE, error);
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
        else if (dictionaryRefresh.streamId() == globalDefsDictionaryStreamId)
        {
            
            RefreshMsg refreshMsg = (RefreshMsg)msg;
            states[1].dataState(refreshMsg.state().dataState());
            states[1].streamState(refreshMsg.state().streamState());               
            ret = fieldSetDef.decode(dIter, Dictionary.VerbosityValues.VERBOSE, error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            if (dictionaryRefresh.checkRefreshComplete())
            {
                globalDefsDictionaryLoaded = true;
                if (!isFieldDictionaryLoaded())
                    System.out.println("Global Defs Dictionary complete, waiting for Field Dictionary...");
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
        if (streamId == GLOBAL_DEFS_DICTIONARY_STREAM_ID)
            globalDefsDictionaryLoaded = false;

        return TransportReturnCodes.SUCCESS;
    }

    /* (non-Javadoc)
     * @see com.thomsonreuters.upa.examples.common.DictionaryHandler#closeStreams(com.thomsonreuters.upa.examples.common.ChannelSession, com.thomsonreuters.upa.transport.Error)
     */
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
        if (!isGlobalDefsDictionaryLoadedFromFile())
        {
            State enumDictStreamState = states[1];
            /*
             * we only want to close a stream if it was not already closed (e.g.
             * rejected by provider, closed via refresh or status)
             */
            if (enumDictStreamState.isFinal())
                return TransportReturnCodes.SUCCESS;
            int ret = closeStream(channel,
                                  GLOBAL_DEFS_DICTIONARY_STREAM_ID, error);
            if (ret != TransportReturnCodes.SUCCESS)
                return ret;
        }

        return TransportReturnCodes.SUCCESS;
    }
}
