/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview.fx;

import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview.ItemNotificationModel;
import javafx.scene.control.ListView;
import javafx.scene.control.TableView;

import java.time.LocalTime;
import java.time.format.DateTimeFormatter;
import java.util.Objects;

public class StatusHelper {


    public static final String STOP_STREAMING_CSS_STYLE = "red-value";


    private DateTimeFormatter formatter = DateTimeFormatter.ISO_LOCAL_TIME;
    private static final int MAX_STORED_STATUS_COUNT = 1000;

    private final ListView<String> statusListView;

    public StatusHelper(ListView<String> statusListView) {
        this.statusListView = statusListView;
    }

    public void updateStatus(ItemNotificationModel request) {
        Msg msg = request.getMsg();
        OmmState ommState = getOmmState(request);
        if (Objects.nonNull(ommState)) {
            statusListView.getItems().add(0, getCurrentTime() + "  " + (msg.hasName() ? msg.name() + ": " : "") + ommState.toString());
            if (statusListView.getItems().size() > MAX_STORED_STATUS_COUNT) {
                statusListView.getItems().remove(statusListView.getItems().size() - 1);
            }
        }
    }

    private String getCurrentTime() {
        return formatter.format(LocalTime.now());
    }

    public void handleTableState(ItemNotificationModel notification, TableView<?> tableView) {
        if (isGoodState(notification)) {
            tableView.getStyleClass().remove(STOP_STREAMING_CSS_STYLE);
        } else {
            tableView.getStyleClass().add(STOP_STREAMING_CSS_STYLE);
        }
    }

    public boolean isGoodState(ItemNotificationModel notification) {
        final OmmState state = getOmmState(notification);
        return Objects.isNull(state) || (Objects.equals(state.dataState(), OmmState.DataState.OK)
                && Objects.equals(state.streamState(), OmmState.StreamState.OPEN));
    }

    public boolean hasOmmState(ItemNotificationModel request) {
        return request.getMsg() instanceof RefreshMsg || request.getMsg() instanceof StatusMsg;
    }

    public OmmState getOmmState(ItemNotificationModel request) {
        final Msg msg = request.getMsg();
        if (msg instanceof RefreshMsg) {
            return ((RefreshMsg) msg).state();
        } else if (msg instanceof StatusMsg) {
            return ((StatusMsg) msg).state();
        }
        return null;
    }
}
