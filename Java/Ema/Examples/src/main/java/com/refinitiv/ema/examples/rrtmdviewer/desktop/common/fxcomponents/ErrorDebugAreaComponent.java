/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rrtmdviewer.desktop.common.fxcomponents;

import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.ApplicationSingletonContainer;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.DebugAreaStream;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.control.CheckBox;
import javafx.scene.control.TextArea;
import javafx.scene.layout.VBox;

import java.io.IOException;

import static com.refinitiv.ema.examples.rrtmdviewer.desktop.common.ApplicationFxComponents.ERROR_DEBUG_AREA_COMPONENT;

public class ErrorDebugAreaComponent extends VBox {

    private final DebugAreaStream debugAreaStream;

    @FXML
    private CheckBox debugCheckbox;

    @FXML
    private TextArea errorDebugAreaInternal;

    public ErrorDebugAreaComponent() {
        debugAreaStream = ApplicationSingletonContainer.getBean(DebugAreaStream.class);
        FXMLLoader fxmlLoader = new FXMLLoader(ERROR_DEBUG_AREA_COMPONENT.getResource());
        fxmlLoader.setRoot(this);
        fxmlLoader.setController(this);
        try {
            fxmlLoader.load();
        } catch (IOException exception) {
            throw new RuntimeException(exception);
        }
    }

    @FXML
    public void handleDebugCheckbox(ActionEvent event) {
        processDebug();
    }

    public void processDebug() {
        if (debugCheckbox.isSelected()) {
            debugAreaStream.enable(errorDebugAreaInternal);
        } else {
            debugAreaStream.disable();
        }
    }

    public void writeError(String s) {
        errorDebugAreaInternal.appendText(s);
    }

    public void setAreaHeight(double areaHeight) {
        errorDebugAreaInternal.setMinHeight(areaHeight);
    }

    public double getAreaHeight() {
        return errorDebugAreaInternal.getPrefHeight();
    }

    public void stopDebugStreaming() {
        debugAreaStream.clear();
        errorDebugAreaInternal.clear();
    }

    public void clearDebugArea() {
        errorDebugAreaInternal.clear();
    }
}
