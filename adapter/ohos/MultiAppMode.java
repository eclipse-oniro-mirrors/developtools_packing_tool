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

import java.util.Objects;

public class MultiAppMode {
    private String multiAppModeType = "";
    private Integer maxCount = 0;

    /**
     * get the type of multi app mode.
     */
    public String getMultiAppModeType() {
        return multiAppModeType;
    }

    /**
     * set the type of multi app mode.
     */
    public void setMultiAppModeType(String multiAppModeType) {
        this.multiAppModeType = multiAppModeType;
    }

    /**
     * get the max count of multi app mode.
     */
    public Integer getMaxCount() {
        return maxCount;
    }

    /**
     * set the max count of multi app mode.
     */
    public void setMaxCount(Integer maxCount) {
        this.maxCount = maxCount;
    }

    /**
     * compare this multi app mode with the specified object.
     *
     * @param o the object to be compared
     * @return true if the given object is a multi app mode with the same fields
     */
    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        MultiAppMode that = (MultiAppMode) o;
        return Objects.equals(multiAppModeType, that.multiAppModeType) && Objects.equals(maxCount, that.maxCount);
    }

    /**
     * compute the hash code of the multi app mode.
     *
     * @return the hash code of the multi app mode
     */
    @Override
    public int hashCode() {
        int result = Objects.hashCode(multiAppModeType);
        result = 31 * result + Objects.hashCode(maxCount);
        return result;
    }
}
