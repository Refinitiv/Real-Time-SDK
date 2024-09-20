/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.directory;


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
     * @return ETA return value indicating success or failure of copy operation.
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
     * @param flags the flags
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
     * interested in. Populated by {@link com.refinitiv.eta.rdm.Directory.ServiceFilterFlags}.
     * 
     * @return filter
     */
    public long filter();

    /**
     * A filter indicating which filters of information the Consumer is
     * interested in. Populated by {@link com.refinitiv.eta.rdm.Directory.ServiceFilterFlags}.
     *
     * @param filter the filter
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
     * @param serviceId the service id
     */
    public void serviceId(int serviceId);
}