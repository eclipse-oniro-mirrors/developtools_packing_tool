/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package ohos;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * module Shortcut info.
 *
 */
class ModuleShortcut {
    /**
     * Indicates the shortcutId of module Shortcut.
     */
    private String shortcutId = "";

    /**
     * Indicates the label of module Shortcut.
     */
    private String label = "";

    private HashMap<String, String> labels = new HashMap<>();

    /**
     * Indicates the icon of module Shortcut.
     */
    private String icon = "";

    /**
     * Indicates the intents of module Shortcut.
     */
    private List<Want> wants = new ArrayList<Want>();

    /**
     * get the shortcut id of module shortcut.
     */
    public String getShortcutId() {
        return shortcutId;
    }

    /**
     * set the shortcut id of module shortcut.
     */
    public void setShortcutId(String shortcutId) {
        this.shortcutId = shortcutId;
    }

    /**
     * get the label of module shortcut.
     */
    public String getLabel() {
        return label;
    }

    /**
     * set the label of module shortcut.
     */
    public void setLabel(String label) {
        this.label = label;
    }

    /**
     * get the labels of module shortcut.
     */
    public HashMap<String, String> getLabels() {
        return labels;
    }

    /**
     * set the labels of module shortcut.
     */
    public void setLabels(HashMap<String, String> labels) {
        this.labels = labels;
    }

    /**
     * get the icon of module shortcut.
     */
    public String getIcon() {
        return icon;
    }

    /**
     * set the icon of module shortcut.
     */
    public void setIcon(String icon) {
        this.icon = icon;
    }

    /**
     * get the wants of module shortcut.
     */
    public List<Want> getWants() {
        return wants;
    }

    /**
     * set the wants of module shortcut.
     */
    public void setWants(List<Want> wants) {
        this.wants = wants;
    }
}
