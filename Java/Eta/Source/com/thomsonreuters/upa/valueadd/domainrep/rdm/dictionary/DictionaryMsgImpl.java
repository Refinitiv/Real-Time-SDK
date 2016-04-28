package com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.rdm.DomainTypes;

class DictionaryMsgImpl implements DictionaryMsg, DictionaryRequest, DictionaryClose, DictionaryStatus, DictionaryRefresh
{
    private DictionaryMsgType rdmMsgType;

    private DictionaryRequestImpl dictionaryRequest;
    private DictionaryCloseImpl dictionaryClose;
    private DictionaryStatusImpl dictionaryStatus;
    private DictionaryRefreshImpl dictionaryRefresh;

    private DictionaryRefreshImpl rdmDictionaryRefresh()
    {
        return dictionaryRefresh;
    }

    private DictionaryStatusImpl rdmDictionaryStatus()
    {
        return dictionaryStatus;
    }

    private DictionaryRequestImpl rdmDictionaryRequest()
    {
        return dictionaryRequest;
    }

    private DictionaryCloseImpl rdmDictionaryClose()
    {
        return dictionaryClose;
    }

    DictionaryMsgImpl()
    {
        rdmMsgType = DictionaryMsgType.UNKNOWN;
    }

    // /////////////////////////// common ////////////////////////
    @Override
    public int streamId()
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                return rdmDictionaryRequest().streamId();
            case CLOSE:
                return rdmDictionaryClose().streamId();
            case STATUS:
                return rdmDictionaryStatus().streamId();
            case REFRESH:
                return rdmDictionaryRefresh().streamId();
            default:
                assert (false);
                return 0;
        }
    }

    @Override
    public void streamId(int streamId)
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                rdmDictionaryRequest().streamId(streamId);
                break;
            case CLOSE:
                rdmDictionaryClose().streamId(streamId);
                break;
            case STATUS:
                rdmDictionaryStatus().streamId(streamId);
                break;
            case REFRESH:
                rdmDictionaryRefresh().streamId(streamId);
                break;
            default:
                assert (false);
        }
    }

    @Override
    public String toString()
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                return rdmDictionaryRequest().toString();
            case CLOSE:
                return rdmDictionaryClose().toString();
            case STATUS:
                return rdmDictionaryStatus().toString();
            case REFRESH:
                return rdmDictionaryRefresh().toString();
            default:
                assert (false);
                return null;
        }
    }

    @Override
    public int encode(EncodeIterator encodeIter)
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                return rdmDictionaryRequest().encode(encodeIter);
            case CLOSE:
                return rdmDictionaryClose().encode(encodeIter);
            case STATUS:
                return rdmDictionaryStatus().encode(encodeIter);
            case REFRESH:
                return rdmDictionaryRefresh().encode(encodeIter);
            default:
                assert (false);
                return 0;
        }
    }

    @Override
    public int decode(DecodeIterator dIter, Msg msg)
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                return rdmDictionaryRequest().decode(dIter, msg);
            case CLOSE:
                return rdmDictionaryClose().decode(dIter, msg);
            case STATUS:
                return rdmDictionaryStatus().decode(dIter, msg);
            case REFRESH:
                return rdmDictionaryRefresh().decode(dIter, msg);
            default:
                assert (false);
                return 0;
        }
    }

    @Override
    public void clear()
    {
        if (rdmDictionaryRefresh() != null)
        	rdmDictionaryRefresh().clear();
        if (rdmDictionaryClose() != null)
        	rdmDictionaryClose().clear();
        if (rdmDictionaryStatus() != null)
        	rdmDictionaryStatus().clear();
        if (rdmDictionaryRequest() != null)
        	rdmDictionaryRequest().clear();
    }

    @Override
    public DictionaryMsgType rdmMsgType()
    {
        return rdmMsgType;
    }

    @Override
    public void rdmMsgType(DictionaryMsgType rdmDictionaryMsgType)
    {
        this.rdmMsgType = rdmDictionaryMsgType;

        switch (rdmMsgType())
        {
            case REQUEST:
                if (dictionaryRequest == null)
                    dictionaryRequest = new DictionaryRequestImpl();
                break;
            case CLOSE:
                if (dictionaryClose == null)
                    dictionaryClose = new DictionaryCloseImpl();
                break;
            case STATUS:
                if (dictionaryStatus == null)
                    dictionaryStatus = new DictionaryStatusImpl();
                break;
            case REFRESH:
                if (dictionaryRefresh == null)
                    dictionaryRefresh = new DictionaryRefreshImpl();
                break;
            default:
                assert (false);
                break;
        }
    }

    @Override
    public void dictionaryName(Buffer dictionaryName)
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                rdmDictionaryRequest().dictionaryName(dictionaryName);
                break;
            case REFRESH:
                rdmDictionaryRefresh().dictionaryName(dictionaryName);
                break;
            default:
                assert (false);
        }
    }
    
    @Override
    public Buffer dictionaryName()
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                return rdmDictionaryRequest().dictionaryName();
            case REFRESH:
                return rdmDictionaryRefresh().dictionaryName();
            default:
                assert (false);
                return null;
        }
    }

    @Override
    public int serviceId()
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                return rdmDictionaryRequest().serviceId();
            case REFRESH:
                return rdmDictionaryRefresh().serviceId();
            default:
                assert (false);
                return 0;
        }
    }

    @Override
    public void serviceId(int serviceId)
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                rdmDictionaryRequest().serviceId(serviceId);
                break;
            case REFRESH:
                rdmDictionaryRefresh().serviceId(serviceId);
                break;
            default:
                assert (false);
        }
    }

    @Override
    public State state()
    {
        switch (rdmMsgType())
        {
            case REFRESH:
                return rdmDictionaryRefresh().state();
            case STATUS:
                return rdmDictionaryStatus().state();
            default:
                assert (false);
                return null;
        }
    }
    
    @Override
    public void state(State state)
    {
        switch (rdmMsgType())
        {
            case REFRESH:
                rdmDictionaryRefresh().state(state);
                break;
            case STATUS:
                rdmDictionaryStatus().state(state);
                break;
            default:
                assert (false);
        }
    }

    // //////////////////// Common to some messages ///////////////////////////
    @Override
    public int flags()
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                return rdmDictionaryRequest().flags();
            case STATUS:
                return rdmDictionaryStatus().flags();
            case REFRESH:
                return rdmDictionaryRefresh().flags();
            default:
                assert (false);
                return 0;
        }
    }

    @Override
    public void flags(int flags)
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                rdmDictionaryRequest().flags(flags);
                break;
            case STATUS:
                rdmDictionaryStatus().flags(flags);
                break;
            case REFRESH:
                rdmDictionaryRefresh().flags(flags);
                break;
            default:
                assert (false);
        }
    }

    @Override
    public int verbosity()
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                return rdmDictionaryRequest().verbosity();
            case REFRESH:
                return rdmDictionaryRefresh().verbosity();
            default:
                assert (false);
                return 0;
        }
    }

    @Override
    public void verbosity(int verbosity)
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                rdmDictionaryRequest().verbosity(verbosity);
                break;
            case REFRESH:
                rdmDictionaryRefresh().verbosity(verbosity);
                break;
            default:
                assert (false);
        }
    }

    // ///////////// Dictionary Close ////////////////////////
    public int copy(DictionaryClose destCloseMsg)
    {
        return rdmDictionaryClose().copy(destCloseMsg);
    }

    // ///////////// Dictionary Request ////////////////////////
    @Override
    public int copy(DictionaryRequest destRequestMsg)
    {
        return rdmDictionaryRequest().copy(destRequestMsg);
    }
    @Override
    public void applyStreaming()
    {
        rdmDictionaryRequest().applyStreaming();
    }

    @Override
    public boolean checkStreaming()
    {
        return rdmDictionaryRequest().checkStreaming();
    }

    // /////////////////////// status message /////////////////////
    @Override
    public int copy(DictionaryStatus destStatusMsg)
    {
        return rdmDictionaryStatus().copy(destStatusMsg);
    }

    @Override
    public boolean checkHasState()
    {
       return rdmDictionaryStatus().checkHasState();
    }

    @Override
    public void applyHasState()
    {
        rdmDictionaryStatus().applyHasState();
    }

    // ////////////////////// Dictionary Refresh /////////////////////
    @Override
    public DataDictionary dictionary()
    {
        return rdmDictionaryRefresh().dictionary();
    }

    @Override
    public void dictionary(DataDictionary encDictionary)
    {
        rdmDictionaryRefresh().dictionary(encDictionary);
    }

    @Override
    public long sequenceNumber()
    {
        return rdmDictionaryRefresh().sequenceNumber();
    }

    @Override
    public void sequenceNumber(long sequenceNumber)
    {
        rdmDictionaryRefresh().sequenceNumber(sequenceNumber);
    }

    @Override
    public int dictionaryType()
    {
        return rdmDictionaryRefresh().dictionaryType();
    }

    @Override
    public void dictionaryType(int dictionaryType)
    {
        rdmDictionaryRefresh().dictionaryType(dictionaryType);
    }

    @Override
    public Buffer dataBody()
    {
        return rdmDictionaryRefresh().dataBody();
    }

    @Override
    public void startFid(int startFid)
    {
        rdmDictionaryRefresh().startFid(startFid);
    }

    @Override
    public int startFid()
    {
        return rdmDictionaryRefresh().startFid();
    }

    @Override
    public int copy(DictionaryRefresh destRefreshMsg)
    {
        return rdmDictionaryRefresh().copy(destRefreshMsg);
    }

    @Override
    public boolean checkSolicited()
    {
       return rdmDictionaryRefresh().checkSolicited();
    }

    @Override
    public boolean checkClearCache()
    {
    	switch (rdmMsgType())
        {
	    	case REFRESH:
	        	return rdmDictionaryRefresh().checkClearCache();
	    	case STATUS:
	    		return rdmDictionaryStatus().checkClearCache();
			default:
				assert (false); // not supported on this message class
				return false;
        }
    }

    @Override
    public boolean checkRefreshComplete()
    {
        return rdmDictionaryRefresh().checkRefreshComplete();
    }

    @Override
    public void applyClearCache()
    {
    	switch (rdmMsgType())
        {
	    	case REFRESH:
	        	rdmDictionaryRefresh().applyClearCache();
	        	break;
	    	case STATUS:
	    		rdmDictionaryStatus().applyClearCache();
	    		break;
			default:
				assert(false);  // not supported on this message class
        }
    }

    @Override
    public void applySolicited()
    {
        rdmDictionaryRefresh().applySolicited();
    }

    @Override
    public void applyRefreshComplete()
    {
        rdmDictionaryRefresh().applyRefreshComplete();
    }

    @Override
    public boolean checkHasSequenceNumber()
    {
        return rdmDictionaryRefresh().checkHasSequenceNumber();
    }

    @Override
    public void applyHasSequenceNumber()
    {
        rdmDictionaryRefresh().applyHasSequenceNumber();
    }

    @Override
    public boolean checkHasInfo()
    {
        return rdmDictionaryRefresh().checkHasInfo();
    }

    @Override
    public void applyHasInfo()
    {
        rdmDictionaryRefresh().applyHasInfo();
    }
    
    @Override
    public int domainType()
    {
        return DomainTypes.DICTIONARY;
    }
}