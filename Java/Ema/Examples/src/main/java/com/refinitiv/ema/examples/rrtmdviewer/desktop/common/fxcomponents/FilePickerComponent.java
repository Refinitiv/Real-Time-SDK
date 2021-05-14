package com.refinitiv.ema.examples.rrtmdviewer.desktop.common.fxcomponents;

import com.refinitiv.ema.examples.rrtmdviewer.desktop.SceneController;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.ApplicationSingletonContainer;
import javafx.fxml.FXML;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.control.Tooltip;
import javafx.scene.layout.VBox;

import java.io.File;

import static com.refinitiv.ema.examples.rrtmdviewer.desktop.common.ApplicationFxComponents.FILE_PICKER_COMPONENT;

public class FilePickerComponent extends VBox {

    @FXML
    private Label filePickerLabel;

    @FXML
    private TextField filePickerTextField;

    public FilePickerComponent() {
        SceneController.loadComponent(FILE_PICKER_COMPONENT, this);
    }

    @FXML
    public void handleFilePickerButton() {
        final SceneController sceneController = ApplicationSingletonContainer.getBean(SceneController.class);
        File keyFile = sceneController.showFileChooserWindow();
        if (keyFile != null) {
            filePickerTextField.setText(keyFile.getAbsolutePath());
        }
    }

    public void setLabel(String label) {
        filePickerLabel.setText(label);
    }

    public String getLabel() {
        return filePickerLabel.getText();
    }

    public void setTextFieldLength(double length) {
        this.filePickerTextField.setPrefWidth(length);
        this.filePickerTextField.setMinWidth(length);
    }

    public double getTextFieldLength() {
        return filePickerTextField.getPrefWidth();
    }

    public TextField getFilePickerTextField() {
        return filePickerTextField;
    }

    public void setFilePickerTooltip(Tooltip tooltip) {
        this.filePickerTextField.setTooltip(tooltip);
    }

    public Tooltip getFilePickerTooltip() {
        return this.filePickerTextField.getTooltip();
    }
}
