/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

import com.alibaba.fastjson.JSONObject;
import java.io.IOException;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.ArrayList;
import java.util.List;
import java.util.zip.ZipEntry;
import org.apache.commons.compress.archivers.zip.ZipArchiveEntry;
import org.apache.commons.compress.archivers.zip.ZipArchiveOutputStream;
import org.apache.commons.compress.archivers.zip.Zip64Mode;

/**
 * Packages application modules and an installer manifest into an APP archive.
 *
 * @since 2026-10-07
 */
final class InstallerPackager {
    private static final int COMPRESSION_LEVEL = 5;
    private static final Log LOG = new Log(InstallerPackager.class.toString());

    private final Utility utility;
    private final List<Input> modules = new ArrayList<>();
    private final List<JSONObject> apps = new ArrayList<>();
    private Path contents;
    private Path output;
    private Path external;

    private InstallerPackager(Utility utility) {
        this.utility = utility;
    }

    static boolean validateArguments(Utility utility) {
        try {
            new InstallerPackager(utility).validateArguments();
            return true;
        } catch (IOException | BundleException | RuntimeException exception) {
            report(exception);
            return false;
        }
    }

    // Called after CompressVerify.commandVerify has accepted the arguments.
    static boolean pack(Utility utility) {
        try {
            InstallerPackager packager = new InstallerPackager(utility);
            packager.initializePaths();
            if (utility.getInstallerManifest() != null) {
                packager.external = Paths.get(utility.getInstallerManifest()).toAbsolutePath();
                InstallerManifest.require(Files.isRegularFile(packager.external), PackingToolErrMsg.INSTALLER_IO_FAILED,
                        "Input must be a regular file: " + packager.external);
            }
            packager.discover();
            byte[] manifest;
            if (packager.external == null) {
                manifest = InstallerManifest.generate(packager.apps);
            } else {
                try {
                    manifest = Files.readAllBytes(packager.external);
                } catch (IOException exception) {
                    throw new InstallerManifest.Failure(
                            PackingToolErrMsg.INSTALLER_IO_FAILED,
                            "Cannot read manifest: " + packager.external, exception);
                }
                InstallerManifest.validate(manifest, packager.apps);
            }
            packager.write(manifest);
            checkUncompressedSize(utility);
            return true;
        } catch (IOException | BundleException | RuntimeException exception) {
            report(exception);
            return false;
        }
    }

    private static void report(Exception exception) {
        if (exception instanceof InstallerManifest.Failure) {
            InstallerManifest.Failure failure = (InstallerManifest.Failure) exception;
            LOG.error(failure.error.toString(failure.getMessage()));
        } else {
            LOG.error(PackingToolErrMsg.INSTALLER_IO_FAILED.toString("Cannot access installer files"));
            LOG.error("[installer] Details: " + exception.getMessage());
        }
        if (exception.getCause() != null) {
            LOG.error("[installer] Details: " + exception.getCause().getMessage());
        }
    }

    private void validateArguments() throws IOException, BundleException {
        InstallerManifest.require(!utility.getInstallerContents().isEmpty(),
            PackingToolErrMsg.INSTALLER_ARGUMENT_INVALID, "--contents is required");
        InstallerManifest.require(!utility.getOutPath().isEmpty(),
            PackingToolErrMsg.INSTALLER_ARGUMENT_INVALID, "--out-path is required");
        InstallerManifest.require(utility.getOutPath().endsWith(".app"),
            PackingToolErrMsg.INSTALLER_ARGUMENT_INVALID, "--out-path must end with lowercase .app");
        InstallerManifest.require("true".equals(utility.getForceRewrite())
                || "false".equals(utility.getForceRewrite()),
                    PackingToolErrMsg.INSTALLER_ARGUMENT_INVALID, "--force must be true or false");
        initializePaths();
        InstallerManifest.require(Files.isDirectory(contents), PackingToolErrMsg.INSTALLER_CONTENTS_INVALID,
                "--contents must be a directory");
        InstallerManifest.require(!output.startsWith(contents),
            PackingToolErrMsg.INSTALLER_PATH_INVALID, "Output must be outside contents");
        if (utility.getInstallerManifest() != null) {
            InstallerManifest.require(!utility.getInstallerManifest().isEmpty(),
                PackingToolErrMsg.INSTALLER_ARGUMENT_INVALID, "--manifest must be a file");
            external = Paths.get(utility.getInstallerManifest()).toAbsolutePath();
            InstallerManifest.require(Files.isRegularFile(external),
                PackingToolErrMsg.INSTALLER_IO_FAILED, "Input must be a regular file: " + external);
            InstallerManifest.require(!output.equals(external)
                    && (!Files.exists(output) || !Files.isSameFile(output, external)),
                        PackingToolErrMsg.INSTALLER_PATH_INVALID,
                    "Output aliases manifest");
        }
        if (Files.exists(output)) {
            InstallerManifest.require(Files.isRegularFile(output)
                    && "true".equals(utility.getForceRewrite()),
                        PackingToolErrMsg.INSTALLER_ARGUMENT_INVALID,
                        "Output exists; use --force true for a regular file");
        }
    }

    private void initializePaths() throws IOException {
        contents = Paths.get(utility.getInstallerContents()).toAbsolutePath();
        output = Paths.get(utility.getOutPath()).toAbsolutePath();
        contents = contents.toRealPath();
        output = resolveExisting(output.normalize());
    }

    private static Path resolveExisting(Path path) throws IOException {
        Path ancestor = path;
        while (!Files.exists(ancestor)) {
            ancestor = ancestor.getParent();
        }
        return ancestor.toRealPath().resolve(ancestor.relativize(path));
    }

    private static List<Path> listChildren(Path directory) throws IOException, BundleException {
        List<Path> result = new ArrayList<>();
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(directory)) {
            for (Path child : stream) {
                result.add(child);
            }
        }
        InstallerManifest.require(!result.isEmpty(),
            PackingToolErrMsg.INSTALLER_CONTENTS_INVALID, "Empty directory: " + directory);
        result.sort((left, right) -> compareNames(left.getFileName().toString(), right.getFileName().toString()));
        return result;
    }

    private static int compareNames(String left, String right) {
        int leftIndex = 0;
        int rightIndex = 0;
        while (leftIndex < left.length() && rightIndex < right.length()) {
            int leftPoint = left.codePointAt(leftIndex);
            int rightPoint = right.codePointAt(rightIndex);
            if (leftPoint != rightPoint) {
                return Integer.compare(leftPoint, rightPoint);
            }
            leftIndex += Character.charCount(leftPoint);
            rightIndex += Character.charCount(rightPoint);
        }
        return Integer.compare(left.length() - leftIndex, right.length() - rightIndex);
    }

    private static void validatePathComponent(String name) throws BundleException {
        InstallerManifest.require(!name.isEmpty() && !".".equals(name) && !"..".equals(name)
                && name.indexOf('/') < 0 && name.indexOf('\\') < 0 && name.indexOf(':') < 0,
                    PackingToolErrMsg.INSTALLER_PATH_INVALID,
                "Invalid archive path component: " + name);
    }

    private void discover() throws IOException, BundleException {
        for (Path directory : listChildren(contents)) {
            InstallerManifest.require(Files.isDirectory(directory), PackingToolErrMsg.INSTALLER_CONTENTS_INVALID,
                    "Contents may only contain application directories: " + directory);
            String name = directory.getFileName().toString();
            validatePathComponent(name);
            List<HapVerifyInfo> infos = new ArrayList<>();
            int minApi = Integer.MIN_VALUE;
            for (Path path : listChildren(directory)) {
                String filename = path.getFileName().toString();
                validatePathComponent(filename);
                InstallerManifest.require(filename.endsWith(".hap") || filename.endsWith(".hsp"),
                    PackingToolErrMsg.INSTALLER_CONTENTS_INVALID,
                        "Application directories only accept lowercase .hap/.hsp: " + path);
                Input input = new Input(path, "Contents/" + name + "/" + filename);
                InstallerManifest.require(!Files.exists(output) || !Files.isSameFile(path, output),
                    PackingToolErrMsg.INSTALLER_PATH_INVALID,
                        "Output aliases an input module");
                HapVerifyInfo info = readModule(input);
                infos.add(info);
                minApi = Math.max(minApi, info.getApiVersion().getCompatibleApiVersion());
                modules.add(input);
            }
            boolean valid;
            try {
                valid = HapVerify.checkInstallerAppFields(infos);
            } catch (BundleException exception) {
                throw new InstallerManifest.Failure(PackingToolErrMsg.INSTALLER_APP_INVALID,
                        "Application module fields are inconsistent: " + directory, exception);
            }
            InstallerManifest.require(valid, PackingToolErrMsg.INSTALLER_APP_INVALID,
                    "Application module fields are inconsistent: " + directory);
            HapVerifyInfo first = infos.get(0);
            JSONObject app = new JSONObject(true);
            app.put("bundleName", first.getBundleName());
            app.put("versionCode", first.getVersion().versionCode);
            app.put("versionName", first.getVersion().versionName);
            app.put("minAPIVersion", minApi);
            app.put("path", "Contents/" + name);
            apps.add(app);
        }
    }

    private static void checkUncompressedSize(Utility utility) throws BundleException {
        String outPath = utility.getOutPath();
        if (outPath.isEmpty()) {
            return;
        }
        long uncompressedSize;
        try {
            long[] sizes = FileUtils.calculateZipSize(outPath);
            uncompressedSize = sizes[1];
        } catch (IOException exception) {
            throw new InstallerManifest.Failure(
                    PackingToolErrMsg.INSTALLER_IO_FAILED,
                    "Cannot read installer output: " + outPath, exception);
        }
        if (uncompressedSize > Constants.MAX_APP_INPUT_SIZE) {
            String errMsg = "Installer uncompressed size is " + uncompressedSize +
                    " bytes, exceeds 200GB limit.";
            LOG.error(PackingToolErrMsg.APP_INPUT_SIZE_EXCEEDS_LIMIT.toString(errMsg));
            throw new BundleException("Installer uncompressed size exceeds 200GB limit.");
        }
    }

    private static HapVerifyInfo readModule(Input input) throws BundleException {
        HapVerifyInfo info;
        try {
            info = Compressor.parseStageHapVerifyInfo(input.path.toString());
        } catch (BundleException | RuntimeException exception) {
            throw new InstallerManifest.Failure(PackingToolErrMsg.INSTALLER_MODULE_INVALID,
                    "Invalid installer module: " + input.path, exception);
        }
        InstallerManifest.require("app".equals(info.getBundleType()),
                PackingToolErrMsg.INSTALLER_MODULE_INVALID, "bundleType must be app: " + input.path);
        InstallerManifest.require(info.getDeviceType().contains("2in1"),
                PackingToolErrMsg.INSTALLER_MODULE_INVALID, "deviceTypes must contain 2in1: " + input.path);
        return info;
    }

    private void write(byte[] manifest) throws BundleException {
        try {
            Files.createDirectories(output.getParent());
            Path temporary = Files.createTempFile(output.getParent(), ".installer-", ".tmp");
            try {
                try (ZipArchiveOutputStream archive = new ZipArchiveOutputStream(temporary.toFile())) {
                    archive.setUseZip64(Zip64Mode.AsNeeded);
                    archive.setEncoding("UTF-8");
                    archive.setMethod(ZipEntry.DEFLATED);
                    archive.setLevel(COMPRESSION_LEVEL);
                    archive.putArchiveEntry(new ZipArchiveEntry("manifest.json"));
                    archive.write(manifest);
                    archive.closeArchiveEntry();
                    for (Input input : modules) {
                        archive.putArchiveEntry(new ZipArchiveEntry(input.entry));
                        long copied = Files.copy(input.path, archive);
                        InstallerManifest.require(copied == input.size,
                                PackingToolErrMsg.INSTALLER_WRITE_FAILED, "Cannot read complete module: " + input.path);
                        archive.closeArchiveEntry();
                    }
                    archive.finish();
                }
                try {
                    if ("true".equals(utility.getForceRewrite())) {
                        Files.move(temporary, output, StandardCopyOption.ATOMIC_MOVE,
                                StandardCopyOption.REPLACE_EXISTING);
                    } else {
                        Files.createLink(output, temporary);
                    }
                } catch (IOException | RuntimeException exception) {
                    throw new InstallerManifest.Failure(
                            PackingToolErrMsg.INSTALLER_COMMIT_FAILED, "Cannot commit output: " + output, exception);
                }
            } finally {
                try {
                    Files.deleteIfExists(temporary);
                } catch (IOException exception) {
                    LOG.error(PackingToolErrMsg.INSTALLER_CLEANUP_FAILED.toString(
                            "Cannot clean temporary output: " + temporary));
                    LOG.error("[installer] Details: " + exception.getMessage());
                }
            }
        } catch (IOException | RuntimeException exception) {
            throw new InstallerManifest.Failure(PackingToolErrMsg.INSTALLER_WRITE_FAILED,
                    "Cannot write installer archive: " + output, exception);
        }
    }

    private static final class Input {
        private final Path path;
        private final String entry;
        private final long size;

        Input(Path path, String entry) throws IOException, BundleException {
            this.path = path;
            this.entry = entry;
            InstallerManifest.require(Files.isRegularFile(path),
                    PackingToolErrMsg.INSTALLER_IO_FAILED, "Input must be a regular file: " + path);
            size = Files.size(path);
        }
    }
}
