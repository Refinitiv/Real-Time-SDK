/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.directory;

import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBase;

/**
 * The RDM Directory Base Message. This RDM directory messages may be reused or
 * pooled in a single collection via their common {@link DirectoryMsg}
 * interface and re-used as a different {@link DirectoryMsgType}.
 * RDMDirectoryMsgType member may be set to one of these to indicate the
 * specific RDMDirectoryMsg class.
 * 
 * @see DirectoryClose
 * @see DirectoryRefresh
 * @see DirectoryRequest
 * @see DirectoryConsumerStatus
 * 
 * @see DirectoryMsgFactory - Factory for creating RDM directory messages
 * 
 * @see DirectoryMsgType
 */
public interface DirectoryMsg extends MsgBase
{
    /**
     * Directory message type. These are defined per-message class basis for
     * directory domain.
     * 
     * @see DirectoryClose
     * @see DirectoryRefresh
     * @see DirectoryRequest
     * @see DirectoryConsumerStatus
     * 
     * @return RDMDirectoryMsgType - directory message type.
     */
    public DirectoryMsgType rdmMsgType();

    /**
     * Directory message type. These are defined per-message class basis for
     * directory domain.
     * 
     * @see DirectoryClose
     * @see DirectoryRefresh
     * @see DirectoryRequest
     * @see DirectoryConsumerStatus
     * 
     * @param rdmMsgType - directory message type.
     */
    public void rdmMsgType(DirectoryMsgType rdmMsgType);
}
