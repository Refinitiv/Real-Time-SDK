/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueMsgType;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBaseImpl;

abstract class QueueMsgImpl extends MsgBaseImpl implements QueueMsg
{
	class OpCodes
	{
		static final int INIT = 0;
		static final int DATA = 1;
		static final int ACK = 2;
		static final int REQUEST = 3;
		static final int DEAD_LETTER = 4;
		static final int REFRESH = -1;
		static final int CLOSE = -2;
		static final int STATUS = -3;
	}		
	
	int _domainType;
	int _opCode;
	int _serviceId;
	int _lastInSeqNum;
	int _lastOutSeqNum;
	int _seqNum;
	int _secondarySeqNum;
	long _timeout;
	int _containerType;
	long _identifier;
	int _undeliverableCode;	

	int _type;
	State _state = CodecFactory.createState();
	
	static final int QMSG_MAX_NAME_LENGTH = 200;
	
	QueueMsgImpl() {
		super();		
	}

	String opCodeToString(int opCode)
	{
		switch(opCode)
		{
			case OpCodes.INIT:
				return "INIT";
			case OpCodes.DATA:
				return "DATA";
			case OpCodes.ACK:
				return "ACK";
			case OpCodes.REQUEST:
				return "REQUEST";
			case OpCodes.DEAD_LETTER:
				return "DEAD_LETTER";
			case OpCodes.REFRESH:
				return "REFRESH";
			default:
				return "Unknown";
		}
	}

	
	public QueueMsgType rdmMsgType()
	{
		return QueueMsgType.UNKNOWN;
	}
	    	
    public int domainType()
    {
        return _domainType;
    }
    
    public void domainType(int domainType)
    {
    	_domainType = domainType;
    }
    
    public int serviceId()
    {
    	return _serviceId;
    }
    
    public void serviceId(int serviceId)
    {
    	_serviceId = serviceId;
    }

	int lastInSeqNum()
	{
		return _lastInSeqNum;
	}

	int lastOutSeqNum()
	{
		return _lastOutSeqNum;
	}

	int secondarySeqNum()
	{
		return _secondarySeqNum;
	}
	
    void lastOutSeqNum(int lastOutSeqNum)
    {
    	_lastOutSeqNum = lastOutSeqNum;
    }
    
    void lastInSeqNum(int lastInSeqNum)
    {
    	_lastInSeqNum = lastInSeqNum;
    }
    
    void secondarySeqNum(int secondarySeqNum)
    {
    	_secondarySeqNum = secondarySeqNum;
    } 
    
	int seqNum()
	{
		return _seqNum;
	}

	void seqNum(int seqNum)
	{
		_seqNum = seqNum;
	}

	int opCode()
	{
		return _opCode;
	}

	void opCode(int opCode)
	{
		_opCode = opCode;
	}
	
	State state()
	{
		return _state;
	}

	void state(State state)
	{
		_state = state;
	}
    
    String xmlDump()
    {
        switch(_opCode)
        {
            case OpCodes.ACK:
                return "<QueueAck streamId=\"" + streamId() + "\" sourceName=\"" + ((QueueAckImpl)this).sourceName() + "\" destName=\"" + ((QueueAckImpl)this).destName() + "\" seqNum=\"" + _seqNum + "\" identifier=\"" + _identifier + "\" />\n";
            case OpCodes.DATA:
                return "<QueueData streamId=\"" + streamId() + "\" sourceName=\"" + ((QueueDataImpl)this).sourceName() + "\" destName=\"" + ((QueueDataImpl)this).destName() + "\" seqNum=\"" + _seqNum + "\" timeout=\"" + _timeout + "\" identifier=\"" + _identifier + "\" />\n";
            case OpCodes.DEAD_LETTER:
                return "<QueueDataExpired streamId=\"" + streamId() + "\" sourceName=\"" + ((QueueDataExpiredImpl)this).sourceName() + "\" destName=\"" + ((QueueDataExpiredImpl)this).destName() + "\" seqNum=\"" + _seqNum + "\" timeout=\"" + _timeout + "\" identifier=\"" + _identifier + "\" undeliverableCode=\"" + _undeliverableCode + "\" />\n";
            case OpCodes.REQUEST:
                return "<QueueRequest streamId=\"" + streamId() + "\" sourceName=\"" + ((QueueRequestImpl)this).sourceName() + "\" lastOutSeqNum=\"" + _lastOutSeqNum + "\" lastInSeqNum=\"" + _lastInSeqNum + "\" />\n";
            case OpCodes.REFRESH:
                return "<QueueRefresh streamId=\"" + streamId() + "\" sourceName=\"" + ((QueueRefreshImpl)this).sourceName() + "\" lastOutSeqNum=\"" + _lastOutSeqNum + "\" lastInSeqNum=\"" + _lastInSeqNum + "\" />\n";
            case OpCodes.CLOSE:
                return "<QueueClose>\n";
            default:
                return "<UnknownQueueMsg />\n";
        }
    }       
}
