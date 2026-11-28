/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

/**
 * Collection of members in ModuleApiVersion.
 */
class ModuleApiVersion {
    /**
     * Indicates the compatibleApiVersion of app.
     */
    private int compatibleApiVersion = -1;

    /**
     * Indicates the compatibleApiVersion of app.
     */
    private int targetApiVersion = -1;

    /**
     * Indicates the releaseType of app.
     */
    private String releaseType = "";

    /**
     * get the release type of api version.
     */
    public String getReleaseType() {
        return releaseType;
    }

    /**
     * set the release type of api version.
     */
    public void setReleaseType(String releaseType) {
        this.releaseType = releaseType;
    }

    /**
     * get the compatible api version.
     */
    public int getCompatibleApiVersion() {
        return compatibleApiVersion;
    }

    /**
     * set the compatible api version.
     */
    public void setCompatibleApiVersion(int compatibleApiVersion) {
        this.compatibleApiVersion = compatibleApiVersion;
    }

    /**
     * get the target api version.
     */
    public int getTargetApiVersion() {
        return targetApiVersion;
    }

    /**
     * set the target api version.
     */
    public void setTargetApiVersion(int targetApiVersion) {
        this.targetApiVersion = targetApiVersion;
    }
}
