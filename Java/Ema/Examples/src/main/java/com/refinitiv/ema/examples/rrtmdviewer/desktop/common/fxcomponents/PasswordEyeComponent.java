package com.refinitiv.ema.examples.rrtmdviewer.desktop.common.fxcomponents;

import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.Group;
import javafx.scene.control.Button;
import javafx.scene.control.PasswordField;
import javafx.scene.control.TextField;
import javafx.scene.input.MouseEvent;

import java.io.IOException;

import static com.refinitiv.ema.examples.rrtmdviewer.desktop.common.ApplicationFxComponents.PASSWORD_EYE_COMPONENT;

public class PasswordEyeComponent extends Group {

    @FXML
    private PasswordField passwordField;

    @FXML
    private Button eyeLabel;

    @FXML
    private TextField textField;

    public PasswordEyeComponent() {
        FXMLLoader fxmlLoader = new FXMLLoader(PASSWORD_EYE_COMPONENT.getResource());
        fxmlLoader.setRoot(this);
        fxmlLoader.setController(this);
        try {
            fxmlLoader.load();
        } catch (IOException exception) {
            throw new RuntimeException(exception);
        }
    }

    @FXML
    public void handleEyePressed(MouseEvent e) {
        passwordField.setVisible(false);
        textField.setVisible(true);
        textField.setText(passwordField.getText());
    }

    @FXML
    public void handleEyeRelease(MouseEvent e) {
        textField.clear();
        textField.setVisible(false);
        passwordField.setVisible(true);
    }

    public void setPopulatedValue(String promptValue) {
        this.passwordField.setPromptText(promptValue);
    }

    public String getPopulatedValue() {
        return this.passwordField.getPromptText();
    }

    public PasswordField getPasswordField() {
        return passwordField;
    }

    public void setCustomWidth(double width) {
        textField.setMinWidth(width - 35);
        textField.setMaxWidth(width - 35);
        passwordField.setMinWidth(width - 35);
        passwordField.setMaxWidth(width - 35);
    }
}
