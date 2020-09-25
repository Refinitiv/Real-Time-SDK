package com.refinitiv.eta.examples.common;

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
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.rdm.Dictionary;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryClose;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequest;

/**
 * This is the dictionary handler for the UPA consumer application.
 * It provides methods for loading the field/enumType dictionaries from a file or
 * requesting from ADH.
 */
public class NIProviderDictionaryHandler
{
    private static final String FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
    private static final String ENUM_TYPE_FILE_NAME = "enumtype.def";

    public static final String FIELD_DICTIONARY_DOWNLOAD_NAME = "RWFFld";
    public static final String ENUM_TYPE_DOWNLOAD_NAME = "RWFEnum";
    
    private static final int FIELD_DICTIONARY_STREAM_ID = -1;
    private static final int ENUM_TYPE_DICTIONARY_STREAM_ID = -2;
    
    private DataDictionary dictionary;

    private boolean bfieldDictionaryDownloaded = false;
    private boolean bEnumTypeDictionaryLoaded = false;
    
    private DictionaryRequest dictionaryRequest = (DictionaryRequest)DictionaryMsgFactory.createMsg();
    private DictionaryRefresh dictionaryRefresh = (DictionaryRefresh)DictionaryMsgFactory.createMsg();
    private DictionaryClose dictionaryClose;
    
    private EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
    
    private State[] states; // first=field dictionary state
    						// second=enum dictionary state
    
    /**
	 * Instantiates a new NI provider dictionary handler.
	 */
	public NIProviderDictionaryHandler()
    {
        dictionary = CodecFactory.createDataDictionary();
        
        dictionaryRequest.rdmMsgType(DictionaryMsgType.REQUEST);
        dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
        
        states = new State[2];
        states[0] = CodecFactory.createState();
        states[1] = CodecFactory.createState();
    }
    
    /**
     * Load dictionary from file.
     *
     * @param error the error
     * @return true if the Field and EnumType dictionaries were loaded,
     *         otherwise false.
     */
    public boolean loadDictionary(Error error)
    {
        dictionary.clear();
        if (dictionary.loadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, error) < 0)
        {
        	System.out.format("Unable to load field dictionary: %s. Error Text: %s\n",FIELD_DICTIONARY_FILE_NAME,error.text() );
            return false;
        }
        else
        {
        	System.out.format("Loaded field dictionary from: %s\n",FIELD_DICTIONARY_FILE_NAME);
        }

        if (dictionary.loadEnumTypeDictionary(ENUM_TYPE_FILE_NAME, error) < 0)
        {
        	System.out.format("Unable to load enum dictionary: %s.Error Text: %s\n", ENUM_TYPE_FILE_NAME,error.text() );
            return false;
        }
        else
        {
        	System.out.format("Loaded enumtype dictionary from: %s\n",ENUM_TYPE_FILE_NAME);
        }

        return true;
    }
   
    /**
     * Returns the data dictionary.
     *
     * @return the data dictionary
     */
    public DataDictionary dictionary()
    {
        return dictionary;
    }
    
    /**
     * Send dictionary requests.
     *
     * @param chnl the chnl
     * @param error the error
     * @param serviceId the service id
     * @return the int
     */
    public int sendDictionaryRequests(ChannelSession chnl, Error error, int serviceId)
	{
		if( dictionaryRequest == null )
		{
			dictionaryRequest = (DictionaryRequest)DictionaryMsgFactory.createMsg();
			dictionaryRequest.rdmMsgType(DictionaryMsgType.REQUEST);
		}
	
		int requestStatus = requestDictionary(chnl,error,serviceId,FIELD_DICTIONARY_STREAM_ID,FIELD_DICTIONARY_DOWNLOAD_NAME);
		
		if( requestStatus == CodecReturnCodes.SUCCESS)
		{
			requestStatus = requestDictionary(chnl,error,serviceId,ENUM_TYPE_DICTIONARY_STREAM_ID,ENUM_TYPE_DOWNLOAD_NAME);
		}
		
		return requestStatus;
	}
	
	int requestDictionary(ChannelSession chnl, Error error, int serviceId,int streamId,String dictName)
	{
		/* get a buffer for the dictionary request */
        TransportBuffer msgBuf = chnl.getTransportBuffer(ChannelSession.MAX_MSG_SIZE, false, error);
		if (msgBuf == null)
		{
			return CodecReturnCodes.FAILURE;
		}
		
		/* encode dictionary request */
		dictionaryRequest.clear();

		dictionaryRequest.dictionaryName().data(dictName);
		dictionaryRequest.streamId(streamId);
		dictionaryRequest.serviceId(serviceId);
		dictionaryRequest.verbosity(Dictionary.VerbosityValues.NORMAL);

		encodeIter.clear();

		int ret = encodeIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());
		if (ret != CodecReturnCodes.SUCCESS)
		{
			error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
			return ret;
		}

		ret = dictionaryRequest.encode(encodeIter);
		if (ret != CodecReturnCodes.SUCCESS)
		{
			error.text("encodeDictionaryRequest(): Failed <code: " + error.errorId() + ">");
			return ret;
		}

		System.out.println(dictionaryRequest.toString());

		//send request
		return chnl.write(msgBuf, error);
	}

	/**
	 * Process response.
	 *
	 * @param msg the msg
	 * @param dIter the d iter
	 * @param error the error
	 * @return the int
	 */
	public int processResponse(Msg msg, DecodeIterator dIter, Error error)
	{
        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:             
                return processRefresh(msg, dIter, error);
                
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
                break;

            default:
                System.out.println("Received Unhandled Dictionary MsgClass: " + msg.msgClass());
                break;
        }

        return CodecReturnCodes.SUCCESS;

	}
	
	int processRefresh(Msg msg, DecodeIterator dIter, Error error)
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
				break;
			case Dictionary.Types.ENUM_TABLES:
				break;
			default:
				error.text("Received unexpected dictionary message on stream " +msg.streamId());
				return CodecReturnCodes.FAILURE;
			}
		}

		if ( dictionaryRefresh.streamId() == FIELD_DICTIONARY_STREAM_ID )
		{
			System.out.println("Received Dictionary Refresh for field dictionary");
			
			RefreshMsg refreshMsg = (RefreshMsg)msg;

			states[0].dataState(refreshMsg.state().dataState());
			states[0].streamState(refreshMsg.state().streamState());        	
			
			ret = dictionary.decodeFieldDictionary(dIter, Dictionary.VerbosityValues.VERBOSE, error);
			if (ret != CodecReturnCodes.SUCCESS)
			{
				System.out.println("Unable to decode Field Dictionary");
				return ret;
			}

			if (dictionaryRefresh.checkRefreshComplete())
			{
				bfieldDictionaryDownloaded = true;              
				if ( ! bEnumTypeDictionaryLoaded )
					System.out.println("Field Dictionary complete, waiting for Enum Table...");
			}
		}
		else if ( dictionaryRefresh.streamId() == ENUM_TYPE_DICTIONARY_STREAM_ID )
		{
			System.out.println("Received Dictionary Refresh for enum type");
			
			RefreshMsg refreshMsg = (RefreshMsg)msg;

			states[1].dataState(refreshMsg.state().dataState());
			states[1].streamState(refreshMsg.state().streamState());        	
			
			ret = dictionary.decodeEnumTypeDictionary(dIter, Dictionary.VerbosityValues.VERBOSE, error);
			if (ret != CodecReturnCodes.SUCCESS)
			{
				System.out.println("Unable to decode Enum Dictionary");
				return ret;
			}

			if (dictionaryRefresh.checkRefreshComplete())
			{
				bEnumTypeDictionaryLoaded = true;
				if ( ! bfieldDictionaryDownloaded )
					System.out.println("Enumerated Types Dictionary complete, waiting for Field Dictionary...");
			}
		}
		else
		{
			error.text("Received unexpected dictionary message on stream " + msg.streamId());
			return CodecReturnCodes.FAILURE;
		}

		if ( bfieldDictionaryDownloaded && bEnumTypeDictionaryLoaded )
			System.out.println("Dictionary Download complete for both field and enum type dictionaries");
		
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
    private boolean closeDictionary(ChannelSession chnl, int streamId, Error error)
    {
		if( dictionaryClose == null )
		{
			dictionaryClose = (DictionaryClose)DictionaryMsgFactory.createMsg();
			dictionaryClose.rdmMsgType(DictionaryMsgType.CLOSE);
		}
		
        /* get a buffer for the dictionary close */
        TransportBuffer msgBuf = chnl.getTransportBuffer(ChannelSession.MAX_MSG_SIZE, false, error);
        if (msgBuf == null)
        {
            return false;
        }

        encodeIter.clear();
        encodeIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());

        /* encode dictionary close */
        dictionaryClose.clear();
        dictionaryClose.streamId(streamId);
        
        int ret = dictionaryClose.encode(encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("encodeDictionaryClose(): Failed <code: " + CodecReturnCodes.toString(ret) + ">");
            return false;
        }

        /* send close */
        ret = chnl.write(msgBuf, error);
        if (ret != TransportReturnCodes.SUCCESS)
        {
            return false;
        }

        dictionary.clear();
        
        return true;
    }

	/**
	 * Close stream.
	 *
	 * @param channel the channel
	 * @param error the error
	 */
	public void closeStream(ChannelSession channel, Error error) 
	{
		/* close dictionary stream */
        if( bfieldDictionaryDownloaded )
        {
            /*
             * we only want to close a stream if it was not already closed (e.g.
             * rejected by provider, closed via refresh or status)
             */
            if (states[0].dataState() == DataStates.OK && states[0].streamState() == StreamStates.NON_STREAMING )
            {
            	if( closeDictionary(channel,FIELD_DICTIONARY_STREAM_ID, error) == true)
            	{
                	bfieldDictionaryDownloaded = false;
                	states[0].clear();
            	}
            }
        }
        
        if( bEnumTypeDictionaryLoaded )
        {
            /*
             * we only want to close a stream if it was not already closed (e.g.
             * rejected by provider, closed via refresh or status)
             */
            if (states[1].dataState() == DataStates.OK && states[1].streamState() == StreamStates.NON_STREAMING )
            {
            	if( closeDictionary(channel,ENUM_TYPE_DICTIONARY_STREAM_ID, error) == true )
            	{
            		bEnumTypeDictionaryLoaded = false;
                	states[1].clear();
            	}
            }
        }
	}
	
}
