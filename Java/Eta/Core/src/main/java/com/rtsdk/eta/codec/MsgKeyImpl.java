package com.rtsdk.eta.codec;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.BufferImpl;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.MsgKey;
import com.rtsdk.eta.codec.MsgKeyFlags;

class MsgKeyImpl implements MsgKey
{
    private int         _flags;
    private int         _serviceId;
    private int         _nameType;
    private final Buffer    _name = CodecFactory.createBuffer();
    private long        _filter;
    private int         _identifier;
    private int         _attribContainerType = DataTypes.CONTAINER_TYPE_MIN;
    private final Buffer    _encodedAttrib = CodecFactory.createBuffer();
    final int SEED = 23;
    final int PRIME = 31;
	
    @Override
    public void clear()
    {
        _flags = 0;
        _serviceId = 0;
        _nameType = 0;
        _name.clear();
        _filter = 0L;
        _identifier = 0;
        _attribContainerType = DataTypes.CONTAINER_TYPE_MIN;
        _encodedAttrib.clear();
    }
    
    void copyReferences(MsgKey msgKey)
    {
        _flags = msgKey.flags();
        _serviceId = msgKey.serviceId();
        _nameType = msgKey.nameType();
        ((BufferImpl)_name).copyReferences(msgKey.name());
        _filter = msgKey.filter();
        _identifier = msgKey.identifier();
        _attribContainerType = msgKey.attribContainerType();
        ((BufferImpl)_encodedAttrib).copyReferences(msgKey.encodedAttrib());
    }
   
    // copies this key into destKey
    // copies name and attrib only if the flags are set
    int copy(MsgKey destKey, int copyFlags)
    {
        int ret;
        destKey.flags(_flags);
        destKey.serviceId(_serviceId);
        destKey.nameType(_nameType);
        destKey.filter(_filter);
        destKey.identifier(_identifier);
        destKey.attribContainerType(_attribContainerType);
        if ((copyFlags & CopyMsgFlags.KEY_NAME) > 0)
        {
            if ((ret = ((BufferImpl)_name).copyWithOrWithoutByteBuffer(destKey.name())) != CodecReturnCodes.SUCCESS)
                return ret;
        }
        else
        {
            destKey.flags(destKey.flags() & ~MsgKeyFlags.HAS_NAME);
        }
        if ((copyFlags & CopyMsgFlags.KEY_ATTRIB) > 0)
        {
            if ((ret = ((BufferImpl)_encodedAttrib).copyWithOrWithoutByteBuffer(destKey.encodedAttrib())) != CodecReturnCodes.SUCCESS)
                return ret;
        }
        else
        {
            destKey.flags(destKey.flags() & ~MsgKeyFlags.HAS_ATTRIB);
        }
        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public boolean checkHasServiceId()
    {
        return ((_flags & MsgKeyFlags.HAS_SERVICE_ID) > 0 ? true : false);
    }

    @Override
    public boolean checkHasName()
    {
        return ((_flags & MsgKeyFlags.HAS_NAME) > 0 ? true : false);
    }

    @Override
    public boolean checkHasNameType()
    {
        return ((_flags & MsgKeyFlags.HAS_NAME_TYPE) > 0 ? true : false);
    }

    @Override
    public boolean checkHasFilter()
    {
        return ((_flags & MsgKeyFlags.HAS_FILTER) > 0 ? true : false);
    }

    @Override
    public boolean checkHasIdentifier()
    {
        return ((_flags & MsgKeyFlags.HAS_IDENTIFIER) > 0 ? true : false);
    }

    @Override
    public boolean checkHasAttrib()
    {
        return ((_flags & MsgKeyFlags.HAS_ATTRIB) > 0 ? true : false);
    }

    @Override
    public void applyHasServiceId()
    {
        _flags |= MsgKeyFlags.HAS_SERVICE_ID;
    }

    @Override
    public void applyHasName()
    {
        _flags |= MsgKeyFlags.HAS_NAME;
    }

    @Override
    public void applyHasNameType()
    {
        _flags |= MsgKeyFlags.HAS_NAME_TYPE;
    }

    @Override
    public void applyHasFilter()
    {
        _flags |= MsgKeyFlags.HAS_FILTER;
    }

    @Override
    public void applyHasIdentifier()
    {
        _flags |= MsgKeyFlags.HAS_IDENTIFIER;
    }

    @Override
    public void applyHasAttrib()
    {
        _flags |= MsgKeyFlags.HAS_ATTRIB;
    }
	
    @Override
    public void flags(int flags)
    {
        assert (flags >= 0 && flags <= 32767) : "flags is out of range (0-32767)"; // uint15-rb

        this._flags = flags;
    }

    @Override
    public int flags()
    {
        return _flags;
    }

    @Override
    public void serviceId(int serviceId)
    {
        assert (serviceId >= 0 && serviceId <= 65535) : "serviceId is out of range (0-65535)"; // uint16

        this._serviceId = serviceId;
    }

    @Override
    public int serviceId()
    {
        return _serviceId;
    }

    @Override
    public void nameType(int nameType)
    {
        assert (nameType >= 0 && nameType <= 255) : "nameType is out of range (0-255)"; // uint8

        this._nameType = nameType;
    }

    @Override
    public int nameType()
    {
        return _nameType;
    }

    @Override
    public void name(Buffer name)
    {
        assert (name != null) : "name must be non-null";

        ((BufferImpl)this._name).copyReferences(name);
    }

    @Override
    public Buffer name()
    {
        return _name;
    }

    @Override
    public void filter(long filter)
    {
        assert (filter >= 0 && filter <= 4294967296L) : "filter is out of range (0-4294967296)"; // uint32

        this._filter = filter;
    }

    @Override
    public long filter()
    {
        return _filter;
    }

    @Override
    public void identifier(int identifier)
    {
        assert (identifier >= -2147483648 && identifier <= 2147483647) : 
            "identifier is out of range ((-2147483648)-2147483647)"; // int32

        this._identifier = identifier;
    }

    @Override
    public int identifier()
    {
        return _identifier;
    }

    @Override
    public void attribContainerType(int attribContainerType)
    {
        assert (attribContainerType >= DataTypes.CONTAINER_TYPE_MIN && attribContainerType <= DataTypes.LAST) : 
            "attribContainerType must be from the DataTypes enumeration in the range CONTAINER_TYPE_MIN to LAST.";

        this._attribContainerType = attribContainerType;
    }

    @Override
    public int attribContainerType()
    {
        return _attribContainerType;
    }

    @Override
    public void encodedAttrib(Buffer encodedAttrib)
    {
        assert (encodedAttrib != null) : "encodedAttrib must be non-null";

        ((BufferImpl)_encodedAttrib).copyReferences(encodedAttrib);
    }

    @Override
    public Buffer encodedAttrib()
    {
        return _encodedAttrib;
    }
    
    @Override
    public boolean equals(MsgKey thatKey)
    {
        if (thatKey == null)
            return false;

        if (((checkHasNameType() && thatKey.checkHasNameType()) && _nameType != thatKey.nameType()) || // they both have a name type and the are NOT equal
                ((checkHasNameType() && _nameType != 1) && !thatKey.checkHasNameType()) || // Only One has a name type and it is NOT set to "1"
                ((thatKey.checkHasNameType() && thatKey.nameType() != 1) && !checkHasNameType())) // One has a name type and it is set to "1" and the other does NOT have a name type
            return false;

        // unset the Name Type from the flag, because we have all ready tested them above.
        int tmpFlags1 = _flags & ~MsgKeyFlags.HAS_NAME_TYPE;
        int tmpFlags2 = thatKey.flags() & ~MsgKeyFlags.HAS_NAME_TYPE;

        if (tmpFlags1 != tmpFlags2) // check the rest of the flags (not including Name Type)
            return false;
        if (checkHasServiceId() && (_serviceId != thatKey.serviceId()))
            return false;
        if (checkHasNameType() && (_nameType != thatKey.nameType()))
            return false;
        if (checkHasName() && !_name.equals(thatKey.name()))
            return false;
        if (checkHasFilter() && (_filter != thatKey.filter()))
            return false;
        if (checkHasIdentifier() && (_identifier != thatKey.identifier()))
            return false;
        if (checkHasAttrib())
        {
            if (_attribContainerType != thatKey.attribContainerType())
                return false;
            if (!_encodedAttrib.equals(thatKey.encodedAttrib()))
                return false;
        }

        return true;
    }
    
    @Override
    public int hashCode()
    {
        int result = SEED;
        result = PRIME * result + _flags;

        if (checkHasServiceId())
        {
            result = PRIME * result + _serviceId;
        }

        if (checkHasNameType())
        {
            result = PRIME * result + _nameType;
        }

        if (checkHasFilter())
        {
            int cc = (int)(_filter ^ (_filter >>> 32));
            result = PRIME * result + cc;
        }

        if (checkHasIdentifier())
        {
            result = PRIME * result + _identifier;
        }

        if (checkHasAttrib())
        {
            result = PRIME * result + _attribContainerType;
        }

        if (checkHasName())
        {
            result = result ^ ((BufferImpl)_name).hashCode();
        }

        if (checkHasAttrib())
        {
            result = result ^ ((BufferImpl)_encodedAttrib).hashCode();
        }
        return result;
    }
    
    @Override
    public int copy(MsgKey destKey)
    {
        if (null == destKey)
            return CodecReturnCodes.INVALID_ARGUMENT;

        return copy(destKey, CopyMsgFlags.KEY);
    }
    
    @Override
    public int addFilterId(int filterId)
    {
        if (!checkHasFilter())
            return CodecReturnCodes.FAILURE;
        if (filterId >= 32)
            return CodecReturnCodes.INVALID_DATA;
        int index = 1 << (filterId & 7);
        _filter = _filter | index;
        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public boolean checkHasFilterId(int filterId)
    {
        if (!checkHasFilter())
            return false;
        int index = 1 << (filterId & 7);
        if ((_filter & index) > 0)
            return true;
        return false;
    }
}
