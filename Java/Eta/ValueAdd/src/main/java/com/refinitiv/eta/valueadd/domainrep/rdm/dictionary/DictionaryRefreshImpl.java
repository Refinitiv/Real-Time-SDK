/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.dictionary;

import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.ElementEntry;
import com.refinitiv.eta.codec.ElementList;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Int;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.Series;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.rdm.Dictionary;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBaseImpl;
import com.refinitiv.eta.transport.Error;

class DictionaryRefreshImpl extends MsgBaseImpl
{
    private int flags;

    private Buffer dictionaryName;
    private Buffer dataBody;
    private State state;
    private int dictionaryType;
    private int serviceId;
    private int verbosity;
    private long sequenceNumber;
    private int startFid;
    private int startEnumTableCount;
    
    private Int tmpInt = CodecFactory.createInt();
    private DataDictionary dictionary = null;
    private Error error = TransportFactory.createError();
    private Series series = CodecFactory.createSeries();
    private ElementList elementList = CodecFactory.createElementList();
    private ElementEntry elementEntry = CodecFactory.createElementEntry();
    private DecodeIterator seriesDecodeIter = CodecFactory.createDecodeIterator();
    private UInt tmpUInt = CodecFactory.createUInt();
    private RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
    private Buffer version = CodecFactory.createBuffer();
    
    private final static String eol = "\n";
    private final static String tab = "\t";
    
    private boolean userSetDictionary = false;
    
    public int flags()
    {
        return flags;
    }

    public void flags(int flags)
    {
        this.flags = flags;
    }

    public boolean checkHasSequenceNumber()
    {
        return (flags & DictionaryRefreshFlags.HAS_SEQ_NUM) != 0;
    }

    public void applyHasSequenceNumber()
    {
        flags |= DictionaryRefreshFlags.HAS_SEQ_NUM;
    }
    
    public boolean checkHasInfo()
    {
        return (flags & DictionaryRefreshFlags.HAS_INFO) != 0;
    }

    public void applyHasInfo()
    {
        flags |= DictionaryRefreshFlags.HAS_INFO;
    }
    
    DictionaryRefreshImpl()
    {
        dictionaryName = CodecFactory.createBuffer();
        state = CodecFactory.createState();
        dataBody = CodecFactory.createBuffer();
    }

    public int copy(DictionaryRefresh destRefreshMsg)
    {
        assert (destRefreshMsg != null) : "destRefreshMsg must be non-null";

        destRefreshMsg.streamId(streamId());
        destRefreshMsg.serviceId(serviceId());
        destRefreshMsg.verbosity(verbosity());
        destRefreshMsg.streamId(streamId());
        destRefreshMsg.serviceId(serviceId());
        destRefreshMsg.dictionaryType(dictionaryType());
        destRefreshMsg.startFid(startFid());
        destRefreshMsg.startEnumTableCount(startEnumTableCount());
        destRefreshMsg.flags(flags);

        // dictionaryname
        {
            ByteBuffer byteBuffer = ByteBuffer.allocate(this.dictionaryName.length());
            this.dictionaryName.copy(byteBuffer);
            destRefreshMsg.dictionaryName().data(byteBuffer);
        }

        // dataBody
        if (dataBody != null && dataBody.length() > 0)
        {
            ByteBuffer byteBuffer = ByteBuffer.allocate(this.dataBody.length());
            this.dataBody.copy(byteBuffer);
            destRefreshMsg.dataBody().data(byteBuffer);
        }

        // state
        {
            destRefreshMsg.state().streamState(this.state.streamState());
            destRefreshMsg.state().dataState(this.state.dataState());
            destRefreshMsg.state().code(this.state.code());

            if( this.state.text().length() > 0)
            {
                ByteBuffer byteBuffer = ByteBuffer.allocate(this.state.text().length());
                this.state.text().copy(byteBuffer);
                destRefreshMsg.state().text().data(byteBuffer);
            }
        }
        return CodecReturnCodes.SUCCESS;
    }

    public void clear()
    {
        state.clear();
        state.streamState(StreamStates.OPEN);
        state.code(StateCodes.NONE);
        state.dataState(DataStates.OK);
        dictionaryName.clear();
        flags = 0;
        serviceId = 0;
        verbosity = Dictionary.VerbosityValues.NORMAL;
        dictionaryType = 0;
        sequenceNumber = 0;
        startFid = -32768; // MIN_FID
        startEnumTableCount = 0;
        dataBody.clear();
        version.clear();
        if (dictionary != null && !userSetDictionary)
        	dictionary.clear();
        userSetDictionary = false;
    }

    @Override
    public int encode(EncodeIterator encodeIter)
    {
        refreshMsg.clear();

        // message header
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.streamId(streamId());
        refreshMsg.domainType(DomainTypes.DICTIONARY);
        refreshMsg.containerType(DataTypes.SERIES);
        refreshMsg.applyHasMsgKey();
        if (checkSolicited())
            refreshMsg.applySolicited();
        
        if (checkClearCache())
            refreshMsg.applyClearCache();

        if (checkHasSequenceNumber())
        {
            refreshMsg.applyHasSeqNum();
            refreshMsg.seqNum(sequenceNumber());
        }

        refreshMsg.state().dataState(state().dataState());
        refreshMsg.state().streamState(state().streamState());
        refreshMsg.state().code(state().code());
        refreshMsg.state().text(state().text());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().applyHasFilter();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().name(dictionaryName());
        refreshMsg.msgKey().filter(verbosity());
        refreshMsg.msgKey().serviceId(serviceId());
      
        
        int ret = refreshMsg.encodeInit(encodeIter, 0);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        //encode dictionary into message
        switch(dictionaryType)
        {
            case Dictionary.Types.FIELD_DEFINITIONS:
            {
                tmpInt.value(startFid);
                
            	if (dictionary == null)
            		dictionary = CodecFactory.createDataDictionary();
                int dictEncodeRet = dictionary.encodeFieldDictionary(encodeIter, tmpInt, verbosity, error);
                if (dictEncodeRet != CodecReturnCodes.SUCCESS)
                {
                    if (dictEncodeRet != CodecReturnCodes.DICT_PART_ENCODED) 
                    {
                        // dictionary encode failed
                        return dictEncodeRet;
                    }
                }
                else
                {
                    // set refresh complete flag
                    ret = encodeIter.setRefreshCompleteFlag();
                    if(ret != CodecReturnCodes.SUCCESS)
                    {
                        error.text("Unable to set refresh complete flag to dictionary refresh msg");
                        return ret;
                    }
                }

                //complete encode message
                ret = refreshMsg.encodeComplete(encodeIter, true);
                if (ret < CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
                
                startFid = (int)tmpInt.toLong();
                return dictEncodeRet;
            }
            case Dictionary.Types.ENUM_TABLES:
            {
                tmpInt.value(startEnumTableCount);            	
                //encode dictionary into message
                int dictEncodeRet = dictionary.encodeEnumTypeDictionaryAsMultiPart(encodeIter, tmpInt, verbosity(), error);
                if (dictEncodeRet != CodecReturnCodes.SUCCESS)
                {
                	if (dictEncodeRet != CodecReturnCodes.DICT_PART_ENCODED) 
                    {
                        // dictionary encode failed
                        return dictEncodeRet;
                    }
                }
                else
                {
                    // set refresh complete flag
                    ret = encodeIter.setRefreshCompleteFlag();
                if (ret != CodecReturnCodes.SUCCESS)
                {
                        error.text("Unable to set refresh complete flag to dictionary refresh msg");
                    return ret;
                }
                }

                //complete encode message
                ret = refreshMsg.encodeComplete(encodeIter, true);
                if (ret < CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
                startEnumTableCount = (int)tmpInt.toLong();
                return dictEncodeRet;
            }
            default:
            	error.text("Invalid Dictionary Type: " + dictionaryType);
            	return CodecReturnCodes.FAILURE;
        }
    }

    public boolean checkSolicited()
    {
        return (flags & DictionaryRefreshFlags.SOLICITED) != 0;
    }

    public boolean checkClearCache()
    {
        return (flags & DictionaryRefreshFlags.CLEAR_CACHE) != 0;
    }

    public int decode(DecodeIterator dIter, Msg msg)
    {
        if (msg.msgClass() != MsgClasses.REFRESH)
        {
            return CodecReturnCodes.FAILURE;
        }

        clear();
        streamId(msg.streamId());
        MsgKey key = msg.msgKey();
        if (key == null || !key.checkHasFilter() || !key.checkHasName() || !key.checkHasServiceId())
            return CodecReturnCodes.FAILURE;

        RefreshMsg refreshMsg = (RefreshMsg)msg;
        Buffer name = key.name();
        dictionaryName().data(name.data(), name.position(), name.length());
        if (key.checkHasServiceId())
            serviceId(key.serviceId());
        if (refreshMsg.checkHasSeqNum())
        {
            applyHasSequenceNumber();
            sequenceNumber(refreshMsg.seqNum());
        }
        verbosity((int)key.filter());
        if (refreshMsg.checkRefreshComplete())
            applyRefreshComplete();
        if (refreshMsg.checkSolicited())
            applySolicited();
        if (refreshMsg.checkClearCache())
            applyClearCache();

        state().code(refreshMsg.state().code());
        state().streamState(refreshMsg.state().streamState());
        state().dataState(refreshMsg.state().dataState());

        if (refreshMsg.state().text().length() > 0)
        {
            Buffer buf = refreshMsg.state().text();
            state().text().data(buf.data(), buf.position(), buf.length());
        }

        // payload
        seriesDecodeIter.clear();
        series.clear();
        if (msg.encodedDataBody().length() > 0)
        {
            Buffer encodedDataBody = msg.encodedDataBody();
            dataBody().data(encodedDataBody.data(), encodedDataBody.position(), encodedDataBody.length());
	        seriesDecodeIter.setBufferAndRWFVersion(encodedDataBody, dIter.majorVersion(), dIter.minorVersion());
	        int ret = series.decode(seriesDecodeIter);
	        if (ret != CodecReturnCodes.SUCCESS)
	            return ret;
	        if (series.checkHasSummaryData())
	        {
	        	applyHasInfo();
	            return decodeDictionaryInfo();
	        }
        }

        return CodecReturnCodes.SUCCESS;
    }

    public void applyClearCache()
    {
        flags |= DictionaryRefreshFlags.CLEAR_CACHE;
    }

    public void applySolicited()
    {
        flags |= DictionaryRefreshFlags.SOLICITED;
    }

    public void applyRefreshComplete()
    {
        this.flags |= DictionaryRefreshFlags.IS_COMPLETE;
    }

    public boolean checkRefreshComplete()
    {
        return (this.flags & DictionaryRefreshFlags.IS_COMPLETE) != 0;
    }

    private int decodeDictionaryInfo()
    {
        elementList.clear();
        int ret = elementList.decode(seriesDecodeIter, null);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;
        
        //If there is no summary data present, don't go looking for info.
        if (!(elementList.checkHasStandardData() || elementList.checkHasSetData()))
            return CodecReturnCodes.FAILURE;
        elementEntry.clear();
        boolean foundVersion = false;
        boolean foundType = false;
        while ((ret = elementEntry.decode(seriesDecodeIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            if (elementEntry.name().equals(ElementNames.DICT_VERSION))
            {
                foundVersion = true;
                Buffer versionString = elementEntry.encodedData();
                version.data(versionString.data(), versionString.position(), versionString.length());
            }
            if (elementEntry.name().equals(ElementNames.DICT_TYPE))
            {
                ret = tmpUInt.decode(seriesDecodeIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                dictionaryType((int)tmpUInt.toLong());
                foundType = true;
            }
            if (!(foundVersion || foundType))
                return CodecReturnCodes.FAILURE;
        }

        return CodecReturnCodes.SUCCESS;
    }

    public int verbosity()
    {
        return verbosity;
    }

    public void verbosity(int verbosity)
    {
        this.verbosity = verbosity;
    }

    public long sequenceNumber()
    {
        return sequenceNumber;
    }

    public void sequenceNumber(long sequenceNumber)
    {
        assert(checkHasSequenceNumber());
        this.sequenceNumber = sequenceNumber;
    }

    public int serviceId()
    {
        return serviceId;
    }

    public void serviceId(int serviceId)
    {
        this.serviceId = serviceId;
    }

    public State state()
    {
        return state;
    }

    public void state(State state)
    {
        state().streamState(state.streamState());
        state().dataState(state.dataState());
        state().code(state.code());
        state().text(state.text());
    }
    
    public int dictionaryType()
    {
        return dictionaryType;
    }

    public void dictionaryType(int dictionaryType)
    {
        this.dictionaryType = dictionaryType;
    }

    public Buffer dictionaryName()
    {
        return dictionaryName;
    }
    
    public void dictionaryName(Buffer dictionaryName)
    {
        this.dictionaryName.data(dictionaryName.data(), dictionaryName.position(), dictionaryName.length());
    }
    
    public Buffer version()
    {
    	return version;
    }

    public Buffer dataBody()
    {
        return dataBody;
    }

    public int startFid()
    {
        return startFid;
    }

    public int startEnumTableCount()
    {
        return startEnumTableCount;
    }    

    public void startFid(int startFid)
    {
        this.startFid = startFid;
    }

    public void startEnumTableCount(int startEnumTableCount)
    {
        this.startEnumTableCount = startEnumTableCount;
    }     
    
    public DataDictionary dictionary()
    {
    	if (userSetDictionary)
    		return dictionary;
    	else if (dictionary == null)
    		dictionary = CodecFactory.createDataDictionary();
        return dictionary;
    }

    public void dictionary(DataDictionary dictionary)
    {
        this.dictionary = dictionary;
        userSetDictionary = true;
    }

    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "DictionaryRefresh: \n");

        stringBuf.append(tab);
        stringBuf.append("dictionaryName: ");
        stringBuf.append(dictionaryName());
        stringBuf.append(eol);
        
        stringBuf.append(tab);
        stringBuf.append(state());
        stringBuf.append(eol);
        

        stringBuf.append(tab);
        stringBuf.append("IsRefreshComplete: " + checkRefreshComplete());
        stringBuf.append(eol);
        
        stringBuf.append(tab);
        stringBuf.append("IsClearCache: " + checkClearCache());
        stringBuf.append(eol);
        
        stringBuf.append(tab);
        stringBuf.append("isSolicited: " + checkSolicited());
        stringBuf.append(eol);
        
        stringBuf.append(tab);
        stringBuf.append("verbosity: ");
        boolean addOr = false;
        long verbosity = verbosity();
        if ((verbosity & Dictionary.VerbosityValues.INFO) != 0)
        {
            stringBuf.append("INFO");
            addOr = true;
        }
        if ((verbosity & Dictionary.VerbosityValues.MINIMAL) != 0)
        {
            if (addOr)
                stringBuf.append(" | ");
            stringBuf.append("MINIMAL");
            addOr = true;
        }
        if ((verbosity & Dictionary.VerbosityValues.NORMAL) != 0)
        {
            if (addOr)
                stringBuf.append(" | ");
            stringBuf.append("NORMAL");
            addOr = true;
        }
        if ((verbosity & Dictionary.VerbosityValues.VERBOSE) != 0)
        {
            if (addOr)
                stringBuf.append(" | ");
            stringBuf.append("VERBOSE");
            addOr = true;
        }
        stringBuf.append(eol);

        return stringBuf.toString();
    }
    
    @Override
    public int domainType()
    {
        return DomainTypes.DICTIONARY;
    }
}