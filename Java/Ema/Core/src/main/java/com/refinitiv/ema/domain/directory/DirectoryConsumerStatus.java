/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.domain.directory;

import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.OmmInvalidUsageException;

import java.util.List;

/**
 * Represents an RDM Directory Consumer Status message.
 * <p>
 * OMM consumer applications send this message to report how one or more
 * services are being used for source mirroring and warm-standby workflows.
 * The message is primarily informational.
 * <p>
 * When this object is populated from a {@link GenericMsg} through the inherited
 * {@link DirectoryMsg#message(Object) message(GenericMsg)} method, the supplied message must be a Directory
 * domain message whose name is
 * {@link com.refinitiv.ema.rdm.EmaRdm#ENAME_CONS_STATUS}.
 * <p>
 * Implementations may be cleared and re-used.
 *
 * @see DirectoryMsg
 * @see DirectoryConsumerStatusService
 */
public interface DirectoryConsumerStatus extends DirectoryMsg<GenericMsg>
{
    @Override
    DirectoryConsumerStatus clear();

    @Override
    DirectoryConsumerStatus message(GenericMsg msg);

    @Override
    DirectoryConsumerStatus streamId(int streamId);

    /**
     * Returns the RDM message name for this type.
     *
     * @return {@link com.refinitiv.ema.rdm.EmaRdm#ENAME_CONS_STATUS}
     */
    String name();

    /**
     * Returns the optional sequence number carried by this message.
     *
     * @return the sequence number
     * @throws OmmInvalidUsageException if {@link #checkHasSequenceNumber()} returns {@code false}
     */
    long sequenceNumber();

    /**
     * Sets the sequence number for this message.
     * <p>
     * Calling this method marks the optional sequence-number field as present.
     *
     * @param sequenceNumber the sequence number
     * @return this directory consumer-status message instance
     */
    DirectoryConsumerStatus sequenceNumber(long sequenceNumber);

    /**
     * Checks whether this message contains a sequence number.
     *
     * @return {@code true} if {@link #sequenceNumber()} is available; otherwise {@code false}
     */
    boolean checkHasSequenceNumber();

    /**
     * Returns the consumer-status service entries carried by this message.
     * <p>
     * The returned list is the live list used by this object. Changes made to the
     * list or to its contained {@link DirectoryConsumerStatusService} objects are
     * reflected in this message instance.
     *
     * @return the live list of consumer-status service entries
     */
    List<DirectoryConsumerStatusService> consumerServiceStatusList();

    /**
     * Replaces this message's consumer-status service entries with those from the
     * supplied list.
     * <p>
     * The list itself is not retained. Instead, this object clears its current
     * entries and copies the references from {@code consumerServiceStatusList}
     * into its own internal list. The individual
     * {@link DirectoryConsumerStatusService} objects are not deep-copied by this
     * method.
     *
     * @param consumerServiceStatusList the consumer-status service entries to assign
     * @return this directory consumer-status message instance
     * @throws OmmInvalidUsageException if {@code consumerServiceStatusList} is {@code null}
     */
    DirectoryConsumerStatus consumerServiceStatusList(List<DirectoryConsumerStatusService> consumerServiceStatusList);

    /**
     * Performs a deep copy of another {@link DirectoryConsumerStatus} into this object.
     * <p>
     * This method replaces this object's current contents with the values from
     * {@code sourceConsumerStatus}. Service entries are deep-copied. If
     * {@code sourceConsumerStatus} is this object, the call is a no-op.
     *
     * @param sourceConsumerStatus the directory consumer-status message to copy from; must not be {@code null}
     * @return this directory consumer-status message instance
     * @throws OmmInvalidUsageException if {@code sourceConsumerStatus} is {@code null}
     */
    DirectoryConsumerStatus copy(DirectoryConsumerStatus sourceConsumerStatus);
}
