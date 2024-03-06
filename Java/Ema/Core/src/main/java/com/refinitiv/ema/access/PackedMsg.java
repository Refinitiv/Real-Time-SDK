///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2024 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * PackedMsg contains a list of messages packed to be sent across the wire together.
 */
public interface PackedMsg {
	/**
	 * For Non-Interactive Provider applications, initializes a new write buffer.
	 * The size of the new packed message write buffer will be set to its default size (6000 bytes).
	 * @throws OmmInvalidUsageException 
	 * 	if used for incorrect client type or buffer cannot be initialized.
	 * @return this PackedMsg
	 */
	public PackedMsg initBuffer();
	/**
	 * For Non-Interactive Provider applications, initializes a new write buffer.
	 * Also sets the maximum size of the new packed message write buffer.
	 * Note that each packed message has an additional header length of 2.
	 * @param maxSize maximum size of the packed message buffer.
	 * @throws OmmInvalidUsageException 
	 * 	if used for incorrect client type or buffer cannot be initialized.
	 * @return this PackedMsg
	 */
	public PackedMsg initBuffer(int maxSize);
	/**
	 * Sets the client handle for this PackedMsg to submit messages to and gets a new write buffer. 
	 * For Interactive Provider applications, the client handle is required to be set 
	 * before adding messages or submitting the packedMsg.
	 * The size of the new packed message write buffer will be set to its default size (6000 bytes).
	 * Note that each packed message has an additional header length of 2.
	 * @throws OmmInvalidUsageException 
	 * 	if used for incorrect client type or buffer cannot be initialized.
	 * @param clientHandle the client handle for the PackedMsg
	 * @return this PackedMsg
	 */
	public PackedMsg initBuffer(long clientHandle);
	/**
	 * Sets the client handle for this PackedMsg to submit messages to and gets a new write buffer. 
	 * For Interactive Provider applications, the client handle is required to be set 
	 * before adding messages or submitting the packedMsg.
	 * Also sets the maximum size of the new packed message write buffer.
	 * Note that each packed message has an additional header length of 2.
	 * @param clientHandle the client handle for the PackedMsg
	 * @param maxSize maximum size of the packed message buffer.
	 * @throws OmmInvalidUsageException 
	 * 	if used for incorrect client type or buffer cannot be initialized.
	 * @return this PackedMsg
	 */
	public PackedMsg initBuffer(long clientHandle, int maxSize);
	/**
	 * Adds a Msg to the packedMsg buffer if there is enough space in the buffer to add the Msg, including header length (2).
	 * Sets the item handle associated with the messages being sent.
	 * @param msg Msg object message to add to this packed Msg
	 * @param itemHandle the item handle associated with the messages being sent
	  * @throws OmmInvalidUsageException 
	 * 	if used incorrectly, connection not established, or a failure occurs when packing.
	 * @return this PackedMsg
	 */
	public PackedMsg addMsg(Msg msg, long itemHandle);
	/**
	 * Returns int value of remaining size in the buffer available for message packing.
	 * @return int
	 */
	public int remainingSize();
	/**
	 * Returns int value of amount of currently packed messages in this PackedMsg object.
	 * @return int
	 */
	public int packedMsgCount();
	/**
	 * Return int value of maximum size of the packed message buffer.
	 * @return int
	 */
	public int maxSize();
	/**
	 * Releases the buffer and clears the count of entries packed.
	 * initBuffer() should be called after this to obtain a new buffer for writing.
	 * @return this PackedMsg
	 */
	public PackedMsg clear();
}
