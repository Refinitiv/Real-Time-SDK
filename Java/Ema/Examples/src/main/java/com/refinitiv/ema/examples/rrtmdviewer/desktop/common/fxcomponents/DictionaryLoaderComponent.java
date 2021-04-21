package com.refinitiv.ema.examples.rrtmdviewer.desktop.common.fxcomponents;

import com.refinitiv.ema.examples.rrtmdviewer.desktop.SceneController;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.ApplicationSingletonContainer;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.OMMViewerError;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.model.DictionaryDataModel;
import javafx.beans.property.DoubleProperty;
import javafx.beans.property.SimpleDoubleProperty;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.control.Button;
import javafx.scene.control.CheckBox;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.layout.VBox;

import java.io.File;
import java.io.IOException;
import java.util.Objects;
import java.util.Optional;

import static com.refinitiv.ema.examples.rrtmdviewer.desktop.common.ApplicationFxComponents.DICTIONARY_LOADER_COMPONENT;

public class DictionaryLoaderComponent extends VBox {

    private static final Double DEFAULT_FIELD_WIDTH = 200D;

    @FXML
    private CheckBox downloadDictCheckbox;

    @FXML
    private Label fieldDictLabel;

    @FXML
    private TextField fieldDictFile;

    @FXML
    private Button fieldDictFileBtn;

    @FXML
    private Label enumDictLabel;

    @FXML
    private TextField enumDictFile;

    @FXML
    private Button enumDictFileBtn;

    private final DoubleProperty fieldWidth = new SimpleDoubleProperty();

    public DictionaryLoaderComponent() {
        FXMLLoader fxmlLoader = new FXMLLoader(DICTIONARY_LOADER_COMPONENT.getResource());
        fxmlLoader.setRoot(this);
        fxmlLoader.setController(this);
        try {
            fxmlLoader.load();
        } catch (IOException exception) {
            throw new RuntimeException(exception);
        }
    }

    @FXML
    public void initialize() {
        fieldWidth.addListener((observable, oldValue, newValue) -> {
            fieldDictFile.setStyle("-fx-max-width: " + newValue);
            fieldDictFile.setStyle("-fx-min-width: " + newValue);
            enumDictFile.setStyle("-fx-max-width: " + newValue);
            enumDictFile.setStyle("-fx-min-width: " + newValue);
            this.setWidth(this.getWidth() - (DEFAULT_FIELD_WIDTH - newValue.doubleValue()));
        });
    }

    @FXML
    public void onDownloadDictionaryChanged(ActionEvent event) {

        if (downloadDictCheckbox.isSelected()) {
            fieldDictLabel.setDisable(true);
            fieldDictFile.setDisable(true);
            fieldDictFileBtn.setDisable(true);
            enumDictLabel.setDisable(true);
            enumDictFile.setDisable(true);
            enumDictFileBtn.setDisable(true);
        } else {
            fieldDictLabel.setDisable(false);
            fieldDictFile.setDisable(false);
            fieldDictFileBtn.setDisable(false);
            enumDictLabel.setDisable(false);
            enumDictFile.setDisable(false);
            enumDictFileBtn.setDisable(false);
        }
    }


    @FXML
    public void onFieldDictChooserBtnClick(ActionEvent event) {
        final SceneController sceneController = ApplicationSingletonContainer.getBean(SceneController.class);
        File keyFile = sceneController.showFileChooserWindow();
        if (keyFile != null && !keyFile.getAbsolutePath().isEmpty()) {
            fieldDictFile.setText(keyFile.getAbsolutePath());
        }
    }

    @FXML
    public void onEnumDictChooserBtnClick(ActionEvent event) {
        final SceneController sceneController = ApplicationSingletonContainer.getBean(SceneController.class);
        File keyFile = sceneController.showFileChooserWindow();
        if (keyFile != null && !keyFile.getAbsolutePath().isEmpty()) {
            enumDictFile.setText(keyFile.getAbsolutePath());
        }
    }

    public DictionaryDataModel createDictionaryModel() {
        return DictionaryDataModel.builder()
                .downloadFromNetwork(downloadDictCheckbox.isSelected())
                .fieldDictionaryPath(
                        Optional.ofNullable(fieldDictFile.getText())
                                .filter((d) -> !downloadDictCheckbox.isSelected())
                                .orElse(null)
                )
                .enumDictionaryPath(
                        Optional.ofNullable(enumDictFile.getText())
                                .filter((d) -> !downloadDictCheckbox.isSelected())
                                .orElse(null)
                ).build();
    }

    public boolean validate(OMMViewerError error) {
        if (!downloadDictCheckbox.isSelected()) {
            if (Objects.isNull(fieldDictFile.getText()) || fieldDictFile.getText().trim().isEmpty()
                    || Objects.isNull(enumDictFile.getText()) || enumDictFile.getText().trim().isEmpty()) {
                error.setFailed(true);
                error.appendErrorText("Dictionary file paths must not be empty.");
                return true;
            }
        }
        return false;
    }

    public CheckBox getDownloadDictCheckbox() {
        return downloadDictCheckbox;
    }

    public TextField getFieldDictFile() {
        return fieldDictFile;
    }

    public TextField getEnumDictFile() {
        return enumDictFile;
    }

    public void setFieldWidth(double widthInPixels) {
        this.fieldWidth.set(widthInPixels);
    }

    public double getFieldWidth() {
        return fieldWidth.get();
    }
}
