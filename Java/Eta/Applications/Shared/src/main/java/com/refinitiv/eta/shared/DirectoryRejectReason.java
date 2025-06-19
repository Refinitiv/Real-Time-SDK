/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared;

/** Reasons a directory request is rejected */
public enum DirectoryRejectReason
{
    MAX_SRCDIR_REQUESTS_REACHED,
    INCORRECT_FILTER_FLAGS,
    DIRECTORY_RDM_DECODER_FAILED;
}
