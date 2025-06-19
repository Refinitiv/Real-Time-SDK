/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.itemview.fx;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.examples.rtviewer.desktop.itemview.ItemNotificationModel;
import javafx.application.Platform;
import javafx.collections.FXCollections;
import javafx.collections.MapChangeListener;
import javafx.collections.ObservableList;
import javafx.collections.ObservableMap;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.control.*;
import javafx.scene.control.cell.PropertyValueFactory;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.Pane;

import java.io.IOException;
import java.time.LocalTime;
import java.util.HashMap;
import java.util.Objects;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;

import static com.refinitiv.ema.examples.rtviewer.desktop.common.ApplicationFxComponents.MARKET_BY_COMPONENT;

public class MarketByOrderComponent extends Pane implements ItemFxComponent, MarketByComponent {

    public static final String ASK_MODEL = "ASK";
    public static final String BID_MODEL = "BID";

    public static final int ORDER_PRC_FID = 3427; /* Order Price */
    public static final int ORDER_SIDE_FID = 3428; /* Order Side */
    public static final int ORDER_SIZE_FID = 3429; /* Order Size */
    public static final int QUOTIM_MS_FID = 3855; /* Quote Time */
    public static final int PR_TIM_MS = 6520; /* The Priority Time Stamp, in GMT, of the order.  */

    @FXML
    private TextField currencyFld;

    @FXML
    private TextField unitsFld;

    @FXML
    private TextField exchIdFld;

    @FXML
    private TextField rnkRulFld;

    @FXML
    private TableView<MarketByOrderModel> tableBid;

    @FXML
    private TableView<MarketByOrderModel> tableAsk;

    @FXML
    private ListView<String> statusLabel;

    @FXML
    private Button unregisterButton;

    private ItemNotificationModel lastItemNotification;

    private final ObservableList<MarketByOrderModel> bidTableList = FXCollections.observableArrayList();
    private final ObservableList<MarketByOrderModel> askTableList = FXCollections.observableArrayList();
    private final ObservableMap<String, MarketByOrderModel> centralMap = FXCollections.observableHashMap();

    private final SummaryDataModel summaryDataModel = new SummaryDataModel();

    private java.util.Map<String, ObservableList<MarketByOrderModel>> strategyMap = new HashMap<>();

    {
        strategyMap.put(ASK_MODEL, askTableList);
        strategyMap.put(BID_MODEL, bidTableList);
    }

    private StatusHelper statusHelper;

    public MarketByOrderComponent(Tab tab) {

        FXMLLoader fxmlLoader = new FXMLLoader(MARKET_BY_COMPONENT.getResource());
        fxmlLoader.setRoot(this);
        fxmlLoader.setController(this);
        try {
            fxmlLoader.load();
        } catch (IOException ex) {
            throw new RuntimeException(ex);
        }

        tab.setContent(this);
    }

    @FXML
    public void initialize() {
        statusHelper = new StatusHelper(statusLabel);
        tableBid.getColumns().forEach(column -> {
            column.setCellValueFactory(new PropertyValueFactory<>(column.getUserData().toString()));
        });
        tableBid.setItems(bidTableList);
        tableAsk.getColumns().forEach(column -> {
            column.setCellValueFactory(new PropertyValueFactory<>(column.getUserData().toString()));
        });
        tableAsk.setItems(askTableList);

        centralMap.addListener(this::handleCentralMapChanging);

        currencyFld.textProperty().bind(summaryDataModel.currencyProperty());
        unitsFld.textProperty().bind(summaryDataModel.unitsProperty());
        exchIdFld.textProperty().bind(summaryDataModel.exchIdProperty());
        rnkRulFld.textProperty().bind(summaryDataModel.rankRuleProperty());

        this.widthProperty().addListener(e -> {
            tableBid.setPrefWidth(this.getWidth() / 2 - 10);
            tableAsk.setPrefWidth(this.getWidth() / 2 - 10);
        });
    }

    public void setUnregisterButtonVisible(boolean visible) {
        unregisterButton.setVisible(visible);
    }

    @Override
    public void configureButton(String name, Consumer<MouseEvent> func) {
        unregisterButton.setText(name);
        unregisterButton.setOnMouseClicked(e -> {
            unregisterButton.setDisable(true);
            func.accept(e);
        });
        unregisterButton.getStyleClass().add(BRAND_BUTTON_STYLE);
    }

    @Override
    public synchronized void handleItem(ItemNotificationModel notification) {
        handleItem(notification, true);
    }


    @Override
    public void handleMsgState(ItemNotificationModel notification) {
        if (!Platform.isFxApplicationThread()) {
            Platform.runLater(() -> handleMsgState(notification));
            return;
        }
        statusHelper.updateStatus(notification);
        if (!notification.getRequest().getView().isSnapshot()) {
            statusHelper.handleTableState(notification, tableAsk);
            statusHelper.handleTableState(notification, tableBid);
        }
    }

    @Override
    public void updateUsingLastActualInfo() {
        if (Objects.nonNull(lastItemNotification)) {
            handleItem(lastItemNotification, false);
        }
    }

    private void decodeMsgData(FieldList fieldList, MarketByOrderModel mbm) {
        for (FieldEntry fieldEntry : fieldList) {
            if (Data.DataCode.BLANK != fieldEntry.code()) {
                final int fieldId = fieldEntry.fieldId();
                switch (fieldId) {
                    case ORDER_PRC_FID:
                        mbm.setPrice(fieldEntry.real().asDouble());
                        break;
                    case ORDER_SIDE_FID:
                        mbm.setModelType(MarketByComponent.decodeValueToStr(fieldEntry));
                        break;
                    case QUOTIM_MS_FID:
                    case PR_TIM_MS:
                        final String msTime = MarketByComponent.decodeValueToStr(fieldEntry);
                        final long nano = TimeUnit.NANOSECONDS.convert(Long.parseLong(msTime), TimeUnit.MILLISECONDS);
                        mbm.setQuoteTime(LocalTime.ofNanoOfDay(nano).toString());
                        break;
                    case ORDER_SIZE_FID:
                        mbm.setSize(fieldEntry.real().asDouble());
                        break;
                }
            }
        }
    }

    private synchronized void handleItem(ItemNotificationModel notification, boolean logMsgState) {
        final Msg msg = notification.getMsg();
        if (logMsgState) {
            handleMsgState(notification);
        }
        this.lastItemNotification = notification;
        if (Objects.equals(msg.payload().dataType(), DataType.DataTypes.MAP)) {
            final Map map = msg.payload().map();
            MarketByComponent.decodeSummaryData(map.summaryData(), summaryDataModel);

            for (MapEntry mapEntry : map) {
                if (DataType.DataTypes.BUFFER != mapEntry.key().dataType()) {
                    return;
                }

                int dType = mapEntry.loadType();
                final String key = EmaUtility.asAsciiString(mapEntry.key().buffer());
                if (dType == DataType.DataTypes.FIELD_LIST) {
                    MarketByOrderModel mbm = null;
                    MarketByOrderModel current = null;
                    switch (mapEntry.action()) {
                        case MapEntry.MapAction.ADD:
                        case MapEntry.MapAction.UPDATE:
                            current = centralMap.get(key);
                            if(current != null)
                                mbm = new MarketByOrderModel(current);
                            else {
                                mbm = new MarketByOrderModel(key);
                            }
                            break;
                    }
                    if (Objects.nonNull(mbm)) {
                        decodeMsgData(mapEntry.fieldList(), mbm);
                        centralMap.put(key, mbm);
                    }
                } else if (Objects.equals(DataType.DataTypes.NO_DATA, dType)
                        && Objects.equals(MapEntry.MapAction.DELETE, mapEntry.action())) {
                    centralMap.remove(key);
                }
            }
        }
        if (notification.getRequest().getView().isSnapshot()) {
            Platform.runLater(() -> unregisterButton.setDisable(false));
        }
    }

    private void sortingOrders(ObservableList<MarketByOrderModel> observableList, String rankRule)
    {
        if(observableList != null) {

            FXCollections.sort(observableList, (o1, o2) -> {
                int result = 0;

                if (o1.getModelType().equals(BID_MODEL))
                    result = Double.compare(o2.getPrice(), o1.getPrice());
                else
                    result = Double.compare(o1.getPrice(), o2.getPrice());

                if(rankRule != null && rankRule.equals("PTS")) {
                    if (result == 0) {
                        result = o1.getQuoteTime().compareTo(o2.getQuoteTime());
                    }

                    if (result == 0) {
                        result = Double.compare(o1.getSize(),o2.getSize());
                    }
                }
                else { /* The default ranking rule is PST */
                    if (result == 0) {
                        result = Double.compare(o1.getSize(),o2.getSize());
                    }

                    if (result == 0) {
                        result = o1.getQuoteTime().compareTo(o2.getQuoteTime());
                    }
                }

                return result;
            });
        }
    }

    private void handleCentralMapChanging(MapChangeListener.Change<? extends String, ? extends MarketByOrderModel> change) {
        if (Platform.isFxApplicationThread()) {
            MarketByOrderModel mbo = null;
            ObservableList<MarketByOrderModel> observableList = null;
            if (change.wasAdded() && change.wasRemoved()) {
                mbo = change.getValueAdded();
                observableList = strategyMap.get(mbo.getModelType());

                if(observableList != null) {
                    observableList.remove(change.getValueRemoved());
                    observableList.add(mbo);
                }
            } else if (change.wasAdded()) {
                mbo = change.getValueAdded();
                observableList = strategyMap.get(mbo.getModelType());

                if(observableList != null) {
                    observableList.add(mbo);
                }
            } else if (change.wasRemoved()) {
                mbo = change.getValueRemoved();
                observableList = strategyMap.get(mbo.getModelType());

                if(observableList != null) {
                    observableList.remove(mbo);
                }
            }

            sortingOrders(observableList, rnkRulFld.getText());

        } else {
            Platform.runLater(() -> handleCentralMapChanging(change));
        }
    }

    @Override
    public void clear() {
        currencyFld.clear();
        unitsFld.clear();
        exchIdFld.clear();
        tableBid.getItems().clear();
        tableAsk.getItems().clear();
        statusLabel.getItems().clear();
        centralMap.clear();
        summaryDataModel.clear();
    }

    @Override
    public void setDisableBtn(boolean value) {
        unregisterButton.setDisable(value);
    }
}