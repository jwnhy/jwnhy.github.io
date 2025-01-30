---
title: Android AOSP kernel build, module build, and misc.
---

Building Android AOSP kernel and adding custom kernel modules!

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

- Bazel setup & reference other module
- Generation of `compile_commands.json`
- Miscellaneous

### Bazel setup & reference other module

In this example, I will introduce how to create a new module named `moye` and reference `mali_kbase` in it.
Let's create a module in `./private/google-modules/gpu/csfparser/moye`. After copying your module codes into it, it should contain the following. Note that we need to create `BUILD.bazel`, `Kbuild`.

```bash
❯ ls
bifrost  BUILD.bazel            kbase_defs.h  main.c  Makefile   moye_fw.h   moye_mmu.h     util.c
build    compile_commands.json  Kbuild        main.h  moye_fw.c  moye_mmu.c  moye_regmap.h  util.h
```

`BUILD.bazel` looks like:

```python
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

Essentially, `BUILD.bazel` decides what is available when compiling the module. That is why we have 

```python
    srcs = glob([
        "**/*.c", # including all C files.
        "**/*.h", # including all header files.
        "Kbuild", # including the Kbuild of the module.
    ]) + [
        # we want to use headers from other modules.
        "//private/google-modules/gpu/mali_kbase:headers",
        "//private/google-modules/gpu/common:headers",
        "//private/google-modules/soc/gs:gs_soc_headers",
    ],
```

In particular, `//private/google-modules/gpu/mali_kbase` in fact refers to the Bazel target of another module named `mali_kbase`. It is located in `./private/google-modules/gpu/mali_kbase`. The `visibility` and `deps` specifies what modules can sees our module and what modules our module depends on. `deps` is particularly important if you want to reference other modules.

Now let's see the `Kbuild` file.

```bash
# make $(src) as absolute path if it isn't already, by prefixing $(srctree)
src:=$(if $(patsubst /%,,$(src)),$(srctree)/$(src),$(src))

obj-m += moye.o	
moye-objs := main.o util.o moye_fw.o moye_mmu.o

ccflags-y += \
    $(DEFINES) \
    -I$(src)/../../common/include \
    -I$(src)/../../mali_kbase \
    -I$(srctree)/include/linux \
    -DMALI_CUSTOMER_RELEASE=1 \
    -DMALI_USE_CSF=1 \
    -DMALI_UNIT_TEST=0 \
    -DMALI_JIT_PRESSURE_LIMIT_BASE=0 \
```

Except the usual `obj-m` and `moye-objs` for kernel module, we add `CFLAGS` via `ccflags-y`. The `BUILD.bazel` only enables our module see the additional headers, they still require manual inclusion, such as `-I$(src)/../../common/include`. Note that Bazel respects the original folder structure, so we need to jump out of our folder using `../`.

As for `-DMALI_XXX_XXX={0,1}`, these defines are for the headers in `mali_kbase`. Since we directly include these header files in a hacky way, these headers need some defines to work properly. So, we just add these defines manually and make sure they align with the defines in `mali_kbase`.

```c
#ifdef MALI_CUSTOMER_RELEASE
// code requires these defines
#endif
```

`Makefile` is rather boring. We need to suppress some warnings as we are referencing another module.

```makefile
KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build
M ?= $(shell pwd)

KBUILD_OPTIONS += $(KBUILD_EXTRA) # Extra config if any

EXTRA_CFLAGS += -I$(M)
EXTRA_CFLAGS += -I$(M)/../../common/include
EXTRA_CFLAGS += -Wno-unused-variable -Wno-unused-function -Wno-missing-prototypes

EXTRA_SYMBOLS = $(OUT_DIR)/../private/google-modules/gpu/mali_kbase/Module.symvers

include $(KERNEL_SRC)/../private/google-modules/soc/gs/Makefile.include

modules modules_install clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) W=1 $(KBUILD_OPTIONS) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" KBUILD_EXTRA_SYMBOLS="$(EXTRA_SYMBOLS)" $(@)
```

### Generation of `compile_commands.json`

Though AOSP provides some vague instructions on how to generate `compile_commands.json` for *common* Android kernel, I didn't find any materials on how to generate this in a dist release. And I'm tired of jujitsu with Bazel. So, I decide just using `bear` to generate `compile_commands.json` for my module.

To begin with, we need to copy `bear` into the building environment. Since `clang` must be visible the environment, we just copy it there (it's dirty but works).

```bash
cp /usr/bin/bear ./prebuilts/clang/host/linux-x86/clang-<version>/bin # let bear visible in the building environment.
```

Then we just simply add `bear` into the `Makefile` of our module, or any module you want to have `compile_commands.json`.

```bash
bear -- $(MAKE) -C $(KERNEL_SRC) M=$(M) W=1 $(KBUILD_OPTIONS) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" KBUILD_EXTRA_SYMBOLS="$(EXTRA_SYMBOLS)" $(@)
```

> PS: You shouldn't add `bear --` to targets like `module_install` or `module_clean` as they invoke *no* compile commands, which generates an empty `compile_commands.json` and overrides the correct one.

### Miscellaneous

- Note that Android kernel module does not support the default `init_module` and `cleanup_module`. Using these two directly crashes the phone. One needs to explicitly specifies the entry point using `module_init()` and `module_exit()` macros.
- Only the `T` symbols in the `/proc/kallsyms` can be referenced in other modules.
