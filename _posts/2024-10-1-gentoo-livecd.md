---
title: Gentoo Live CD Adventures
---

Build a custom Gentoo Live CD.

## Background

To study Intel's latest Linear Address Masking feature, I have my supervisor bought me a ASUS Laptop with latest Intel Ultra 258V CPU. As my usual development environment is built upon Gentoo, I decided to install Gentoo on it and test the LAM feature.

However, when I booted into Gentoo's Live CD, I found that the kernel is too old and nothing works. The network card is gone, the kernel is spitting out tons of errors in `dmesg`.

## Gentoo's Live CD

So, I learnt that Gentoo provides tools to build your own Live CD, named `catalyst` (not AMD's driver). This is the guide (https://wiki.gentoo.org/wiki/Catalyst/Custom_Media_Image). The guide is ... well ... OK.
It just misses the things I need. Long story short, I need solve the following things to get a usable Live CD.

1. Linux Firmware is too old, only the latest one includes the iwlwifi-bz-***-92 firmware.
2. The kernel is too old, it supports neither the firmware nor the CPU.
3. Catalyst provides neither guides on how to tweak with the kernel nor the firmware.

## Linux Firmware

The Linux firmware is rather easy to solve. Gentoo has synced with upstream and provides `sys-kernel/linux-firmware-20240909-r1`.
By `catalyst`'s guide, one can change its `portage` behavior by modifying `releng`'s config. So, I changed `package.mask` to force using the latest firmware.

```
<sys-kernel/linux-firmware-20240909-r1
```

## From distkernel to Source Kernel

Let alone the Catalyst's configuration, we first need to let Catalyst to use the latest kernel package.
This is still done via `portage`'s configuration. I changed `package.mask` to force using the latest kernel.

```
<sys-kernel/gentoo-sources-6.11.0
```

and allow the latest kernel in `package.accept_keywords`.

```
=sys-kernel/gentoo-sources-6.11.0 ~amd64
```

## The hard part --- Source Kernel

The kernel used by Catalyst is the `gentoo-kernel`, which is a pre-built kernel and the latest version is `6.10.12`. Sadly, this version
does not support the `iwlwifi-bz-***-92` firmware. It only supports `iwlwifi-bz-***-90`, which does not exist at all...

So, I need to change the Catalyst's specification to do the following things.

1. Add necessary tools for building from the source.
2. Change the kernel configuration.
3. Change the Catalyst configuration.

### Add necessary tools

I added the following packages to `/var/tmp/catalyst/installcd-stage1.spec`

```
sys-kernel/genkernel        # for building the kernel
dev-util/pahole             # for checking the kernel's data structure   
sys-fs/squashfs-tools       # for mounting the rootfs
```

### Change the kernel configuration

To begin with, the Catalyst does not actually provide a kernel config for sources. So, I just copied the `6.10.12` configuration from distkernel.
You can find it in `gentoo-kernel`'s [ebuild](https://gitweb.gentoo.org/repo/gentoo.git/tree/sys-kernel/gentoo-kernel/gentoo-kernel-6.10.12.ebuild)

After I boot into the kernel, the `rootfs` is not mounted and the Live CD complains not valid rootfs. Basically, the kernel does not recognize the squashfs filesystem, we need to enable it, not as a module, but as a built-in feature.

```
CONFIG_BLK_DEV_LOOP=y
CONFIG_SQUASHFS=m
```

### Change Catalyst configuration

The original Catalyst configuration contains a part of `dracut`, which is for generating the initramfs. However, if you're using `gentoo-sources`, the `genkernel` is used to generate the initramfs. So, I removed the `dracut` part.

```
subarch: amd64
version_stamp: custom_livecd
target: livecd-stage2
rel_type: 23.0-default
profile: default/linux/amd64/23.0/no-multilib
snapshot_treeish: current.xz
source_subpath: 23.0-default/livecd-stage1-amd64-custom_livecd
portage_confdir: /home/john/documents/releng/releases/portage/isos

livecd/bootargs: dokeymap 
livecd/fstype: squashfs
livecd/iso: install-amd64-minimal.iso
livecd/type: gentoo-release-minimal
livecd/volid: Gentoo-amd64

boot/kernel: gentoo
boot/kernel/gentoo/config: /var/tmp/catalyst/kconfig
boot/kernel/gentoo/packages: net-wireless/broadcom-sta 
```

And finally, the Live CD boots up with the network card.
