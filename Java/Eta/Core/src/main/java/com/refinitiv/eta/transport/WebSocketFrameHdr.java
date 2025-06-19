/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.nio.ByteBuffer;

class WebSocketFrameHdr {
	
	ByteBuffer buffer;
	int ctrlHdrIndex;
	int extHdrIndex;
	int cursor;
	int hdrLen;
	int extHdrLen;
	boolean partial;
	boolean finSet;
	boolean rsv1Set;
	boolean rsv2Set;
	boolean rsv3Set;
	int opcode = WebSocketFrameParser._WS_OPC_NONE;
	int dataType;
	boolean control;
	boolean fragment;
	boolean compressed;
	boolean maskSet;
	byte[] mask = new byte[4];
	int maskVal;
	long payloadLen;
	int payloadIndex;
	
	void clear()
	{
		buffer = null;
		ctrlHdrIndex = 0;
		extHdrIndex = 0;
		cursor = 0;
		hdrLen = 0;
		extHdrLen = 0;
		partial = false;
		finSet = false;
		rsv1Set = false;
		rsv2Set = false;
		rsv3Set = false;
		opcode = WebSocketFrameParser._WS_OPC_NONE;
		dataType = 0;
		control = false;
		fragment = false;
		compressed = false;
		maskSet = false;
		mask[0] = 0;
		mask[1] = 0;
		mask[2] = 0;
		mask[3] = 0;
		maskVal = 0;
		payloadLen = 0;
		payloadIndex = 0;
	}
	
	void reset()
	{
		/* This function does not reset the following frame fields:
		 *		->fragment
		 *		->dataType
		 *		->compressed
		 *		->finset
		 *		->opcode
		 *	These fields keep the state for any fragmented frames being 
		 *	reassembled along with possible control frames intermixed
		 */
		
		buffer = null;
		ctrlHdrIndex = 0;
		extHdrIndex = 0;
		cursor = 0;
		hdrLen = 0;
		extHdrLen = 0;
		partial = false;
		rsv1Set = false;
		rsv2Set = false;
		rsv3Set = false;
		control = false;
		maskSet = false;
		mask[0] = 0;
		mask[1] = 0;
		mask[2] = 0;
		mask[3] = 0;
		maskVal = 0;
		payloadLen = 0;
		payloadIndex = 0;
	}
}
