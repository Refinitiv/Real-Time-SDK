/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.directory;

import java.util.List;

/**
 * The Directory Consumer Status is sent by OMM Consumer applications to inform
 * a service of how it is being used for Source Mirroring. This message is
 * primarily informational.
 */
public interface DirectoryConsumerStatus extends DirectoryMsg
{
    /**
     * Performs a deep copy of {@link DirectoryConsumerStatus} object.
     * 
     * @param destConsumerStatus Message to copy directory consumer status
     *            object into. It cannot be null.
     * 
     * @return ETA return value indicating success or failure of copy operation.
     */
    public int copy(DirectoryConsumerStatus destConsumerStatus);
    
    /**
     *  The list of Consumer Service Status elements
     *  
     * @return list of consumer service status elements.
     */
    public List<ConsumerStatusService> consumerServiceStatusList();
    
    
    /**
     * Sets consumer service status elements to the directory consumer
     * status. This object's ConsumerStatusService elements will be set to
     * ConsumerStatusService elements from list in the parameter passed in.
     * 
     * @param consumerServiceStatusList -list of consumer service status
     *            elements.
     */
    public void consumerServiceStatusList(List<ConsumerStatusService> consumerServiceStatusList);
}