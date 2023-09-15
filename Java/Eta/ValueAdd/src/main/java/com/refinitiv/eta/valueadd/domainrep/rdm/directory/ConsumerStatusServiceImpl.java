/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.directory;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.ElementEntry;
import com.refinitiv.eta.codec.ElementList;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.rdm.Directory;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.rdm.Directory.WarmStandbyDirectoryServiceTypes;


class ConsumerStatusServiceImpl implements ConsumerStatusService
{
    private long serviceId;
    private int action;
    private long sourceMirroringMode;

    private final static String eol = System.getProperty("line.separator");
    private final static String tab = "\t";
    private ElementList elementList = CodecFactory.createElementList();
    private ElementEntry elementEntry = CodecFactory.createElementEntry();
    private UInt tmpUInt = CodecFactory.createUInt();
    private StringBuilder stringBuffer = new StringBuilder();
    private int _flags;
    private long _wsbMode;

    
    ConsumerStatusServiceImpl()
    {
    	clear();
    }

    public void clear()
    {
        action = MapEntryActions.ADD;
        serviceId = 0;
        _flags = ConsumerStatusServiceFlags.HAS_SOURCE_MIRRORING_MODE;
        sourceMirroringMode = Directory.SourceMirroringMode.ACTIVE_NO_STANDBY;
        _wsbMode = WarmStandbyDirectoryServiceTypes.ACTIVE;
    }
    
    public int flags()
    {
    	return _flags;
    }
    
    public void flags(int flags)
    {
    	_flags = flags;
    }


    public int copy(ConsumerStatusService destConsumerStatusService)
    {
        assert (destConsumerStatusService != null) : "destConsumerStatusService must be non-null";
        
        destConsumerStatusService.serviceId(serviceId());
        destConsumerStatusService.action(action());
        if(checkHasSourceMirroringMode())
        {
        	destConsumerStatusService.applyHasSourceMirroringMode();
        	destConsumerStatusService.sourceMirroringMode(sourceMirroringMode());
        }
        destConsumerStatusService.flags(flags());
        if(checkHasWarmStandbyMode())
        {
        	destConsumerStatusService.applyHasWarmStandbyMode();
        	destConsumerStatusService.warmStandbyMode(warmStandbyMode());
        }
        return CodecReturnCodes.SUCCESS;
    }
    
    public int decode(DecodeIterator dIter, Msg msg)
    {
        clear();
        elementList.clear();
        int ret = 0;
        if ((ret = elementList.decode(dIter, null)) < CodecReturnCodes.SUCCESS)
        {
             return ret;
        }
        elementEntry.clear();
        boolean foundSourceMirroringMode = false;
        boolean foundWarmStandbyMode = false;
        while ((ret = elementEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            if (elementEntry.name().equals(ElementNames.SOURCE_MIRROR_MODE))
            {
                if (elementEntry.dataType() != DataTypes.UINT)
                {
                    return ret;
                }

                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
                {
                    return ret;
                }
                sourceMirroringMode(tmpUInt.toLong());
                _flags |= ConsumerStatusServiceFlags.HAS_SOURCE_MIRRORING_MODE;
                foundSourceMirroringMode = true;
            }
            
            if (elementEntry.name().equals(ElementNames.WARMSTANDBY_MODE))
            {
                if (elementEntry.dataType() != DataTypes.UINT)
                {
                    return ret;
                }

                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
                {
                    return ret;
                }
                warmStandbyMode(tmpUInt.toLong());
                _flags |= ConsumerStatusServiceFlags.HAS_WARM_STANDY_MODE;
                foundWarmStandbyMode = true;
            }
        }

        if (!foundSourceMirroringMode && !foundWarmStandbyMode)
        {
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    public int encode(EncodeIterator encodeIter)
    {
        elementList.clear();
        elementList.applyHasStandardData();
        int ret = elementList.encodeInit(encodeIter, null, 0);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;

        if(checkHasSourceMirroringMode())
        {
	        elementEntry.clear();
	        elementEntry.name(ElementNames.SOURCE_MIRROR_MODE);
	        elementEntry.dataType(DataTypes.UINT);
	        tmpUInt.value(sourceMirroringMode);
	        ret = elementEntry.encode(encodeIter, tmpUInt);
	        if (ret < CodecReturnCodes.SUCCESS)
	            return ret;
        }
        
        if(checkHasWarmStandbyMode())
        {
	        elementEntry.clear();
	        elementEntry.name(ElementNames.WARMSTANDBY_MODE);
	        elementEntry.dataType(DataTypes.UINT);
	        tmpUInt.value(_wsbMode);
	        ret = elementEntry.encode(encodeIter, tmpUInt);
	        if (ret < CodecReturnCodes.SUCCESS)
	            return ret;
        }
    
        ret = elementList.encodeComplete(encodeIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;

        return ret;
    }

    public long serviceId()
    {
        return serviceId;
    }

    public void serviceId(long serviceId)
    {
        this.serviceId = serviceId;
    }

    public int action()
    {
        return action;
    }

    public void action(int action)
    {
        this.action = action;
    }

    public long sourceMirroringMode()
    {
    	assert(checkHasSourceMirroringMode());
        return sourceMirroringMode;
    }

    public void sourceMirroringMode(long sourceMirroringMode)
    {
        this.sourceMirroringMode = sourceMirroringMode;
    }
    
    public boolean checkHasSourceMirroringMode()
    {
    	return (this._flags & ConsumerStatusServiceFlags.HAS_SOURCE_MIRRORING_MODE) != 0;
    }

    public void applyHasSourceMirroringMode()
    {
    	_flags |= ConsumerStatusServiceFlags.HAS_SOURCE_MIRRORING_MODE;
    }
    
    public long warmStandbyMode()
    {
    	assert(checkHasWarmStandbyMode());
        return _wsbMode;
    }

    public void warmStandbyMode(long warmStandbyMode)
    {
        this._wsbMode = warmStandbyMode;
    }
    
    public boolean checkHasWarmStandbyMode()
    {
    	return (this._flags & ConsumerStatusServiceFlags.HAS_WARM_STANDY_MODE) != 0;
    }

    public void applyHasWarmStandbyMode()
    {
    	_flags |= ConsumerStatusServiceFlags.HAS_WARM_STANDY_MODE;
    }

    StringBuilder buildStringBuf()
    {
        stringBuffer.setLength(0);
        stringBuffer.insert(0, "ConsumerStatusService: \n");
        stringBuffer.append(tab);

        stringBuffer.append(tab);
        stringBuffer.append("action: ");
        stringBuffer.append(action());
        stringBuffer.append(eol);
        stringBuffer.append(tab);
        stringBuffer.append("serviceId: ");
        stringBuffer.append(serviceId());
        stringBuffer.append(eol);
        if(checkHasSourceMirroringMode())
        {
	        stringBuffer.append(tab);
	        stringBuffer.append("sourceMirroringMode: ");
	        stringBuffer.append(sourceMirroringMode());
	        stringBuffer.append(eol);
        }
        
        if(checkHasWarmStandbyMode())
        {
	        stringBuffer.append(tab);
	        stringBuffer.append("warmStandbyMode: ");
	        stringBuffer.append(warmStandbyMode());
	        stringBuffer.append(eol);
        }
        

        return stringBuffer;
    }

    public String toString()
    {
        return buildStringBuf().toString();
    }

}
