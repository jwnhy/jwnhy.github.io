---
title: Android AOSP kernel build, module build, and misc.
---

## Build release kernel.

First we need to follow the official [guide](https://source.android.com/docs/setup/build/building-pixel-kernels).
For example, if we want to build kernel for Pixel 8 Pro (Husky), we run the following commands to initialize the repo.

```bash
repo init https://android.googlesource.com/kernel/manifest --depth=1 --groups=default,-mips,-darwin,-x86,-riscv -b android-gs-shusky-5.15-android14-d1
repo sync -c --no-tags
```

Let's go through these options one by one.

1. `https://android.googlesource.com/kernel/manifest` is the manifest url of your android build, you might use other mirror like [Tsinghua](https://mirrors.tuna.tsinghua.edu.cn/help/AOSP/).
2. `--depth=1` means we don't want any history, shrinking the disk usage by much.
3. `--groups=default,-mips,-darwin,-x86,-riscv` means we don't want to build Android for {mips, x86, and riscv} or install MacOS toolchains (darwin).
4. `-b android-gs-shusky-5.15-android14-d1` is the manifest branch for Pixel 8 Pro, found on [guide](https://source.android.com/docs/setup/build/building-pixel-kernels).

## Build QPR kernel (Quarterly Platform Release)

You might notice that `https://android.googlesource.com/kernel/manifest` does not contain QPR builds (simply replacing the branch suffix `-d1` with `qprX-beta` does not work). We need to manually modify the `default.xml` in `.repo/manifests`.

Note that manifests are just declarations indicating where to pull the sources from. Though there is NO `qprX-beta` branch manifest for a complete kernel build, there are sub-manifests in each sub-repo. We just need to enable pointing the manifest to them by changing the `<default revision>` tag in the manifest `default.xml`.

```xml
<default revision="android-gs-shusky-5.15-android14-qpr3-beta" remote="aosp" sync-j="4" />
```

After modifying the `default.xml`, syncing and building the repo, you might encounter errors that say something is missing.

```
<I am too lazy to re-run the building process to show the error message.>
```

We just add the relevant declarations, like this. You can find the list of AOSP modules in `https://android.googlesource.com/kernel/`.

```xml
<project path="private/google-modules/uwb/qorvo/qm35" name="kernel/google-modules/uwb/qorvo/qm35" groups="partner" />
```

## Build Kernel Module

Most existing modules are in `./private/google-modules`. For now, I only know how to add a new module in this folder and its sub-folders. I will introduce the following contents.

- Create boilerplate code
- Reference other module
- Miscellaneous

### Create boilerplate code

Let's create a module in `./private/google-modules/gpu/csfparser/moye`. After copying your module codes into it, it should contain the following. Note that we need to create `BUILD.bazel`, `Kbuild`.

```bash
❯ ls
bifrost  BUILD.bazel            kbase_defs.h  main.c  Makefile   moye_fw.h   moye_mmu.h     util.c
build    compile_commands.json  Kbuild        main.h  moye_fw.c  moye_mmu.c  moye_regmap.h  util.h
```

`BUILD.bazel` looks like:

<details>
<summary>Click to check</summary>

```
load("//build/kernel/kleaf:kernel.bzl", "kernel_module")

kernel_module(
    name = "moye",
    srcs = glob([
        "**/*.c",
        "**/*.h",
        "Kbuild",
    ]) + [
        "//private/google-modules/gpu/mali_kbase:headers",
        "//private/google-modules/gpu/common:headers",
        "//private/google-modules/soc/gs:gs_soc_headers",
    ],
    outs = [
        "moye.ko",
    ],
    kernel_build = "//private/google-modules/soc/gs:gs_kernel_build",
    visibility = [
        "//private/devices/google:__subpackages__",
        "//private/google-modules/gpu/mali_kbase:__pkg__",
        "//private/google-modules/soc/gs:__pkg__",
    ],
    deps = [
        "//private/google-modules/gpu/mali_kbase",
        "//private/google-modules/soc/gs:gs_soc_module",
    ],
)
```

</details>

Essentially, `BUILD.bazel` decides what is available when compiling the module. That is why we have 

```
    srcs = glob([
        "**/*.c",
        "**/*.h",
        "Kbuild",
    ]) + [
        "//private/google-modules/gpu/mali_kbase:headers",
        "//private/google-modules/gpu/common:headers",
        "//private/google-modules/soc/gs:gs_soc_headers",
    ],
```
