package com.thomsonreuters.upa.valueadd.domainrep.rdm.directory;

import java.util.List;

/**
 * The RDM Directory Update. Used by an OMM Provider to provide updates about available services. 
 *
 * @see DirectoryMsg
 * @see ServiceImpl
 */
public interface DirectoryUpdate extends DirectoryMsg
{
    
    /**
     * The RDM Directory Update flags.
     *
     * @param flags the flags
     * @see DirectoryUpdateFlags
     */
    public void flags(int flags);

    /**
     * The RDM Directory Update flags.
     *
     * @return flags.
     * @see DirectoryUpdateFlags
     */
    public int flags();

    /**
     * Performs a deep copy of {@link DirectoryUpdate} object.
     *
     * @param destUpdateMsg Message to copy directory update object into. It cannot be null.
     * 
     * @return UPA return value indicating success or failure of copy operation.
     */
    public int copy(DirectoryUpdate destUpdateMsg);

    /**
     * List of service entries.
     * 
     * @return service list
     */
    public List<Service> serviceList();
    
    /**
     * Sets service entries into the directory refresh message. This object's
     * Service elements will be set to Service elements from list in the
     * parameter passed in.
     * 
     * @param serviceList -list of service entries.
     */
    public void serviceList(List<Service> serviceList);

    /**
     * Filter indicating which filters may appear on this stream. Where
     * possible, this should match the consumer's request. Populated by
     * 
     * {@link com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags}.
     * 
     * @return filter
     */
    public long filter();

    /**
     * Filter indicating which filters may appear on this stream. Where
     * possible, this should match the consumer's request. Populated by
     * 
     * {@link com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags}.
     *
     * @param filter the filter
     */
    public void filter(long filter);

    /**
     * Checks the presence of the filter field.
     * 
     * Flags may also be bulk-set via {@link #flags(int)}.
     *
     * @return true - if filter field exists, false - if not.
     * @see #flags(int)
     *      
     */
    public boolean checkHasFilter();

    /**
     * Applies the presence of the filter field. 
     * 
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     */
    public void applyHasFilter();

    /**
     * sequenceNumber- Sequence number of this message.
     * 
     * @return sequenceNumber
     */
    public long sequenceNumber();

    /**
     * sequenceNumber - Sequence number of this message.
     *
     * @param sequenceNumber the sequence number
     */
    public void sequenceNumber(long sequenceNumber);

    /**
     * Checks the presence of sequence number field.
     * 
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasSequenceNumber();

    /**
     * Applies the sequence number flag.
     * 
     * Flags may also be bulk-set via {@link #flags(int)}.
     */
    public void applyHasSequenceNumber();

    /**
     * serviceId - The ID of the service whose information is provided by this stream(if not
     * present, all services should be provided). Should match the Consumer's
     * request if possible.
     * 
     * @return serviceId
     */
    public int serviceId();

    /**
     * serviceId - The ID of the service whose information is provided by this stream(if not
     * present, all services should be provided). Should match the Consumer's
     * request if possible.
     *
     * @param serviceId the service id
     */
    public void serviceId(int serviceId);

    /**
     * Checks the presence of service id field.
     * 
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasServiceId();

    /**
     * Applies the service id flag.
     * 
     * Flags may also be bulk-set via {@link #flags(int)}.
     */
    public void applyHasServiceId();
}