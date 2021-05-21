# labeled-RISC-V-boot

从Zynq PS复位启动标签化RISC-V子系统的参考代码，由张传奇编写

## 使用方法

### Berkeley Bootloader + Linux

将编译好的 linux.bin 放到同一目录下，在shell中执行

```shell
dtc -I dts -O dtb -o rocket.dtb zcu102.dts
gcc 4-core-reset.c -o 4-core-reset
chmod +x smp_run.sh
./smp_run.sh
```

### Labeled RISC-V Rust Bootloader + uCore-SMP

将 lrv-rust-bl.bin 和 ucore-smp.bin 放在同一目录下，执行

```shell
dtc -I dts -O dtb -o rocket.dtb zcu102.dts
gcc 4-core-reset-ucore.c -o 4-core-reset-ucore
chmod +x smp_run_ucore.sh
./smp_run_ucore.sh
```
