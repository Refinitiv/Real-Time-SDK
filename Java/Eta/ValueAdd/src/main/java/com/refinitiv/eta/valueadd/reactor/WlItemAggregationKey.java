package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.valueadd.common.VaNode;

/* Key used for aggregating item requests by MsgKey, domain type and Qos. */
class WlItemAggregationKey extends VaNode
{
    MsgKey _msgKey = CodecFactory.createMsgKey();
    MsgKey _msgKeyReference;
    int _domainType;
    Qos _qos = CodecFactory.createQos();
    Qos _qosReference;
    final int SEED = 23;
    final int PRIME = 31;
    
    /* Returns the MsgKey of the item aggregation key. Use to get or set MsgKey. */
    MsgKey msgKey()
    {
        if (_msgKeyReference != null)
        {
            return _msgKeyReference;
        }
        else
        {
            return _msgKey;
        }
    }
    
    /* Sets the MsgKey of the item aggregation key. Sets as a reference. */
    void msgKey(MsgKey msgKey)
    {
        _msgKeyReference = msgKey;
    }
    
    /* Returns the domain type of the item aggregation key. */
    int domainType()
    {
        return _domainType;
    }

    /* Sets the domain type of the item aggregation key. */
    void domainType(int domainType)
    {
        _domainType = domainType;
    }

    /* Returns the Qos of the item aggregation key. Use to get or set Qos. */
    Qos qos()
    {
        if (_qosReference != null)
        {
            return _qosReference;
        }
        else
        {
            return _qos;
        }
    }

    /* Sets the Qos of the item aggregation key. Sets as a reference. */
    void qos(Qos qos)
    {
        _qosReference = qos;
    }

    /* Performs a deep copy of this Object to destItemAggregationKey. */
    void copy(WlItemAggregationKey destItemAggregationKey)
    {
        if (_msgKeyReference != null)
        {
            _msgKeyReference.copy(destItemAggregationKey.msgKey());
        }
        else
        {
            _msgKey.copy(destItemAggregationKey.msgKey());
        }
        destItemAggregationKey.domainType(_domainType);
        if (_qosReference != null)
        {
            _qosReference.copy(destItemAggregationKey.qos());
        }
        else
        {
            _qos.copy(destItemAggregationKey.qos());
        }
    }

    @Override
    public boolean equals(Object obj)
    {
        if (obj == this)
        {
            return true;
        }
        
        WlItemAggregationKey thatKey;
        
        try
        {
            thatKey = (WlItemAggregationKey)obj;
        }
        catch (ClassCastException e)
        {
            return false;
        }        
        
        MsgKey msgKey = null;
        if (_msgKeyReference != null)
        {
            msgKey = _msgKeyReference;
        }
        else
        {
            msgKey = _msgKey;
        }
        
        Qos qos = null;
        if (_qosReference != null)
        {
            qos = _qosReference;
        }
        else
        {
            qos = _qos;
        }
        
        return msgKey.equals(thatKey.msgKey()) &&
               _domainType == thatKey.domainType() &&
               qos.equals(thatKey.qos());
    }
    
    @Override
    public int hashCode()
    {
        int result = SEED;
        
        MsgKey msgKey = null;
        if (_msgKeyReference != null)
        {
            msgKey = _msgKeyReference;
        }
        else
        {
            msgKey = _msgKey;
        }
        
        Qos qos = null;
        if (_qosReference != null)
        {
            qos = _qosReference;
        }
        else
        {
            qos = _qos;
        }
        
        result = PRIME * result * msgKey.hashCode();
        result = PRIME * result * _domainType;
        result = PRIME * result * qos.hashCode();
        
        return result;
    }
    
    /* Clears the object for reuse. */
    void clear()
    {
        _msgKey.clear();
        _msgKeyReference = null;
        _domainType = 0;
        _qos.clear();
        _qosReference = null;
    }
}
