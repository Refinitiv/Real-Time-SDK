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

import static com.refinitiv.ema.examples.rtviewer.desktop.common.ApplicationFxComponents.MARKET_BY_PRICE_COMPONENT;

public class MarketByPriceComponent extends Pane implements ItemFxComponent, MarketByComponent {

    public static final String ASK_MODEL = "ASK";
    public static final String BID_MODEL = "BID";

    public static final int ORDER_PRC_FID = 3427; /* Order Price */
    public static final int ORDER_SIDE_FID = 3428; /* Order Side */
    public static final int ACC_SIZE_FID = 4356; /* The total quantity of shares represented in an aggregated MBP row */
    public static final int NO_ORD = 3430; /* The number of orders in the aggregate MBP row */
    public static final int QUOTIM_MS_FID = 3855; /* Quote Time */
    public static final int LV_TIM_MS = 6527; /* The time, in GMT, an aggregated MBP row was most recently updated. */

    @FXML
    private TextField currencyFld;

    @FXML
    private TextField unitsFld;

    @FXML
    private TextField exchIdFld;

    @FXML
    private TextField rnkRulFld;

    @FXML
    private TableView<MarketByPriceModel> tableBid;

    @FXML
    private TableView<MarketByPriceModel> tableAsk;

    @FXML
    private ListView<String> statusLabel;

    @FXML
    private Button unregisterButton;

    private ItemNotificationModel lastItemNotification;

    private final ObservableList<MarketByPriceModel> bidTableList = FXCollections.observableArrayList();
    private final ObservableList<MarketByPriceModel> askTableList = FXCollections.observableArrayList();
    private final ObservableMap<String, MarketByPriceModel> centralMap = FXCollections.observableHashMap();

    private final SummaryDataModel summaryDataModel = new SummaryDataModel();

    private java.util.Map<String, ObservableList<MarketByPriceModel>> strategyMap = new HashMap<>();

    {
        strategyMap.put(ASK_MODEL, askTableList);
        strategyMap.put(BID_MODEL, bidTableList);
    }

    private StatusHelper statusHelper;

    public MarketByPriceComponent(Tab tab) {

        FXMLLoader fxmlLoader = new FXMLLoader(MARKET_BY_PRICE_COMPONENT.getResource());
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

    private void decodeMsgData(FieldList fieldList, MarketByPriceModel mbp) {
        for (FieldEntry fieldEntry : fieldList) {
            if (Data.DataCode.BLANK != fieldEntry.code()) {
                final int fieldId = fieldEntry.fieldId();
                switch (fieldId) {
                    case ORDER_PRC_FID:
                        mbp.setPrice(fieldEntry.real().asDouble());
                        break;
                    case ORDER_SIDE_FID:
                        mbp.setModelType(MarketByComponent.decodeValueToStr(fieldEntry));
                        break;
                    case QUOTIM_MS_FID:
                    case LV_TIM_MS:
                        final String msTime = MarketByComponent.decodeValueToStr(fieldEntry);
                        final long nano = TimeUnit.NANOSECONDS.convert(Long.parseLong(msTime), TimeUnit.MILLISECONDS);
                        mbp.setQuoteTime(LocalTime.ofNanoOfDay(nano).toString());
                        break;
                    case ACC_SIZE_FID:
                        mbp.setSize(fieldEntry.real().asDouble());
                        break;
                    case NO_ORD:
                        mbp.setNoOrd(MarketByComponent.decodeValueToStr(fieldEntry));
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
                    MarketByPriceModel mbp = null;
                    switch (mapEntry.action()) {
                        case MapEntry.MapAction.ADD:
                        case MapEntry.MapAction.UPDATE:
                            MarketByPriceModel current = centralMap.get(key);
                            if(current != null)
                                mbp = new MarketByPriceModel(current);
                            else {
                                mbp = new MarketByPriceModel(key);
                            }
                            break;
                    }
                    if (Objects.nonNull(mbp)) {
                        decodeMsgData(mapEntry.fieldList(), mbp);
                        centralMap.put(key, mbp);
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

    private void sortingOrders(ObservableList<MarketByPriceModel> observableList, String rankRule)
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

    private void handleCentralMapChanging(MapChangeListener.Change<? extends String, ? extends MarketByPriceModel> change) {
        if (Platform.isFxApplicationThread()) {
            MarketByPriceModel mbp = null;
            ObservableList<MarketByPriceModel> observableList = null;
            if (change.wasAdded() && change.wasRemoved()) {
                mbp = change.getValueAdded();
                observableList = strategyMap.get(mbp.getModelType());

                if(observableList != null) {
                    observableList.remove(change.getValueRemoved());
                    observableList.add(mbp);
                }
            } else if (change.wasAdded()) {
                mbp = change.getValueAdded();
                observableList = strategyMap.get(mbp.getModelType());

                if(observableList != null) {
                    observableList.add(mbp);
                }
            } else if (change.wasRemoved()) {
                mbp = change.getValueRemoved();
                observableList = strategyMap.get(mbp.getModelType());

                if(observableList != null) {
                    observableList.remove(mbp);
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