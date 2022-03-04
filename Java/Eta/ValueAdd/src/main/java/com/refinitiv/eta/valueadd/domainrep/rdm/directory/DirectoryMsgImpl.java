/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.directory;

import java.util.List;

import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.rdm.DomainTypes;

class DirectoryMsgImpl implements DirectoryMsg, DirectoryClose, DirectoryStatus, DirectoryRequest, DirectoryConsumerStatus, DirectoryUpdate, DirectoryRefresh
{
    private DirectoryMsgType rdmMsgType;
    private DirectoryCloseImpl directoryClose;
    private DirectoryStatusImpl directoryStatus;
    private DirectoryRequestImpl directoryRequest;
    private DirectoryUpdateImpl directoryUpdate;
    private DirectoryRefreshImpl directoryRefresh;
    private DirectoryConsumerStatusImpl directoryConsumerStatus;

    DirectoryMsgImpl()
    {
    	rdmMsgType = DirectoryMsgType.UNKNOWN;
    }

    private DirectoryCloseImpl rdmDirectoryClose()
    {
        return directoryClose;
    }

    private DirectoryStatusImpl rdmDirectoryStatus()
    {
        return directoryStatus;
    }

    private DirectoryRequestImpl rdmDirectoryRequest()
    {
        return directoryRequest;
    }

    private DirectoryConsumerStatusImpl rdmDirectoryConsumerStatus()
    {
        return directoryConsumerStatus;
    }

    private DirectoryUpdateImpl rdmDirectoryUpdate()
    {
        return directoryUpdate;
    }

    private DirectoryRefreshImpl rdmDirectoryRefresh()
    {
        return directoryRefresh;
    }

    // ///////////
    @Override
    public DirectoryMsgType rdmMsgType()
    {
        return rdmMsgType;
    }

    @Override
    public void rdmMsgType(DirectoryMsgType rdmMsgType)
    {
        this.rdmMsgType = rdmMsgType;
        
        switch (rdmMsgType())
        {
            case REQUEST:
                if (directoryRequest == null)
                    directoryRequest = new DirectoryRequestImpl();        	
                break;
            case CLOSE:
                if (directoryClose == null)
                    directoryClose = new DirectoryCloseImpl();        	
                break;
            case CONSUMER_STATUS:
                if (directoryConsumerStatus == null)
                    directoryConsumerStatus = new DirectoryConsumerStatusImpl();        	
                break;
            case STATUS:
                if (directoryStatus == null)
                    directoryStatus = new DirectoryStatusImpl();        	
                break;
            case UPDATE:
                if (directoryUpdate == null)
                    directoryUpdate = new DirectoryUpdateImpl();        	
                break;
            case REFRESH:
                if (directoryRefresh == null)
                    directoryRefresh = new DirectoryRefreshImpl();        	
                break;
            default:
                assert (false);
                break;
        }
    }

    @Override
    public int streamId()
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                return rdmDirectoryRequest().streamId();
            case CLOSE:
                return rdmDirectoryClose().streamId();
            case CONSUMER_STATUS:
                return rdmDirectoryConsumerStatus().streamId();
            case STATUS:
                return rdmDirectoryStatus().streamId();
            case UPDATE:
                return rdmDirectoryUpdate().streamId();
            case REFRESH:
                return rdmDirectoryRefresh().streamId();
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
                rdmDirectoryRequest().streamId(streamId);
                break;
            case CLOSE:
                rdmDirectoryClose().streamId(streamId);
                break;
            case CONSUMER_STATUS:
                rdmDirectoryConsumerStatus().streamId(streamId);
                break;
            case STATUS:
                rdmDirectoryStatus().streamId(streamId);
                break;
            case UPDATE:
                rdmDirectoryUpdate().streamId(streamId);
                break;
            case REFRESH:
                rdmDirectoryRefresh().streamId(streamId);
                break;
            default:
                assert (false);
        }
    }

    public String toString()
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                return rdmDirectoryRequest().toString();
            case CLOSE:
                return rdmDirectoryClose().toString();
            case CONSUMER_STATUS:
                return rdmDirectoryConsumerStatus().toString();
            case STATUS:
                return rdmDirectoryStatus().toString();
            case UPDATE:
                return rdmDirectoryUpdate().toString();
            case REFRESH:
                return rdmDirectoryRefresh().toString();
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
                return rdmDirectoryRequest().encode(encodeIter);
            case CLOSE:
                return rdmDirectoryClose().encode(encodeIter);
            case CONSUMER_STATUS:
                return rdmDirectoryConsumerStatus().encode(encodeIter);
            case STATUS:
                return rdmDirectoryStatus().encode(encodeIter);
            case UPDATE:
                return rdmDirectoryUpdate().encode(encodeIter);
            case REFRESH:
                return rdmDirectoryRefresh().encode(encodeIter);
            default:
                assert (false);
                return CodecReturnCodes.FAILURE;
        }
    }

    @Override
    public int decode(DecodeIterator dIter, Msg msg)
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                return rdmDirectoryRequest().decode(dIter, msg);
            case CLOSE:
                return rdmDirectoryClose().decode(dIter, msg);
            case CONSUMER_STATUS:
                return rdmDirectoryConsumerStatus().decode(dIter, msg);
            case STATUS:
                return rdmDirectoryStatus().decode(dIter, msg);
            case UPDATE:
                return rdmDirectoryUpdate().decode(dIter, msg);
            case REFRESH:
                return rdmDirectoryRefresh().decode(dIter, msg);
            default:
                assert (false);
                return CodecReturnCodes.FAILURE;
        }
    }

    @Override
    public void clear()
    {
        if (rdmDirectoryRefresh() != null)
        	rdmDirectoryRefresh().clear();
        if (rdmDirectoryClose() != null)
        	rdmDirectoryClose().clear();
        if (rdmDirectoryStatus() != null)
        	rdmDirectoryStatus().clear();
        if (rdmDirectoryConsumerStatus() != null)
        	rdmDirectoryConsumerStatus().clear();
        if (rdmDirectoryRequest() != null)
        	rdmDirectoryRequest().clear();
        if (rdmDirectoryUpdate() != null)
        	rdmDirectoryUpdate().clear();
    }

    @Override
    public void flags(int flags)
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                rdmDirectoryRequest().flags(flags);
                break;
            case STATUS:
                rdmDirectoryStatus().flags(flags);
                break;
            case UPDATE:
                rdmDirectoryUpdate().flags(flags);
                break;
            case REFRESH:
                rdmDirectoryRefresh().flags(flags);
                break;
            default:
                assert (false);
        }
    }

    @Override
    public int flags()
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                return rdmDirectoryRequest().flags();
            case STATUS:
                return rdmDirectoryStatus().flags();
            case UPDATE:
                return rdmDirectoryUpdate().flags();
            case REFRESH:
                return rdmDirectoryRefresh().flags();
            default:
                assert (false);
                return 0;
        }
    }

    // ///////////////////////////// Status and Refresh
    // /////////////////////////////////
    @Override
    public State state()
    {
        switch (rdmMsgType())
        {
            case STATUS:
                return rdmDirectoryStatus().state();
            case REFRESH:
                return rdmDirectoryRefresh().state();
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
            case STATUS:
                rdmDirectoryStatus().state(state);
                break;
            case REFRESH:
                rdmDirectoryRefresh().state(state);
                break;
            default:
                assert (false);
        }
    }

    // ///////////////////////////// Close //////////////////////////////////
    @Override
    public int copy(DirectoryClose destCloseMsg)
    {
        return rdmDirectoryClose().copy(destCloseMsg);
    }

    // ///////////////////////////// Status //////////////////////////////////
    @Override
    public int copy(DirectoryStatus destStatusMsg)
    {
        return rdmDirectoryStatus().copy(destStatusMsg);
    }

    @Override
    public void applyHasFilter()
    {
        switch (rdmMsgType())
        {
            case STATUS:
                rdmDirectoryStatus().applyHasFilter();
                break;
            case UPDATE:
                rdmDirectoryUpdate().applyHasFilter();
                break;
            default:
                assert (false);

        }
    }

    @Override
    public boolean checkHasFilter()
    {
        switch (rdmMsgType())
        {
            case STATUS:
                return rdmDirectoryStatus().checkHasFilter();
            case UPDATE:
                return rdmDirectoryUpdate().checkHasFilter();
            default:
                assert (false);
                return false;
        }
    }

    @Override
    public void applyHasState()
    {
        rdmDirectoryStatus().applyHasState();
    }

    @Override
    public boolean checkHasState()
    {
        return rdmDirectoryStatus().checkHasState();
    }

    // ///////////////////////////// Request //////////////////////////////////
    @Override
    public int copy(DirectoryRequest destRequestMsg)
    {
        return rdmDirectoryRequest().copy(destRequestMsg);
    }

    public void applyStreaming()
    {
        rdmDirectoryRequest().applyStreaming();
    }

    public boolean checkStreaming()
    {
        return rdmDirectoryRequest().checkStreaming();
    }

    // ////////////////////// Request and Status ///////////////////////////
    @Override
    public int serviceId()
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                return rdmDirectoryRequest().serviceId();
            case STATUS:
                return rdmDirectoryStatus().serviceId();
            case UPDATE:
                return rdmDirectoryUpdate().serviceId();
            case REFRESH:
                return rdmDirectoryRefresh().serviceId();
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
                rdmDirectoryRequest().serviceId(serviceId);
                break;
            case STATUS:
                rdmDirectoryStatus().serviceId(serviceId);
                break;
            case UPDATE:
                rdmDirectoryUpdate().serviceId(serviceId);
                break;
            case REFRESH:
                rdmDirectoryRefresh().serviceId(serviceId);
                break;
            default:
                assert (false);
        }
    }

    @Override
    public long filter()
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                return rdmDirectoryRequest().filter();
            case STATUS:
                return rdmDirectoryStatus().filter();
            case UPDATE:
                return rdmDirectoryUpdate().filter();
            case REFRESH:
                return rdmDirectoryRefresh().filter();
            default:
                assert (false);
                return 0;
        }
    }

    @Override
    public void filter(long filter)
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                rdmDirectoryRequest().filter(filter);
                break;
            case STATUS:
                rdmDirectoryStatus().filter(filter);
                break;
            case UPDATE:
                rdmDirectoryUpdate().filter(filter);
                break;
            case REFRESH:
                rdmDirectoryRefresh().filter(filter);
                break;
            default:
                assert (false);
        }
    }

    @Override
    public void applyHasServiceId()
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                rdmDirectoryRequest().applyHasServiceId();
                break;
            case STATUS:
                rdmDirectoryStatus().applyHasServiceId();
                break;
            case UPDATE:
                rdmDirectoryUpdate().applyHasServiceId();
                break;
            case REFRESH:
                rdmDirectoryRefresh().applyHasServiceId();
                break;
            default:
                assert (false);
        }
    }

    @Override
    public boolean checkHasServiceId()
    {
        switch (rdmMsgType())
        {
            case REQUEST:
                return rdmDirectoryRequest().checkHasServiceId();
            case STATUS:
                return rdmDirectoryStatus().checkHasServiceId();
            case UPDATE:
                return rdmDirectoryUpdate().checkHasServiceId();
            case REFRESH:
                return rdmDirectoryRefresh().checkHasServiceId();
            default:
                assert (false);
                return false;
        }
    }

    // ///////////////////////////// DirectoryConsumerStatus
    // //////////////////////////
    @Override
    public List<ConsumerStatusService> consumerServiceStatusList()
    {
        return rdmDirectoryConsumerStatus().consumerServiceStatusList();
    }

    @Override
    public void consumerServiceStatusList(List<ConsumerStatusService> consumerServiceStatusList)
    {
        rdmDirectoryConsumerStatus().consumerServiceStatusList(consumerServiceStatusList);
    }
    
    @Override
    public int copy(DirectoryConsumerStatus destConsumerStatus)
    {
        return rdmDirectoryConsumerStatus().copy(destConsumerStatus);
    }

    // ///////////////////////////// DirectoryUpdate and DirectoryRefresh
    // //////////////////////////
    @Override
    public List<Service> serviceList()
    {
        switch (rdmMsgType)
        {
            case REFRESH:
                return rdmDirectoryRefresh().serviceList();
            case UPDATE:
                return rdmDirectoryUpdate().serviceList();
            default:
                assert (false);
                return null;
        }
    }
    
    @Override
    public void serviceList(List<Service> serviceList)
    {
        switch (rdmMsgType)
        {
            case REFRESH:
                rdmDirectoryRefresh().serviceList(serviceList);
                break;
            case UPDATE:
                rdmDirectoryUpdate().serviceList(serviceList);
                break;
            default:
                assert (false);
        }
    }

    @Override
    public long sequenceNumber()
    {
        switch (rdmMsgType)
        {
            case REFRESH:
                return rdmDirectoryRefresh().sequenceNumber();
            case UPDATE:
                return rdmDirectoryUpdate().sequenceNumber();
            default:
                assert (false);
                return 0;
        }
    }

    @Override
    public void sequenceNumber(long sequenceNumber)
    {
        switch (rdmMsgType)
        {
            case REFRESH:
                rdmDirectoryRefresh().sequenceNumber(sequenceNumber);
                break;
            case UPDATE:
                rdmDirectoryUpdate().sequenceNumber(sequenceNumber);
                break;
            default:
                assert (false);
        }
    }

    @Override
    public boolean checkHasSequenceNumber()
    {
        switch (rdmMsgType)
        {
            case REFRESH:
                return rdmDirectoryRefresh().checkHasSequenceNumber();
            case UPDATE:
                return rdmDirectoryUpdate().checkHasSequenceNumber();
            default:
                assert (false);
                return false;
        }
    }

    @Override
    public void applyHasSequenceNumber()
    {
        switch (rdmMsgType)
        {
            case REFRESH:
                rdmDirectoryRefresh().applyHasSequenceNumber();
                break;
            case UPDATE:
                rdmDirectoryUpdate().applyHasSequenceNumber();
                break;
            default:
                assert (false);
        }
    }

    // /////////////////////////// DirectoryUpdate ////////////////////////////
    @Override
    public int copy(DirectoryUpdate decRDMMsg)
    {
        return rdmDirectoryUpdate().copy(decRDMMsg);
    }

    // /////////////////////////// DirectoryRefresh ////////////////////////////
    @Override
    public boolean checkClearCache()
    {
    	switch (rdmMsgType())
        {
	    	case REFRESH:
	        	return rdmDirectoryRefresh().checkClearCache();
	    	case STATUS:
	    		return rdmDirectoryStatus().checkClearCache();
			default:
				assert (false); // not supported on this message class
				return false;
        }
    }

    @Override
    public void applyClearCache()
    {
    	switch (rdmMsgType())
        {
	    	case REFRESH:
	        	rdmDirectoryRefresh().applyClearCache();
	        	break;
	    	case STATUS:
	    		rdmDirectoryStatus().applyClearCache();
	    		break;
			default:
				assert(false);  // not supported on this message class
        }
    }

    @Override
    public boolean checkSolicited()
    {
        return rdmDirectoryRefresh().checkSolicited();
    }

    @Override
    public void applySolicited()
    {
        rdmDirectoryRefresh().applySolicited();
    }

    @Override
    public int copy(DirectoryRefresh rdmDirRefreshMsg2)
    {
        return rdmDirectoryRefresh().copy(rdmDirRefreshMsg2);
    }
    
    @Override
    public int domainType()
    {
        return DomainTypes.SOURCE;
    }
}
