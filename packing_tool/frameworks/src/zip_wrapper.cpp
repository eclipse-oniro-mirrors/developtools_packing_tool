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

#include <fstream>
#include <iostream>
#include "zip_wrapper.h"
#include "log.h"
#include "app_log_wrapper.h"

namespace OHOS {
namespace AppPackingTool {
namespace {}

ZipWrapper::ZipWrapper() : zipFile_(nullptr)
{}

ZipWrapper::ZipWrapper(std::string zipPath) : zipFile_(nullptr), zipFilePath_(zipPath)
{}

ZipWrapper::~ZipWrapper()
{
    Close();
}

int ZipWrapper::Open(std::string& zipPath, int append)
{
    zipFilePath_ = zipPath;
    return Open(append);
}

int ZipWrapper::Open(int append)
{
    if (zipFile_ != nullptr) {
        LOGE("zip file handle has open");
        return ZIP_ERR_SUCCESS;
    }
    zipFile_ = zipOpen64(zipFilePath_.c_str(), append);
    if (zipFile_ == nullptr) {
        LOGE("zip file handle open failed");
        return ZIP_ERR_FAILURE;
    }
    return ZIP_ERR_SUCCESS;
}

void ZipWrapper::Close()
{
    if (zipFile_ != nullptr) {
        zipClose(zipFile_, nullptr);
        zipFile_ = nullptr;
    }
}

int ZipWrapper::AddFileOrDirectoryToZip(const std::string &filePath, const std::string &zipPath)
{
    return AddFileOrDirectoryToZip(fs::path(filePath), fs::path(zipPath));
}

int ZipWrapper::AddFileOrDirectoryToZip(const fs::path &fsFilePath, const fs::path &fsZipPath)
{
    APP_LOGD("Add [%{public}s] into zip[%{public}s]", fsFilePath.string().c_str(), fsZipPath.string().c_str());
    LOGI("Add [%s] into zip[%s]", fsFilePath.string().c_str(), fsZipPath.string().c_str());
    int ret = ZIP_ERR_SUCCESS;
    if (fs::is_directory(fsFilePath)) {
        LOGI("[%s] is directory to [%s]", fsFilePath.string().c_str(), fsZipPath.string().c_str());
        int count = 0;
        for (const auto &entry : fs::directory_iterator(fsFilePath)) {
            fs::path fsZipFullPath = fsZipPath / entry.path().filename();
            LOGE("Add File %s from %s", entry.path().string().c_str(), fsZipFullPath.string().c_str());
            ret = AddFileOrDirectoryToZip(entry.path(), fsZipFullPath);
            if (ret != ZIP_ERR_SUCCESS) {
                LOGE("AddFileOrDirectoryToZip failed![%s]", fsFilePath.string().c_str());
                return ret;
            }
            count++;
        }
        if (count == 0) {
            ret = AddEmptyDirToZip(fsZipPath);
            if (ret != ZIP_ERR_SUCCESS) {
                LOGE("AddDirToZip failed![%s]", fsFilePath.string().c_str());
                return ret;
            }
        }
    } else if (fs::is_regular_file(fsFilePath)) {
        ret = AddFileToZip(fsFilePath, fsZipPath);
        if (ret != ZIP_ERR_SUCCESS) {
            LOGE("AddFileToZip failed![%s]", fsFilePath.string().c_str());
            return ret;
        }
    }
    return ZIP_ERR_SUCCESS;
}

int ZipWrapper::WriteStringToZip(const std::string &content, const std::string& zipPath)
{
    if (!IsOpen()) {
        LOGE("zip file is not open");
        return ZIP_ERR_FAILURE;
    }
    int ret = zipOpenNewFileInZip64(zipFile_, zipPath.c_str(), &zipFileInfo_, nullptr, 0,
        nullptr, 0, nullptr, 0, static_cast<int>(zipLevel_), 1);
    if (ret != ZIP_OK) {
        LOGE("open file in zip failed![ret=%d][zipPath=%s]", ret, zipPath.c_str());
        return ZIP_ERR_FAILURE;
    }
    ret = zipWriteInFileInZip(zipFile_, content.data(), content.length());
    if (ret < 0) {
        LOGE("write file in zip failed![errno=%d][ret=%d]", errno, ret);
        ret = ZIP_ERR_FAILURE;
    }
    zipCloseFileInZip(zipFile_);
    return ret;
}

int ZipWrapper::AddEmptyDirToZip(const std::string &zipPath)
{
    return AddEmptyDirToZip(fs::path(zipPath));
}

int ZipWrapper::AddEmptyDirToZip(const fs::path &fsZipPath)
{
    fs::path fsZipDirPath = fs::path(fsZipPath.string() + "/");
    LOGI("Add [%s] into zip", fsZipDirPath.string().c_str());
    if (!IsOpen()) {
        LOGE("zip file is not open");
        return ZIP_ERR_FAILURE;
    }
    zipFileInfo_.external_fa = ZIP_FILE_ATTR_DIRECTORY;
    int ret = zipOpenNewFileInZip64(zipFile_, fsZipDirPath.string().c_str(), &zipFileInfo_, nullptr, 0,
        nullptr, 0, nullptr, 0, static_cast<int>(zipLevel_), 1);
    if (ret != ZIP_OK) {
        LOGE("open file in zip failed![ret=%d][zipPath=%s]", ret, fsZipPath.string().c_str());
        return ZIP_ERR_FAILURE;
    }
    zipCloseFileInZip(zipFile_);
    return ret;
}

int ZipWrapper::AddFileToZip(const std::string &filePath, const std::string &zipPath)
{
    return AddFileToZip(fs::path(filePath), fs::path(zipPath));
}

int ZipWrapper::AddFileToZip(const fs::path &fsFilePath, const fs::path &fsZipPath)
{
    if (!IsOpen()) {
        LOGE("zip file is not open");
        return ZIP_ERR_FAILURE;
    }
    if (!fs::is_regular_file(fsFilePath)) {
        LOGE("filePath is not regular file![filePath=%s]", fsFilePath.string().c_str());
        return ZIP_ERR_FAILURE;
    }
    std::ifstream file(fsFilePath.string(), std::ios::binary);
    if (!file.is_open()) {
        LOGE("open file failed![filePath=%s]", fsFilePath.string().c_str());
        return ZIP_ERR_FAILURE;
    }
    zipFileInfo_.external_fa = ZIP_FILE_ATTR_DEFAULT;
    int ret = zipOpenNewFileInZip64(zipFile_, fsZipPath.string().c_str(), &zipFileInfo_, nullptr, 0,
        nullptr, 0, nullptr, 0, static_cast<int>(zipLevel_), 1);
    if (ret != ZIP_OK) {
        LOGE("open file in zip failed![ret=%d][fsZipPath=%s]", ret, fsZipPath.string().c_str());
        file.close();
        return ZIP_ERR_FAILURE;
    }
    if (fs::file_size(fsFilePath) > 0) {
        ret = ZIP_ERR_SUCCESS;
        char buffer[MAX_ZIP_BUFFER_SIZE];
        while (file.good()) {
            file.read(buffer, sizeof(buffer));
            auto bytesRead = file.gcount();
            if (bytesRead <= 0) {
                LOGE("read file bytes error![filePath=%s][bytesRead=%u]", fsFilePath.c_str(), bytesRead);
                ret = ZIP_ERR_FAILURE;
                break;
            }
            if (zipWriteInFileInZip(zipFile_, buffer, bytesRead) < 0) {
                LOGE("write file in zip failed!");
                ret = ZIP_ERR_FAILURE;
                break;
            }
        }
    }
    zipCloseFileInZip(zipFile_);
    file.close();
    return ret;
}

} // namespace AppPackingTool
} // namespace OHOS
