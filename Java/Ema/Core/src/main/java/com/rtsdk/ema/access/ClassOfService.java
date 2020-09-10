///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

/*
 * ClassOfService is used to negotiate behaviors of a tunnel stream. Negotiated behaviors are divided into
 * give catagories: common, authentication, flow control, data integrity, and guarantee
 */
public interface ClassOfService
{
	/*
	 * Clears the ClassOfService object
	 */
	public ClassOfService clear();

	/*
	 * Specifies the CosCommon member
	 */
	public ClassOfService common(CosCommon cosCommon);
	
	/*
	 * Specifies the CosAuthentication member
	 */
	public ClassOfService authentication(CosAuthentication cosAuthentication);
	
	/*
	 * Specifies the CosFlowControl member
	 */
	public ClassOfService flowControl(CosFlowControl cosFlowControl);
	
	/*
	 * Specifies the CosDataIntegrity member
	 */
	
	public ClassOfService dataIntegrity(CosDataIntegrity cosDataIntegrity);
	
	/*
	 * Specifies CosGuarantee member
	 */
	public ClassOfService guarantee(CosGuarantee cosGuarantee);
	
	/*
	 * Returns CosCommon member
	 */
	public CosCommon common();
	
	/*
	 * Returns CosAuthentication member
	 */
	
	public CosAuthentication authentication();
	
	/*
	 * Returns CosFlowControl member
	 */
	
	public CosFlowControl flowControl();
	
	/*
	 * Returns CosDataIntegrity member
	 */
	
	public CosDataIntegrity dataIntegrity();
	
	/*
	 * Returns CosGuarantee
	 */
	
	public CosGuarantee guarantee();
}