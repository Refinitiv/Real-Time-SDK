package com.thomsonreuters.upa.valueadd.reactor;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
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
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.rdm.ClassesOfService;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportFactory;

/**
 * Class of service for a qualified or tunnel stream.
 * 
 * @see CosCommon
 * @see CosAuthentication
 * @see CosDataIntegrity
 * @see CosFlowControl
 * @see CosGuarantee
 * */
public class ClassOfService
{
    private CosCommon _commonProperties = new CosCommon();
    private CosAuthentication _authentication = new CosAuthentication();
    private CosFlowControl _flowControl = new CosFlowControl();
    private CosDataIntegrity _dataIntegrity = new CosDataIntegrity();
    private CosGuarantee _guarantee = new CosGuarantee();
    
    private ElementList _elemList = CodecFactory.createElementList();
    private ElementEntry _elemEntry = CodecFactory.createElementEntry();
    private FilterList _filterList = CodecFactory.createFilterList();
    private FilterEntry _filterEntry = CodecFactory.createFilterEntry();
    private UInt _tempUInt = CodecFactory.createUInt();
    private Int _tempInt = CodecFactory.createInt();
    private DecodeIterator _dIter = CodecFactory.createDecodeIterator();
    private EncodeIterator _eIter = CodecFactory.createEncodeIterator();
    private Buffer _tempBuffer = CodecFactory.createBuffer();
    private  com.thomsonreuters.upa.transport.Error _error = TransportFactory.createError(); 
    private Buffer _encodedCOSBuffer;
    private boolean _allCOSDecoded;
    private boolean _isServer;
    
    /**
     * Instantiates a new class of service.
     */
    public ClassOfService()
    {
        _tempBuffer.data(ByteBuffer.allocate(512));
    }
    
    /**
     * Returns the common class of service properties.
     * Use to set common class of service properties.
     *
     * @return the cos common
     * @see ClassesOfService
     */
    public CosCommon common()
    {
        return _commonProperties;
    }
    
    /**
     * Returns the authentication class of service.
     * Use to set authentication class of service.
     *
     * @return the cos authentication
     * @see ClassesOfService
     */
    public CosAuthentication authentication()
    {
        if (!_allCOSDecoded && _encodedCOSBuffer != null)
        {
            decodeRemainder(_error);
        }
        
        return _authentication;
    }
    
    /**
     * Returns the flow control class of service.
     * Use to set flow control class of service.
     *
     * @return the cos flow control
     * @see ClassesOfService
     */
    public CosFlowControl flowControl()
    {
        if (!_allCOSDecoded && _encodedCOSBuffer != null)
        {
            decodeRemainder(_error);
        }

        return _flowControl;
    }
    
    /**
     * Returns the data integrity class of service.
     * Use to set data integrity class of service.
     *
     * @return the cos data integrity
     * @see ClassesOfService
     */
    public CosDataIntegrity dataIntegrity()
    {
        if (!_allCOSDecoded && _encodedCOSBuffer != null)
        {
            decodeRemainder(_error);
        }

        return _dataIntegrity;
    }
    
    /**
     * Returns the guarantee class of service.
     * Use to set guarantee class of service.
     *
     * @return the cos guarantee
     * @see ClassesOfService
     */
    public CosGuarantee guarantee()
    {
        if (!_allCOSDecoded && _encodedCOSBuffer != null)
        {
            decodeRemainder(_error);
        }

        return _guarantee;
    }
    
    /**
     * Returns the filter flags for this class of service.
     *
     * @return the int
     * @see ClassesOfService
     */
    public int filterFlags()
    {
        int flags = ClassesOfService.FilterFlags.COMMON_PROPERTIES;
        
        if (_authentication.type() != ClassesOfService.AuthenticationTypes.NOT_REQUIRED)
        {
            flags |= ClassesOfService.FilterFlags.AUTHENTICATION;
        }
        
        if (_flowControl.type() != ClassesOfService.FlowControlTypes.NONE)
        {
            flags |= ClassesOfService.FilterFlags.FLOW_CONTROL;
        }
        
        if (_dataIntegrity.type() != ClassesOfService.DataIntegrityTypes.BEST_EFFORT)
        {
            flags |= ClassesOfService.FilterFlags.DATA_INTEGRITY;
        }
        
        if (_guarantee.type() != ClassesOfService.GuaranteeTypes.NONE)
        {
            flags |= ClassesOfService.FilterFlags.GUARANTEE;
        }
        
        return flags;
    }
    
    /**
     * Decode.
     *
     * @param reactorChannel the reactor channel
     * @param encodedBuffer the encoded buffer
     * @param errorInfo the error info
     * @return the int
     */
    /* decodes all classes of service */
    int decode(ReactorChannel reactorChannel, Buffer encodedBuffer, ReactorErrorInfo errorInfo)
    {
        int ret;
        
        if (encodedBuffer == null || encodedBuffer.data() == null)
        {
            errorInfo.error().text("encodedBuffer cannot be null");
            return ReactorReturnCodes.FAILURE;
        }
        
        _dIter.clear();
        _dIter.setBufferAndRWFVersion(encodedBuffer, reactorChannel.majorVersion(), reactorChannel.minorVersion());
        
        if ((ret = _filterList.decode(_dIter)) >= CodecReturnCodes.SUCCESS)
        {
            while ((ret = _filterEntry.decode(_dIter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret >= CodecReturnCodes.SUCCESS &&
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
                                        _tempUInt.decode(_dIter);
                                        _authentication.type((int)_tempUInt.toLong());
                                        break;
                                    case ClassesOfService.FilterIds.FLOW_CONTROL:
                                        if (_elemEntry.name().equals(ClassesOfService.ElementNames.TYPE))
                                        {
                                            _tempUInt.decode(_dIter);
                                            _flowControl.type((int)_tempUInt.toLong());
                                        }
                                        else if (_elemEntry.name().equals(ClassesOfService.ElementNames.RECV_WINDOW_SIZE))
                                        {
                                            _tempInt.decode(_dIter);
                                            _flowControl.sendWindowSize((int)_tempInt.toLong());
                                        } 
                                        break;
                                    case ClassesOfService.FilterIds.DATA_INTEGRITY:
                                        _tempUInt.decode(_dIter);
                                        _dataIntegrity.type((int)_tempUInt.toLong());
                                        break;
                                    case ClassesOfService.FilterIds.GUARANTEE:
                                        _tempUInt.decode(_dIter);
                                        _guarantee.type((int)_tempUInt.toLong());
                                        break;
                                    default:
                                        break;
                                }
                            }
                            else
                            {
                                errorInfo.error().text("ElementEntry decode failed with return code: " + ret);
                                return ReactorReturnCodes.FAILURE;
                            }
                        }
                    }
                    else
                    {
                        errorInfo.error().text("ElementList decode failed with return code: " + ret);
                        return ReactorReturnCodes.FAILURE;
                    }
                }
                else
                {
                    errorInfo.error().text("FilterEntry decode failed with return code: " + ret);
                    return ReactorReturnCodes.FAILURE;
                }
            }
        }
        else
        {
            errorInfo.error().text("FilterList decode failed with return code: " + ret);
            return ReactorReturnCodes.FAILURE;
        }
        
        _encodedCOSBuffer = encodedBuffer;
        _allCOSDecoded = true;
        
        return ReactorReturnCodes.SUCCESS;         
    }
    
    /**
     * Decode common properties.
     *
     * @param reactorChannel the reactor channel
     * @param encodedBuffer the encoded buffer
     * @param errorInfo the error info
     * @return the int
     */
    /* decodes class of service common properties, returns when found and decoded */
    int decodeCommonProperties(ReactorChannel reactorChannel, Buffer encodedBuffer, ReactorErrorInfo errorInfo)
    {
        int ret, filterEntryRet;
        
        if (encodedBuffer == null || encodedBuffer.data() == null)
        {
            errorInfo.error().text("encodedBuffer cannot be null");
            return ReactorReturnCodes.FAILURE;
        }
        
        _encodedCOSBuffer = encodedBuffer;
        
        _dIter.clear();
        _dIter.setBufferAndRWFVersion(encodedBuffer, reactorChannel.majorVersion(), reactorChannel.minorVersion());
        
        if ((ret = _filterList.decode(_dIter)) >= CodecReturnCodes.SUCCESS)
        {
            while ((filterEntryRet = _filterEntry.decode(_dIter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (filterEntryRet >= CodecReturnCodes.SUCCESS &&
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
                                        if (_elemEntry.name().equals(ClassesOfService.ElementNames.MAX_FRAGMENT_SIZE))
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
                                        else if (_elemEntry.name().equals(ClassesOfService.ElementNames.SUPPS_FRAGMENTATION))
                                        {
                                            _tempUInt.decode(_dIter);                                          
                                         	_commonProperties.supportFragmentation((int)_tempUInt.toLong());                                      
                                        } 
                                        break;
                                    case ClassesOfService.FilterIds.AUTHENTICATION:
                                        _tempUInt.decode(_dIter);
                                        _authentication.type((int)_tempUInt.toLong());
                                        break;
                                    case ClassesOfService.FilterIds.FLOW_CONTROL:
                                        if (_elemEntry.name().equals(ClassesOfService.ElementNames.TYPE))
                                        {
                                            _tempUInt.decode(_dIter);
                                            _flowControl.type((int)_tempUInt.toLong());
                                        }
                                        else if (_elemEntry.name().equals(ClassesOfService.ElementNames.RECV_WINDOW_SIZE))
                                        {
                                            _tempInt.decode(_dIter);
                                            _flowControl.sendWindowSize((int)_tempInt.toLong());
                                        } 
                                        break;
                                    case ClassesOfService.FilterIds.DATA_INTEGRITY:
                                        _tempUInt.decode(_dIter);
                                        _dataIntegrity.type((int)_tempUInt.toLong());
                                        break;
                                    case ClassesOfService.FilterIds.GUARANTEE:
                                        _tempUInt.decode(_dIter);
                                        _guarantee.type((int)_tempUInt.toLong());
                                        break;
                                    default:
                                        break;
                                }
                            }
                            else
                            {
                                errorInfo.error().text("ElementEntry decode failed with return code: " + ret);
                                return ReactorReturnCodes.FAILURE;
                            }
                        }
                        
                        if (_filterEntry.id() == ClassesOfService.FilterIds.COMMON_PROPERTIES)
                        {
                            // break once common properties found
                            break;
                        }
                    }
                    else
                    {
                        errorInfo.error().text("ElementList decode failed with return code: " + ret);
                        return ReactorReturnCodes.FAILURE;
                    }
                }
                else
                {
                    errorInfo.error().text("FilterEntry decode failed with return code: " + ret);
                    return ReactorReturnCodes.FAILURE;
                }
            }
        }
        else
        {
            errorInfo.error().text("FilterList decode failed with return code: " + ret);
            return ReactorReturnCodes.FAILURE;
        }
        
        if (filterEntryRet == CodecReturnCodes.END_OF_CONTAINER)
        {
            _allCOSDecoded = true;
        }
        
        return ReactorReturnCodes.SUCCESS;         
    }
    
    /**
     * Decode remainder.
     *
     * @param error the error
     * @return the int
     */
    /* decodes remainder of classes of service, must be called after decodeCommonProperties() method */
    int decodeRemainder(Error error)
    {
        int ret;
        
        while ((ret = _filterEntry.decode(_dIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret >= CodecReturnCodes.SUCCESS &&
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
                                case ClassesOfService.FilterIds.AUTHENTICATION:
                                    _tempUInt.decode(_dIter);
                                    _authentication.type((int)_tempUInt.toLong());
                                    break;
                                case ClassesOfService.FilterIds.FLOW_CONTROL:
                                    if (_elemEntry.name().equals(ClassesOfService.ElementNames.TYPE))
                                    {
                                        _tempUInt.decode(_dIter);
                                        _flowControl.type((int)_tempUInt.toLong());
                                    }
                                    else if (_elemEntry.name().equals(ClassesOfService.ElementNames.RECV_WINDOW_SIZE))
                                    {
                                        _tempInt.decode(_dIter);
                                        _flowControl.recvWindowSize((int)_tempInt.toLong());
                                    } 
                                    break;
                                case ClassesOfService.FilterIds.DATA_INTEGRITY:
                                    _tempUInt.decode(_dIter);
                                    _dataIntegrity.type((int)_tempUInt.toLong());
                                    break;
                                case ClassesOfService.FilterIds.GUARANTEE:
                                    _tempUInt.decode(_dIter);
                                    _guarantee.type((int)_tempUInt.toLong());
                                    break;
                                default:
                                    break;
                            }
                        }
                        else
                        {
                            error.text("ElementEntry decode failed with return code: " + ret);
                            return ReactorReturnCodes.FAILURE;
                        }
                    }
                }
                else
                {
                    error.text("ElementList decode failed with return code: " + ret);
                    return ReactorReturnCodes.FAILURE;
                }
            }
            else
            {
                error.text("FilterEntry decode failed with return code: " + ret);
                return ReactorReturnCodes.FAILURE;
            }
        }
        
        _allCOSDecoded = true;
        
        return ReactorReturnCodes.SUCCESS;         
    }
    
    /**
     * Encode.
     *
     * @param reactorChannel the reactor channel
     * @return the buffer
     */
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
     * Encodes the class of service.
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
        
        // encode common properties (always present)
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
        
        // encode maxMsgSize only if server
        if (_isServer)
        {
            _elemEntry.clear();
            _elemEntry.name(ClassesOfService.ElementNames.MAX_MSG_SIZE);
            _elemEntry.dataType(DataTypes.UINT);
            if ( _commonProperties.streamVersion() > 1)
            {
            _tempUInt.value(_commonProperties.maxMsgSize());
            }
            else
            {
            	_tempUInt.value(_commonProperties.maxFragmentSize());            	
            }
            if ((ret = _elemEntry.encode(eIter, _tempUInt)) < CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
            
            // maxFragmentSize and supportFragmentation flag
            if ( _commonProperties.streamVersion() > 1)
            {	
            	
            	_elemEntry.clear();
            	_elemEntry.name(ClassesOfService.ElementNames.MAX_FRAGMENT_SIZE);
            	_elemEntry.dataType(DataTypes.UINT);
            	_tempUInt.value(_commonProperties.maxFragmentSize());
            	if ((ret = _elemEntry.encode(eIter, _tempUInt)) < CodecReturnCodes.SUCCESS)
            	{
            		return ret;
            	}
            	
              	_elemEntry.clear();
            	_elemEntry.name(ClassesOfService.ElementNames.SUPPS_FRAGMENTATION);
            	_elemEntry.dataType(DataTypes.UINT);
            	_tempUInt.value(_commonProperties.supportFragmentation());
            if ((ret = _elemEntry.encode(eIter, _tempUInt)) < CodecReturnCodes.SUCCESS)
            {
                return ret;
            	}
            }
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
        
        // encode authentication if not NOT_REQUIRED
        if (_authentication.type() != ClassesOfService.AuthenticationTypes.NOT_REQUIRED)
        {
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
            _elemEntry.dataType(DataTypes.UINT);
            _tempUInt.value(_authentication.type());
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
        }
        
        // encode flow control if not NONE
        if (_flowControl.type() != ClassesOfService.FlowControlTypes.NONE)
        {
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
            _elemEntry.dataType(DataTypes.UINT);
            _tempUInt.value(_flowControl.type());
            if ((ret = _elemEntry.encode(eIter, _tempUInt)) < CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
    
            _elemEntry.clear();
            _elemEntry.name(ClassesOfService.ElementNames.RECV_WINDOW_SIZE);
            _elemEntry.dataType(DataTypes.INT);
            int recvWindowSize = _flowControl.recvWindowSize();
            _tempInt.value(recvWindowSize);
            if ((ret = _elemEntry.encode(eIter, _tempInt)) < CodecReturnCodes.SUCCESS)
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
        }
        
        // encode data integrity if not BEST_EFFORT
        if (_dataIntegrity.type() != ClassesOfService.DataIntegrityTypes.BEST_EFFORT)
        {
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
            _elemEntry.dataType(DataTypes.UINT);
            _tempUInt.value(_dataIntegrity.type());
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
        }
        
        // encode guarantee if not NONE
        if (_guarantee.type() != ClassesOfService.GuaranteeTypes.NONE)
        {
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
            _elemEntry.dataType(DataTypes.UINT);
            _tempUInt.value(_guarantee.type());
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
        }
        
        if ((ret = _filterList.encodeComplete(eIter, true)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        return ret;
    }

    /**
     * Checks if is valid.
     *
     * @param isServer the is server
     * @param errorInfo the error info
     * @return true, if is valid
     */
    boolean isValid(boolean isServer, ReactorErrorInfo errorInfo)
    {
        _isServer = isServer;
        
        // check if reliability is minimally set
        if (_dataIntegrity.type() != ClassesOfService.DataIntegrityTypes.RELIABLE)
        {
            errorInfo.clear();
            errorInfo.code(ReactorReturnCodes.FAILURE).location("ClassOfService.isValid");
            errorInfo.error().errorId(ReactorReturnCodes.FAILURE);
            errorInfo.error().text("ClassOfService must minimally set reliability");
            
            return false;
        }
        
        // check if queue consumer uses RWF protocol
        if (!isServer && _guarantee.type() == ClassesOfService.GuaranteeTypes.PERSISTENT_QUEUE &&
            _commonProperties.protocolType() != Codec.protocolType())
        {
            errorInfo.clear();
            errorInfo.code(ReactorReturnCodes.FAILURE).location("ClassOfService.isValid");
            errorInfo.error().errorId(ReactorReturnCodes.FAILURE);
            errorInfo.error().text("Queue consumers must use RWF protocol type");
            
            return false;            
        }
        
        // check if queue consumer uses RWF protocol when using authentication type of OMM login
        if (_commonProperties.protocolType() != Codec.protocolType() &&
            _authentication.type() == ClassesOfService.AuthenticationTypes.OMM_LOGIN)
        {
            errorInfo.clear();
            errorInfo.code(ReactorReturnCodes.FAILURE).location("ClassOfService.isValid");
            errorInfo.error().errorId(ReactorReturnCodes.FAILURE);
            errorInfo.error().text("Must use RWF protocol type when using authentication type of OMM login");
            
            return false;            
        }    
        
        // check if provider supports persistence - only Queue Provider can support persistence
        if (isServer && _guarantee.type() == ClassesOfService.GuaranteeTypes.PERSISTENT_QUEUE)
        {
            errorInfo.clear();
            errorInfo.code(ReactorReturnCodes.FAILURE).location("ClassOfService.isValid");
            errorInfo.error().errorId(ReactorReturnCodes.FAILURE);
            errorInfo.error().text("Only Queue Provider can support persistence");
            
            return false;            
        }

        return true;
    }
    
    /**
     * Decoded properly.
     *
     * @return true, if successful
     */
    boolean decodedProperly()
    {
        return _allCOSDecoded;
    }
    
    /**
     * Checks if is server.
     *
     * @param isServer the is server
     */
    void isServer(boolean isServer)
    {
        _isServer = isServer;
    }
    
    /**
     * Clears the ClassOfService for re-use.
     */
    public void clear()
    {
        _commonProperties.clear();
        _authentication.clear();
        _flowControl.clear();
        _dataIntegrity.clear();
        _guarantee.clear();
        _encodedCOSBuffer = null;
        _allCOSDecoded = false;
        _isServer = false;
    }

    /**
     * Performs a deep copy of a {@link ClassOfService} object.
     * 
     * @param destCos ClassOfService to copy into. It cannot be null.
     * 
	 * @return {@link ReactorReturnCodes} indicating success or failure
     */
    public int copy(ClassOfService destCos)
    {
        destCos.clear();

        /* Common */
        destCos.common().maxMsgSize(common().maxMsgSize());
        destCos.common().protocolType(common().protocolType());
        destCos.common().protocolMajorVersion(common().protocolMajorVersion());
        destCos.common().protocolMinorVersion(common().protocolMinorVersion());
        destCos.common().streamVersion(common().streamVersion());
        
        /* Authentication */
        destCos.authentication().type(authentication().type());
        
        /* Flow Control */
        destCos.flowControl().type(flowControl().type());
        destCos.flowControl().recvWindowSize(flowControl().recvWindowSize());
        destCos.flowControl().sendWindowSize(flowControl().sendWindowSize());
        
        /* Data Integrity */
        destCos.dataIntegrity().type(dataIntegrity().type());
        
        /* Guarantee */
        destCos.guarantee().type(guarantee().type());
        destCos.guarantee().persistenceFilePath(guarantee().persistenceFilePath());
        destCos.guarantee().persistLocally(guarantee().persistLocally());

        return ReactorReturnCodes.SUCCESS;
    }
}
