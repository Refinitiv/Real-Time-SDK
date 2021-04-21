package com.refinitiv.ema.examples.rrtmdviewer.desktop.specified_endpoint;

import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.OMMViewerError;

public interface SpecifiedEndpointSettingsService {

    int connect(SpecifiedEndpointSettingsModel settings, OMMViewerError error);
}
