/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.directory;



/**
 * Factory for RDM directory messages.
 */
public class DirectoryMsgFactory
{
    /**
     * This class is not instantiated
     */
    private DirectoryMsgFactory()
    {
        throw new AssertionError();
    }

    /**
     * Creates a RDMDirectoryMsg that may be cast to any message class defined
     * by {@link DirectoryMsgType} (e.g. {@link DirectoryClose},
     * {@link DirectoryConsumerStatus}, {@link DirectoryRequest},
     * {@link DirectoryRefresh}
     * 
     * @return RDMDirectoryMsg object
     * 
     * @see DirectoryClose
     * @see DirectoryRefresh
     * @see DirectoryRequest
     * @see DirectoryConsumerStatus
     */
    public static DirectoryMsg createMsg()
    {
        return new DirectoryMsgImpl();
    }
    

    /**
     * Creates {@link ConsumerStatusService}.
     * 
     * @return ConsumerStatusService object
     * 
     * @see ConsumerStatusService
     */
    public static ConsumerStatusService createConsumerStatusService()
    {
        return new ConsumerStatusServiceImpl();
    }
    
    /**
     * Creates {@link Service}.
     * 
     * @return Service object
     * 
     * @see Service
     */
    public static Service createService()
    {
        return new ServiceImpl();
    }
}
