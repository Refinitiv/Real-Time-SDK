package com.thomsonreuters.upa.valueadd.reactor;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.ElementList;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FilterEntry;
import com.thomsonreuters.upa.codec.FilterEntryActions;
import com.thomsonreuters.upa.codec.FilterList;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.rdm.ClassesOfService;

/**
 * Tunnel stream class of service.
 * 
 * @see TunnelStreamCommonProperties
 */
public class TunnelStreamClassOfService
{
    private TunnelStreamCommonProperties _commonProperties = new TunnelStreamCommonProperties();
    int _authenticationType;
    int _flowControlType;
    int _flowControlRecvWindowSize;
    int _dataIntegrityType;
    int _guaranteeType;
    String _persistenceFilePath;
    boolean _persistLocally;
    
    private ElementList _elemList = CodecFactory.createElementList();
    private ElementEntry _elemEntry = CodecFactory.createElementEntry();
    private FilterList _filterList = CodecFactory.createFilterList();
    private FilterEntry _filterEntry = CodecFactory.createFilterEntry();
    private UInt _tempUInt = CodecFactory.createUInt();
    private com.thomsonreuters.upa.codec.Enum _tempEnum = CodecFactory.createEnum();
    private DecodeIterator _dIter = CodecFactory.createDecodeIterator();
    private EncodeIterator _eIter = CodecFactory.createEncodeIterator();
    private Buffer _tempBuffer = CodecFactory.createBuffer();
    
    public TunnelStreamClassOfService()
    {
        _persistLocally = true;
        _tempBuffer.data(ByteBuffer.allocate(256));
    }
    
    /**
     * Returns the common properties of the TunnelStream class of service.
     * Use to set common properties for the TunnelStream.
     * 
     * @see ClassesOfService
     */
    public TunnelStreamCommonProperties commonProperties()
    {
        return _commonProperties;
    }

    /**
     * Returns the authentication type of the TunnelStream class of service.
     * 
     * @see ClassesOfService
     */
    public int authenticationType()
    {
        return _authenticationType;
    }
    
    /**
     * Sets the authentication type of the TunnelStream class of service.
     * 
     * @see ClassesOfService
     */
    public void authenticationType(int authenticationType)
    {
        _authenticationType = authenticationType;
    }
    
    /**
     * Returns the flow control type of the TunnelStream class of service.
     * 
     * @see ClassesOfService
     */
    public int flowControlType()
    {
        return _flowControlType;
    }
    
    /**
     * Sets the flow control type of the TunnelStream class of service.
     * 
     * @see ClassesOfService
     */
    public void flowControlType(int flowControlType)
    {
        _flowControlType = flowControlType;
    }
    
    /**
     * Returns the flow control receive window size of the TunnelStream class of service.
     * 
     * @see ClassesOfService
     */
    public int flowControlRecvWindowSize()
    {
        return _flowControlRecvWindowSize;
    }

    /**
     * Sets the flow control receive window size of the TunnelStream class of service.
     * Must be in the range of 1 - 2,147,483,647.
     * 
     * @see ClassesOfService
     */
    public void flowControlRecvWindowSize(int flowControlRecvWindowSize)
    {
        _flowControlRecvWindowSize = flowControlRecvWindowSize;
    }
    
    /**
     * Returns the data integrity type of the TunnelStream class of service.
     * 
     * @see ClassesOfService
     */
    public int dataIntegrityType()
    {
        return _dataIntegrityType;
    }
    
    /**
     * Sets the data integrity type of the TunnelStream class of service.
     * 
     * @see ClassesOfService
     */
    public void dataIntegrityType(int dataIntegrityType)
    {
        _dataIntegrityType = dataIntegrityType;
    }
    
    /**
     * Returns the guarantee type of the TunnelStream class of service.
     * 
     * @see ClassesOfService
     */
    public int guaranteeType()
    {
        return _guaranteeType;
    }
    
    /**
     * Sets the guarantee type of the TunnelStream class of service.
     * 
     * @see ClassesOfService
     */
    public void guaranteeType(int guaranteeType)
    {
        _guaranteeType = guaranteeType;
    }
    
    /**
     * Set the path for the guarantee class of service persistence file.
     * If not specified, the current working directory is used.
     * Use only when guarantee type is set to PERSISTENT_QUEUE.
     */
    public void persistenceFilePath(String persistenceFilePath)
    {
        _persistenceFilePath = persistenceFilePath;
    }

    /**
     * Returns the persistence file path for guarantee class of service.
     * Use only when guarantee type is set to PERSISTENT_QUEUE.
     */
    public String persistenceFilePath()
    {
        return _persistenceFilePath;
    }

    /**
     * Returns whether guarantee class of service will create local persistence files.
     * Use only when guarantee type is set to PERSISTENT_QUEUE.
     */
    public boolean persistLocally()
    {
        return _persistLocally;
    }

    /**
     * Enable or disable local file persistence. Default: Enabled.
     * Use only when guarantee class of service is set to PERSISTENT_QUEUE.
     */
    public void persistLocally(boolean persistLocally)
    {
        _persistLocally = persistLocally;
    }
    
    /**
     * Decodes the class of service for the tunnel stream.
     *  
     * @param reactorChannel the reactor channel associated with the tunnel stream
     * @param encodedBuffer the encoded class of service (the message payload)
     *  
     * @return {@link ReactorReturnCodes}
     */
    public int decode(ReactorChannel reactorChannel, Buffer encodedBuffer)
    {
        int ret;
        
        if (encodedBuffer == null || encodedBuffer.data() == null)
        {
            return ReactorReturnCodes.FAILURE;
        }
        
        _dIter.clear();
        _dIter.setBufferAndRWFVersion(encodedBuffer, reactorChannel.majorVersion(), reactorChannel.minorVersion());
        
        if ((ret = _filterList.decode(_dIter)) >= CodecReturnCodes.SUCCESS)
        {
            while ((ret = _filterEntry.decode(_dIter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret >= CodecReturnCodes.SUCCESS &&
                    _filterEntry.checkHasContainerType() &&
                    _filterEntry.containerType() == DataTypes.ELEMENT_LIST)
                {
                    if ((ret = _elemList.decode(_dIter, null)) >= CodecReturnCodes.SUCCESS)
                    {
                        while ((ret = _elemEntry.decode(_dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                        {
                            if (ret >= CodecReturnCodes.SUCCESS)
                            {
                                switch (_filterEntry.id())
                                {
                                    case ClassesOfService.FilterIds.COMMON_PROPERTIES:
                                        if (_elemEntry.name().equals(ClassesOfService.ElementNames.MAX_MSG_SIZE))
                                        {
                                            _tempUInt.decode(_dIter);
                                            _commonProperties.maxMsgSize((int)_tempUInt.toLong());
                                        }
                                        else if (_elemEntry.name().equals(ClassesOfService.ElementNames.PROTOCOL_TYPE))
                                        {
                                            _tempUInt.decode(_dIter);
                                            _commonProperties.protocolType((int)_tempUInt.toLong());
                                        } 
                                        else if (_elemEntry.name().equals(ClassesOfService.ElementNames.PROTOCOL_MAJOR_VERSION))
                                        {
                                            _tempUInt.decode(_dIter);
                                            _commonProperties.protocolMajorVersion((int)_tempUInt.toLong());
                                        } 
                                        else if (_elemEntry.name().equals(ClassesOfService.ElementNames.PROTOCOL_MINOR_VERSION))
                                        {
                                            _tempUInt.decode(_dIter);
                                            _commonProperties.protocolMinorVersion((int)_tempUInt.toLong());
                                        } 
                                        else if (_elemEntry.name().equals(ClassesOfService.ElementNames.STREAM_VERSION))
                                        {
                                            _tempUInt.decode(_dIter);
                                            _commonProperties.streamVersion((int)_tempUInt.toLong());
                                        } 
                                        break;
                                    case ClassesOfService.FilterIds.AUTHENTICATION:
                                        _tempEnum.decode(_dIter);
                                        _authenticationType = _tempEnum.toInt();
                                        break;
                                    case ClassesOfService.FilterIds.FLOW_CONTROL:
                                        if (_elemEntry.name().equals(ClassesOfService.ElementNames.TYPE))
                                        {
                                            _tempEnum.decode(_dIter);
                                            _flowControlType = _tempEnum.toInt();
                                        }
                                        else if (_elemEntry.name().equals(ClassesOfService.ElementNames.RECV_WINDOW_SIZE))
                                        {
                                            _tempUInt.decode(_dIter);
                                            _flowControlRecvWindowSize = (int)_tempUInt.toLong();
                                        } 
                                        break;
                                    case ClassesOfService.FilterIds.DATA_INTEGRITY:
                                        _tempEnum.decode(_dIter);
                                        _dataIntegrityType = _tempEnum.toInt();
                                        break;
                                    case ClassesOfService.FilterIds.GUARANTEE:
                                        _tempEnum.decode(_dIter);
                                        _guaranteeType = _tempEnum.toInt();
                                        break;
                                    default:
                                        break;
                                }
                            }
                            else
                            {
                                return ReactorReturnCodes.FAILURE;
                            }
                        }
                    }
                    else
                    {
                        return ReactorReturnCodes.FAILURE;
                    }
                }
                else
                {
                    return ReactorReturnCodes.FAILURE;
                }
            }
        }
        else
        {
            return ReactorReturnCodes.FAILURE;
        }
        
        return ReactorReturnCodes.SUCCESS;         
    }
    
    Buffer encode(ReactorChannel reactorChannel)
    {
        _eIter.clear();
        _tempBuffer.data().clear();
        _eIter.setBufferAndRWFVersion(_tempBuffer, reactorChannel.majorVersion(), reactorChannel.minorVersion());
        
        if (encode(_eIter) == CodecReturnCodes.SUCCESS)
        {
            return _tempBuffer;
        }
        else
        {
            return null;
        }
    }
    
    /**
     * Encodes the class of service for the tunnel stream.
     *  
     * @param eIter the iterator to encode class of service into
     *  
     * @return {@link CodecReturnCodes}
     */
    public int encode(EncodeIterator eIter)
    {
        int ret = CodecReturnCodes.SUCCESS;
        
        assert null != eIter : "Invalid parameters or parameters passed in as NULL";
        
        _filterList.clear();
        _filterList.containerType(DataTypes.ELEMENT_LIST);
        
        if ((ret = _filterList.encodeInit(eIter)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        // encode common properties
        _filterEntry.clear();
        _filterEntry.action(FilterEntryActions.SET);
        _filterEntry.applyHasContainerType();
        _filterEntry.containerType(DataTypes.ELEMENT_LIST);
        _filterEntry.id(ClassesOfService.FilterIds.COMMON_PROPERTIES);

        if ((ret = _filterEntry.encodeInit(eIter, 0)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        _elemList.clear();
        _elemList.applyHasStandardData();
        if ((ret = _elemList.encodeInit(eIter, null, 0)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        _elemEntry.clear();
        _elemEntry.name(ClassesOfService.ElementNames.MAX_MSG_SIZE);
        _elemEntry.dataType(DataTypes.UINT);
        _tempUInt.value(_commonProperties.maxMsgSize());
        if ((ret = _elemEntry.encode(eIter, _tempUInt)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        _elemEntry.clear();
        _elemEntry.name(ClassesOfService.ElementNames.PROTOCOL_TYPE);
        _elemEntry.dataType(DataTypes.UINT);
        _tempUInt.value(_commonProperties.protocolType());
        if ((ret = _elemEntry.encode(eIter, _tempUInt)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        _elemEntry.clear();
        _elemEntry.name(ClassesOfService.ElementNames.PROTOCOL_MAJOR_VERSION);
        _elemEntry.dataType(DataTypes.UINT);
        _tempUInt.value(_commonProperties.protocolMajorVersion());
        if ((ret = _elemEntry.encode(eIter, _tempUInt)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        _elemEntry.clear();
        _elemEntry.name(ClassesOfService.ElementNames.PROTOCOL_MINOR_VERSION);
        _elemEntry.dataType(DataTypes.UINT);
        _tempUInt.value(_commonProperties.protocolMinorVersion());
        if ((ret = _elemEntry.encode(eIter, _tempUInt)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        _elemEntry.clear();
        _elemEntry.name(ClassesOfService.ElementNames.STREAM_VERSION);
        _elemEntry.dataType(DataTypes.UINT);
        _tempUInt.value(_commonProperties.streamVersion());
        if ((ret = _elemEntry.encode(eIter, _tempUInt)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if ((ret = _elemList.encodeComplete(eIter, true)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if ((ret = _filterEntry.encodeComplete(eIter, true)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        // encode authentication
        _filterEntry.clear();
        _filterEntry.action(FilterEntryActions.SET);
        _filterEntry.applyHasContainerType();
        _filterEntry.containerType(DataTypes.ELEMENT_LIST);
        _filterEntry.id(ClassesOfService.FilterIds.AUTHENTICATION);

        if ((ret = _filterEntry.encodeInit(eIter, 0)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        _elemList.clear();
        _elemList.applyHasStandardData();
        if ((ret = _elemList.encodeInit(eIter, null, 0)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        _elemEntry.clear();
        _elemEntry.name(ClassesOfService.ElementNames.TYPE);
        _elemEntry.dataType(DataTypes.ENUM);
        _tempEnum.value(_authenticationType);
        if ((ret = _elemEntry.encode(eIter, _tempEnum)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if ((ret = _elemList.encodeComplete(eIter, true)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if ((ret = _filterEntry.encodeComplete(eIter, true)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        // encode flow control
        _filterEntry.clear();
        _filterEntry.action(FilterEntryActions.SET);
        _filterEntry.applyHasContainerType();
        _filterEntry.containerType(DataTypes.ELEMENT_LIST);
        _filterEntry.id(ClassesOfService.FilterIds.FLOW_CONTROL);

        if ((ret = _filterEntry.encodeInit(eIter, 0)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        _elemList.clear();
        _elemList.applyHasStandardData();
        if ((ret = _elemList.encodeInit(eIter, null, 0)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        _elemEntry.clear();
        _elemEntry.name(ClassesOfService.ElementNames.TYPE);
        _elemEntry.dataType(DataTypes.ENUM);
        _tempEnum.value(_flowControlType);
        if ((ret = _elemEntry.encode(eIter, _tempEnum)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        _elemEntry.clear();
        _elemEntry.name(ClassesOfService.ElementNames.RECV_WINDOW_SIZE);
        _elemEntry.dataType(DataTypes.UINT);
        _tempUInt.value(_flowControlRecvWindowSize);
        if ((ret = _elemEntry.encode(eIter, _tempUInt)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if ((ret = _elemList.encodeComplete(eIter, true)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if ((ret = _filterEntry.encodeComplete(eIter, true)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        // encode data integrity
        _filterEntry.clear();
        _filterEntry.action(FilterEntryActions.SET);
        _filterEntry.applyHasContainerType();
        _filterEntry.containerType(DataTypes.ELEMENT_LIST);
        _filterEntry.id(ClassesOfService.FilterIds.DATA_INTEGRITY);

        if ((ret = _filterEntry.encodeInit(eIter, 0)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        _elemList.clear();
        _elemList.applyHasStandardData();
        if ((ret = _elemList.encodeInit(eIter, null, 0)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        _elemEntry.clear();
        _elemEntry.name(ClassesOfService.ElementNames.TYPE);
        _elemEntry.dataType(DataTypes.ENUM);
        _tempEnum.value(_dataIntegrityType);
        if ((ret = _elemEntry.encode(eIter, _tempEnum)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if ((ret = _elemList.encodeComplete(eIter, true)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if ((ret = _filterEntry.encodeComplete(eIter, true)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        // encode guarantee
        _filterEntry.clear();
        _filterEntry.action(FilterEntryActions.SET);
        _filterEntry.applyHasContainerType();
        _filterEntry.containerType(DataTypes.ELEMENT_LIST);
        _filterEntry.id(ClassesOfService.FilterIds.GUARANTEE);

        if ((ret = _filterEntry.encodeInit(eIter, 0)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        _elemList.clear();
        _elemList.applyHasStandardData();
        if ((ret = _elemList.encodeInit(eIter, null, 0)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        _elemEntry.clear();
        _elemEntry.name(ClassesOfService.ElementNames.TYPE);
        _elemEntry.dataType(DataTypes.ENUM);
        _tempEnum.value(_guaranteeType);
        if ((ret = _elemEntry.encode(eIter, _tempEnum)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if ((ret = _elemList.encodeComplete(eIter, true)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if ((ret = _filterEntry.encodeComplete(eIter, true)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        if ((ret = _filterList.encodeComplete(eIter, true)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        return ret;
    }

    boolean isValid(boolean isServer, ReactorErrorInfo errorInfo)
    {
        // check if flow control and reliability are minimally set
        if (_flowControlType != ClassesOfService.FlowControlTypes.BIDIRECTIONAL ||
            _dataIntegrityType != ClassesOfService.DataIntegrityTypes.RELIABLE)
        {
            errorInfo.clear();
            errorInfo.code(ReactorReturnCodes.FAILURE).location("TunnelStreamClassOfService.isValid");
            errorInfo.error().errorId(ReactorReturnCodes.FAILURE);
            errorInfo.error().text("TunnelStreamClassOfService must minimally set flow control and reliability");
            
            return false;
        }
        
        // check if provider supports persistence - only Queue Provider can support persistence
        if (isServer && _guaranteeType == ClassesOfService.GuaranteeTypes.PERSISTENT_QUEUE)
        {
            errorInfo.clear();
            errorInfo.code(ReactorReturnCodes.FAILURE).location("TunnelStreamClassOfService.isValid");
            errorInfo.error().errorId(ReactorReturnCodes.FAILURE);
            errorInfo.error().text("Only Queue Provider can support persistence");
            
            return false;            
        }

        return true;
    }
    
    /**
     * Clears the TunnelStreamClassOfService for re-use.
     */
    public void clear()
    {
        _commonProperties.clear();
        _authenticationType = 0;
        _flowControlType = 0;
        _dataIntegrityType = 0;
        _flowControlRecvWindowSize = 0;
        _guaranteeType = 0;
        _persistenceFilePath = null;
        _persistLocally = true;
    }
}
