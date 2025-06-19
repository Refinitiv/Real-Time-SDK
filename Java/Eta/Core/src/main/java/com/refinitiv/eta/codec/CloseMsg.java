/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/**
 * ETA close message is used by a Consumer to indicate no further interest in a
 * stream. The stream should be closed as a result. The streamId indicates the
 * item stream to which {@link CloseMsg} applies.
 * 
 * @see Msg
 * @see CloseMsgFlags
 */
public interface CloseMsg extends Msg
{
    /**
     * Checks the presence of the Extended Header presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasExtendedHdr();

    /**
     * Checks the presence of the Acknowledgment indication flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkAck();

    /**
     * Sets the Extended Header presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasExtendedHdr();

    /**
     * Sets the Acknowledgment indication flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyAck();
}