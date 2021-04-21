package com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview.fx;

import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview.ItemNotificationModel;
import com.refinitiv.eta.codec.*;
import javafx.application.Platform;
import javafx.beans.binding.Bindings;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.control.*;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.Pane;

import java.io.IOException;
import java.util.*;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Consumer;

import static com.refinitiv.ema.examples.rrtmdviewer.desktop.common.ApplicationFxComponents.MARKET_PRICE_COMPONENT;

public class MarketPriceComponent extends Pane implements ItemFxComponent {

    private static final String ID_COLUMN = "RIC";

    private final Map<Long, ItemNotificationModel> lastItemNotificationStorage = new ConcurrentHashMap<>();

    private Tab parent;

    @FXML
    private TableView<MarketPriceTableItem> table;

    @FXML
    private ListView<String> statusLabel;

    @FXML
    private Button unregisterButton;

    private Map<Long, MarketPriceTableItem> rows = new HashMap<>();
    private Set<String> fids = new HashSet<>();
    private final ObservableList<MarketPriceTableItem> tableItems = FXCollections.observableArrayList();

    private StatusHelper statusHelper;

    private final static int RMTES_BUFFER_LENGTH = 50;

    public MarketPriceComponent(Tab parent) {

        FXMLLoader fxmlLoader = new FXMLLoader(MARKET_PRICE_COMPONENT.getResource());
        fxmlLoader.setRoot(this);
        fxmlLoader.setController(this);
        try {
            fxmlLoader.load();
            this.parent = parent;
        } catch (IOException ex) {
            throw new RuntimeException(ex);
        }
    }

    @FXML
    public void initialize() {
        this.widthProperty().addListener(e -> {
            table.setPrefWidth(this.getWidth() - 5);
        });
        statusHelper = new StatusHelper(statusLabel);
        table.setItems(tableItems);
        table.setRowFactory((table) -> new MarketPriceRow());
    }

    private void addFidColumn(MarketPriceTableItem item, String key, String value) {
        if (!fids.contains(key)) {
            final TableColumn<MarketPriceTableItem, String> tableColumnField = new TableColumn<>(key);
            tableColumnField.setCellValueFactory(ce -> Bindings.createStringBinding(() -> ce.getValue().getColumns().getOrDefault(ce.getTableColumn().getText(), "")));
            table.getColumns().add(tableColumnField);
            fids.add(key);
        }
        item.getColumns().put(key, value);
    }

    public Set<Long> getCurrentHandles() {
        return rows.keySet();
    }

    public void configureButton(String name, Consumer<MouseEvent> func) {
        unregisterButton.setText(name);
        unregisterButton.setOnMouseClicked(func::accept);
    }

    public void setDisableBtn(boolean value) {
        unregisterButton.setDisable(value);
    }

    public void clear() {
        rows.clear();
        fids.clear();
        tableItems.clear();
        table.refresh();
        table.getColumns().clear();
        lastItemNotificationStorage.clear();
    }

    @Override
    public void handleItem(ItemNotificationModel notification) {
        lastItemNotificationStorage.put(notification.getHandle(), notification);
        handleMsgState(notification);
        if (parent.isSelected()) {
            if (Platform.isFxApplicationThread()) {
                updateRows(notification);
            } else {
                Platform.runLater(() -> {
                    updateRows(notification);
                });
            }
        }
    }

    @Override
    public void handleMsgState(ItemNotificationModel notification) {
        if (!Platform.isFxApplicationThread()) {
            Platform.runLater(() -> handleMsgState(notification));
            return;
        }
        statusHelper.updateStatus(notification);
        if (statusHelper.hasOmmState(notification)) {
            final MarketPriceTableItem mp = notification.getMsg().hasName()
                    ? loadItem(notification.getHandle(), notification.getMsg())
                    : rows.get(notification.getHandle());
            if (Objects.nonNull(mp)) {
                mp.setStreaming(notification.getRequest().getView().isSnapshot() || statusHelper.isGoodState(notification));
                updateMarketPriceItem(mp);
            }

        }
    }

    @Override
    public void updateUsingLastActualInfo() {
        lastItemNotificationStorage.forEach((key, value) -> {
            if (Objects.nonNull(value)) {
                updateRows(value);
            }
        });
    }

    private void updateRows(ItemNotificationModel notification) {
        final Msg msg = notification.getMsg();
        final MarketPriceTableItem tableItem = loadItem(notification.getHandle(), msg);
        if (msg.payload().dataType() == DataType.DataTypes.FIELD_LIST) {
            Iterator<FieldEntry> iter = msg.payload().fieldList().iterator();
            FieldEntry fieldEntry;
            while (iter.hasNext()) {
                fieldEntry = iter.next();

                if(fieldEntry.loadType() == DataType.DataTypes.ENUM && fieldEntry.hasEnumDisplay()) {
                    RmtesCacheBuffer rmtesCache = CodecFactory.createRmtesCacheBuffer(RMTES_BUFFER_LENGTH);
                    RmtesBuffer rmtesBuffer =CodecFactory.createRmtesBuffer(RMTES_BUFFER_LENGTH);
                    RmtesDecoder decoder = CodecFactory.createRmtesDecoder();

                    Buffer data = CodecFactory.createBuffer();
                    String enmStr = fieldEntry.enumDisplay();
                    data.data(java.nio.ByteBuffer.wrap(enmStr.getBytes(java.nio.charset.StandardCharsets.ISO_8859_1)));

                    // apply RMTES content to cache, if successful convert to UCS-2
                    if ( decoder.RMTESApplyToCache(data, rmtesCache) >= CodecReturnCodes.SUCCESS)
                    {
                        if (decoder.RMTESToUCS2(rmtesBuffer, rmtesCache) >= CodecReturnCodes.SUCCESS)
                        {
                            addFidColumn(tableItem, fieldEntry.name(), rmtesBuffer.toString());
                        }
                        else
                        {
                            addFidColumn(tableItem, fieldEntry.name(), fieldEntry.enumDisplay());
                        }
                    }
                    else {
                        addFidColumn(tableItem, fieldEntry.name(), fieldEntry.enumDisplay());
                    }
                }
                else {
                    addFidColumn(tableItem, fieldEntry.name(), fieldEntry.load().toString());
                }
            }
            updateMarketPriceItem(tableItem);
        }
    }

    private MarketPriceTableItem loadItem(long handle, Msg msg) {
        MarketPriceTableItem tableItem;
        if (rows.containsKey(handle)) {
            tableItem = rows.get(handle);
        } else {
            tableItem = new MarketPriceTableItem();
            tableItem.setHandle(handle);
            tableItem.setId(msg.name());
            tableItem.setItemIndex(tableItems.size());
            addFidColumn(tableItem, ID_COLUMN, msg.name());
            rows.put(handle, tableItem);
            tableItems.add(tableItem);
        }
        return tableItem;
    }

    private void updateMarketPriceItem(MarketPriceTableItem item) {
        tableItems.set(item.getItemIndex(), item);
    }

    private static class MarketPriceRow extends TableRow<MarketPriceTableItem> {

        private boolean isStreaming = true;
        private static final String ROW_STATE_CSS = "mp-row-red-value";

        @Override
        protected void updateItem(MarketPriceTableItem item, boolean empty) {
            super.updateItem(item, empty);
            if (empty || Objects.isNull(item)) {
                return;
            }
            if (item.isStreaming() && !isStreaming) {
                getStyleClass().remove(ROW_STATE_CSS);
                isStreaming = item.isStreaming();
            } else if (!item.isStreaming() && isStreaming) {
                getStyleClass().add(ROW_STATE_CSS);
                isStreaming = item.isStreaming();
            }
        }
    }
}
