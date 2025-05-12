/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.common.fxcomponents;

import javafx.beans.property.StringProperty;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.control.ScrollBar;
import javafx.scene.control.TextField;
import javafx.scene.control.Tooltip;
import javafx.scene.layout.VBox;

import java.io.IOException;

import static com.refinitiv.ema.examples.rtviewer.desktop.common.ApplicationFxComponents.SCROLLABLE_TEXT_FIELD;

public class ScrollableTextField extends VBox {

    @FXML
    private TextField textField;

    @FXML
    private ScrollBar scrollBar;

    @FXML
    private VBox rootBox;

    private String tooltipText;

    private String prompt;

    /* Holds widths of ASCII characters for Segoe UI font family with font size 14pt
       In case of font size or family changes update this array as well */
    private static final double[] asciiCharWidths14ptSegoeUI = { 3.834961, 3.9785156, 5.489258, 8.271484, 7.546875, 11.457031, 11.204102, 3.2197266, 4.2246094, 4.2246094,
            5.8378906, 9.577148, 3.0351562, 5.598633, 3.0351562, 5.455078, 7.546875, 7.546875, 7.546875, 7.546875, 7.546875, 7.546875, 7.546875, 7.546875,
            7.546875, 7.546875, 3.0351562, 3.0351562, 9.577148, 9.577148, 9.577148, 6.2753906, 13.371094, 9.030273, 8.025391, 8.667969, 9.816406, 7.0820312,
            6.8359375, 9.604492, 9.939453, 3.725586, 4.9970703, 8.121094, 6.5898438, 12.571289, 10.472656, 10.5546875, 7.8408203, 10.5546875, 8.374023,
            7.4375, 7.334961, 9.618164, 8.6953125, 13.077148, 8.2578125, 7.7382812, 7.984375, 4.2246094, 5.3046875, 4.2246094, 9.577148, 5.810547, 3.7529297,
            7.123047, 8.230469, 6.466797, 8.244141, 7.321289, 4.381836, 8.244141, 7.9228516, 3.390625, 3.390625, 6.9589844, 3.390625, 12.058594, 7.9228516, 8.203125,
            8.230469, 8.244141, 4.8671875, 5.9404297, 4.7441406, 7.9228516, 6.7060547, 10.1171875, 6.4257812, 6.774414, 6.330078, 4.2246094, 3.3496094, 4.2246094, 9.577148 };

    private static final double DEFAULT_CHAR_WIDTH = 11;

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
            if (textField.getText().charAt(i) < 32 || textField.getText().charAt(i) > 126) {
                l += DEFAULT_CHAR_WIDTH;
            } else {
                l += asciiCharWidths14ptSegoeUI[textField.getText().charAt(i) - 32];
            }
        }
        if (l > textField.getWidth() - 5) {
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
