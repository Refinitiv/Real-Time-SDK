///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/*
 * CosFlowControl contains options related to flow control, such as the type and the allowed window 
 * of outstanding data.
 */
public interface CosFlowControl
{
	public class CosFlowControlType
	{
		public static final int NONE = 0;
		public static final int BIDIRECTIONAL = 1;
	}

	/*
	 * Clears the CosFlowControl object
	 */
	public CosFlowControl clear();

	/*
	 * Specifies flow control type
	 */
	public CosFlowControl type(int type);

	/*
	 * Specifies the amount of data (in bytes) that the remote peer can receive
	 * from the application over a reliable tunnel stream.
	 */
	public CosFlowControl recvWindowSize(int size);

	/*
	 * Returns the flow control type
	 */
	public int type();

	/*
	 * Returns the receive window size
	 */
	public int recvWindowSize();

	/*
	 * Returns the send window size
	 */
	public int sendWindowSize();
}