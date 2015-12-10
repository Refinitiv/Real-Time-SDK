package com.thomsonreuters.upa.valueadd.domainrep.rdm.directory;

import com.thomsonreuters.upa.codec.State;

/**
 * The RDM Directory Status. Used by an OMM Provider to indicate changes to the
 * Directory stream.
 * 
 * @see DirectoryMsg
 */
public interface DirectoryStatus extends DirectoryMsg
{
    /**
     * The RDM Directory Status flags.
     * 
     * @see DirectoryStatusFlags
     * 
     * @param flags
     */
    public void flags(int flags);

    /**
     * The RDM Directory Update flags.
     * 
     * @see DirectoryStatusFlags
     * 
     * @return flags.
     */
    public int flags();

    /**
     * Filter indicating which filters may appear on this stream. Where
     * possible, this should match the consumer's request. Populated by
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
     * @param filter
     */
    public void filter(long filter);

    /**
     * Applies the presence of the filter field.
     * 
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     */
    public void applyHasFilter();

    /**
     * Checks the presence of the filter field.
     * 
     * Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     * 
     * @return true - if filter field exists, false - if not.
     * 
     */
    public boolean checkHasFilter();

    /**
     * The ID of the service whose information is provided by this stream(if not
     * present, all services should be provided). Should match the Consumer's
     * request if possible.
     * 
     * @return service id.
     */
    public int serviceId();

    /**
     * serviceId - The ID of the service whose information is provided by this
     * stream(if not present, all services should be provided). Should match the
     * Consumer's request if possible.
     * 
     * @param serviceId
     */
    public void serviceId(int serviceId);

    /**
     * Applies the service id flag.
     * 
     * Flags may also be bulk-set via {@link #flags(int)}.
     */
    public void applyHasServiceId();

    /**
     * Checks the presence of service id field.
     * 
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasServiceId();

    /**
     * Returns current state of the stream.
     * 
     * @return state
     */
    public State state();
    
    /**
     * Sets state for the directory status message.
     * 
     * @param state
     */
    public void state(State state);

    /**
     * Applies state presence flag.
     * 
     * Flags may also be bulk-set via {@link #flags(int)}.
     */
    public void applyHasState();

    /**
     * Checks the presence of state field.
     * 
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasState();

    /**
     * Performs a deep copy of {@link DirectoryStatus} object.
     *
     * @param destStatusMsg Message to copy directory status object into. It cannot be null.
     * 
     * @return UPA return value indicating success or failure of copy operation.
     */
    public int copy(DirectoryStatus destStatusMsg);
}