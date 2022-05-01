# 在 DragonBoard 410c 上激活 ETM Tracing

家贫，无以 Juno 以观，然 Juno 皆被 Fuzzing 所占，无可为，只得求一贫板，名 DragonBoard 410c。
经 Hacking，得可用之 ETM。

## 相关教程

[Linaro 工具链](https://www.linaro.org/downloads)
[Linaro 替换内核教程](https://www.96boards.org/documentation/consumer/dragonboard/dragonboard410c/build/kernel.md.html)
[Linaro fastboot 教程](https://www.96boards.org/documentation/consumer/dragonboard/dragonboard410c/installation/linux-fastboot.md.html#linarodebian-recall-location-of-boot-and-rootfs-downloaded-from-the-downloads-page)

## 内核下载与配置

- 克隆 Linaro 的内核仓库（用美利坚的代理比较快，共 1.57 G，不用会很痛苦）

```bash
git clone http://git.linaro.org/landing-teams/working/qualcomm/kernel.git
```

- 配置内核

找到 CoreSight 相关的配置选项全部打开来即可。

```bash
cd kernel
# 重要，选择正式发行版
git checkout origin/release/qcomlt-5.15
# 下面两句很重要，否则配置里不会有 arm 相关的内容
export ARCH=arm64
export CROSS_COMPILE=aarch64-none-linux-gnu-
make defconfig
make menuconfig
```

基本相关的内容都在 `Kernel hacking -> arm64 Debugging -> CoreSight Tracing Support` 内。

- 编译内核

```bash
make -j$(nproc) Image.gz dtbs
make -j$(nproc) modules
```

- 制作镜像

```bash
cat arch/$ARCH/boot/Image.gz arch/$ARCH/boot/dts/qcom/apq8016-sbc.dtb > Image.gz+dtb
echo "not a ramdisk" > ramdisk.img
# 这里的 root 换成我们烧写的 rootfs，我默认是 mmcblk0p14，可以根据 Linux 启动日志判断。
abootimg --create boot-db410c.img -k Image.gz+dtb -r ramdisk.img \
           -c pagesize=2048 -c kerneladdr=0x80008000 -c ramdiskaddr=0x81000000 \
           -c cmdline="root=/dev/mmcblk0p10 rw rootwait console=tty0 console=ttyMSM0,115200n8"
```

- 烧写

```bash
fastboot flash boot boot-db410c.img
```

- 安装模块

由于内核更新了，因此内核模块也要对应更新。

```bash
# In kernel directory
mkdir db410c-modules
make modules_install INSTALL_MOD_PATH=./db410c-modules
```

把这个文件夹搬到 `/lib/modules/` 即可，注意文件夹层级。

- 细节
插入 SD 卡会影响 `rootfs` 的编号导致启动失败。
启动的时候有概率 `panic` 再启动一次即可。

- 结果
可以在 `/sys/bus/coresight/devices` 下看到 etm 组件。

```bash
root@linaro-developer:~# ls /sys/bus/coresight/devices/
cti_cpu0  cti_cpu2  cti_sys0  etm0  etm2  funnel0  replicator0  tmc_etf0  tpiu0
cti_cpu1  cti_cpu3  cti_sys1  etm1  etm3  funnel1  stm0         tmc_etr0
```
