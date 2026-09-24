# Installer packager tests

The `installer_packager_test` GN target exercises the installer-specific packaging
flow, strict manifest parsing and CLI parameter dispatch. It uses the existing
GTest/HWTEST framework and creates synthetic ZIP fixtures in an exclusively owned
temporary directory. These fixtures are unit-test inputs, not DevEco-built demos.

Coverage includes:

- Generated and external manifests, exact external bytes, required fields,
  application identity, numeric boundaries and unknown fields.
- HAP-only/HSP-only inputs, per-application isolation, selected APP field rules,
  disjoint distribution filters and installer-specific diagnostic codes.
- Reused distribution-filter parsing: both node names, unrelated profiles before
  the first filter, stopping before later missing profiles, empty arrays/strings
  and signed API values accepted by the existing parser. Native parser failures,
  profile traversal and malformed JSON remain failure cases.
- Unicode code-point ordering, case-sensitive extensions, empty/invalid directory
  structures and strict UTF-8 JSON syntax, including duplicate keys and NUL tails.
- Fixed DEFLATE level 5 entries for both debug values (raw stream compared with
  zlib level 5), module byte preservation, corrupt ZIP CRC/size and failed reads.
- Links, output aliases, overwrite policy, changed inputs and failed commit;
  platform-specific cases are conditional on filesystem support.

From a configured OpenHarmony source root, build:

```sh
hb build --build-target installer_packager_test
hb build --build-target shell_command_test
hb build --build-target hap_verify_utils_test_part2
```

Run the resulting test binaries in the configured host/test environment and save
their exit status and GTest XML. Building a test target is not evidence of running
it. Also run Java and native CLI integration tests using the same DevEco-built
HAP/HSP inputs; check manifests, all archive entries, module hashes, input hashes
and preservation of existing outputs on failure.

Large-file ZIP64 consumption, disk-full/close-error fault injection and actual
installation require separate integration coverage. No Java unit tests are added.

## Reuse boundaries

Installer delegates profile node selection to Java
`ModuleJsonUtil.parseStageDistroFilter` and native
`ModuleJson::ParseModuleMetadatasToDistroFilter`, then uses their existing model
deserialization mechanisms. It does not impose an extra filter-field validator.
Java `JsonUtil` is not part of the packing JAR source set and must not be used
as a packing dependency. Existing Java/native coercion differences remain:
Java can convert numeric country-code values to strings, while native
`DistroFilter::ParseFromJson` rejects non-string values. Test and report this
case separately rather than claiming malformed-input parity.

Selected APP field and module-name validation already delegates to
`HapVerify`/`HapVerifyUtils`; calling the complete APP validator would introduce
unrequested entry/dependency checks. Installer retains its path checks,
external manifest validation and transactional ZIP writer.

## Module parsing

Installer reads module information through `Compressor.parseStageHapVerifyInfo`
and `ModuleJsonUtils::GetStageHapVerifyInfo`, including profile resources and
distribution filters. Field conversion, defaults and parser failures follow
these existing entry points; installer does not add module-field type or
nonnegative-int32 checks. The installer bundle type, device type and selected
cross-module checks still apply. External manifest validation is separate.
Malformed-input acceptance can differ between the existing Java and native
parsers; this reuse does not introduce new conversions into either parser.
