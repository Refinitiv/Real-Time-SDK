package com.refinitiv.ema.examples.rrtmdviewer.desktop.common.fxcomponents;

import com.sun.javafx.tk.FontMetrics;
import com.sun.javafx.tk.Toolkit;
import javafx.beans.property.StringProperty;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.control.ScrollBar;
import javafx.scene.control.TextField;
import javafx.scene.control.Tooltip;
import javafx.scene.layout.VBox;
import javafx.scene.text.Font;
import javafx.scene.text.FontWeight;

import java.io.IOException;

import static com.refinitiv.ema.examples.rrtmdviewer.desktop.common.ApplicationFxComponents.SCROLLABLE_TEXT_FIELD;

public class ScrollableTextField extends VBox {

    @FXML
    private TextField textField;

    @FXML
    private ScrollBar scrollBar;

    @FXML
    private VBox rootBox;

    private String tooltipText;

    private String prompt;

    private FontMetrics metrics = Toolkit.getToolkit().getFontLoader().getFontMetrics(Font.font("Segoe UI", FontWeight.NORMAL, 14));

    public ScrollableTextField() {
        FXMLLoader fxmlLoader = new FXMLLoader(SCROLLABLE_TEXT_FIELD.getResource());
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
        textField.setPrefHeight(35);
        scrollBar.setVisible(false);
        textField.textProperty().addListener(e -> adjustScrollBar());

        textField.caretPositionProperty().addListener((obs, oldVal, newVal) -> {
            if (scrollBar.isVisible()) {
                scrollBar.valueProperty().setValue(textField.getCaretPosition());
            }
        });

        scrollBar.setMin(0);
        scrollBar.valueProperty().addListener((obs, oldVal, newVal) -> {
            textField.positionCaret(newVal.intValue());
        });

        rootBox.widthProperty().addListener((obs, oldVal, newVal) -> {
            textField.setMinWidth(newVal.doubleValue());
            scrollBar.setMinWidth(newVal.doubleValue());
            textField.setMaxWidth(newVal.doubleValue());
            scrollBar.setMaxWidth(newVal.doubleValue());
            adjustScrollBar();
        });
    }

    public StringProperty textProperty() {
        return textField.textProperty();
    }

    public String getText() {
        return textField.getText();
    }

    public TextField getTextField() {
        return textField;
    }

    public void setText(String text) {
        textField.setText(text);
        adjustScrollBar();
    }

    public void adjustScrollBar(){
        double l = 0;
        for (int i = 0; i < textField.getText().length(); i++) {
            l += metrics.getCharWidth(textField.getText().charAt(i));
        }
        if (l > textField.getWidth()) {
            scrollBar.setMax(textField.getText().length());
            scrollBar.valueProperty().setValue(textField.getText().length());
            scrollBar.setVisible(true);
            textField.setPrefHeight(30);
            scrollBar.setPrefHeight(5);
            scrollBar.setMaxHeight(5);
        } else {
            scrollBar.setVisible(false);
            textField.setPrefHeight(35);
            scrollBar.setPrefHeight(0);
        }
    }

    public void setCustomWidth(double width) {
        textField.setPrefWidth(width);
        textField.setMinWidth(width);
        textField.setMaxWidth(width);
        scrollBar.setPrefWidth(width);
        scrollBar.setMinWidth(width);
        scrollBar.setMaxWidth(width);
    }

    public void setPrompt(String prompt) {
        textField.setPromptText(prompt);
    }

    public String getPrompt() {
        return prompt;
    }

    public void setTooltipText(String text) {
        Tooltip tooltip = new Tooltip(text);
        tooltip.setMaxWidth(300);
        tooltip.setWrapText(true);
        textField.setTooltip(tooltip);
    }

    public String getTooltipText() {
        return tooltipText;
    }
}
