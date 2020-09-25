package com.refinitiv.eta.valueadd.domainrep.rdm.directory;

import java.util.List;

import com.refinitiv.eta.codec.State;

/**
 * The RDM Directory Refresh. Used by an OMM Provider to provide information
 * about available services.
 * 
 * @see DirectoryMsg
 * 
 * @see ServiceImpl
 */
public interface DirectoryRefresh extends DirectoryMsg
{

    /**
     * The RDM Directory refresh flags. Populated by
     * {@link DirectoryRefreshFlags}.
     *
     * @param flags the flags
     */
    public void flags(int flags);

    /**
     * The RDM Directory refresh flags. Populated by
     * {@link DirectoryRefreshFlags}.
     * 
     * @return flags
     */
    public int flags();

    /**
     * Performs a deep copy of {@link DirectoryRefresh} object.
     *
     * @param destRefreshMsg Message to copy directory refresh object into. It cannot be null.
     * 
     * @return UPA return value indicating success or failure of copy operation.
     */
    public int copy(DirectoryRefresh destRefreshMsg);

    /**
     * List of service entries.
     *
     * @return service list
     */
    public List<Service> serviceList();

    /**
     * Sets service entries to the directory refresh message. This object's
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
     * {@link com.refinitiv.eta.rdm.Directory.ServiceFilterFlags}.
     * 
     * @return filter
     */
    public long filter();

    /**
     * Filter indicating which filters may appear on this stream. Where
     * possible, this should match the consumer's request. Populated by
     * 
     * {@link com.refinitiv.eta.rdm.Directory.ServiceFilterFlags}.
     *
     * @param filter the filter
     */
    public void filter(long filter);

    /**
     * sequenceNumber - Sequence number of this message.
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
     * 
     * @see #flags(int)
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
     *
     */
    public void applyHasServiceId();

    /**
     * Checks the presence of clear cache flag. 
     * 
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkClearCache();

    /**
     * Applies clear cache flag.
     * 
     * Flags may also be bulk-set via {@link #flags(int)}.
     *
     */
    public void applyClearCache();

    /**
     * Checks the presence of solicited flag.
     * 
     * @return true - if solicited flag is set, false if not.
     */
    public boolean checkSolicited();

    /**
     * Applies solicited flag.
     * 
     * Flags may also be bulk-set via {@link #flags(int)}.
     * 
     */
    public void applySolicited();

    /**
     * Returns the current state of the stream.
     * 
     * @return state.
     */
    public State state();
    
    /**
     * Sets state for the directory refresh message.
     *
     * @param state the state
     */
    public void state(State state);
}