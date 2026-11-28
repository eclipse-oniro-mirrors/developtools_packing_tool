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

import java.util.List;
import java.util.ArrayList;

/**
 * Module result.
 *
 */
class ModuleResult {
    /**
     * Indicates the packages in pack.info.
     */
    public List<PackInfo> packInfos = new ArrayList<PackInfo>();
    /**
     * Indicates the ModuleProfileInfo of ModuleJson.
     */
    public List<ModuleProfileInfo> moduleProfileInfos = new ArrayList<ModuleProfileInfo>();
    /**
     * Indicates the ModuleProfileInfo of ModuleJson.
     */
    public List<String> moduleProfileStr = new ArrayList<String>();
    /**
     * Indicates the icon of Module.
     */
    private String icon = "";
    /**
     * Indicates the label of Module.
     */
    private String label = "";
    /**
     * Indicates the result of ModuleJson.
     */
    private boolean result = true;
    /**
     * Indicates the message of ModuleJson.
     */
    private String message = "Success";

    /**
     * get the result of ModuleJson.
     */
    public boolean getResult() {
        return result;
    }

    /**
     * set the result of ModuleJson.
     */
    public void setResult(boolean result) {
        this.result = result;
    }

    /**
     * get the message of ModuleJson.
     */
    public String getMessage() {
        return message;
    }

    /**
     * set the message of ModuleJson.
     */
    public void setMessage(String message) {
        this.message = message;
    }

    /**
     * get the icon of ModuleJson.
     */
    public String getIcon() {
        return icon;
    }

    /**
     * set the icon of ModuleJson.
     */
    public void setIcon(String iconPath) {
        icon = iconPath;
    }

    /**
     * get the label of ModuleJson.
     */
    public String getLabel() {
        return label;
    }

    /**
     * set the label of ModuleJson.
     */
    public void setLabel(String labelRes) {
        label = labelRes;
    }

    /**
     * Add the ModuleProfileInfo to the profileInfoStr list.
     *
     * @param moduleProfileInfo the moduleProfileInfo to be added.
     */
    public void addModuleProfileInfo(ModuleProfileInfo moduleProfileInfo) {
        if (moduleProfileInfos != null && moduleProfileInfo != null) {
            this.moduleProfileInfos.add(moduleProfileInfo);
        }
    }
}
