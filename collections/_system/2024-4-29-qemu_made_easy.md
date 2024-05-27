---
title: QEMU made easy.
---

# QEMU Basic.

QEMU is a common virtualization tool used to emulate a full system/user application. In this post, we will cover how to set up a basic QEMU full system emulation. In essence, you need two things to boot a full system in QEMU.

- Kernel Image
- Root File-system

# Rootfs made easy.

Building a rootfs from scratch can be a daunting task. However, there are tools available that can help you build a rootfs with ease. We can obtain a script named `create-image.sh` from [here](https://github.com/google/syzkaller/blob/master/tools/create-image.sh). This script is a part of syzkaller project and is used to create a rootfs for QEMU. The script is well documented and easy to understand. The script uses `debootstrap` to create a rootfs. You can install `debootstrap` using your package manager.

## Downloading nightmare under slow network.

There is a small issue with the script's configuration. By default, `create-image.sh` does not cache anything previously downloaded. If the script is interrupted for any reason, it will download everything again. This can be a problem if you have a slow internet connection. To fix this issue, you can change the following line in the script.

```bash
DEBOOTSTRAP_PARAMS="--arch=$DEBARCH --include=$PREINSTALL_PKGS --components=main,contrib,non-free,non-free-firmware --cache-dir=$(pwd)/.cache $RELEASE $DIR"
```

# Kernel made easy.

There are some mythical configuration besides `$ARCH_defconfig` that you need if you want to boot smoothly in QEMU. These are the following. These stuff can be found at [here](https://github.com/google/syzkaller/issues/760).

```
CONFIG_CONFIGFS_FS=y
CONFIG_SECURITYFS=y
CONFIG_BINFMT_MISC=y
```

# QEMU made easy.

Now that we have a rootfs and kernel image, we can boot the system using QEMU. The following command will boot the system. I use aarch64 as an example. You can change the architecture according to your kernel image.

> Note that the kernel cmdline argument `net.ifnames=0` is NOT redundant, otherwise the image will boot into emergency mode.

```bash
qemu-system-aarch64 \
        -machine virt,virtualization=true,gic-version=3 \
        -nographic \
        -m size=1024M \
        -cpu max \
        -smp 2 \
        -hda ./bookworm.img \                               # change this to your image path
        -nic user,model=virtio-net-pci \
        -kernel ./linux-6.8.8/arch/arm64/boot/Image \       # change this to your kernel path
        --append "console=ttyAMA0 root=/dev/vda rw net.ifnames=0"
```

# Sharing between host and guest.

> The following content is taken from [here](https://superuser.com/questions/628169/how-to-share-a-directory-with-the-host-without-networking-in-qemu).

To begin with, we need to enable the following kernel configuration in the *guest* kernel.

```
CONFIG_9P_FS=y
CONFIG_9P_FS_POSIX_ACL=y
CONFIG_9P_FS_SECURITY=y
CONFIG_NETWORK_FILESYSTEMS=y
CONFIG_NET_9P=y
CONFIG_NET_9P_DEBUG=y
CONFIG_NET_9P_VIRTIO=y
# if you are using aarch64, add the following as well.
CONFIG_PCI=y
CONFIG_PCI_HOST_COMMON=y
CONFIG_PCI_HOST_GENERIC=y
CONFIG_VIRTIO_PCI=y
CONFIG_VIRTIO_BLK=y
CONFIG_VIRTIO_NET=y
```

We then add the following stuff, telling QEMU to map a host directory into the guest.

```
# original post says to use security_model=passthrough, but it doesn't work for me.
-virtfs local,path=<host-path>,mount_tag=host0,security_model=mapped-xattr,id=host0
```

In the guest, we can mount the shared directory using the following command.

```
mount -t 9p -o trans=virtio,version=9p2000.L host0 <guest-path>
```

Or you can just add a line in `/etc/fstab` to mount the directory automatically.

```
host0   <guest-path>  9p      trans=virtio,version=9p2000.L   0 0
```
