package com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview;

public interface ItemViewService {

    void loadServiceData(ServiceInfoResponseModel serviceInfoResponse);

    void sendItemRequest(ItemRequestModel request);

    void unregister(long handle);
}
