/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
 * ModuleJsonInfo
 *
 * @since 2024-06-18
 */
public class ModuleJsonInfo {
    private String moduleType = "";
    private String moduleName = "";
    private boolean compressNativeLibs = false;
    private boolean generateBuildHash = false;
    private List<String> skillProfileNames = new ArrayList<>();

    /**
     * Get the type of the module.
     *
     * @return the type of the module
     */
    public String getModuleType() {
        return moduleType;
    }

    /**
     * Set the type of the module.
     *
     * @param moduleType the type of the module
     */
    public void setModuleType(String moduleType) {
        this.moduleType = moduleType;
    }

    /**
     * Get the name of the module.
     *
     * @return the name of the module
     */
    public String getModuleName() {
        return moduleName;
    }

    /**
     * Set the name of the module.
     *
     * @param moduleName the name of the module
     */
    public void setModuleName(String moduleName) {
        this.moduleName = moduleName;
    }

    /**
     * Check whether the native libs are compressed.
     *
     * @return true if the native libs are compressed
     */
    public boolean isCompressNativeLibs() {
        return compressNativeLibs;
    }

    /**
     * Set whether the native libs are compressed.
     *
     * @param compressNativeLibs whether the native libs are compressed
     */
    public void setCompressNativeLibs(boolean compressNativeLibs) {
        this.compressNativeLibs = compressNativeLibs;
    }

    /**
     * Check whether the build hash is generated.
     *
     * @return true if the build hash is generated
     */
    public boolean isGenerateBuildHash() {
        return generateBuildHash;
    }

    /**
     * Set whether the build hash is generated.
     *
     * @param generateBuildHash whether the build hash is generated
     */
    public void setGenerateBuildHash(boolean generateBuildHash) {
        this.generateBuildHash = generateBuildHash;
    }

    /**
     * check the module type is shared or not
     *
     * @return true if module type is shared
     */
    public boolean isShared() {
        return moduleType != null && moduleType.equals(Constants.TYPE_SHARED);
    }

    /**
     * Get the skill profile names of the module.
     *
     * @return the skill profile names of the module
     */
    public List<String> getSkillProfileNames() {
        return skillProfileNames;
    }

    /**
     * Set the skill profile names of the module.
     *
     * @param skillProfileNames the skill profile names of the module
     */
    public void setSkillProfileNames(List<String> skillProfileNames) {
        this.skillProfileNames = skillProfileNames;
    }
}
