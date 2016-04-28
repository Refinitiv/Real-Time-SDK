package com.thomsonreuters.upa.valueadd.domainrep.rdm.directory;


/**
 * A Directory request message is encoded and sent by OMM consumer applications.
 * A consumer can request information about all services by omitting serviceId
 * information, or specify a serviceId to request information about only that
 * service.
 * 
 * @see DirectoryMsg
 */
public interface DirectoryRequest extends DirectoryMsg
{
    /**
     * Performs a deep copy of {@link DirectoryRequest} object.
     *
     * @param destRequestMsg Message to copy directory request object into. It cannot be null.
     * 
     * @return UPA return value indicating success or failure of copy operation.
     */
    public int copy(DirectoryRequest destRequestMsg);

    /**
     * The RDM Directory request flags. Populated by
     * {@link DirectoryRequestFlags}.
     * 
     * @return flags
     */
    public int flags();

    /**
     * The RDM Directory request flags. Populated by
     * {@link DirectoryRequestFlags}.
     * 
     * @param flags
     */
    public void flags(int flags);

    /**
     * Checks if this request is streaming or not.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if request is streaming, false - if not.
     */
    public boolean checkStreaming();

    /**
     * Makes this request streaming request.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyStreaming();

    /**
     * Applies the serviceId presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasServiceId();

    /**
     * Checks the presence of the serviceId field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if serviceId field is present, false - if not.
     */
    public boolean checkHasServiceId();

    /**
     * A filter indicating which filters of information the Consumer is
     * interested in. Populated by {@link com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags}.
     * 
     * @return filter
     */
    public long filter();

    /**
     * A filter indicating which filters of information the Consumer is
     * interested in. Populated by {@link com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags}.
     * 
     * @param filter
     */
    public void filter(long filter);

    /**
     * serviceId - The ID of the service to request the directory from.
     * 
     * @return serviceId
     */
    public int serviceId();

    /**
     * The ID of the service to request the directory from.
     * 
     * @param serviceId
     */
    public void serviceId(int serviceId);
}