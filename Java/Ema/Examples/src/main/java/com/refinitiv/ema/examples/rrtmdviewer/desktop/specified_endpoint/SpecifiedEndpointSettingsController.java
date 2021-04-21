package com.refinitiv.ema.examples.rrtmdviewer.desktop.specified_endpoint;

import com.refinitiv.ema.examples.rrtmdviewer.desktop.SceneController;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.ApplicationLayouts;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.ApplicationSingletonContainer;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.fxcomponents.DictionaryLoaderComponent;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.fxcomponents.ErrorDebugAreaComponent;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.fxcomponents.PasswordEyeComponent;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.model.ConnectionDataModel;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.model.EncryptionDataModel;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.OMMViewerError;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.SupportedJsonVersions;
import com.refinitiv.eta.codec.CodecReturnCodes;
import javafx.collections.FXCollections;
import javafx.concurrent.Task;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.*;
import javafx.scene.layout.VBox;

import java.io.File;
import java.util.Objects;
import java.util.concurrent.ExecutorService;

public class SpecifiedEndpointSettingsController {

    private static final String DEFAULT_APP_ID = "256";

    private static final String RWF = "rssl.rwf";
    private static final String JSON = "tr_json2,rssl.json.v2";

    private SceneController sceneController;
    private SpecifiedEndpointSettingsService specifiedEndpointSettingsService = new SpecifiedEndpointSettingsServiceImpl();

    private OMMViewerError error = new OMMViewerError();

    @FXML
    private TextField host;

    @FXML
    private TextField port;

    @FXML
    private TextField host2;

    @FXML
    private TextField port2;

    @FXML
    private ComboBox<SpecifiedEndpointConnectionTypes> connType;

    @FXML
    private Label protocolsLabel;

    @FXML
    private CheckBox rwfCheckbox;

    @FXML
    private CheckBox jsonCheckbox;

    @FXML
    private Label jsonVersions;

    @FXML
    private CheckBox encrypted;

    @FXML
    private TextField encrFile;

    @FXML
    private Button keyfileChooser;

    @FXML
    private Label passwLabel;

    @FXML
    private Label keyFileLabel;

    @FXML
    private PasswordEyeComponent keyPassword;

    @FXML
    private TextField appId;

    @FXML
    private TextField position;

    @FXML
    private TextField username;

    @FXML
    private DictionaryLoaderComponent dictionaryLoader;

    @FXML
    private VBox encryptedVBox;

    @FXML
    private MenuButton jsonVersionsMenu;

    @FXML
    private Button connect;

    @FXML
    private ErrorDebugAreaComponent errorDebugArea;

    private Tooltip encryptionTooltip;

    @FXML
    public void initialize() {

        sceneController = ApplicationSingletonContainer.getBean(SceneController.class);

        encryptionTooltip = new Tooltip();
        encryptionTooltip.setText("Set custom encryption options: key file and key password.\nThese parameters are optional.");
        encrypted.setTooltip(encryptionTooltip);

        setupVersionMenu();
    }

    @FXML
    public void onConnectionTypeChanged(ActionEvent event) {
        switch (connType.getSelectionModel().getSelectedItem()) {
            case SOCKET:
                jsonVersionsMenu.setDisable(true);
                jsonVersions.setDisable(true);
                jsonCheckbox.setSelected(false);
                rwfCheckbox.setSelected(true);
                jsonCheckbox.setDisable(true);
                rwfCheckbox.setDisable(true);
                protocolsLabel.setDisable(true);
                encryptedVBox.setVisible(false);
                break;
            case WEBSOCKET:
                jsonCheckbox.setDisable(false);
                rwfCheckbox.setDisable(false);
                protocolsLabel.setDisable(false);
                encryptedVBox.setVisible(false);
                break;
            case ENCRYPTED_SOCKET:
                jsonVersions.setDisable(true);
                jsonVersionsMenu.setDisable(true);
                jsonCheckbox.setSelected(false);
                rwfCheckbox.setSelected(true);
                jsonCheckbox.setDisable(true);
                rwfCheckbox.setDisable(true);
                protocolsLabel.setDisable(true);
                encryptedVBox.setVisible(true);
                break;
            case ENCRYPTED_WEBSOCKET:
                jsonCheckbox.setDisable(false);
                rwfCheckbox.setDisable(false);
                protocolsLabel.setDisable(false);
                encryptedVBox.setVisible(true);
                break;
            default:
                break;
        }
    }

    @FXML
    public void onJsonSelected(ActionEvent event) {
        if (jsonCheckbox.isSelected()) {
            jsonVersionsMenu.setDisable(false);
            jsonVersions.setDisable(false);
        } else {
            jsonVersionsMenu.setDisable(true);
            jsonVersions.setDisable(true);
        }
    }

    @FXML
    public void onEncryptionChanged(ActionEvent event) {
        if (encrypted.isSelected()) {
            keyFileLabel.setDisable(false);
            encrFile.setDisable(false);
            keyfileChooser.setDisable(false);
            passwLabel.setDisable(false);
            keyPassword.setDisable(false);
        } else {
            keyFileLabel.setDisable(true);
            encrFile.setDisable(true);
            keyfileChooser.setDisable(true);
            passwLabel.setDisable(true);
            keyPassword.setDisable(true);
        }
    }



    @FXML
    public void onKeyFileChooserBtnClick(ActionEvent event) {
        File keyFile = sceneController.showFileChooserWindow();
        if (keyFile != null && keyFile.getAbsolutePath() != null && !keyFile.getAbsolutePath().equals("")) {
            encrFile.setText(keyFile.getAbsolutePath());
        }
    }


    @FXML
    public void onSubmitBtnClick(ActionEvent event) {

        SpecifiedEndpointSettingsModel settings;
        SpecifiedEndpointSettingsModel.SpecifiedEndpointSettingsModelBuilder specifiedEndpointBuilder = SpecifiedEndpointSettingsModel.builder();

        /* Connectivity options */
        if(!host.getText().trim().isEmpty() && !port.getText().trim().isEmpty()) {
            specifiedEndpointBuilder.addServer(host.getText().trim(), port.getText().trim());
        }

        if(!host2.getText().trim().isEmpty() && !port2.getText().trim().isEmpty()) {
            specifiedEndpointBuilder.addServer(host2.getText().trim(), port2.getText().trim());
        }

        ConnectionDataModel.ConnectionDataModelBuilder connSettingsBuilder = ConnectionDataModel.builder();
        connSettingsBuilder.connectionType(connType.getSelectionModel().getSelectedItem());

        StringBuilder sb = new StringBuilder();
        if (rwfCheckbox.isSelected()) {
            appendProtocol(sb, RWF);
        }
        /* As for now, basically only one json version is supported */
        if (jsonCheckbox.isSelected()) {
            if (jsonVersionsMenu.getText() != null) {
                appendProtocol(sb, jsonVersionsMenu.getText());
            }
        }
        connSettingsBuilder.protocolList(sb.toString());

        specifiedEndpointBuilder.connectionSettings(connSettingsBuilder.build());

        if (isEncryptedConnection() && encrypted.isSelected()) {
            specifiedEndpointBuilder.hasCustomEncrOptions(true);
            EncryptionDataModel encryptModel = EncryptionDataModel.builder()
                    .keyFilePath(encrFile.getText())
                    .keyPassword(keyPassword.getPasswordField().getText())
                    .build();
            specifiedEndpointBuilder.encryptionSettings(encryptModel);
        }

        /* Application options */
        specifiedEndpointBuilder.applicationId(appId.getText() != null && !appId.getText().isEmpty() ? appId.getText() : DEFAULT_APP_ID);

        if(position.getText() != null) {
            specifiedEndpointBuilder.position(position.getText());
        }

        if(username.getText() != null) {
            specifiedEndpointBuilder.username(username.getText());
        }

        /*dictionary options*/
        specifiedEndpointBuilder.dictionaryData(dictionaryLoader.createDictionaryModel());

        settings = specifiedEndpointBuilder.build();
        settings.validate(error);
        dictionaryLoader.validate(error);
        if (!error.isFailed()) {
            SceneController sceneController = ApplicationSingletonContainer.getBean(SceneController.class);
            connect.setDisable(true);
            error.clear();

            Task<Boolean> connectTask = new Task<Boolean>() {
                @Override
                protected Boolean call() {
                    return specifiedEndpointSettingsService.connect(settings, error) != CodecReturnCodes.FAILURE;
                }
            };

            connectTask.setOnSucceeded(e -> {
                if (error.isFailed()) {
                    errorDebugArea.writeError(error.toString());
                    connect.setDisable(false);
                } else {
                    errorDebugArea.stopDebugStreaming();
                    connect.setDisable(false);
                    sceneController.addScene(ApplicationLayouts.ITEM_VIEW);
                    sceneController.showLayout(ApplicationLayouts.ITEM_VIEW);
                }
            });
            connectTask.setOnFailed(e -> {
                errorDebugArea.writeError("Failed to create OMM Consumer. Please try again.");
                connect.setDisable(false);
            });
            Objects.requireNonNull(ApplicationSingletonContainer.getBean(ExecutorService.class))
                    .submit(connectTask);
        } else {
            errorDebugArea.writeError(error.toString());
        }
    }

    private void appendProtocol(StringBuilder sb, String protocolText) {
        if (sb.length() > 0) {
            sb.append(",");
        }
        sb.append(protocolText);
    }

    private boolean isEncryptedConnection() {
        return connType.getSelectionModel().getSelectedItem().equals(SpecifiedEndpointConnectionTypes.ENCRYPTED_SOCKET)
                || connType.getSelectionModel().getSelectedItem().equals(SpecifiedEndpointConnectionTypes.ENCRYPTED_WEBSOCKET);
    }

    private void setupVersionMenu() {
        CheckMenuItem v2 = new CheckMenuItem(SupportedJsonVersions.TR_JSON.toString());
        v2.setMnemonicParsing(false);

        v2.setOnAction(e -> {
            StringBuilder sb = new StringBuilder();
            for (MenuItem item : jsonVersionsMenu.getItems()) {
                if (((CheckMenuItem)item).isSelected()) {
                    sb.append(item.getText());
                }
            }
            jsonVersionsMenu.setText(sb.toString());
            jsonVersionsMenu.setMnemonicParsing(false);
        });

        CheckMenuItem v3 = new CheckMenuItem(SupportedJsonVersions.RSSL_JSON.toString());

        v3.setOnAction(e -> {
            StringBuilder sb = new StringBuilder();
            for (MenuItem item : jsonVersionsMenu.getItems()) {
                if (((CheckMenuItem)item).isSelected()) {
                    sb.append(item.getText());
                    sb.append(',');
                }
            }

            if(sb.length() > 0) {
                sb.delete(sb.length() - 1, sb.length());
            }
            jsonVersionsMenu.setText(sb.toString());
        });


        jsonVersionsMenu.getItems().addAll(FXCollections.observableArrayList(v2, v3));
    }
}
