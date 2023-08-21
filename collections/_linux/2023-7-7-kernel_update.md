---
title: Gentoo system/kernel update procedure
---

Due to the forgetfulness of myself, I write this article to record the procedure
of updating my Gentoo system (kernel).

## System update

This is not that complicated.

```bash
sudo emerge --ask --update --deep --newuse @world # update everything
sudo emerge --depclean                            # clean unneeded packages
```

The following notification is NOT real conflict but simple lagged packages.

> WARNING: One or more updates/rebuilds have been skipped due to a dependency conflict

## Kernel update

### Mount `/boot` partition

I have a seperated partition for `/boot` (only 100M), so mount it first before
installing the kernel.

```bash
sudo mount /dev/nvme0n1p1 /boot
```

### Before building the kernel

You may want to copy the `.config` and symbollink `linux` to the newer linux
kernel (but this seems optional).

```bash
sudo cp linux/.config <new-kernel>/.config
sudo rm linux
sudo ln -s <new-kernel> linux
```

### Building the kernel

```bash
sudo make -j21 && sudo make install && sudo make modules_install
```

### After building the kernel

You probably need to update the grub configuration to use the new kernel.
Don't forget to install the `nvidia-driver`, otherwise the graphic card won't work.
(Fuck you Nvidia)

```bash
sudo grub-mkconfig -o /boot/grub/grub.cfg
sudo emerge -av nvidia-drivers
```
