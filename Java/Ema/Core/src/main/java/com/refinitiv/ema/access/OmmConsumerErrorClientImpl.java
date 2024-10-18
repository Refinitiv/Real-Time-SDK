///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019, 2024 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;


class OmmConsumerErrorClientImpl implements OmmConsumerErrorClient
{
	@Override
	public void onInvalidHandle(long handle, String text) {}

	@Override
	public void onInvalidUsage(String text) {}

	@Override
	public void onInvalidUsage(String text, int errorCode) {}

	@Override
	public void onJsonConverterError(ConsumerSessionInfo consumerSessionInfo, int errorCode, String text) {}

	@Override
	public void onDispatchError(String text, int errorCode) {}
}