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
import java.util.List;

/**
 * collection of HQF verify info.
 */
public class HQFInfo {
    private String bundleName = "";

    private int versionCode = -1;

    private String versionName = "";

    private int patchVersionCode = -1;

    private String patchVersionName = "";

    private String moduleName = "";

    private String type = "";

    private List<String> deviceTypes = new ArrayList<>();

    private String originalModuleHash = "";

    /**
     * get the bundle name of hqf.
     */
    public String getBundleName() {
        return bundleName;
    }

    /**
     * set the bundle name of hqf.
     */
    public void setBundleName(String bundleName) {
        this.bundleName = bundleName;
    }

    /**
     * get the version code of hqf.
     */
    public int getVersionCode() {
        return versionCode;
    }

    /**
     * set the version code of hqf.
     */
    public void setVersionCode(int versionCode) {
        this.versionCode = versionCode;
    }

    /**
     * get the version name of hqf.
     */
    public String getVersionName() {
        return versionName;
    }

    /**
     * set the version name of hqf.
     */
    public void setVersionName(String versionName) {
        this.versionName = versionName;
    }

    /**
     * get the patch version code of hqf.
     */
    public int getPatchVersionCode() {
        return patchVersionCode;
    }

    /**
     * set the patch version code of hqf.
     */
    public void setPatchVersionCode(int patchVersionCode) {
        this.patchVersionCode = patchVersionCode;
    }

    /**
     * get the patch version name of hqf.
     */
    public String getPatchVersionName() {
        return patchVersionName;
    }

    /**
     * set the patch version name of hqf.
     */
    public void setPatchVersionName(String patchVersionName) {
        this.patchVersionName = patchVersionName;
    }

    /**
     * get the module name of hqf.
     */
    public String getModuleName() {
        return moduleName;
    }

    /**
     * set the module name of hqf.
     */
    public void setModuleName(String moduleName) {
        this.moduleName = moduleName;
    }

    /**
     * get the type of hqf.
     */
    public String getType() {
        return type;
    }

    /**
     * set the type of hqf.
     */
    public void setType(String type) {
        this.type = type;
    }

    /**
     * get the device types of hqf.
     */
    public List<String> getDeviceTypes() {
        return deviceTypes;
    }

    /**
     * set the device types of hqf.
     */
    public void setDeviceTypes(List<String> deviceTypes) {
        this.deviceTypes = deviceTypes;
    }

    /**
     * get the original module hash of hqf.
     */
    public String getOriginalModuleHash() {
        return originalModuleHash;
    }

    /**
     * set the original module hash of hqf.
     */
    public void setOriginalModuleHash(String originalModuleHash) {
        this.originalModuleHash = originalModuleHash;
    }
}
