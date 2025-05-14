/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022, 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.specified_endpoint;

import com.refinitiv.ema.examples.rtviewer.desktop.common.OMMViewerError;

public interface SpecifiedEndpointSettingsService {

    int connect(SpecifiedEndpointSettingsModel settings, OMMViewerError error);
}
