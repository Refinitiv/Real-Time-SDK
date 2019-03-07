package com.thomsonreuters.upa.valueadd.domainrep.rdm.directory;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import com.thomsonreuters.upa.codec.Array;
import com.thomsonreuters.upa.codec.ArrayEntry;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.ElementList;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FilterEntryActions;
import com.thomsonreuters.upa.codec.LocalElementSetDefDb;
import com.thomsonreuters.upa.codec.Map;
import com.thomsonreuters.upa.codec.MapEntry;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.MapEntryFlags;
import com.thomsonreuters.upa.codec.MapFlags;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.codec.Vector;
import com.thomsonreuters.upa.codec.VectorEntry;
import com.thomsonreuters.upa.rdm.Directory;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.ElementNames;

/**
 * The RDM Service. Contains information about a particular service.
 * 
 * @see ServiceFlags
 * @see ServiceInfo
 * @see ServiceState
 * @see ServiceGroup
 * @see ServiceLoad
 * @see ServiceData
 * @see ServiceLinkInfo
 */
public interface Service
{

    /**
     * Clears an RDMService.
     */
    public void clear();

    /**
     * Returns action associated with this service.
     * 
     * @return action
     */
    public int action();

    /**
     * Sets action associated with this service.
     *
     * @param action the action
     */
    public void action(int action);

    /**
     * Returns service state flags. Populated with {@link ServiceFlags}
     * 
     * @return flags
     */
    public int flags();

    /**
     * Sets service state flags. Populate with {@link ServiceFlags}
     *
     * @param flags the flags
     */
    public void flags(int flags);

    /**
     * Applies info presence flag.
     * 
     * Flags may also be bulk-get via {@link #flags()}.
     */
    public void applyHasInfo();

    /**
     * Checks the presence of the info field.
     * 
     * Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @return true - if info field exists, false - if not.
     */
    public boolean checkHasInfo();

    /**
     * Applies data presence flag.
     * 
     * Flags may also be bulk-get via {@link #flags()}.
     */
    public void applyHasData();

    /**
     * Checks the presence of the data field.
     * 
     * Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @return true - if data field exists, false - if not.
     */
    public boolean checkHasData();

    /**
     * Applies load presence flag.
     * 
     * Flags may also be bulk-get via {@link #flags()}.
     */
    public void applyHasLoad();

    /**
     * Checks the presence of the load field.
     * 
     * Flags may also be bulk-set via {@link #flags(int)}.
     *
     * @return true, if successful
     */
    public boolean checkHasLoad();

    /**
     * Applies link presence flag.
     * 
     * Flags may also be bulk-get via {@link #flags()}.
     */
    public void applyHasLink();

    /**
     * Checks the presence of the link field.
     * 
     * Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @return true - if link field exists, false - if not.
     */
    public boolean checkHasLink();

    /**
     * Applies state presence flag.
     * 
     * Flags may also be bulk-get via {@link #flags()}.
     */
    public void applyHasState();

    /**
     * Checks the presence of the state field.
     * 
     * Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @return true - if state field exists, false - if not.
     */
    public boolean checkHasState();

    /**
     * Encode an RDM Service entry.
     * 
     * @param encIter The Encode Iterator
     * 
     * @return UPA return value
     */
    public int encode(EncodeIterator encIter);

    /**
     * Decode a UPA service entry into an RDM service entry.
     * 
     * @param dIter The Decode Iterator
     * 
     * @return UPA return value
     */
    public int decode(DecodeIterator dIter);

    /**
     * Returns serviceId - number identifying this service.
     * 
     * @return serviceId
     */
    public int serviceId();

    /**
     * Sets serviceId - number identifying this service.
     *
     * @param serviceId the service id
     */
    public void serviceId(int serviceId);

    /**
     * Returns info filter for this service.
     * 
     * @return info filter.
     */
    public ServiceInfo info();

    /**
     * Sets info filter for this service.
     * 
     * @param info -info filter.
     */
    public void info(ServiceInfo info);

    /**
     * Returns state filter for this service.
     * 
     * @return state filter.
     */
    public ServiceState state();

    /**
     * Sets state filter for this service.
     * 
     * @param state -service state filter.
     */
    public void state(ServiceState state);

    /**
     * Returns list of group filters for this service.
     * 
     * @return list of group filters.
     */
    public List<ServiceGroup> groupStateList();

    /**
     * Sets group filter elements for this service.
     *
     * @param groupStateList the group state list
     */
    public void groupStateList(List<ServiceGroup> groupStateList);

    /**
     * Returns load filter for this service.
     * 
     * @return load filter.
     */
    public ServiceLoad load();

    /**
     * Sets load filter for this service.
     * 
     * @param load -service load filter.
     */
    public void load(ServiceLoad load);

    /**
     * Returns data filter for this service.
     * 
     * @return data filter.
     */
    public ServiceData data();

    /**
     * Sets data filter for this service.
     * 
     * @param data -service data filter.
     */
    public void data(ServiceData data);

    /**
     * Returns link filter for this service.
     * 
     * @return link filter.
     */
    public ServiceLinkInfo link();

    /**
     * Sets link filter for this service.
     * 
     * @param link -service link filter.
     */
    public void link(ServiceLinkInfo link);

    /**
     * Performs a deep copy of {@link ServiceImpl} object.
     *
     * @param destService Service object to copy this object into. It cannot be null.
     * 
     * @return UPA return value indicating success or failure of copy operation.
     */
    public int copy(Service destService);
    
    /**
     * Performs an update of this {@link ServiceImpl} object to the destination
     * {@link ServiceImpl} object. Only updated filter entries are copied to the
     * destination.
     *
     * @param destService Service object to be updated by this object. It cannot be null.
     * 
     * @return UPA return value indicating success or failure of update operation.
     */
    public int applyUpdate(Service destService);
    
    /** 
     *  Returns information about Sequenced Multicast connections with regards
     *  to EDF connections to Snapshot server, Reference Data Server, Gap Fill and Request servers,
     *  and multicast groups.
     *  
     * @return Service Sequenced Multicast information
     */
    public ServiceSeqMcastInfo seqMcastInfo();


    /**
     * The RDM Service Data. Contains information provided by the Source
     * Directory Data filter.
     * 
     * @see ServiceDataFlags
     * @see ServiceImpl
     */
    public static class ServiceData
    {
        private int flags;
        private int dataType;
        private Buffer data;
        private long type;
        private int action;

        private final static String eol = "\n";
        private final static String tab = "\t";

        private StringBuilder stringBuf = new StringBuilder();
        private UInt tmpUInt = CodecFactory.createUInt();
        private ElementList elementList = CodecFactory.createElementList();
        private ElementEntry element = CodecFactory.createElementEntry();

        /**
         * Instantiates a new service data.
         */
        public ServiceData()
        {
            data = CodecFactory.createBuffer();
            clear();
        }

        /**
         * Clears an RDM Service Data.
         */
        public void clear()
        {
            flags = 0;
            dataType = DataTypes.NO_DATA;
            data.clear();
            action = FilterEntryActions.SET;
            type = Directory.DataTypes.NONE;
        }

        /**
         * Applies data presence flag.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         */
        public void applyHasData()
        {
            flags |= Service.ServiceDataFlags.HAS_DATA;
        }

        /**
         * Checks the presence of the data field.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         *
         * @return true - if info field exists, false - if not.
         * @see #flags(int)
         */
        public boolean checkHasData()
        {
            return (flags & Service.ServiceDataFlags.HAS_DATA) != 0;
        }

        /**
         * Encode an RDM Service Data.
         * 
         * @param encIter The Encode Iterator
         * 
         * @return UPA return value
         */
        public int encode(EncodeIterator encIter)
        {
            elementList.clear();
            elementList.applyHasStandardData();
            int ret = elementList.encodeInit(encIter, null, 0);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            if (checkHasData())
            {
                element.clear();
                element.name(ElementNames.TYPE);
                element.dataType(DataTypes.UINT);
                tmpUInt.value(type());
                ret = element.encode(encIter, tmpUInt);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                element.clear();
                element.name(ElementNames.DATA);
                element.dataType(dataType());
                ret = element.encode(encIter, data());
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            return elementList.encodeComplete(encIter, true);
        }

        /**
         * Decode a UPA service data field into an RDM service data field.
         * 
         * @param dIter The Decode Iterator
         * 
         * @return UPA return value
         */
        public int decode(DecodeIterator dIter)
        {
            clear();
            elementList.clear();
            element.clear();

            int ret = elementList.decode(dIter, null);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            boolean foundType = false;
            boolean foundData = false;
            while ((ret = element.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                /* get service data information */
                /* Type */
                if (element.name().equals(ElementNames.TYPE))
                {
                    tmpUInt.clear();
                    ret = tmpUInt.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    foundType = true;
                    type(tmpUInt.toLong());
                }
                /* Data */
                else if (element.name().equals(ElementNames.DATA))
                {
                    Buffer tmpBuf = element.encodedData();
                    data().data(tmpBuf.data(), tmpBuf.position(), tmpBuf.length());
                    dataType(element.dataType());
                    applyHasData();
                    foundData = true;
                }
            }

            /* If Data element is present, type must be too. */
            if ((foundData && !foundType) || (foundType && !foundData))
                return CodecReturnCodes.FAILURE;

            return CodecReturnCodes.SUCCESS;
        }

        /**
         * Performs a deep copy of {@link ServiceData} object.
         *
         * @param destServiceData ServiceData object to copy this object into. It cannot be null.
         * 
         * @return UPA return value indicating success or failure of copy operation.
         */
        public int copy(ServiceData destServiceData)
        {
            assert (destServiceData != null) : "destServiceData can not be null";
            destServiceData.clear();
            destServiceData.action(action());
            destServiceData.type(type());
            destServiceData.flags(flags());
            if (checkHasData())
            {
                destServiceData.applyHasData();
                destServiceData.dataType(dataType());
                ByteBuffer byteBuffer = ByteBuffer.allocate(data().length());
                data().copy(byteBuffer);
                destServiceData.data().data(byteBuffer);
            }

            return CodecReturnCodes.SUCCESS;
        }
        
        /**
         * Performs an update of {@link ServiceData} object.
         *
         * @param destServiceData ServiceData object to update with information from this object. It cannot be null.
         * 
         * @return UPA return value indicating success or failure of update operation.
         */
        public int update(ServiceData destServiceData)
        {
            assert (destServiceData != null) : "destServiceData can not be null";
            destServiceData.action(action());
            destServiceData.type(type());
            destServiceData.flags(flags());
            if (checkHasData())
            {
                destServiceData.applyHasData();
                destServiceData.dataType(dataType());
                ByteBuffer byteBuffer = ByteBuffer.allocate(data().length());
                data().copy(byteBuffer);
                destServiceData.data().data(byteBuffer);
            }

            return CodecReturnCodes.SUCCESS;
        }

        /**
         * Sets service data flags. Populate with {@link ServiceDataFlags}.
         *
         * @param flags the flags
         */
        public void flags(int flags)
        {
            this.flags = flags;
        }

        /**
         * Returns service data flags. Populated by {@link ServiceDataFlags}.
         * 
         * @return flags
         */
        public int flags()
        {
            return flags;
        }

        /**
         * Sets action associated with data filter.
         *
         * @param action the action
         */
        public void action(int action)
        {
            this.action = action;
        }

        /**
         * Returns action associated with data filter.
         * 
         * @return action
         */
        public int action()
        {
            return action;
        }

        /**
         * Returns dataType - The OMM type of the data. Populated by
         * {@link DataTypes}.
         * 
         * @return dataType
         */
        public int dataType()
        {
            return dataType;
        }

        /**
         * dataType - The OMM type of the data. Populated by {@link DataTypes}.
         *
         * @param dataType the data type
         */
        public void dataType(int dataType)
        {
            this.dataType = dataType;
        }

        /**
         * Directory data type. Populated by {@link com.thomsonreuters.upa.rdm.Directory.DataTypes}.
         * 
         * @return type
         */
        public long type()
        {
            return type;
        }

        /**
         * Directory data type. Populated by {@link com.thomsonreuters.upa.rdm.Directory.DataTypes}.
         *
         * @param type the type
         */
        public void type(long type)
        {
            this.type = type;
        }

        /**
         * Returns buffer representing the encoded data, to be applied to all items being provided by this
         * service.
         * 
         * @return data.
         */
        public Buffer data()
        {
            return data;
        }
        
        /**
         * Sets encoded data for this service to the user specified
         * buffer. Data and position of encoded data of this service will be set
         * to passed in buffer's data and position. Note that this creates
         * garbage if buffer is backed by String object.
         *
         * @param data the data
         */
        public void data(Buffer data)
        {
            this.data.data(data.data(), data.position(), data.length());
        }

        /**
         * Returns filterId - Populated by {@link com.thomsonreuters.upa.rdm.Directory.ServiceFilterIds}.
         * 
         * @return filterId
         */
        public int filterId()
        {
            return Directory.ServiceFilterIds.DATA;
        }

        /* (non-Javadoc)
         * @see java.lang.Object#toString()
         */
        public String toString()
        {
            stringBuf.setLength(0);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("DataFilter:");
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("Type: ");
            stringBuf.append(type());
            stringBuf.append(eol);

            if (checkHasData())
            {
                stringBuf.append(tab);
                stringBuf.append(tab);
                stringBuf.append(tab);
                stringBuf.append("Data: ");
                stringBuf.append(data());
                stringBuf.append(eol);

                stringBuf.append(tab);
                stringBuf.append(tab);
                stringBuf.append(tab);
                stringBuf.append("DataType: ");
                stringBuf.append(DataTypes.toString(dataType()));
                stringBuf.append(eol);
            }
            return stringBuf.toString();
        }

    }
    
    
    /**
     * The RDM Service Data flags. A combination of bit values that indicate the presence of optional {@link ServiceData} filter content.
     * 
     * @see ServiceData
     */
    public static class ServiceDataFlags
    {
        /** (0x00) No flags set. */
        public static final int NONE        = 0x00; 
        
        /** (0x01) Indicates presen*ce of the type, dataType and data members. */
        public static final int HAS_DATA    = 0x01;  
        
        /**
         * Instantiates a new service data flags.
         */
        private ServiceDataFlags()
        {
            throw new AssertionError();
        }
    }

    /**
     * The RDM Service filter flags. A combination of bit values that indicate the presence of optional {@link ServiceImpl} content.
     * 
     * @see ServiceImpl
     */
    public static class ServiceFlags
    {
        /** (0x00) No flags set. */
        public static final int NONE       = 0x00; 
        
        /** (0x01) Indicates presence of the info member. */
        public static final int HAS_INFO   = 0x01; 
        
        /** (0x02) Indicates presence of the state member. */
        public static final int HAS_STATE  = 0x02; 
        
        /** (0x04) Indicates presence of the load member. */
        public static final int HAS_LOAD   = 0x04; 
        
        /** (0x08) Indicates presence of the data member. */
        public static final int HAS_DATA   = 0x08; 
        
        /** (0x10) Indicates presence of the linkInfo member. */
        public static final int HAS_LINK   = 0x10; 
          
        /**
         * Instantiates a new service flags.
         */
        private ServiceFlags()
        {
            throw new AssertionError();
        }
    }
    
    /**
     * The RDM Service Group State. Contains information provided by the Source
     * Directory Group filter.
     * 
     * @see ServiceImpl
     */
    public static class ServiceGroup
    {
        private int flags;
        private Buffer group;
        private Buffer mergedToGroup;
        private com.thomsonreuters.upa.codec.State status;
        private int action;
        
        private final static String eol = "\n";
        private final static String tab = "\t";

        private StringBuilder stringBuf = new StringBuilder();
        private ElementList elementList = CodecFactory.createElementList();
        private ElementEntry element = CodecFactory.createElementEntry();
        private com.thomsonreuters.upa.codec.State tmpStatus = CodecFactory.createState();
        
        /**
         * Instantiates a new service group.
         */
        public ServiceGroup()
        {
            status = CodecFactory.createState();
            group = CodecFactory.createBuffer();
            mergedToGroup = CodecFactory.createBuffer();
            clear();
        }

        /**
         * Clears an RDMService group filter.
         */
        public void clear()
        {
            flags = 0;
            group.clear();
            mergedToGroup.clear();
            status.clear();
            status.streamState(StreamStates.OPEN);
            status.code(StateCodes.NONE);
            status.dataState(DataStates.OK);
            action = FilterEntryActions.SET;
        }

        /**
         * Applies status presence flag.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @see #flags()
         */
        public void applyHasStatus()
        {
            flags |= ServiceGroupFlags.HAS_STATUS;
        }

        /**
         * Checks the presence of the status field.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         *
         * @return true - if info field exists, false - if not.
         * @see #flags(int)
         */
        public boolean checkHasStatus()
        {
            return (flags & ServiceGroupFlags.HAS_STATUS) != 0;
        }

        /**
         * Apply has merged to group.
         */
        public void applyHasMergedToGroup()
        {
            flags |= ServiceGroupFlags.HAS_MERGED_TO_GROUP;
        }

        /**
         * Checks the presence of the mergedToGroup field.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         *
         * @return true - if info field exists, false - if not.
         * @see #flags(int)
         */
        public boolean checkHasMergedToGroup()
        {
            return (flags & ServiceGroupFlags.HAS_MERGED_TO_GROUP) != 0;
        }

        /* (non-Javadoc)
         * @see java.lang.Object#toString()
         */
        public String toString()
        {
            stringBuf.setLength(0);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("GroupFilter:");
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("group: ");
            stringBuf.append(group().toHexString());
            stringBuf.append(eol);

            if(checkHasMergedToGroup())
            {
                stringBuf.append(tab);
                stringBuf.append(tab);
                stringBuf.append(tab);
                stringBuf.append("mergedToGroup: ");
                stringBuf.append(mergedToGroup().toHexString());
                stringBuf.append(eol);
            }
            if (checkHasStatus())
            {
                stringBuf.append(tab);
                stringBuf.append(tab);
                stringBuf.append(tab);
                stringBuf.append("status: ");
                stringBuf.append(status());
                stringBuf.append(eol);
            }

            return stringBuf.toString();
        }

        /**
         * Returns service group flags. Populated by {@link ServiceGroupFlags}.
         * 
         * @return flags
         */
        public int flags()
        {
            return flags;
        }

        /**
         * Sets service group flags. Populate with {@link ServiceGroupFlags}.
         *
         * @param flags the flags
         */
        public void flags(int flags)
        {
            this.flags = flags;
        }

        /**
         * Returns action associated with group state filter.
         * 
         * @return action
         */
        public int action()
        {
            return action;
        }

        /**
         * Sets action associated with group state filter..
         *
         * @param action the action
         */
        public void action(int action)
        {
            this.action = action;
        }

        /**
         * The Item Group name associated with this status.
         * 
         * @return group buffer.
         */
        public Buffer group()
        {
            return group;
        }
        
        /**
         * Sets group for this service to the user specified buffer. Data
         * and position of group buffer of this service will be set to passed in
         * buffer's data and position. Note that this creates garbage if buffer
         * is backed by String object.
         *
         * @param group the group
         */
        public void group(Buffer group)
        {
            this.group.data(group.data(), group.position(), group.length());
        }

        /**
         * Sets mergedToGroup buffer for this service to the user specified
         * buffer. Data and position of mergedToGroup buffer of this service
         * will be set to passed in buffer's data and position. Note that this
         * creates garbage if buffer is backed by String object.
         * 
         * @return mergedToGroup
         */
        public Buffer mergedToGroup()
        {
            return mergedToGroup;
        }

        /**
         * Sets the mergedToGroup for this service to the user specified buffer.
         * Buffer used by this object's mergedToGroup field will be set to
         * passed in buffer's data and position. Note that this creates garbage
         * if buffer is backed by String object.
         *
         * @param mergedToGroup the merged to group
         */
        public void mergedToGroup(Buffer mergedToGroup)
        {
            this.mergedToGroup.data(mergedToGroup.data(), mergedToGroup.position(), mergedToGroup.length());
        }
        
        /**
         * The Item Group name associated with this status.
         * 
         * @return status
         */
        public State status()
        {
            return status;
        }
        
        /**
         * Sets status field for service group.
         *
         * @param status the status
         */
        public void status(State status)
        {
            status().streamState(status.streamState());
            status().dataState(status.dataState());
            status().code(status.code());
            status().text(status.text());
        }

        /**
         * Returns filterId - Populated by {@link com.thomsonreuters.upa.rdm.Directory.ServiceFilterIds}.
         * 
         * @return filterId
         */
        public int filterId()
        {
            return Directory.ServiceFilterIds.GROUP;
        }

        /**
         * Encode an RDM Service Data filter.
         * 
         * @param encIter The Encode Iterator
         * 
         * @return UPA return value
         */
        public int encode(EncodeIterator encIter)
        {
            elementList.clear();
            elementList.applyHasStandardData();
            int ret = elementList.encodeInit(encIter, null, 0);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            element.clear();

            if (checkHasMergedToGroup())
            {
                element.clear();
                element.name(ElementNames.MERG_TO_GRP);
                element.dataType(DataTypes.BUFFER);
                ret = element.encode(encIter, mergedToGroup());
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (checkHasStatus())
            {
                element.clear();
                element.name(ElementNames.STATUS);
                element.dataType(DataTypes.STATE);
                ret = element.encode(encIter, status());
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            element.clear();
            element.name(ElementNames.GROUP);
            element.dataType(DataTypes.BUFFER);
            ret = element.encode(encIter, group());
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            ret = elementList.encodeComplete(encIter, true);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;
            return CodecReturnCodes.SUCCESS;
        }

        /**
         * Decode a UPA service data filter into an RDM service data filter.
         * 
         * @param dIter The Decode Iterator
         * 
         * @return UPA return value
         */
        public int decode(DecodeIterator dIter)
        {
            clear();
            elementList.clear();
            element.clear();

            int ret = elementList.decode(dIter, null);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            Buffer tmpBuf = null;
            
            //decode element list elements
            while ((ret = element.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                //Group
                if (element.name().equals(ElementNames.GROUP))
                {
                    tmpBuf = element.encodedData();
                    group.data(tmpBuf.data(), tmpBuf.position(), tmpBuf.length());
                }
                //MergedToGroup
                else if (element.name().equals(ElementNames.MERG_TO_GRP))
                {
                    tmpBuf = element.encodedData();
                    mergedToGroup.data(tmpBuf.data(), tmpBuf.position(), tmpBuf.length());
                    applyHasMergedToGroup();
                }
                //Status
                else if (element.name().equals(ElementNames.STATUS))
                {
                    ret = tmpStatus.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    status().streamState(tmpStatus.streamState());
                    status().dataState(tmpStatus.dataState());
                    status().code(tmpStatus.code());
                    if (tmpStatus.text().length() > 0)
                    {
                        Buffer text = tmpStatus.text();
                        status().text().data(text.data(), text.position(), text.length());
                    }
                    applyHasStatus();
                }

            }

            return CodecReturnCodes.SUCCESS;
        }

        /**
         * Performs a deep copy of {@link ServiceGroup} object.
         *
         * @param destServiceGroup ServiceGroup object to copy this object into. It cannot be null.
         * 
         * @return UPA return value indicating success or failure of copy operation.
         */
        public int copy(ServiceGroup destServiceGroup)
        {
            assert (destServiceGroup != null) : "destServiceGroup can not be null";

            destServiceGroup.clear();
            destServiceGroup.flags(flags());
            destServiceGroup.action(action());
            if (checkHasMergedToGroup())
            {
                ByteBuffer byteBuffer = ByteBuffer.allocate(mergedToGroup().length());
                mergedToGroup().copy(byteBuffer);
                destServiceGroup.mergedToGroup().data(byteBuffer);
                destServiceGroup.applyHasMergedToGroup();
            }

            if (checkHasStatus())
            {
                destServiceGroup.status().streamState(status().streamState());
                destServiceGroup.status().dataState(status().dataState());
                destServiceGroup.status().code(status().code());
                if (status().text().length() > 0)
                {
                    ByteBuffer byteBuffer = ByteBuffer.allocate(status().text().length());
                    status().text().copy(byteBuffer);
                    destServiceGroup.status().text().data(byteBuffer);
                }
                destServiceGroup.applyHasStatus();
            }

            ByteBuffer byteBuffer = ByteBuffer.allocate(group().length());
            group().copy(byteBuffer);
            destServiceGroup.group().data(byteBuffer);

            return CodecReturnCodes.SUCCESS;
        }
    }
    
    /**
     * The RDM Service Group flags. A combination of bit values that indicate the presence of optional {@link ServiceGroup} filter content.
     * 
     * @see ServiceGroup
     */
    public class ServiceGroupFlags
    {
        /** (0x00) No flags set. */
        public static final int NONE = 0x00;

        /**
         * (0x01) Indicates presence of the mergedToGroup member.
         */
        public static final int HAS_MERGED_TO_GROUP = 0x01;

        /**
         * (0x02) Indicates presence of the status member.
         */
        public static final int HAS_STATUS = 0x02;

        /**
         * Instantiates a new service group flags.
         */
        private ServiceGroupFlags()
        {
            throw new AssertionError();
        }
    }
    
    /**
     * The RDM Service Info. Contains information provided by the Source Directory
     * Info filter.
     * 
     * @see Service.ServiceInfoFlags
     * @see ServiceImpl
     */
    public static class ServiceInfo
    {
        private int flags;
        private int action;
        private Buffer serviceName;
        private Buffer vendor;
        private long isSource;
        private List<Long> capabilitiesList;
        private List<String> dictionariesProvidedList;
        private List<String> dictionariesUsedList;
        private List<Qos> qosList;
        private Buffer itemList;

        private long supportsQosRange;
        private long supportsOutOfBandSnapshots;
        private long acceptingConsumerStatus;

        private StringBuilder stringBuf = new StringBuilder();
        private final static String eol = "\n";
        private final static String tab = "\t";

        private ElementList elementList = CodecFactory.createElementList();
        private ElementEntry element = CodecFactory.createElementEntry();
        private UInt tmpUInt = CodecFactory.createUInt();
        private Array array = CodecFactory.createArray();
        private ArrayEntry arrayEntry = CodecFactory.createArrayEntry();
        private Buffer tmpBuffer = CodecFactory.createBuffer();
        
        private Qos bestQos;

        /**
         * Instantiates a new service info.
         */
        public ServiceInfo()
        {
            capabilitiesList = new ArrayList<Long>();
            dictionariesProvidedList = new ArrayList<String>();
            dictionariesUsedList = new ArrayList<String>();
            qosList = new ArrayList<Qos>();
            supportsOutOfBandSnapshots = 1;
            acceptingConsumerStatus = 1;
            serviceName = CodecFactory.createBuffer();
            vendor = CodecFactory.createBuffer();
            itemList = CodecFactory.createBuffer();
            serviceName = CodecFactory.createBuffer();
            clear();
        }

        /**
         * Clears an RDMService info filter.
         */
        public void clear()
        {
            flags = 0;
            action = FilterEntryActions.SET;
            isSource = 0;
            capabilitiesList.clear();
            dictionariesProvidedList.clear();
            dictionariesUsedList.clear();
            qosList.clear();
            serviceName.clear();
            itemList.clear();
            vendor.clear();
            supportsOutOfBandSnapshots = 1;
            acceptingConsumerStatus = 1;
            supportsQosRange = 0;
            bestQos = null;
        }

        /**
         * Returns the service name that identifies this service.
         * 
         * @return service name.
         */
        public Buffer serviceName()
        {
            return serviceName;
        }
        
        /**
         * Sets serviceName for this service to the user specified
         * buffer. Data and position of serviceName buffer will be set to passed
         * in buffer's data and position. Note that this creates garbage if
         * buffer is backed by String object.
         *
         * @param serviceName the service name
         */
        public void serviceName(Buffer serviceName)
        {
            assert (serviceName != null) : "serviceName can not be null";

            serviceName().data(serviceName.data(), serviceName.position(), serviceName.length());
        }

        /**
         * Returns the vendor name of data provided by this service.
         * 
         * @return Vendor.
         */
        public Buffer vendor()
        {
            return vendor;
        }

        /**
         * Sets vendor name for this service to the user specified buffer. Data
         * and position of vendor field buffer will be set to passed in buffer's
         * data and position. Note that this creates garbage if buffer is backed
         * by String object.
         *
         * @param vendor the vendor
         */
        public void vendor(Buffer vendor)
        {
            assert (vendor != null) : "vendor can not be null";

            vendor().data(vendor.data(), vendor.position(), vendor.length());
        }
        
        /**
         * Applies vendor flag.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         * 
         */
        public void applyHasVendor()
        {
            flags |= Service.ServiceInfoFlags.HAS_VENDOR;
        }

        /**
         * Checks the presence of the vendor field.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @return true - if vendor field exists, false - if not.
         * 
         */
        public boolean checkHasVendor()
        {
            return (flags & Service.ServiceInfoFlags.HAS_VENDOR) != 0;
        }

        /**
         * Returns isSource field that indicates whether the service is provided directly by a
         * publisher or consolidated from multiple sources.
         * 
         * @return isSource
         */
        public long isSource()
        {
            return isSource;
        }

        /**
         * Sets isSource field that indicates whether the service is provided directly by a
         * publisher or consolidated from multiple sources.
         *
         * @param isSource the is source
         */
        public void isSource(long isSource)
        {
            assert(checkHasIsSource());
            this.isSource = isSource;
        }

        /**
         * Applies isSource flag.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         * 
         */
        public void applyHasIsSource()
        {
            flags |= Service.ServiceInfoFlags.HAS_IS_SOURCE;
        }

        /**
         * Checks the presence of the isSource field.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @return true - if isSource field exists, false - if not.
         * 
         */
        public boolean checkHasIsSource()
        {
            return (flags & Service.ServiceInfoFlags.HAS_IS_SOURCE) != 0;
        }

        /**
         * supportsQosRange - Field that indicates whether items can be requested using a
         * QoS range(using both the qos and worstQos members of a {@link RequestMsg}).
         * 
         * @return supportsQosRange field.
         */
        public long supportsQosRange()
        {
            return supportsQosRange;
        }

        /**
         * Sets supportsQosRange - Field that indicates whether items can be requested using a QoS
         * range(using both the qos and worstQos members of a {@link RequestMsg}).
         *
         * @param supportsQosRange the supports qos range
         */
        public void supportsQosRange(long supportsQosRange)
        {
            assert(checkHasSupportsQosRange());
            this.supportsQosRange = supportsQosRange;
        }

        /**
         * Applies supportsQosRange flag.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         */
        public void applyHasSupportsQosRange()
        {
            flags |= Service.ServiceInfoFlags.HAS_SUPPORT_QOS_RANGE;
        }

        /**
         * Checks the presence of the supportsQosRange field.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @return true - if supportsQosRange field exists, false - if not.
         */
        public boolean checkHasSupportsQosRange()
        {
            return (flags & Service.ServiceInfoFlags.HAS_SUPPORT_QOS_RANGE) != 0;
        }

        /**
         * supportsOutOfBandSnapshots - Field that indicates whether Snapshot(requests without
         * the STREAMING flag) can be made when the OpenLimit is reached.
         * 
         * @return supportsOutOfBandSnapshots
         */
        public long supportsOutOfBandSnapshots()
        {
            return supportsOutOfBandSnapshots;
        }

        /**
         * supportsOutOfBandSnapshots - Field that indicates whether Snapshot(requests without
         * the STREAMING flag) can be made when the OpenLimit is reached.
         *
         * @param supportsOutOfBandSnapshots the supports out of band snapshots
         */
        public void supportsOutOfBandSnapshots(long supportsOutOfBandSnapshots)
        {
            assert(checkHasSupportsOutOfBandSnapshots());
            this.supportsOutOfBandSnapshots = supportsOutOfBandSnapshots;
        }

        /**
         * Applies the supportsOutOfBandSnapshots presence flag.
         */
        public void applyHasSupportsOutOfBandSnapshots()
        {
            flags |= Service.ServiceInfoFlags.HAS_SUPPORT_OOB_SNAPSHOTS;
        }

        /**
         * Checks the presence of the supportsOutOfBandSnapshots field.
         * 
         * @return true - if supportsOutOfBandSnapshots field present, false - if
         *         not.
         */
        public boolean checkHasSupportsOutOfBandSnapshots()
        {
            return (flags & Service.ServiceInfoFlags.HAS_SUPPORT_OOB_SNAPSHOTS) != 0;
        }

        /**
         * Returns acceptingConsumerStatus - Field that indicates whether the service accepts messages
         * related to Source Mirroring.
         *
         * @return acceptingConsumerStatus
         * @see DirectoryConsumerStatus
         */
        public long acceptingConsumerStatus()
        {
            return acceptingConsumerStatus;
        }

        /**
         * Sets acceptingConsumerStatus - Field that indicates whether the service accepts messages
         * related to Source Mirroring.
         *
         * @param acceptingConsumerStatus the accepting consumer status
         * @see DirectoryConsumerStatus
         */
        public void acceptingConsumerStatus(long acceptingConsumerStatus)
        {
            assert(checkHasAcceptingConsumerStatus());
            this.acceptingConsumerStatus = acceptingConsumerStatus;
            flags |= Service.ServiceInfoFlags.HAS_ACCEPTING_CONS_STATUS;
        }

        /**
         * Applies the acceptingConsumerStatus presence flag.
         */
        public void applyHasAcceptingConsumerStatus()
        {
            flags |= Service.ServiceInfoFlags.HAS_ACCEPTING_CONS_STATUS;
        }

        /**
         * Checks the presence of the acceptingConsumerStatus field.
         * 
         * @return true - if acceptingConsumerStatus field present, false - if not.
         */
        public boolean checkHasAcceptingConsumerStatus()
        {
            return (flags & Service.ServiceInfoFlags.HAS_ACCEPTING_CONS_STATUS) != 0;
        }

        /**
         * Returns the list of item names a Consumer can request to get a symbol list
         * of all item names available from this service.
         * 
         * @return itemList
         */
        public Buffer itemList()
        {
            return itemList;
        }

        
        /**
         * Sets itemList for this service to the user specified buffer. Data and
         * position of itemList buffer will be set to passed in buffer's data
         * and position. Note that this creates garbage if buffer is backed by
         * String object.
         *
         * @param itemList the item list
         */
        public void itemList(Buffer itemList)
        {
            assert (itemList != null) : "itemList can not be null";

            itemList().data(itemList.data(), itemList.position(), itemList.length());
        }
        
        /**
         * Applies the itemList presence field.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         * 
         */
        public void applyHasItemList()
        {
            flags |= Service.ServiceInfoFlags.HAS_ITEM_LIST;
        }

        /**
         * Checks the presence of the itemList field.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @return true - if itemList field exists, false - if not.
         * 
         */
        public boolean checkHasItemList()
        {
            return (flags & Service.ServiceInfoFlags.HAS_ITEM_LIST) != 0;
        }

        /**
         * Returns list of capabilities the service supports. Capability in the
         * list is populated by {@link DomainTypes}.
         * 
         * @return capabilities list.
         */
        public List<Long> capabilitiesList()
        {
            return capabilitiesList;
        }

        /**
         * Sets capabilities for this service.
         *
         * @param capabilitiesList the capabilities list
         */
        public void capabilitiesList(List<Long> capabilitiesList)
        {
            assert (capabilitiesList != null) : "capabilitiesList can not be null";

            capabilitiesList().clear();
           
            for (Long capability : capabilitiesList)
            {
                capabilitiesList().add(capability);
            }
        }
        
        /**
         * Returns the list of dictionary names that this service provides.
         * 
         * @return dictionariesProvided list.
         */
        public List<String> dictionariesProvidedList()
        {
            return dictionariesProvidedList;
        }

        /**
         * Sets dictionary names provided by this service.
         *
         * @param dictionariesProvidedList the dictionaries provided list
         */
        public void dictionariesProvidedList(List<String> dictionariesProvidedList)
        {
            assert (dictionariesProvidedList != null) : "dictionariesProvidedList can not be null";

            dictionariesProvidedList().clear();
           
            for (String dictionaryProvided : dictionariesProvidedList)
            {
                dictionariesProvidedList().add(dictionaryProvided);
            }
        }
        
        /**
         * Applies the dictionariesProvided presence field.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         * 
         */
        public void applyHasDictionariesProvided()
        {
            flags |= Service.ServiceInfoFlags.HAS_DICTS_PROVIDED;
        }

        /**
         * Checks the presence of the dictionariesProvided field.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @return true - if dictionariesProvided field exists, false - if not.
         */
        public boolean checkHasDictionariesProvided()
        {
            return (flags & Service.ServiceInfoFlags.HAS_DICTS_PROVIDED) != 0;
        }

        /**
         * returns the list of dictionary names that a Consumer will require to decode the
         * service's market data.
         * 
         * @return list of dictionary names.
         */
        public List<String> dictionariesUsedList()
        {
            return dictionariesUsedList;
        }
        
        /**
         * Sets dictionary names that a consumer will require to decode the
         * service's market data content.
         *
         * @param dictionariesUsedList the dictionaries used list
         */
        public void dictionariesUsedList(List<String> dictionariesUsedList)
        {
            assert (dictionariesUsedList != null) : "dictionariesUsedList can not be null";

            dictionariesUsedList().clear();
           
            for (String dictionaryUsed : dictionariesUsedList)
            {
                dictionariesUsedList().add(dictionaryUsed);
            }
        }

        /**
         * Applies the dictionariesUsed presence field.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         */
        public void applyHasDictionariesUsed()
        {
            flags |= Service.ServiceInfoFlags.HAS_DICTS_USED;
        }

        /**
         * dictionariesUsed - Checks the presence of the dictionariesUsed field.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @return true - if dictionariesUsed field exists, false - if not.
         */
        public boolean checkHasDictionariesUsed()
        {
            return (flags & Service.ServiceInfoFlags.HAS_DICTS_USED) != 0;
        }

        /**
         * Returns the list of qualities of service that this service provides.
         * 
         * @return qosList
         */
        public List<Qos> qosList()
        {
            return qosList;
        }
        
        /**
         * Sets qualities of service that this service provides.
         *
         * @param qosList the qos list
         */
        public void qosList(List<Qos> qosList)
        {
            assert (qosList != null) : "qosList can not be null";

            qosList().clear();
           
            for (Qos qos : qosList)
            {
                qosList().add(qos);
            }
        }
        
        /**
         * Returns the best quality of service that this service provides.
         * 
         * @return bestQos
         */
        public Qos bestQos()
        {
            if (bestQos == null)
            {
                bestQos = CodecFactory.createQos();
                for (Qos qos : qosList)
                {
                   if (qos.isBetter(bestQos))
                   {
                       bestQos = qos;
                   }
                }
                
            }

            return bestQos;
        }

        /**
         * Applies the qosList presence field.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         */
        public void applyHasQos()
        {
            flags |= Service.ServiceInfoFlags.HAS_QOS;
        }

        /**
         * Checks the presence of the qosList field.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @return true - if qosList field exists, false - if not.
         */
        public boolean checkHasQos()
        {
            return (flags & Service.ServiceInfoFlags.HAS_QOS) != 0;
        }

        /**
         * action - Action associated with this info filter.
         * 
         * @return action -{@link MapEntryActions}
         */
        public int action()
        {
            return action;
        }

        /**
         * Sets action associated with this info filter.
         * 
         * @param action -{@link MapEntryActions}
         */
        public void action(int action)
        {
            this.action = action;
        }
      
        /**
         * Returns service info flags. Populated by {@link ServiceInfoFlags}
         * 
         * 
         * @return flags
         */
        public int flags()
        {
            return flags;
        }

        /**
         * The service info flags. Populate with {@link ServiceInfoFlags}
         *
         * @param flags the flags
         */
        public void flags(int flags)
        {
            this.flags = flags;
        }
        
        /**
         * Returns filterId - Populated by {@link com.thomsonreuters.upa.rdm.Directory.ServiceFilterIds}.
         * 
         * @return filterId
         */
        public int filterId()
        {
            return Directory.ServiceFilterIds.INFO;
        }

        /**
         * Encode an RDM Service info filter.
         * 
         * @param encodeIter The Encode Iterator
         * 
         * @return UPA return value
         */
        public int encode(EncodeIterator encodeIter)
        {
            elementList.clear();
            elementList.applyHasStandardData();
            int ret = elementList.encodeInit(encodeIter, null, 0);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            element.clear();
            element.name(ElementNames.NAME);
            element.dataType(DataTypes.ASCII_STRING);
            ret = element.encode(encodeIter, serviceName());
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            if (checkHasVendor())
            {
                element.name(ElementNames.VENDOR);
                element.dataType(DataTypes.ASCII_STRING);
                ret = element.encode(encodeIter, vendor());
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (checkHasIsSource())
            {
                element.name(ElementNames.IS_SOURCE);
                element.dataType(DataTypes.UINT);
                tmpUInt.value(isSource());
                ret = element.encode(encodeIter, tmpUInt);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            /* Capabilities */
            element.name(ElementNames.CAPABILITIES);
            element.dataType(DataTypes.ARRAY);
            ret = element.encodeInit(encodeIter, 0);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            array.clear();
            array.primitiveType(DataTypes.UINT);
            array.itemLength(0);
            ret = array.encodeInit(encodeIter);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            for (long capability : capabilitiesList())
            {
                tmpUInt.value(capability);
                ret = arrayEntry.encode(encodeIter, tmpUInt);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            ret = array.encodeComplete(encodeIter, true);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            ret = element.encodeComplete(encodeIter, true);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            /* DictionariesProvided */
            if (checkHasDictionariesProvided())
            {
                element.name(ElementNames.DICTIONARIES_PROVIDED);
                element.dataType(DataTypes.ARRAY);
                ret = element.encodeInit(encodeIter, 0);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                array.clear();
                array.primitiveType(DataTypes.ASCII_STRING);
                array.itemLength(0);
                ret = array.encodeInit(encodeIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                for (String dictProvided : dictionariesProvidedList())
                {
                    tmpBuffer.data(dictProvided);
                    ret = arrayEntry.encode(encodeIter, tmpBuffer);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                ret = array.encodeComplete(encodeIter, true);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                ret = element.encodeComplete(encodeIter, true);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            /* DictionariesUsed */
            if (checkHasDictionariesUsed())
            {
                element.name(ElementNames.DICTIONARIES_USED);
                element.dataType(DataTypes.ARRAY);
                ret = element.encodeInit(encodeIter, 0);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                array.clear();
                array.primitiveType(DataTypes.ASCII_STRING);
                array.itemLength(0);
                ret = array.encodeInit(encodeIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                for (String dictUsed : dictionariesUsedList())
                {
                    tmpBuffer.data(dictUsed);
                    ret = arrayEntry.encode(encodeIter, tmpBuffer);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                ret = array.encodeComplete(encodeIter, true);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                ret = element.encodeComplete(encodeIter, true);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (checkHasQos())
            {
                element.name(ElementNames.QOS);
                element.dataType(DataTypes.ARRAY);
                ret = element.encodeInit(encodeIter, 0);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                array.clear();
                array.primitiveType(DataTypes.QOS);
                array.itemLength(0);
                ret = array.encodeInit(encodeIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                for (Qos qos : qosList())
                {
                    ret = arrayEntry.encode(encodeIter, qos);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                ret = array.encodeComplete(encodeIter, true);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                ret = element.encodeComplete(encodeIter, true);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (checkHasSupportsQosRange())
            {
                element.name(ElementNames.SUPPS_QOS_RANGE);
                element.dataType(DataTypes.UINT);
                tmpUInt.value(supportsQosRange());
                ret = element.encode(encodeIter, tmpUInt);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (checkHasItemList())
            {
                element.name(ElementNames.ITEM_LIST);
                element.dataType(DataTypes.ASCII_STRING);
                ret = element.encode(encodeIter, itemList());
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (checkHasSupportsOutOfBandSnapshots())
            {
                element.name(ElementNames.SUPPS_OOB_SNAPSHOTS);
                element.dataType(DataTypes.UINT);
                tmpUInt.value(supportsOutOfBandSnapshots());
                ret = element.encode(encodeIter, tmpUInt);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (checkHasAcceptingConsumerStatus())
            {
                element.name(ElementNames.ACCEPTING_CONS_STATUS);
                element.dataType(DataTypes.UINT);
                tmpUInt.value(acceptingConsumerStatus());
                ret = element.encode(encodeIter, tmpUInt);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            return elementList.encodeComplete(encodeIter, true);
        }

        /**
         * Decode a UPA service data filter into an RDM service info filter.
         * 
         * @param dIter The Decode Iterator
         * 
         * @return UPA return value
         */
        public int decode(DecodeIterator dIter)
        {
            clear();
            elementList.clear();
            element.clear();
            array.clear();

            int ret = elementList.decode(dIter, null);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;

            Buffer tmpBuffer = null;
            while ((ret = element.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }

                if (element.name().equals(ElementNames.NAME))
                {
                    tmpBuffer = element.encodedData();
                    serviceName().data(tmpBuffer.data(), tmpBuffer.position(), tmpBuffer.length());
                }
                else if (element.name().equals(ElementNames.VENDOR))
                {
                    applyHasVendor();
                    tmpBuffer = element.encodedData();
                    vendor().data(tmpBuffer.data(), tmpBuffer.position(), tmpBuffer.length());
                }
                else if (element.name().equals(ElementNames.IS_SOURCE))
                {
                    ret = tmpUInt.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    applyHasIsSource();
                    isSource(tmpUInt.toLong());
                }
                else if (element.name().equals(ElementNames.CAPABILITIES))
                {
                    if ((ret = array.decode(dIter)) < CodecReturnCodes.SUCCESS)
                    {
                        return ret;
                    }
                    
                    while ((ret = arrayEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                    {
                        if (ret == CodecReturnCodes.SUCCESS)
                        {
                            ret = tmpUInt.decode(dIter);
                            capabilitiesList().add(tmpUInt.toLong());
                            if (ret != CodecReturnCodes.SUCCESS
                                    && ret != CodecReturnCodes.BLANK_DATA)
                            {
                                return ret;
                            }
                        }
                        else if (ret != CodecReturnCodes.BLANK_DATA)
                        {
                            return ret;
                        }
                    }
                }
                else if (element.name()
                        .equals(ElementNames.DICTIONARIES_PROVIDED))
                {
                    ret = array.decode(dIter);
                    if (ret < CodecReturnCodes.SUCCESS)
                        return ret;
                    applyHasDictionariesProvided();
                    while ((ret = arrayEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                    {
                        if (ret == CodecReturnCodes.SUCCESS)
                        {
                            dictionariesProvidedList().add(arrayEntry.encodedData().toString());
                        }
                        else if (ret != CodecReturnCodes.BLANK_DATA)
                        {
                            return ret;
                        }
                    }
                }
                else if (element.name().equals(ElementNames.DICTIONARIES_USED))
                {
                    ret = array.decode(dIter);
                    if (ret < CodecReturnCodes.SUCCESS)
                        return ret;

                    applyHasDictionariesUsed();
                    while ((ret = arrayEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                    {
                        if (ret == CodecReturnCodes.SUCCESS)
                        {
                            dictionariesUsedList().add(arrayEntry.encodedData().toString());
                        }
                        else if (ret != CodecReturnCodes.BLANK_DATA)
                        {
                            return ret;
                        }
                    }
                }
                else if (element.name().equals(ElementNames.QOS))
                {
                    if ((ret = array.decode(dIter)) < CodecReturnCodes.SUCCESS)
                    {
                        return ret;
                    }
                    applyHasQos();
                    while ((ret = arrayEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                    {
                        if (ret == CodecReturnCodes.SUCCESS)
                        {
                            Qos qos = CodecFactory.createQos();
                            ret = qos.decode(dIter);
                            qosList().add(qos);
                            if (ret != CodecReturnCodes.SUCCESS
                                    && ret != CodecReturnCodes.BLANK_DATA)
                            {
                                return ret;
                            }
                        }
                        else if (ret != CodecReturnCodes.BLANK_DATA)
                        {
                            return ret;
                        }
                    }
                }
                else if (element.name().equals(ElementNames.SUPPS_QOS_RANGE))
                {
                    ret = tmpUInt.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    applyHasSupportsQosRange();
                    supportsQosRange(tmpUInt.toLong());
                }
                else if (element.name().equals(ElementNames.ITEM_LIST))
                {
                    applyHasItemList();
                    tmpBuffer = element.encodedData();
                    itemList().data(tmpBuffer.data(), tmpBuffer.position(), tmpBuffer.length());
                }
                else if (element.name()
                        .equals(ElementNames.SUPPS_OOB_SNAPSHOTS))
                {
                    ret = tmpUInt.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    applyHasSupportsOutOfBandSnapshots();
                    supportsOutOfBandSnapshots(tmpUInt.toLong());
                }
                else if (element.name()
                        .equals(ElementNames.ACCEPTING_CONS_STATUS))
                {
                    ret = tmpUInt.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    applyHasAcceptingConsumerStatus();
                    acceptingConsumerStatus(tmpUInt.toLong());
                }
            }

            return CodecReturnCodes.SUCCESS;
        }

        /**
         * Performs a deep copy of {@link ServiceInfo} object.
         *
         * @param destServiceInfo ServiceInfo object to copy this object into. It cannot be null.
         * 
         * @return UPA return value indicating success or failure of copy operation.
         */
        public int copy(ServiceInfo destServiceInfo)
        {
            assert (destServiceInfo != null) : "destServiceInfo can not be null";
            ByteBuffer byteBuffer = ByteBuffer.allocate(serviceName().length());
            serviceName().copy(byteBuffer);
            destServiceInfo.clear();
            destServiceInfo.serviceName().data(byteBuffer);
            destServiceInfo.action(action());
            destServiceInfo.capabilitiesList().addAll(capabilitiesList());
            if (checkHasAcceptingConsumerStatus())
            {
                destServiceInfo.applyHasAcceptingConsumerStatus();
                destServiceInfo.acceptingConsumerStatus(acceptingConsumerStatus());
            }
            if (checkHasDictionariesProvided())
            {
                destServiceInfo.applyHasDictionariesProvided();
                for (String dictProvided : dictionariesProvidedList())
                {
                    destServiceInfo.dictionariesProvidedList().add(dictProvided);
                }
            }
            if (checkHasDictionariesUsed())
            {
                destServiceInfo.applyHasDictionariesUsed();
                for (String dictUsed : dictionariesUsedList())
                {
                    destServiceInfo.dictionariesUsedList().add(dictUsed);
                }
            }
            if (checkHasIsSource())
            {
                destServiceInfo.applyHasIsSource();
                destServiceInfo.isSource(isSource());
            }
            if (checkHasItemList())
            {
                destServiceInfo.applyHasItemList();
                byteBuffer = ByteBuffer.allocate(itemList().length());
                itemList().copy(byteBuffer);
                destServiceInfo.itemList().data(byteBuffer);
            }
            if (checkHasQos())
            {
                destServiceInfo.applyHasQos();
                for (Qos qos : qosList())
                {
                    Qos copyqos = CodecFactory.createQos();
                    copyqos.dynamic(qos.isDynamic());
                    copyqos.rate(qos.rate());
                    copyqos.rateInfo(qos.rateInfo());
                    copyqos.timeInfo(qos.timeInfo());
                    copyqos.timeliness(qos.timeliness());
                    destServiceInfo.qosList().add(copyqos);
                }
            }
            if (checkHasSupportsOutOfBandSnapshots())
            {
                destServiceInfo.applyHasSupportsOutOfBandSnapshots();
                destServiceInfo.supportsOutOfBandSnapshots(supportsOutOfBandSnapshots());
            }
            if (checkHasSupportsQosRange())
            {
                destServiceInfo.applyHasSupportsQosRange();
                destServiceInfo.supportsQosRange(supportsQosRange());
            }

            if (checkHasVendor())
            {
                destServiceInfo.applyHasVendor();
                byteBuffer = ByteBuffer.allocate(vendor().length());
                vendor().copy(byteBuffer);
                destServiceInfo.vendor().data(byteBuffer);
            }

            return CodecReturnCodes.SUCCESS;
        }

        /**
         * Performs an update of {@link ServiceInfo} object.
         *
         * @param destServiceInfo ServiceInfo object to update with information from this object. It cannot be null.
         * 
         * @return UPA return value indicating success or failure of update operation.
         */
        public int update(ServiceInfo destServiceInfo)
        {
            assert (destServiceInfo != null) : "destServiceInfo can not be null";
            ByteBuffer byteBuffer = ByteBuffer.allocate(serviceName().length());
            serviceName().copy(byteBuffer);
            destServiceInfo.serviceName().data(byteBuffer);
            destServiceInfo.action(action());
            destServiceInfo.capabilitiesList().addAll(capabilitiesList());
            if (checkHasAcceptingConsumerStatus())
            {
                destServiceInfo.applyHasAcceptingConsumerStatus();
                destServiceInfo.acceptingConsumerStatus(acceptingConsumerStatus());
            }
            if (checkHasDictionariesProvided())
            {
                destServiceInfo.applyHasDictionariesProvided();
                for (String dictProvided : dictionariesProvidedList())
                {
                    destServiceInfo.dictionariesProvidedList().add(dictProvided);
                }
            }
            if (checkHasDictionariesUsed())
            {
                destServiceInfo.applyHasDictionariesUsed();
                for (String dictUsed : dictionariesUsedList())
                {
                    destServiceInfo.dictionariesUsedList().add(dictUsed);
                }
            }
            if (checkHasIsSource())
            {
                destServiceInfo.applyHasIsSource();
                destServiceInfo.isSource(isSource());
            }
            if (checkHasItemList())
            {
                destServiceInfo.applyHasItemList();
                byteBuffer = ByteBuffer.allocate(itemList().length());
                itemList().copy(byteBuffer);
                destServiceInfo.itemList().data(byteBuffer);
            }
            if (checkHasQos())
            {
                destServiceInfo.applyHasQos();
                destServiceInfo.qosList().clear();
                for (Qos qos : qosList())
                {
                    Qos copyqos = CodecFactory.createQos();
                    copyqos.dynamic(qos.isDynamic());
                    copyqos.rate(qos.rate());
                    copyqos.rateInfo(qos.rateInfo());
                    copyqos.timeInfo(qos.timeInfo());
                    copyqos.timeliness(qos.timeliness());
                    destServiceInfo.qosList().add(copyqos);
                }
            }
            if (checkHasSupportsOutOfBandSnapshots())
            {
                destServiceInfo.applyHasSupportsOutOfBandSnapshots();
                destServiceInfo.supportsOutOfBandSnapshots(supportsOutOfBandSnapshots());
            }
            if (checkHasSupportsQosRange())
            {
                destServiceInfo.applyHasSupportsQosRange();
                destServiceInfo.supportsQosRange(supportsQosRange());
            }

            if (checkHasVendor())
            {
                destServiceInfo.applyHasVendor();
                byteBuffer = ByteBuffer.allocate(vendor().length());
                vendor().copy(byteBuffer);
                destServiceInfo.vendor().data(byteBuffer);
            }

            return CodecReturnCodes.SUCCESS;
        }
        
        /* (non-Javadoc)
         * @see java.lang.Object#toString()
         */
        public String toString()
        {
            stringBuf.setLength(0);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("InfoFilter:");
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("serviceName: ");
            stringBuf.append(serviceName());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("vendor: ");
            stringBuf.append(vendor());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("isSource: ");
            stringBuf.append(isSource());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("supportsQosRange: ");
            stringBuf.append(supportsQosRange());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("supportsOutOfBandSnapshots: ");
            stringBuf.append(supportsOutOfBandSnapshots());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("acceptingConsumerStatus: ");
            stringBuf.append(acceptingConsumerStatus());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("capabilities: ");
            stringBuf.append(capabilitiesList());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("dictionariesProvided: ");
            stringBuf.append(dictionariesProvidedList());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("dictionariesUsed: ");
            stringBuf.append(dictionariesUsedList());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("qos: ");
            stringBuf.append(qosList());
            stringBuf.append(eol);

            return stringBuf.toString();
        }
    }
    
    /**
     * The RDM Service Info flags. A combination of bit values that indicate the presence of optional {@link ServiceInfo} filter content.
     * 
     * @see ServiceInfo
     */
    public class ServiceInfoFlags
    {
        /** (0x000) No flags set. */
        public static final int NONE = 0x0000;

        /**
         * (0x001) Indicates presence of the vendor member.
         */
        public static final int HAS_VENDOR = 0x001;

        /**
         * (0x002) Indicates presence of the isSource member.
         */
        public static final int HAS_IS_SOURCE = 0x002;

        /**
         * (0x004) Indicates presence of the dictionariesProvidedList member.
         */
        public static final int HAS_DICTS_PROVIDED = 0x004;

        /**
         * (0x008) Indicates presence of the dictionariesUsedList member.
         */
        public static final int HAS_DICTS_USED = 0x008;

        /**
         * (0x010) Indicates presence of the qosList member.
         */
        public static final int HAS_QOS = 0x010;

        /**
         * (0x020) Indicates presence of the supportsQosRange member.
         */
        public static final int HAS_SUPPORT_QOS_RANGE = 0x020;

        /**
         * (0x040) Indicates presence of the itemList member.
         */
        public static final int HAS_ITEM_LIST = 0x040;

        /**
         * (0x080) Indicates presence of the supportsOutOfBandSnapshots member.
         */
        public static final int HAS_SUPPORT_OOB_SNAPSHOTS = 0x080;

        /**
         * (0x100) Indicates presence of the acceptingConsumerStatus member.
         */
        public static final int HAS_ACCEPTING_CONS_STATUS = 0x100;

        /**
         * Instantiates a new service info flags.
         */
        private ServiceInfoFlags()
        {
            throw new AssertionError();
        }
    }
    
    /**
     * The RDM Service Link. Contains information about an upstream source
     * associated with the service.
     * 
     * @see ServiceLinkFlags
     * 
     * @see ServiceImpl
     */
    public static class ServiceLink
    {
        private Buffer name;
        private long type;
        private long linkState;
        private long linkCode;
        private Buffer text;
        private int flags;
        private int action; // map entry action

        private final static String eol = "\n";
        private final static String tab = "\t";

        private StringBuilder stringBuf = new StringBuilder();
        private ElementList elementList = CodecFactory.createElementList();
        private ElementEntry element = CodecFactory.createElementEntry();
        private UInt tmpUInt = CodecFactory.createUInt();
        
        /**
         * Instantiates a new service link.
         */
        public ServiceLink()
        {
            text = CodecFactory.createBuffer();
            name = CodecFactory.createBuffer();
            clear();
        }

        /* (non-Javadoc)
         * @see java.lang.Object#toString()
         */
        public String toString()
        {
            stringBuf.setLength(0);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("Name: ");
            stringBuf.append(name());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("LinkType: ");
            stringBuf.append(type());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("LinkState: ");
            stringBuf.append(linkState());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("LinkCode: ");
            stringBuf.append(linkCode());
            stringBuf.append(eol);
            
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("LinkText: ");
            stringBuf.append(text());
            stringBuf.append(eol);

            return stringBuf.toString();
        }

        /**
         * Clears an RDMServiceLink.
         */
        public void clear()
        {
            flags = 0;
            name.clear();
            type = Directory.LinkTypes.INTERACTIVE;
            linkState = 0;
            linkCode = Directory.LinkCodes.NONE;
            text.clear();
            action = MapEntryActions.ADD;
        }

        /**
         * Performs a deep copy of {@link ServiceLink} object.
         *
         * @param destServiceLink ServiceLink object to copy this object into. It cannot be null.
         * 
         * @return UPA return value indicating success or failure of copy operation.
         */
        public int copy(ServiceLink destServiceLink)
        {
            assert (destServiceLink != null) : "destServiceLink can not be null";
            if (checkHasCode())
            {
                destServiceLink.applyHasCode();
                destServiceLink.linkCode(linkCode());
            }

            if (checkHasText())
            {
                destServiceLink.applyHasText();
                ByteBuffer byteBuffer = ByteBuffer.allocate(text().length());
                text().copy(byteBuffer);
                destServiceLink.text().data(byteBuffer);
            }

            if (checkHasType())
            {
                destServiceLink.applyHasType();
                destServiceLink.type(type());
            }

            ByteBuffer nameBytebuffer = ByteBuffer.allocate(name().length());
            name().copy(nameBytebuffer);
            destServiceLink.name.data(nameBytebuffer);

            destServiceLink.linkState(linkState());

            return CodecReturnCodes.SUCCESS;
        }

        /**
         * Returns name identifying this upstream source.
         * 
         * @return type
         */
        public Buffer name()
        {
            return name;
        }
        
        /**
         * Sets upstream source name to the user specified buffer. Data and
         * position of name field buffer will be set to passed in buffer's data
         * and position. Note that this creates garbage if buffer is backed by
         * String object.
         *
         * @param name the name
         */
        public void name(Buffer name)
        {
            assert(name != null) : "name can not be null";
            name().data(name.data(), name.position(), name.length());
        }

        /**
         * Returns type of this service link. Populated by {@link com.thomsonreuters.upa.rdm.Directory.LinkTypes}
         * 
         * @return type
         */
        public long type()
        {
            return type;
        }

        /**
         * Sets type of this service link. Populated by {@link com.thomsonreuters.upa.rdm.Directory.LinkTypes}
         *
         * @param type the type
         */
        public void type(long type)
        {
            assert(checkHasType());
            this.type = type;
        }

        /**
         * Returns linkState - Flag indicating whether the source is up or down. Populated by {@link com.thomsonreuters.upa.rdm.Directory.LinkStates}.
         * 
         * @return linkState
         */
        public long linkState()
        {
            return linkState;
        }

        /**
         * Sets linkState - Flag indicating whether the source is up or down. Populated by {@link com.thomsonreuters.upa.rdm.Directory.LinkStates}.
         *
         * @param linkState the link state
         */
        public void linkState(long linkState)
        {
            this.linkState = linkState;
        }

        /**
         * Returns linkCode - Code indicating additional information about the status of the source. Populated by {@link com.thomsonreuters.upa.rdm.Directory.LinkCodes}.
         *  
         * @return linkCode
         */
        public long linkCode()
        {
            return linkCode;
        }

        /**
         * Sets linkCode - Code indicating additional information about the status of the source. Populated by {@link com.thomsonreuters.upa.rdm.Directory.LinkCodes}.
         *  
         *
         * @param linkCode the link code
         */
        public void linkCode(long linkCode)
        {
            assert(checkHasCode());
            this.linkCode = linkCode;
        }

        /**
         * Returns text - Text further describing the state provided by the linkState and
         * linkCode members.
         * 
         * @return text
         */
        @Deprecated
        public Buffer linkText()	 	 	 
        {	 	 	 
        	return text;	 	 	 
        }

        /**
         * Returns text - Text further describing the state provided by the linkState and
         * linkCode members.
         * 
         * @return text
         */
        public Buffer text()
        {
            return text;
        }
        
        /**
         * Sets text field buffer for this service to the user specified
         * buffer. Data and position of text field buffer will be set to
         * passed in buffer's data and position. Note that this creates garbage
         * if buffer is backed by String object.
         *
         * @param text the text
         */
        public void text(Buffer text)
        {
            assert(text != null) : "text can not be null";
            text().data(text.data(), text.position(), text.length());
        }

        /**
         * Returns the service sink flags. It is populated by {@link ServiceLinkFlags}
         * 
         * @return flags
         */
        public int flags()
        {
            return flags;
        }

        /**
         * Sets service link flags. Populate with {@link ServiceLinkFlags}
         *
         * @param flags the flags
         */
        public void flags(int flags)
        {
            this.flags = flags;
        }

        /**
         * Returns filterId - Populated by {@link com.thomsonreuters.upa.rdm.Directory.ServiceFilterIds}.
         * 
         * @return filterId
         */
        public int filterId()
        {
            return Directory.ServiceFilterIds.LINK;
        }

        /**
         * Encode a single RDM Service Link filter entry.
         * 
         * @param encIter The Encode Iterator
         * 
         * @return UPA return value
         */
        public int encode(EncodeIterator encIter)
        {
            elementList.clear();
            elementList.applyHasStandardData();
            int ret = elementList.encodeInit(encIter, null, 0);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            if (checkHasCode())
            {
                element.clear();
                element.name(ElementNames.LINK_CODE);
                element.dataType(DataTypes.UINT);
                tmpUInt.value(linkCode());
                ret = element.encode(encIter, tmpUInt);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (checkHasText())
            {
                element.clear();
                element.name(ElementNames.TEXT);
                element.dataType(DataTypes.ASCII_STRING);
                ret = element.encode(encIter, text());
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (checkHasType())
            {
                element.clear();
                element.name(ElementNames.TYPE);
                element.dataType(DataTypes.UINT);
                tmpUInt.value(type());
                ret = element.encode(encIter, tmpUInt);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            element.clear();
            element.name(ElementNames.LINK_STATE);
            element.dataType(DataTypes.UINT);
            tmpUInt.value(linkState());
            ret = element.encode(encIter, tmpUInt);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            return elementList.encodeComplete(encIter, true);
        }

        /**
         * Indicates presence of the link text field.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @return true - if text field is present, false - if not.
         */
        public boolean checkHasText()
        {
            return (flags & ServiceLinkFlags.HAS_TEXT) != 0;
        }

        /**
         * Applies text presence flag.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         */
        public void applyHasText()
        {
            flags |= ServiceLinkFlags.HAS_TEXT;
        }

        /**
         * Indicates presence of the link code field.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @return true - if link code field is present, false - if not.
         */
        public boolean checkHasCode()
        {
            return (flags & ServiceLinkFlags.HAS_CODE) != 0;
        }

        /**
         * Applies link code flag.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         */
        public void applyHasCode()
        {
            flags |= ServiceLinkFlags.HAS_CODE;
        }

        /**
         * Indicates presence of the link type field.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @return true - if link code field is present, false - if not.
         */
        public boolean checkHasType()
        {
            return (flags & ServiceLinkFlags.HAS_TYPE) != 0;
        }

        /**
         * Applies link code flag.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         */
        public void applyHasType()
        {
            flags |= ServiceLinkFlags.HAS_TYPE;
        }

        /**
         * Decode a UPA service link filter into an RDM service link filter.
         * 
         * @param dIter The Decode Iterator
         * 
         * @return UPA return value
         */
        public int decode(DecodeIterator dIter)
        {
            elementList.clear();
            element.clear();

            int ret = elementList.decode(dIter, null);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            while ((ret = element.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                if (element.name().equals(ElementNames.TYPE))
                {
                    ret = tmpUInt.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    applyHasType();
                    type(tmpUInt.toLong());
                }
                else if (element.name().equals(ElementNames.LINK_STATE))
                {
                    ret = tmpUInt.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    linkState(tmpUInt.toLong());
                }
                else if (element.name().equals(ElementNames.LINK_CODE))
                {
                    ret = tmpUInt.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    applyHasCode();
                    linkCode(tmpUInt.toLong());
                }
                else if (element.name().equals(ElementNames.TEXT))
                {
                    Buffer encodedData = element.encodedData();
                    text().data(encodedData.data(), encodedData.position(), encodedData.length());
                    applyHasText();
                }
            }

            return CodecReturnCodes.SUCCESS;
        }

        /**
         * Returns the action associated with this link filter.
         * 
         * action - {@link MapEntryActions}
         *
         * @return the int
         */
        public int action()
        {
            return action;
        }

        /**
         * Sets action associated with this link filter.
         * 
         * @param action - Populated by {@link MapEntryActions}
         */
        public void action(int action)
        {
            this.action = action;
        }
    }
    
    /**
     * The RDM Service Link flags. A combination of bit values that indicate the
     * presence of optional {@link ServiceLink} filter content.
     * 
     * @see ServiceLink
     */
    public class ServiceLinkFlags
    {
        /** (0x00) No flags set. */
        public static final int NONE        = 0x00; 
        
        /** Indicates presence of the source type. */
        public static final int HAS_TYPE    = 0x01;
        
        /** (0x02) Indicates presence of the link code. */
        public static final int HAS_CODE    = 0x02;     
        
        /** (0x04) Indicates presence of link text. */
        public static final int HAS_TEXT    = 0x04;      

        /**
         * Instantiates a new service link flags.
         */
        private ServiceLinkFlags()
        {
            throw new AssertionError();
        }
    }
    
    /**
     * The RDM Service Link Info. Contains information provided by the Source
     * Directory Link filter.
     * 
     * @see ServiceImpl
     */
    public static class ServiceLinkInfo
    {
        private List<ServiceLink> linkList;
        private int action; //filter entry action

        /**
         * Instantiates a new service link info.
         */
        public ServiceLinkInfo()
        {
            linkList = new ArrayList<ServiceLink>();
        }

        /**
         * Returns link information elements -List of entries with information
         * about upstream sources.
         * 
         * @return Link filter list
         */
        public List<ServiceLink> linkList()
        {
            return linkList;
        }

        /**
         * Sets link information elements -List of entries with information
         * about upstream sources.
         *
         * @param linkList the link list
         */
        public void linkList(List<ServiceLink> linkList)
        {
            assert (linkList != null) : "linkList can not be null";

            linkList().clear();
           
            for (ServiceLink serviceLink : linkList)
            {
                linkList().add(serviceLink);
            }
        }

        /**
         * Returns the action associated with this link filter.
         * 
         * @param action -{@link MapEntryActions}
         */
        public void action(int action)
        {
            this.action = action;
        }

        /**
         * Sets action associated with this link filter.
         * 
         * @return action - Populated by {@link MapEntryActions}
         */
        public int action()
        {
            return action;
        }

        /**
         * Encode an RDM Service Link filter.
         * 
         * @param encIter The Encode Iterator
         * 
         * @return UPA return value
         */
        public int encode(EncodeIterator encIter)
        {
            linkMap.clear();
            linkMap.flags(MapFlags.NONE);
            linkMap.containerType(DataTypes.ELEMENT_LIST);
            linkMap.keyPrimitiveType(DataTypes.ASCII_STRING);

            int ret = linkMap.encodeInit(encIter, 0, 0);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            for (ServiceLink serviceLink : linkList())
            {
                linkMapEntry.clear();
                linkMapEntry.flags(MapEntryFlags.NONE);
                linkMapEntry.action(serviceLink.action());
                if (linkMapEntry.action() == MapEntryActions.DELETE)
                {
                    ret = linkMapEntry.encode(encIter, serviceLink.name());
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    ret = linkMapEntry.encodeInit(encIter, serviceLink.name(), 0);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;

                    ret = serviceLink.encode(encIter);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;

                    ret = linkMapEntry.encodeComplete(encIter, true);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;

                }
            }

            return linkMap.encodeComplete(encIter, true);
        }

        /**
         * Decode a UPA service data filter into an RDM service data filter.
         * 
         * @param dIter The Decode Iterator
         * 
         * @return UPA return value
         */
        public int decode(DecodeIterator dIter)
        {
            clear();
            linkMap.clear();
            linkMapEntry.clear();
            mapKey.clear();

            int ret = linkMap.decode(dIter);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            if (linkMap.containerType() != DataTypes.ELEMENT_LIST || (linkMap.keyPrimitiveType() != DataTypes.BUFFER && linkMap.keyPrimitiveType() != DataTypes.ASCII_STRING))
                return CodecReturnCodes.FAILURE;

            while ((ret = linkMapEntry.decode(dIter, mapKey)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
                    return ret;

                ServiceLink serviceLink = new ServiceLink();
                serviceLink.name().data(mapKey.data(), mapKey.position(), mapKey.length());
                serviceLink.action(linkMapEntry.action());
                if (serviceLink.action() != MapEntryActions.DELETE)
                {
                    ret = serviceLink.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        return ret;
                    }
                    linkList().add(serviceLink);
                }
            }

            return CodecReturnCodes.SUCCESS;
        }

        /**
         * Clears an RDMService link filter.
         */
        public void clear()
        {
            linkList.clear();
        }

        /* (non-Javadoc)
         * @see java.lang.Object#toString()
         */
        public String toString()
        {
            for (ServiceLink link : linkList)
            {
                stringBuf.append(tab);
                stringBuf.append(tab);
                stringBuf.append("LinkFilter: ");
                stringBuf.append(eol);
                stringBuf.append(link);
            }

            return stringBuf.toString();
        }

        private final static String eol = "\n";
        private final static String tab = "\t";

        private StringBuilder stringBuf = new StringBuilder();

        private Buffer mapKey = CodecFactory.createBuffer();
        private Map linkMap = CodecFactory.createMap();
        private MapEntry linkMapEntry = CodecFactory.createMapEntry();

        /**
         * Performs a deep copy of {@link ServiceLinkInfo} object.
         *
         * @param destServiceLinkInfo ServiceLinkInfo object to copy this object into. It cannot be null.
         * 
         * @return UPA return value indicating success or failure of copy operation.
         */
        public int copy(ServiceLinkInfo destServiceLinkInfo)
        {
            assert (destServiceLinkInfo != null) : "destServiceLinkInfo can not be null";
            destServiceLinkInfo.clear();
            destServiceLinkInfo.action(action());
            int ret = CodecReturnCodes.SUCCESS;
            for (ServiceLink serviceLink : linkList())
            {
                ServiceLink serviceLink2 = new ServiceLink();
                ret = serviceLink.copy(serviceLink2);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                destServiceLinkInfo.linkList().add(serviceLink2);
            }

            return ret;
        }
        
        
        /**
         * Performs an update of {@link ServiceLinkInfo} object.
         *
         * @param destServiceLinkInfo ServiceLinkInfo object to update with information from this object. It cannot be null.
         * 
         * @return UPA return value indicating success or failure of update operation.
         */
        public int update(ServiceLinkInfo destServiceLinkInfo)
        {
            assert (destServiceLinkInfo != null) : "destServiceLinkInfo can not be null";
            
            int ret = CodecReturnCodes.SUCCESS;

            destServiceLinkInfo.action(action());
        	for (int i = 0; i < linkList().size(); ++i)
        	{
        		switch (linkList().get(i).action())
        		{
        		case MapEntryActions.ADD:
        		case MapEntryActions.UPDATE:
        			boolean foundService = false;
        			for (int j = 0; j < destServiceLinkInfo.linkList().size(); ++j)
        			{
        				if (destServiceLinkInfo.linkList().get(j).name().equals(linkList().get(i).name()))
        				{
        					ret = linkList().get(i).copy(destServiceLinkInfo.linkList().get(j));
        					foundService = true;
        					break;
        				}
        			}
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    
        			if (!foundService)
        			{
        				ServiceLink serviceLink = new ServiceLink();
        				destServiceLinkInfo.linkList().add(serviceLink);
        				ret = linkList().get(i).copy(serviceLink);
        	            if (ret != CodecReturnCodes.SUCCESS)
        	                return ret;
        			}
        			break;
        		case MapEntryActions.DELETE:
        			for (int j = 0; j < destServiceLinkInfo.linkList().size(); ++j)
        			{
        				if (destServiceLinkInfo.linkList().get(j).name().equals(linkList().get(i).name()))
        				{
        					destServiceLinkInfo.linkList().remove(j);
        					break;
        				}
        			}
        			break;
        		default:
        			break;
        		}
        	}
            return ret;
        }
    }
    

    
    
    /**
     * The RDM Service Sequenced Multicast. Contains information about an upstream source
     * associated with the service.
     * 
     * @see ServiceSeqMcastFlags
     * 
     * @see ServiceImpl
     */
    public static class ServiceSeqMcast
    {
        private Buffer name;
        private long type;
        private long linkState;
        private long linkCode;
        private boolean foundSnapshotPort = false, foundSnapshotAddr = false,
                                      foundGapRecPort = false, foundGapRecAddr = false,
                                      foundRefDataPort = false, foundRefDataAddr = false;
        private Buffer text;
        private int flags;
        private int action; // map entry action

        private final static String eol = "\n";
        private final static String tab = "\t";

        private StringBuilder stringBuf = new StringBuilder();
        private Vector vector = CodecFactory.createVector();
        private VectorEntry vectorEntry = CodecFactory.createVectorEntry();
        private ElementList vectorElementList = CodecFactory.createElementList();
        private ElementList elementList = CodecFactory.createElementList();
        private ElementEntry element = CodecFactory.createElementEntry();
        private UInt tmpUInt = CodecFactory.createUInt();
        private Buffer tmpBuffer = CodecFactory.createBuffer();
        
        private boolean foundPort = false;
        private boolean foundMCGroup = false;
        private boolean foundDomain = false;
        
        private LocalElementSetDefDb elementSetDefDb = CodecFactory.createLocalElementSetDefDb();
        
        public ServiceSeqMcastInfo seqMcastInfo = new ServiceSeqMcastInfo();
        
        /**
         * Instantiates a new service seq mcast.
         */
        public ServiceSeqMcast()
        {
            text = CodecFactory.createBuffer();
            name = CodecFactory.createBuffer();
            clear();
        }
        
        /**
         * Seq mcast info.
         *
         * @return the service seq mcast info
         */
        public ServiceSeqMcastInfo seqMcastInfo()
        {
            return seqMcastInfo;
        }

        /* (non-Javadoc)
         * @see java.lang.Object#toString()
         */
        public String toString()
        {
            stringBuf.setLength(0);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("Name: ");
            stringBuf.append(name());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("Snapshort Address:Port: ");
            stringBuf.append(seqMcastInfo.snapshotServer.address().data().array());
            stringBuf.append(":");
            stringBuf.append(seqMcastInfo.snapshotServer.port());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("Gap Recieve Address:Port: ");
            stringBuf.append(seqMcastInfo.gapRecoveryServer.address().data().array());
            stringBuf.append(":");
            stringBuf.append(seqMcastInfo.gapRecoveryServer.port());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("Reference Data Address:Port: ");
            stringBuf.append(seqMcastInfo.refDataServer.address().data().array());
            stringBuf.append(":");
            stringBuf.append(seqMcastInfo.refDataServer.port());
            stringBuf.append(eol);

            return stringBuf.toString();
        }

        /**
         * Clears an RDMServiceLink.
         */
        public void clear()
        {
            flags = 0;
            name.clear();
            type = Directory.LinkTypes.INTERACTIVE;
            linkState = 0;
            linkCode = Directory.LinkCodes.NONE;
            text.clear();
            action = MapEntryActions.ADD;
        }

        /**
         * Performs a deep copy of {@link ServiceLink} object.
         *
         * @param destServiceLink ServiceLink object to copy this object into. It cannot be null.
         * 
         * @return UPA return value indicating success or failure of copy operation.
         */
        public int copy(ServiceLink destServiceLink)
        {
            assert (destServiceLink != null) : "destServiceLink can not be null";
            if (checkHasCode())
            {
                destServiceLink.applyHasCode();
                destServiceLink.linkCode(linkCode());
            }

            if (checkHasText())
            {
                destServiceLink.applyHasText();
                ByteBuffer byteBuffer = ByteBuffer.allocate(text().length());
                text().copy(byteBuffer);
                destServiceLink.text().data(byteBuffer);
            }

            if (checkHasType())
            {
                destServiceLink.applyHasType();
                destServiceLink.type(type());
            }

            destServiceLink.linkState(linkState());

            return CodecReturnCodes.SUCCESS;
        }

        /**
         * Returns name identifying this upstream source.
         * 
         * @return type
         */
        public Buffer name()
        {
            return name;
        }
        
        /**
         * Sets upstream source name to the user specified buffer. Data and
         * position of name field buffer will be set to passed in buffer's data
         * and position. Note that this creates garbage if buffer is backed by
         * String object.
         *
         * @param name the name
         */
        public void name(Buffer name)
        {
            assert(name != null) : "name can not be null";
            text().data(name.data(), name.position(), name.length());
        }

        /**
         * Returns type of this service link. Populated by {@link com.thomsonreuters.upa.rdm.Directory.LinkTypes}
         * 
         * @return type
         */
        public long type()
        {
            return type;
        }

        /**
         * Sets type of this service link. Populated by {@link com.thomsonreuters.upa.rdm.Directory.LinkTypes}
         *
         * @param type the type
         */
        public void type(long type)
        {
            assert(checkHasType());
            this.type = type;
        }

        /**
         * Returns linkState - Flag indicating whether the source is up or down. Populated by {@link com.thomsonreuters.upa.rdm.Directory.LinkStates}.
         * 
         * @return linkState
         */
        public long linkState()
        {
            return linkState;
        }

        /**
         * Sets linkState - Flag indicating whether the source is up or down. Populated by {@link com.thomsonreuters.upa.rdm.Directory.LinkStates}.
         *
         * @param linkState the link state
         */
        public void linkState(long linkState)
        {
            this.linkState = linkState;
        }

        /**
         * Returns linkCode - Code indicating additional information about the status of the source. Populated by {@link com.thomsonreuters.upa.rdm.Directory.LinkCodes}.
         *  
         * @return linkCode
         */
        public long linkCode()
        {
            return linkCode;
        }

        /**
         * Sets linkCode - Code indicating additional information about the status of the source. Populated by {@link com.thomsonreuters.upa.rdm.Directory.LinkCodes}.
         *  
         *
         * @param linkCode the link code
         */
        public void linkCode(long linkCode)
        {
            assert(checkHasCode());
            this.linkCode = linkCode;
        }

        /**
         * Returns text - Text further describing the state provided by the linkState and
         * linkCode members.
         * 
         * @return text
         */
        public Buffer text()
        {
            return text;
        }
        
        /**
         * Sets text field buffer for this service to the user specified
         * buffer. Data and position of text field buffer will be set to
         * passed in buffer's data and position. Note that this creates garbage
         * if buffer is backed by String object.
         *
         * @param text the text
         */
        public void text(Buffer text)
        {
            assert(text != null) : "text can not be null";
            text().data(text.data(), text.position(), text.length());
        }

        /**
         * Returns the service sink flags. It is populated by {@link ServiceLinkFlags}
         * 
         * @return flags
         */
        public int flags()
        {
            return flags;
        }

        /**
         * Sets service link flags. Populate with {@link ServiceLinkFlags}
         *
         * @param flags the flags
         */
        public void flags(int flags)
        {
            this.flags = flags;
        }

        /**
         * Returns filterId - Populated by {@link com.thomsonreuters.upa.rdm.Directory.ServiceFilterIds}.
         * 
         * @return filterId
         */
        public int filterId()
        {
            return Directory.ServiceFilterIds.LINK;
        }

        /**
         * Encode a single RDM Service Link filter entry.
         * 
         * @param encIter The Encode Iterator
         * 
         * @return UPA return value
         */
        public int encode(EncodeIterator encIter)
        {
            elementList.clear();
            elementList.applyHasStandardData();
            int ret = elementList.encodeInit(encIter, null, 0);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            if (checkHasCode())
            {
                element.clear();
                element.name(ElementNames.LINK_CODE);
                element.dataType(DataTypes.UINT);
                tmpUInt.value(linkCode());
                ret = element.encode(encIter, tmpUInt);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (checkHasText())
            {
                element.clear();
                element.name(ElementNames.TEXT);
                element.dataType(DataTypes.ASCII_STRING);
                ret = element.encode(encIter, text());
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (checkHasType())
            {
                element.clear();
                element.name(ElementNames.TYPE);
                element.dataType(DataTypes.UINT);
                tmpUInt.value(type());
                ret = element.encode(encIter, tmpUInt);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            element.clear();
            element.name(ElementNames.LINK_STATE);
            element.dataType(DataTypes.UINT);
            tmpUInt.value(linkState());
            ret = element.encode(encIter, tmpUInt);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            return elementList.encodeComplete(encIter, true);
        }

        /**
         * Indicates presence of the link text field.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @return true - if text field is present, false - if not.
         */
        public boolean checkHasText()
        {
            return (flags & ServiceLinkFlags.HAS_TEXT) != 0;
        }

        /**
         * Applies text presence flag.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         */
        public void applyHasText()
        {
            flags |= ServiceLinkFlags.HAS_TEXT;
        }

        /**
         * Indicates presence of the link code field.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @return true - if link code field is present, false - if not.
         */
        public boolean checkHasCode()
        {
            return (flags & ServiceLinkFlags.HAS_CODE) != 0;
        }

        /**
         * Applies link code flag.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         */
        public void applyHasCode()
        {
            flags |= ServiceLinkFlags.HAS_CODE;
        }

        /**
         * Indicates presence of the link type field.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @return true - if link code field is present, false - if not.
         */
        public boolean checkHasType()
        {
            return (flags & ServiceLinkFlags.HAS_TYPE) != 0;
        }

        /**
         * Applies link code flag.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         */
        public void applyHasType()
        {
            flags |= ServiceLinkFlags.HAS_TYPE;
        }

        /**
         * Decode a UPA service link filter into an RDM service link filter.
         * 
         * @param dIter The Decode Iterator
         * 
         * @return UPA return value
         */
        public int decode(DecodeIterator dIter)
        {
            elementList.clear();
            element.clear();

            int ret = elementList.decode(dIter, null);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            while ((ret = element.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                if (element.name().equals(ElementNames.SNAPSHOT_SERVER_HOST))
                {
                    ret = tmpBuffer.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    seqMcastInfo.snapshotServer.address(tmpBuffer);
                    foundSnapshotAddr = true;
                }
                else if (element.name().equals(ElementNames.SNAPSHOT_SERVER_PORT))
                {
                    ret = tmpUInt.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    seqMcastInfo.snapshotServer.port(tmpUInt.toLong());
                    foundSnapshotPort = true;
                }
                else if (element.name().equals(ElementNames.GAP_RECOVERY_SERVER_HOST))
                {
                    ret = tmpBuffer.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    seqMcastInfo.gapRecoveryServer.address(tmpBuffer);
                    foundGapRecAddr = true;
                }
                else if (element.name().equals(ElementNames.GAP_RECOVERY_SERVER_PORT))
                {
                    ret = tmpUInt.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    seqMcastInfo.gapRecoveryServer.port(tmpUInt.toLong());
                    foundGapRecPort = true;
                }
                else if (element.name().equals(ElementNames.REFERENCE_DATA_SERVER_HOST))
                {
                    ret = tmpBuffer.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    tmpBuffer.copy(seqMcastInfo.refDataServer.address);
                    foundRefDataAddr = true;
                }
                else if (element.name().equals(ElementNames.REFERENCE_DATA_SERVER_PORT))
                {
                    ret = tmpUInt.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    seqMcastInfo.refDataServer.port(tmpUInt.toLong());
                    foundRefDataPort = true;
                }
                else if (element.name().equals(ElementNames.STREAMING_MCAST_CHANNELS))
                {
                    vector.clear();
                    ret = vector.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    
                    if (vector.checkHasSetDefs())
                    {
                        elementSetDefDb.clear();
                        elementSetDefDb.decode(dIter);
                    }
                    
                    vectorEntry.clear();
                    if ((ret = vectorEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                    {
                        foundPort = false;
                        foundMCGroup = false;
                        foundDomain = false;
                        do
                        {         
                            vectorElementList.clear();
                            vectorElementList.decode(dIter, elementSetDefDb);

                            while ((ret = element.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                            {
                                if (element.name().equals(ElementNames.MULTICAST_GROUP))
                                {
                                    ret = tmpBuffer.decode(dIter);
                                    if (ret != CodecReturnCodes.SUCCESS
                                            && ret != CodecReturnCodes.BLANK_DATA)
                                    {
                                        return ret;
                                    }
                                    seqMcastInfo.StreamingMcastChanServerList.address(tmpBuffer, seqMcastInfo.streamingMCastChanServerCount());
                                    foundMCGroup = true;
                                }
                                else if (element.name().equals(ElementNames.PORT))
                                {
                                    ret = tmpUInt.decode(dIter);
                                    if (ret != CodecReturnCodes.SUCCESS
                                            && ret != CodecReturnCodes.BLANK_DATA)
                                    {
                                        return ret;
                                    }
                                    seqMcastInfo.StreamingMcastChanServerList.port(tmpUInt.toLong(), seqMcastInfo.streamingMCastChanServerCount());
                                    foundPort = true;
                                }
                                else if (element.name().equals(ElementNames.DOMAIN))
                                {
                                    ret = tmpUInt.decode(dIter);
                                    if (ret != CodecReturnCodes.SUCCESS
                                            && ret != CodecReturnCodes.BLANK_DATA)
                                    {
                                        return ret;
                                    }
                                    seqMcastInfo.StreamingMcastChanServerList.domain(tmpUInt.toLong(), seqMcastInfo.streamingMCastChanServerCount());
                                    foundDomain = true;
                                }
                            }
                            
                            seqMcastInfo.StreamingMCastChanServerCount++;
                            
                        } while ((ret = vectorEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER);

                        if (foundPort && foundMCGroup && foundDomain)
                            seqMcastInfo.flags |= ServiceSeqMcastInfo.RDM_SVC_SMF_HAS_SMC_SERV;
                    }
                }
                else if (element.name().equals(ElementNames.GAP_MCAST_CHANNELS))
                {
                    vector.clear();
                    ret = vector.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    
                    if (vector.checkHasSetDefs())
                    {
                        elementSetDefDb.clear();
                        elementSetDefDb.decode(dIter);
                    }
                    
                    vectorEntry.clear();
                    if ((ret = vectorEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                    {
                        foundPort = false;
                        foundMCGroup = false;
                        foundDomain = false;
                        do
                        {
                            vectorElementList.clear();
                            vectorElementList.decode(dIter, elementSetDefDb);
                            while ((ret = element.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                            {
                                if (element.name().equals(ElementNames.MULTICAST_GROUP))
                                {
                                    ret = tmpBuffer.decode(dIter);
                                    if (ret != CodecReturnCodes.SUCCESS
                                            && ret != CodecReturnCodes.BLANK_DATA)
                                    {
                                        return ret;
                                    }
                                    seqMcastInfo.GapMCastChanServerList.address(tmpBuffer, seqMcastInfo.gapMCastChanServerCount());
                                    foundMCGroup = true;
                                }
                                else if (element.name().equals(ElementNames.PORT))
                                {
                                    ret = tmpUInt.decode(dIter);
                                    if (ret != CodecReturnCodes.SUCCESS
                                            && ret != CodecReturnCodes.BLANK_DATA)
                                    {
                                        return ret;
                                    }
                                    seqMcastInfo.GapMCastChanServerList.port(tmpUInt.toLong(), seqMcastInfo.gapMCastChanServerCount());
                                    foundPort = true;
                                }
                                else if (element.name().equals(ElementNames.DOMAIN))
                                {
                                    ret = tmpUInt.decode(dIter);
                                    if (ret != CodecReturnCodes.SUCCESS
                                            && ret != CodecReturnCodes.BLANK_DATA)
                                    {
                                        return ret;
                                    }
                                    seqMcastInfo.GapMCastChanServerList.domain(tmpUInt.toLong(), seqMcastInfo.gapMCastChanServerCount());
                                    foundDomain = true;
                                }
                            }
                            
                            seqMcastInfo.GapMCastChanServerCount++;
                            
                        } while ((ret = vectorEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER);
                        
                        
                        if (foundPort && foundMCGroup && foundDomain)
                            seqMcastInfo.flags |= ServiceSeqMcastInfo.RDM_SVC_SMF_HAS_GMC_SERV;
                    }
                }
                
                if (foundSnapshotPort && foundSnapshotAddr)
                    seqMcastInfo.flags |= ServiceSeqMcastInfo.RDM_SVC_SMF_HAS_SNAPSHOT_SERV;

                if (foundGapRecPort && foundGapRecAddr)
                    seqMcastInfo.flags |= ServiceSeqMcastInfo.RDM_SVC_SMF_HAS_GAP_REC_SERV;

                if (foundRefDataPort && foundRefDataAddr)
                    seqMcastInfo.flags |= ServiceSeqMcastInfo.RDM_SVC_SMF_HAS_REF_DATA_SERV;
            }

            return CodecReturnCodes.SUCCESS;
        }

        /**
         * Returns the action associated with this link filter.
         * 
         * action - {@link MapEntryActions}
         *
         * @return the int
         */
        public int action()
        {
            return action;
        }

        /**
         * Sets action associated with this link filter.
         * 
         * @param action - Populated by {@link MapEntryActions}
         */
        public void action(int action)
        {
            this.action = action;
        }
    }
    
    /**
     * The RDM Service Sequenced Multicast flags. A combination of bit values that indicate the
     * presence of optional {@link ServiceSeqMcast} filter content.
     * 
     * @see ServiceSeqMcast
     */
    public class ServiceSeqMcastFlags
    {
        /** (0x00) No flags set. */
        public static final int NONE        = 0x00; 
        
        /** Indicates presence of the source type. */
        public static final int HAS_TYPE    = 0x01;
        
        /** (0x02) Indicates presence of the link code. */
        public static final int HAS_CODE    = 0x02;     
        
        /** (0x04) Indicates presence of link text. */
        public static final int HAS_TEXT    = 0x04;      

        /**
         * Instantiates a new service seq mcast flags.
         */
        private ServiceSeqMcastFlags()
        {
            throw new AssertionError();
        }
    }
    
    /**
     * The RDM Service Sequenced Multicast  Info. Contains information provided by the Source
     * Directory Link filter.
     * 
     * @see ServiceImpl
     */
    public static class ServiceSeqMcastInfo
    {
        private final static int RDM_SVC_SMF_HAS_SNAPSHOT_SERV       =  0x001;   /*!< (0x001) Indicates presence of Snapshot Server Info */
        private final static int RDM_SVC_SMF_HAS_GAP_REC_SERV        =  0x002;   /*!< (0x002) Indicates presence of Gap Recovery Server Info */
        private final static int RDM_SVC_SMF_HAS_REF_DATA_SERV       =  0x004;   /*!< (0x004) Indicates presence of Reference Data Server Info */
        private final static int RDM_SVC_SMF_HAS_SMC_SERV        =  0x008;   /*!< (0x008) Indicates presence of Streaming Multicast Channels Server Info */
        private final static int RDM_SVC_SMF_HAS_GMC_SERV        =  0x010;    /*!< (0x010) Indicates presence of Gap Multicast Channel Server Info */
        
        private List<ServiceLink> linkList;
        private int action; //filter entry action

        /**
         * Instantiates a new service seq mcast info.
         */
        public ServiceSeqMcastInfo()
        {
            linkList = new ArrayList<ServiceLink>();
        }

        /**
         * Returns link information elements -List of entries with information
         * about upstream sources.
         * 
         * @return Link filter list
         */
        public List<ServiceLink> linkList()
        {
            return linkList;
        }

        /**
         * Sets link information elements -List of entries with information
         * about upstream sources.
         *
         * @param linkList the link list
         */
        public void linkList(List<ServiceLink> linkList)
        {
            assert (linkList != null) : "linkList can not be null";

            linkList().clear();
           
            for (ServiceLink serviceLink : linkList)
            {
                linkList().add(serviceLink);
            }
        }

        /**
         * Returns the action associated with this link filter.
         * 
         * @param action -{@link MapEntryActions}
         */
        public void action(int action)
        {
            this.action = action;
        }

        /**
         * Sets action associated with this link filter.
         * 
         * @return action - Populated by {@link MapEntryActions}
         */
        public int action()
        {
            return action;
        }

        /**
         * Encode an RDM Service Link filter.
         * 
         * @param encIter The Encode Iterator
         * 
         * @return UPA return value
         */
        public int encode(EncodeIterator encIter)
        {
            linkMap.clear();
            linkMap.flags(MapFlags.NONE);
            linkMap.containerType(DataTypes.ELEMENT_LIST);
            linkMap.keyPrimitiveType(DataTypes.ASCII_STRING);

            int ret = linkMap.encodeInit(encIter, 0, 0);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            for (ServiceLink serviceLink : linkList())
            {
                linkMapEntry.clear();
                linkMapEntry.flags(MapEntryFlags.NONE);
                linkMapEntry.action(serviceLink.action());
                if (linkMapEntry.action() == MapEntryActions.DELETE)
                {
                    ret = linkMapEntry.encode(encIter, serviceLink.name());
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    ret = linkMapEntry.encodeInit(encIter, serviceLink.name(), 0);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;

                    ret = serviceLink.encode(encIter);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;

                    ret = linkMapEntry.encodeComplete(encIter, true);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;

                }
            }

            return linkMap.encodeComplete(encIter, true);
        }

        /**
         * Decode a UPA service data filter into an RDM service data filter.
         * 
         * @param dIter The Decode Iterator
         * 
         * @return UPA return value
         */
        public int decode(DecodeIterator dIter)
        {
            int ret;
            ServiceSeqMcast seqMcast = new ServiceSeqMcast();
            seqMcast.name().data(mapKey.data(), mapKey.position(), mapKey.length());
            seqMcast.action(linkMapEntry.action());
            if (seqMcast.action() != MapEntryActions.DELETE)
            {
                ret = seqMcast.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }

            flags = seqMcast.seqMcastInfo().flags;
            actions = seqMcast.seqMcastInfo().actions;
            snapshotServer = seqMcast.seqMcastInfo().snapshotServer;
            gapRecoveryServer = seqMcast.seqMcastInfo().gapRecoveryServer;
            refDataServer = seqMcast.seqMcastInfo().refDataServer;
            StreamingMCastChanServerCount = seqMcast.seqMcastInfo().StreamingMCastChanServerCount;
            StreamingMcastChanServerList = seqMcast.seqMcastInfo().StreamingMcastChanServerList;
            GapMCastChanServerCount = seqMcast.seqMcastInfo().GapMCastChanServerCount;
            GapMCastChanServerList = seqMcast.seqMcastInfo().GapMCastChanServerList;
            
            return CodecReturnCodes.SUCCESS;
        }

        /**
         * Clears an RDMService link filter.
         */
        public void clear()
        {
            linkList.clear();
        }

        /* (non-Javadoc)
         * @see java.lang.Object#toString()
         */
        public String toString()
        {
            for (ServiceLink link : linkList)
            {
                stringBuf.append(tab);
                stringBuf.append(tab);
                stringBuf.append("LinkFilter: ");
                stringBuf.append(eol);
                stringBuf.append(link);
            }

            return stringBuf.toString();
        }

        private final static String eol = "\n";
        private final static String tab = "\t";

        private StringBuilder stringBuf = new StringBuilder();

        private Buffer mapKey = CodecFactory.createBuffer();
        private Map linkMap = CodecFactory.createMap();
        private MapEntry linkMapEntry = CodecFactory.createMapEntry();
               
        private int flags;                                                                                                 /*!< The RDM Sequenced Multicast Info flags. */
        private FilterEntryActions actions;                                                                  /*!< Action associated with this Service Info. */
        private RDMAddressPortInfo snapshotServer = new RDMAddressPortInfo();              /*!< Snapshot Server Connection Infomation. */
        private RDMAddressPortInfo gapRecoveryServer = new RDMAddressPortInfo();                                  /*!< Gap Recovery Server Connection Infomation. */
        private RDMAddressPortInfo refDataServer = new RDMAddressPortInfo();                         /*!< Reference Data Server Connection Infomation. */
        private int StreamingMCastChanServerCount;                                          /*!< Streaming Multicast Channel Server count. */
        private RDMMCAddressPortInfo StreamingMcastChanServerList = new RDMMCAddressPortInfo();       /*!< Streaming Multicast Channel Server list. */
        private int GapMCastChanServerCount;                                                     /*!< Gap Multicast Channel Server count. */
        private RDMMCAddressPortInfo GapMCastChanServerList = new RDMMCAddressPortInfo();                  /*!< Gap Multicast Channel Server list. */

        /**
         * Performs a deep copy of {@link ServiceLinkInfo} object.
         *
         * @param destServiceLinkInfo ServiceLinkInfo object to copy this object into. It cannot be null.
         * 
         * @return UPA return value indicating success or failure of copy operation.
         */
        public int copy(ServiceLinkInfo destServiceLinkInfo)
        {
            assert (destServiceLinkInfo != null) : "destServiceLinkInfo can not be null";
            destServiceLinkInfo.clear();
            destServiceLinkInfo.action(action());
            int ret = CodecReturnCodes.SUCCESS;
            for (ServiceLink serviceLink : linkList())
            {
                ServiceLink serviceLink2 = new ServiceLink();
                ret = serviceLink.copy(serviceLink2);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                destServiceLinkInfo.linkList().add(serviceLink2);
            }

            return ret;
        }
        
        /**
         * Snapshot server address.
         *
         * @return the buffer
         */
        public Buffer snapshotServerAddress()
        {
            return snapshotServer.address();
        }
        
        /**
         * Snapshot server port.
         *
         * @return the long
         */
        public Long snapshotServerPort()
        {
            return snapshotServer.port();
        }
        
        /**
         * Ref data server address.
         *
         * @return the buffer
         */
        public Buffer refDataServerAddress()
        {
            return refDataServer.address();
        }
        
        /**
         * Ref data server port.
         *
         * @return the long
         */
        public Long refDataServerPort()
        {
            return refDataServer.port();
        }
        
        /**
         * Gap recovery server address.
         *
         * @return the buffer
         */
        public Buffer gapRecoveryServerAddress()
        {
            return gapRecoveryServer.address();
        }
        
        /**
         * Gap recovery server port.
         *
         * @return the long
         */
        public Long gapRecoveryServerPort()
        {
            return gapRecoveryServer.port();
        }
        
        /**
         * Gap M cast chan server list.
         *
         * @return the list
         */
        public List<Buffer> gapMCastChanServerList()
        {
            return GapMCastChanServerList.addressList();
        }
        
        /**
         * Gap M cast chan server count.
         *
         * @return the int
         */
        public int gapMCastChanServerCount()
        {
            return GapMCastChanServerCount;
        }
        
        /**
         * Gap M cast chan port list.
         *
         * @return the list
         */
        public List<Long> gapMCastChanPortList()
        {
            return GapMCastChanServerList.port();
        }
        
        /**
         * Gap M cast chan domain list.
         *
         * @return the list
         */
        public List<Long> gapMCastChanDomainList()
        {
            return GapMCastChanServerList.domain();
        }
        
        /**
         * Streaming M cast chan server list.
         *
         * @return the list
         */
        public List<Buffer> streamingMCastChanServerList()
        {
            return StreamingMcastChanServerList.addressList();
        }
        
        /**
         * Streaming M cast chan port list.
         *
         * @return the list
         */
        public List<Long> streamingMCastChanPortList()
        {
            return StreamingMcastChanServerList.port();
        }
        
        /**
         * Streaming M cast chan server count.
         *
         * @return the int
         */
        public int streamingMCastChanServerCount()
        {
            return StreamingMCastChanServerCount;
        }
        
        /**
         * Streaming M cast chan domain list.
         *
         * @return the list
         */
        public List<Long> streamingMCastChanDomainList()
        {
            return StreamingMcastChanServerList.domain();
        }
    }
    
    
    /**
     * 
     * Contains RDM Address and port information
     * and getter/setter/clear methods for all of it
     */
    public class RDMAddressPortInfo
    {
        private Buffer address;
        private Long port;

        /**
         * Instantiates a new RDM address port info.
         */
        RDMAddressPortInfo()
        {
            address = CodecFactory.createBuffer();
            port = Long.valueOf(0);
        }
        
        /**
         * Address.
         *
         * @return the buffer
         */
        Buffer address() 
        {
            return address;
        }
        
        /**
         * Address.
         *
         * @param setAddress the set address
         */
        void address(Buffer setAddress)
        {
            address.data(ByteBuffer.allocate(setAddress.length()));
            setAddress.copy(address);
        }
        
        /**
         * Port.
         *
         * @return the long
         */
        Long port() 
        {
            return port;
        }
        
        /**
         * Port.
         *
         * @param setPort the set port
         */
        void port(Long setPort)
        {
            port = setPort;
        }
        
        /**
         * Clear.
         */
        void clear()
        {
            address = CodecFactory.createBuffer();
            port = Long.valueOf(0);
        }
    }
    
    /**
     * 
     * Contains RDM Address, port, and domain information
     * and getter/setter/clear methods for all of it
     *
     */
    public class RDMMCAddressPortInfo 
    {
        private List<Buffer> address;
        private List<Long> port;
        private List<Long> domain;
        private int addressCount;
        
        /**
         * Instantiates a new RDMMC address port info.
         */
        RDMMCAddressPortInfo()
        {
            address = new ArrayList<Buffer>();
            port = new ArrayList<Long>();
            domain = new ArrayList<Long>();
            addressCount = 0;
        }
        
        /**
         * Address list.
         *
         * @return the list
         */
        List<Buffer> addressList() {
            return address;
        }

        /**
         * Address.
         *
         * @param location the location
         * @return the buffer
         */
        Buffer address(int location) 
        {
            return address.get(location);
        }
        
        /**
         * Address.
         *
         * @param setAddress the set address
         * @param location the location
         */
        void address(Buffer setAddress, int location)
        {
            address.add(location, CodecFactory.createBuffer());
            address.get(location).data(ByteBuffer.allocate(setAddress.length()));
            setAddress.copy(address.get(location));
        }
        
        /**
         * Address count.
         *
         * @return the int
         */
        int addressCount() 
        {
            return addressCount;
        }
        
        /**
         * Address count.
         *
         * @param setAddressCount the set address count
         */
        void addressCount(int setAddressCount)
        {
            addressCount = setAddressCount;
        }
        
        /**
         * Increment address count.
         */
        void incrementAddressCount()
        {
            addressCount += 1;
        }
        
        /**
         * Port.
         *
         * @return the list
         */
        List<Long> port()
        {
            return port;
        }
        
        /**
         * Port.
         *
         * @param location the location
         * @return the long
         */
        Long port(int location) 
        {
            return port.get(location);
        }
        
        /**
         * Port.
         *
         * @param setPort the set port
         * @param location the location
         */
        void port(Long setPort, int location)
        {
            port.add(location, setPort);
        }
        
        /**
         * Domain.
         *
         * @return the list
         */
        List<Long> domain()
        {
            return domain;
        }
        
        /**
         * Domain.
         *
         * @param location the location
         * @return the long
         */
        Long domain(int location) 
        {
            return domain.get(location);
        }
        
        /**
         * Domain.
         *
         * @param setDomains the set domains
         * @param location the location
         */
        void domain(Long setDomains, int location)
        {
            domain.add(location, setDomains);
        }
        
        /**
         * Clear.
         */
        void clear()
        {
            address.clear();
            port.clear();
            domain.clear();
            addressCount = 0;
        }
    }
    
    
    /**
     * The RDM Service Load. Contains information provided by the Source Directory Load filter.
     * @see ServiceLoadFlags
     * @see ServiceImpl
     */
    public static class ServiceLoad
    {
        private long openLimit;
        private long openWindow;
        private long loadFactor;
        private int action;
        private int flags;

        private ElementList elementList = CodecFactory.createElementList();
        private ElementEntry element = CodecFactory.createElementEntry();
        private UInt tmpUInt = CodecFactory.createUInt();
        
        private StringBuilder stringBuf = new StringBuilder();
        private final static String eol = "\n";
        private final static String tab = "\t";

        /**
         * Instantiates a new service load.
         */
        public ServiceLoad()
        {
            clear();
        }

        

        /**
         * Clears an RDM Service Load.
         * 
         * @see ServiceLoad
         */
        public void clear()
        {
            flags = 0;
            action = FilterEntryActions.SET;
            openLimit = 4294967295L;
            openWindow = 4294967295L;
            loadFactor = 65535;
        }

        /* (non-Javadoc)
         * @see java.lang.Object#toString()
         */
        public String toString()
        {
            stringBuf.setLength(0);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("LoadFilter:");
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("OpenLimit: ");
            stringBuf.append(openLimit());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("OpenWindow: ");
            stringBuf.append(openWindow());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("LoadFactor: ");
            stringBuf.append(loadFactor());
            stringBuf.append(eol);

            return stringBuf.toString();
        }

 
        /**
         * Returns the service load flags. It is populated by
         * {@link ServiceLoadFlags}
         * 
         * @return flags
         */
        public int flags()
        {
            return flags;
        }

        /**
         * Sets service load flags. Populate with {@link ServiceLoadFlags}
         *
         * @param flags the flags
         */
        public void flags(int flags)
        {
            this.flags = flags;
        }

        /**
         * Returns action associated with this Load filter.
         * 
         * @return action
         */
        public int action()
        {
            return action;
        }

        /**
         * Sets action associated with this Load filter.
         *
         * @param action the action
         */
        public void action(int action)
        {
            this.action = action;
        }

        /**
         * Returns openLimit - The maximum number of items the Consumer is allowed to open
         * from this service.
         * 
         * @return openLimit
         */
        public long openLimit()
        {
            return openLimit;
        }

        /**
         * Sets openLimit - The maximum number of items the Consumer is allowed to open
         * from this service.
         *
         * @param openLimit the open limit
         */
        public void openLimit(long openLimit)
        {
            assert(checkHasOpenLimit());
            this.openLimit = openLimit;
        }

        /**
         * Indicates presence of the openLimit field.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @return true - if openLimit field is present, false - if not.
         */
        public boolean checkHasOpenLimit()
        {
            return (flags & ServiceLoadFlags.HAS_OPEN_LIMIT) != 0;
        }

        /**
         * Applies openLimit presence flag.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         * 
         */
        public void applyHasOpenLimit()
        {
            flags |= ServiceLoadFlags.HAS_OPEN_LIMIT;
        }

        /**
         * Returns openWindow - The maximum number of items the Consumer may have
         * outstanding(i.e. waiting for a RefreshMsg) from this service.
         * 
         * @return openWindow
         */
        public long openWindow()
        {
            return openWindow;
        }

        /**
         * Sets openWindow - The maximum number of items the Consumer may have
         * outstanding(i.e. waiting for a RefreshMsg) from this service.
         *
         * @param openWindow the open window
         */
        public void openWindow(long openWindow)
        {
            assert(checkHasOpenWindow());
            this.openWindow = openWindow;
        }

        /**
         * Indicates presence of the openWindow field.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @return true - if openWindow field is present, false - if not.
         */
        public boolean checkHasOpenWindow()
        {
            return (flags & ServiceLoadFlags.HAS_OPEN_WINDOW) != 0;
        }

        /**
         * Applies openWindow presence flag.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         * 
         */
        public void applyHasOpenWindow()
        {
            flags |= ServiceLoadFlags.HAS_OPEN_WINDOW;
        }

        /**
         * Returns the load factor - a number indicating the current workload of
         * the source providing the data.
         * 
         * @return loadFactor
         */
        public long loadFactor()
        {
            return loadFactor;
        }

        /**
         * Sets load factor - a number indicating the current workload of
         * the source providing the data.
         *
         * @param loadFactor the load factor
         */
        public void loadFactor(long loadFactor)
        {
            assert(checkHasLoadFactor());
            this.loadFactor = loadFactor;
        }

        /**
         * Indicates presence of the loadFactor field.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @return true - if loadFactor field is present, false - if not.
         */
        public boolean checkHasLoadFactor()
        {
            return (flags & ServiceLoadFlags.HAS_LOAD_FACTOR) != 0;
        }

        /**
         * Applies loadFactor presence flag.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         */
        public void applyHasLoadFactor()
        {
            flags |= ServiceLoadFlags.HAS_LOAD_FACTOR;
        }

        /**
         * Performs a deep copy of {@link ServiceLoad} object.
         *
         * @param destServiceLoad ServiceLoad object to copy this object into. It cannot be null.
         * 
         * @return UPA return value indicating success or failure of copy operation.
         */
        public int copy(ServiceLoad destServiceLoad)
        {
            assert (destServiceLoad != null) : "destServiceLoad can not be null";
            destServiceLoad.clear();
            if (checkHasLoadFactor())
            {
                destServiceLoad.applyHasLoadFactor();
                destServiceLoad.loadFactor(loadFactor());
            }

            if (checkHasOpenLimit())
            {
                destServiceLoad.applyHasOpenLimit();
                destServiceLoad.openLimit(openLimit());
            }

            if (checkHasOpenWindow())
            {
                destServiceLoad.applyHasOpenWindow();
                destServiceLoad.openWindow(openWindow());
            }

            destServiceLoad.action(action());

            return CodecReturnCodes.SUCCESS;
        }
        
        /**
         * Performs an update of {@link ServiceLoad} object.
         *
         * @param destServiceLoad ServiceLoad object to update with information from this object. It cannot be null.
         * 
         * @return UPA return value indicating success or failure of update operation.
         */
        public int update(ServiceLoad destServiceLoad)
        {
            assert (destServiceLoad != null) : "destServiceLoad can not be null";

            if (checkHasLoadFactor())
            {
                destServiceLoad.applyHasLoadFactor();
                destServiceLoad.loadFactor(loadFactor());
            }

            if (checkHasOpenLimit())
            {
                destServiceLoad.applyHasOpenLimit();
                destServiceLoad.openLimit(openLimit());
            }

            if (checkHasOpenWindow())
            {
                destServiceLoad.applyHasOpenWindow();
                destServiceLoad.openWindow(openWindow());
            }

            destServiceLoad.action(action());

            return CodecReturnCodes.SUCCESS;
        }

        /**
         * Returns filterId - Populated by {@link com.thomsonreuters.upa.rdm.Directory.ServiceFilterIds}.
         * 
         * @return filterId
         */
        public int filterId()
        {
            return Directory.ServiceFilterIds.LOAD;
        }

        /**
         * Encode an RDM Service Load filter.
         * 
         * @param encIter The Encode Iterator
         * 
         * @return UPA return value
         */
        public int encode(EncodeIterator encIter)
        {
            elementList.clear();
            elementList.applyHasStandardData();
            int ret = elementList.encodeInit(encIter, null, 0);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            if (checkHasOpenLimit())
            {
                element.clear();
                element.name(ElementNames.OPEN_LIMIT);
                element.dataType(DataTypes.UINT);
                tmpUInt.value(openLimit());
                ret = element.encode(encIter, tmpUInt);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (checkHasOpenWindow())
            {
                element.clear();
                element.name(ElementNames.OPEN_WINDOW);
                element.dataType(DataTypes.UINT);
                tmpUInt.value(openWindow());
                ret = element.encode(encIter, tmpUInt);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (checkHasLoadFactor())
            {
                element.clear();
                element.name(ElementNames.LOAD_FACT);
                element.dataType(DataTypes.UINT);
                tmpUInt.value(loadFactor());
                ret = element.encode(encIter, tmpUInt);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            ret = elementList.encodeComplete(encIter, true);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;
            return CodecReturnCodes.SUCCESS;
        }

        /**
         * Decode a UPA service load filter into an RDM service load filter.
         * 
         * @param dIter The Decode Iterator
         * 
         * @return UPA return value
         */
        public int decode(DecodeIterator dIter)
        {
            elementList.clear();
            element.clear();

            // decode element list
            int ret = elementList.decode(dIter, null);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;

            // decode element list elements
            while ((ret = element.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                //OpenLimit
                if (element.name().equals(ElementNames.OPEN_LIMIT))
                {
                    ret = tmpUInt.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    applyHasOpenLimit();
                    openLimit(tmpUInt.toLong());
                }
                //OpenWindow
                else if (element.name().equals(ElementNames.OPEN_WINDOW))
                {
                    ret = tmpUInt.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    applyHasOpenWindow();
                    openWindow(tmpUInt.toLong());
                }
                //LoadFactor
                else if (element.name().equals(ElementNames.LOAD_FACT))
                {
                    ret = tmpUInt.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    applyHasLoadFactor();
                    loadFactor(tmpUInt.toLong());
                }
            }

            return CodecReturnCodes.SUCCESS;
        }
     }
    
    /**
     * The RDM Service Load flags. A combination of bit values that indicate the presence of optional {@link ServiceLoad} filter content.
     * 
     * @see ServiceLoad
     */
    public class ServiceLoadFlags
    {
        /** (0x00) No flags set. */
        public static final int NONE          =  0x00;       
        
        /** (0x01) Indicates presence of an open limit on the service. */
        public static final int HAS_OPEN_LIMIT   =  0x01;    
        
        /** (0x02) Indicates presence of an open window on the service. */
        public static final int HAS_OPEN_WINDOW  =  0x02;    
        
        /** (0x04) Indicates presence of a load factor. */
        public static final int HAS_LOAD_FACTOR  =  0x04;    
        
        /**
         * Instantiates a new service load flags.
         */
        private ServiceLoadFlags()
        {
            throw new AssertionError();
        }
    }
    
    /**
     * The RDM Service State. Contains information provided by the Source Directory
     * State filter.
     * 
     * @see ServiceStateFlags
     * 
     * @see ServiceImpl
     */
    public static class ServiceState
    {
        private long serviceState;
        private long acceptingRequests;
        private com.thomsonreuters.upa.codec.State status;
        private int action;
        private int flags;


        private ElementList elementList = CodecFactory.createElementList();
        private ElementEntry element = CodecFactory.createElementEntry();

        private UInt tmpUInt = CodecFactory.createUInt();
        private com.thomsonreuters.upa.codec.State tmpStatus = CodecFactory.createState();
        
        /**
         * Instantiates a new service state.
         */
        public ServiceState()
        {
            status = CodecFactory.createState();
            clear();
        }

        /**
         * Clears a ServiceState.
         */
        public void clear()
        {
            flags = 0;
            status.clear();
            status.streamState(StreamStates.OPEN);
            status.code(StateCodes.NONE);
            status.dataState(DataStates.OK);
            action = FilterEntryActions.SET;
            serviceState = Directory.ServiceStates.UP;
            acceptingRequests = 1;
        }

        /**
         * Returns service state flags. It is populated by
         * {@link ServiceStateFlags}.
         * 
         * @return flags.
         */
        public int flags()
        {
            return flags;
        }

        /**
         * Sets service state flags. Populate with {@link ServiceStateFlags}.
         *
         * @param flags the flags
         */
        public void flags(int flags)
        {
            this.flags = flags;
        }

        /**
         * Performs a deep copy of {@link ServiceState} object.
         *
         * @param destServiceState ServiceState object to copy this object into. It cannot be null.
         * 
         * @return UPA return value indicating success or failure of copy operation.
         */
        public int copy(ServiceState destServiceState)
        {
            assert (destServiceState != null) : "destServiceState can not be null";
            destServiceState.clear();
            destServiceState.flags(flags());
            destServiceState.action(action());
            destServiceState.serviceState(serviceState());
            
            if (checkHasAcceptingRequests())
            {
                destServiceState.applyHasAcceptingRequests();
                destServiceState.acceptingRequests(acceptingRequests());
            }

            if (checkHasStatus())
            {
                destServiceState.applyHasStatus();
                destServiceState.status().streamState(status().streamState());
                destServiceState.status().dataState(status().dataState());
                destServiceState.status().code(status().code());
                if (status().text().length() > 0)
                {
                    ByteBuffer byteBuffer = ByteBuffer.allocate(status().text().length());
                    status().text().copy(byteBuffer);
                    destServiceState.status().text().data(byteBuffer);
                }
            }

            return CodecReturnCodes.SUCCESS;
        }
        
        /**
         * Performs an update of {@link ServiceState} object.
         *
         * @param destServiceState ServiceState object to update with information from this object. It cannot be null.
         * 
         * @return UPA return value indicating success or failure of update operation.
         */
        public int update(ServiceState destServiceState)
        {
            assert (destServiceState != null) : "destServiceState can not be null";
            destServiceState.flags(flags());
            destServiceState.action(action());
            destServiceState.serviceState(serviceState());
            
            if (checkHasAcceptingRequests())
            {
                destServiceState.applyHasAcceptingRequests();
                destServiceState.acceptingRequests(acceptingRequests());
            }

            if (checkHasStatus())
            {
                destServiceState.applyHasStatus();
                destServiceState.status().streamState(status().streamState());
                destServiceState.status().dataState(status().dataState());
                destServiceState.status().code(status().code());
                if (status().text().length() > 0)
                {
                    ByteBuffer byteBuffer = ByteBuffer.allocate(status().text().length());
                    status().text().copy(byteBuffer);
                    destServiceState.status().text().data(byteBuffer);
                }
            }

            return CodecReturnCodes.SUCCESS;
        }

        /* (non-Javadoc)
         * @see java.lang.Object#toString()
         */
        public String toString()
        {
            stringBuf.setLength(0);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("StateFilter:");
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("ServiceState: ");
            stringBuf.append(serviceState());
            stringBuf.append(eol);

            if (checkHasAcceptingRequests())
            {
                stringBuf.append(tab);
                stringBuf.append(tab);
                stringBuf.append(tab);
                stringBuf.append("AcceptingRequests: ");
                stringBuf.append(acceptingRequests());
                stringBuf.append(eol);
            }

            if (checkHasStatus())
            {
                stringBuf.append(tab);
                stringBuf.append(tab);
                stringBuf.append(tab);
                stringBuf.append(status());
                stringBuf.append(eol);
            }

            return stringBuf.toString();
        }

        private StringBuilder stringBuf = new StringBuilder();
        private final static String eol = "\n";
        private final static String tab = "\t";

        /**
         * Returns action - Action associated with this Service Info.
         * 
         * @return action
         */
        public int action()
        {
            return action;
        }

        /**
         * Sets action - Action associated with this Service Info.
         *
         * @param action the action
         */
        public void action(int action)
        {
            this.action = action;
        }

        /**
         * Returns current state of the service.
         * 
         * @return service state.
         */
        public long serviceState()
        {
            return serviceState;
        }

        /**
         * Sets current state of the service.
         *
         * @param serviceState the service state
         */
        public void serviceState(long serviceState)
        {
            this.serviceState = serviceState;
        }

        /**
         * Returns acceptingRequests - Flag indicating whether the service is accepting item
         * requests.
         * 
         * @return acceptingRequests
         */
        public long acceptingRequests()
        {
            return acceptingRequests;
        }

        /**
         * Sets acceptingRequests - Flag indicating whether the service is
         * accepting item requests.
         *
         * @param acceptingRequests the accepting requests
         */
        public void acceptingRequests(long acceptingRequests)
        {
            assert(checkHasAcceptingRequests());
            this.acceptingRequests = acceptingRequests;
        }

        /**
         * Indicates presence of the acceptingRequests field.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @return true - if acceptingRequests present, false - if not.
         */
        public boolean checkHasAcceptingRequests()
        {
            return (flags & Service.ServiceStateFlags.HAS_ACCEPTING_REQS) != 0;
        }

        /**
         * Applies acceptingRequests presence flag.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         */
        public void applyHasAcceptingRequests()
        {
            flags |= Service.ServiceStateFlags.HAS_ACCEPTING_REQS;
        }

        /**
         * Returns status to be applied to all items being provided by this
         * service.
         * 
         * @return status
         */
        public State status()
        {
            return status;
        }
        
        /**
         * Sets status to be applied to all items being provided by this
         * service.
         *
         * @param status the status
         */
        public void status(State status)
        {
            status().streamState(status.streamState());
            status().dataState(status.dataState());
            status().code(status.code());
            status().text(status.text());
        }

        /**
         * Indicates presence of the status field.
         * 
         * Flags may also be bulk-get via {@link #flags()}.
         * 
         * @return true - if status present, false - if not.
         */
        public boolean checkHasStatus()
        {
            return (flags & Service.ServiceStateFlags.HAS_STATUS) != 0;
        }

        /**
         * Applies status presence flag.
         * 
         * Flags may also be bulk-set via {@link #flags(int)}.
         * 
         */
        public void applyHasStatus()
        {
            flags |= Service.ServiceStateFlags.HAS_STATUS;
        }

        /**
         * Returns filterId - Populated by {@link com.thomsonreuters.upa.rdm.Directory.ServiceFilterIds}.
         * 
         * @return filterId
         */
        public int filterId()
        {
            return Directory.ServiceFilterIds.STATE;
        }

        /**
         * Encode an RDM Service State filter.
         * 
         * @param encIter The Encode Iterator
         * 
         * @return UPA return value
         */
        public int encode(EncodeIterator encIter)
        {
            elementList.clear();
            elementList.applyHasStandardData();
            int ret = elementList.encodeInit(encIter, null, 0);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            element.clear();
            element.name(ElementNames.SVC_STATE);
            element.dataType(DataTypes.UINT);
            tmpUInt.value(serviceState());
            ret = element.encode(encIter, tmpUInt);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            if (checkHasAcceptingRequests())
            {
                element.clear();
                element.name(ElementNames.ACCEPTING_REQS);
                element.dataType(DataTypes.UINT);
                tmpUInt.value(acceptingRequests());

                ret = element.encode(encIter, tmpUInt);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (checkHasStatus())
            {
                element.clear();
                element.name(ElementNames.STATUS);
                element.dataType(DataTypes.STATE);
                ret = element.encode(encIter, status());
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            ret = elementList.encodeComplete(encIter, true);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;
            return CodecReturnCodes.SUCCESS;
        }

        /**
         * Decode a UPA service status filter into an RDM service status filter.
         * 
         * @param dIter The Decode Iterator
         * 
         * @return UPA return value
         */
        public int decode(DecodeIterator dIter)
        {
            elementList.clear();
            element.clear();
            int ret = elementList.decode(dIter, null);
            if (ret < CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
            boolean foundServiceState = false;
            //decode element list elements
            while ((ret = element.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                //ServiceState
                if (element.name().equals(ElementNames.SVC_STATE))
                {
                    ret = tmpUInt.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    serviceState(tmpUInt.toLong());
                    foundServiceState = true;
                }
                //AcceptingRequests 
                else if (element.name()
                        .equals(ElementNames.ACCEPTING_REQS))
                {
                    ret = tmpUInt.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    applyHasAcceptingRequests();
                    acceptingRequests(tmpUInt.toLong());
                }
                //Status
                else if (element.name().equals(ElementNames.STATUS))
                {
                    ret = tmpStatus.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS
                            && ret != CodecReturnCodes.BLANK_DATA)
                    {
                        return ret;
                    }
                    status().streamState(tmpStatus.streamState());
                    status().dataState(tmpStatus.dataState());
                    status().code(tmpStatus.code());
                    if (tmpStatus.text().length() > 0)
                    {
                        Buffer text = tmpStatus.text();
                        status().text().data(text.data(), text.position(), text.length());
                    }
                    applyHasStatus();
                }
            }

            if (!foundServiceState)
                return CodecReturnCodes.FAILURE;

            return CodecReturnCodes.SUCCESS;
        }
    }
    
    /**
     * The RDM Service State flags.  A combination of bit values that indicate the presence of optional {@link ServiceState} filter content.
     * 
     * @see ServiceState
     */
    public class ServiceStateFlags
    {
        /** (0x00) No flags set. */
        public static final int NONE = 0x00;

        /** (0x01) Indicates presence of the acceptingRequests member. */
        public static final int HAS_ACCEPTING_REQS = 0x01;

        /** (0x02) Indicates presence of the status member. */
        public static final int HAS_STATUS = 0x02;

        /**
         * Instantiates a new service state flags.
         */
        private ServiceStateFlags()
        {
            throw new AssertionError();
        }
    }

}
