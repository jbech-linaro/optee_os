# Raspberry Pi 3 on OP-TEE
[Sequitur Labs](http://www.sequiturlabs.com) did the initial port which besides
the actual OP-TEE port also patched U-boot, ARM Trusted Firmware and Linux
kernel. Sequitur Labs also pulled together patches for OpenOCD to be able to
debug the solution using cheap JTAG debuggers. For more information about the
work, please see the [press
release](http://www.sequiturlabs.com/media_portfolio/sequitur-labs-collaborates-with-linaro-to-lower-barriers-to-iot-security-education-for-raspberry-pi-maker-community)
from June 8 2016.

# Contents
1. [Disclaimer](#1-disclaimer)
2. [Upstream?](#2-upstream)
3. [Build instructions](#3-build-instructions)
4. [Known problems](#4-known-problems)
5. [NFS boot](#5-nfs-boot)
6. [OpenOCD](#6-openocd)

# 1. Disclaimer
```
This port of ARM Trusted Firmware and OP-TEE to Raspberry Pi3

                   IS NOT SECURE!

Although the Raspberry Pi3 processor provides ARM TrustZone
exception states, the mechanisms and hardware required to
implement secure boot, memory, peripherals or other secure
functions are not available. Use of OP-TEE or TrustZone capabilities
within this package _does not result_ in a secure implementation.

This package is provided solely for educational purposes.
```

# 2. Upstream?
This is an initial drop with a working setup. But, there are quite a few
patches that are put on top of forks and some of the patches has been put
together by just pulling files instead of (correctly) cherry-pick patches from
various projects. For OP-TEE related gits, we will rather soon put together
proper patches and merge it upstream. But for the other projects it could take
some time to get the work accepted upstream. Due to this, everything will
initially not be on official Linaro git's and everything will be kept on a
separate branch. But as time goes by we will gradually move it over to the
official gits. We are fully aware that this is not the optimal way to do this,
but we also know that there is a strong interest among developers, students,
researches to start work and learn more about TEE's using a Raspberry Pi. So
instead of delaying this, we have decided to make what we have available
right away. Hopefully there will be some enthusiast that will help out
making proper upstream patches sooner or later.

| Project | Base fork | What to do |
|--------|--------|--------|
| build | the official build master branch | Rebase and do a pull request |
| optee_os | the official optee_os master branch | Rebase and do a pull request |
| linux | https://github.com/Electron752/linux.git commit: 51d1fa5c3208f15e80d25d85ce03330909916cc8 | Two things here. 1. The base is a fork itself and should be upstreamed. 2. The current OP-TEE patches should be replaced with cherry-picked patches from the official OP-TEE Linux kernel branch |
| arm-trusted-firmware | https://github.com/96boards-hikey/arm-trusted-firmware commit: bdec62eeb8f3153a4647770e08aafd56a0bcd42b | This should instead be based on the official OP-TEE fork or even better the official ARM repository. The patch itself should also be upstreamed. |
| U-boot | https://github.com:linaro-swg/u-boot.git | This is just a mirror of the official U-boot git. The patches should be upstreamed. |
| OpenOCD | TBD | TBD |

# 3. Build instructions
- First thing to pay attention to are the prerequisites stated
  [here](https://github.com/OP-TEE/optee_os#41-prerequisites) in the README.md
  file. If you forget that, then you can get all sorts of strange errors.

- From the [README.md](https://github.com/OP-TEE/optee_os#5-repo-manifests),
  you should follow section 5.1, 5.2. In short if you have repo installed, what
  you need to do is something like this:
```
$ mkdir rpi3
$ cd rpi3
$ repo init -u https://github.com/OP-TEE/manifest.git -m rpi3_experimental.xml
$ repo sync -j3
```
  Now it's probably a good idea to read the [Tips and tricks](https://github.com/OP-TEE/optee_os#58-tips-and-tricks)
  section also, since that will save a lot of time in the long run.

- Next step is to get the toolchains
```
$ cd build
$ make toolchains
```

- Then it is time to build everything. Note that the initial build will download
  a couple of files, like the official Raspberry Pi 3 firmware, the overlay root
  fs etc. However, that is only done once, so subsequent builds won't re-download
  them again (as long as you don't delete them).
```
$ make all
$ make update_rootfs
```

- The last step is to partition and format the memory card and to put the files
  onto the same. That is something we don't want to automate, since if anything
  goes wrong, in worst case it might wipe one of your regular hard disks. Instead
  what we have done, is that we have created another makefile target that will tell
  you exactly what to do. Run that command and follow the instructions there.
```
$ make img-help
```

- Boot up the Pi. With all files on the memory card, put the memory card into
the Raspberry Pi 3 and boot up the system. On the UART (we will add some wiring
diagram soon, but until then, please use Google and search for UART on Raspberry
Pi and you will get lots of guides) you will see the system booting up. When you
have a shell, then it's simply just to follow the instructions on
[here](https://github.com/OP-TEE/optee_os#6-load-driver-tee-supplicant-and-run-xtest)
in the README.md to load tee-supplicant and run xtest.

# 4. Known problems
We encourage anyone interested in getting this into a better shape to help out.
We have identified a couple issues while working with this. Some are harder to
solve than others.

## 4.1 Root file system
Currently we are using a cpio archive with busybox as a base, that works fine
and has a rather small footprint it terms of size. However in some cases it's
convenient to use something that reminds of what is used in distros. For
example having the ability to use a package manager like apt-get, pacman or rpm,
to make it easy to add new applications and developer tools.

Suggestions to look into regarding creating a better rootfs
- Create a setup where one use [buildroot](https://buildroot.org) instead of
  manually creating the cpio archive.
- Create a 64bit [Raspbian](https://www.raspbian.org) image. This would be the
  ultimate goal. Besides just the big work with building a 64bit Raspian image,
  one would also need to ensure that Linux kernel gets updated accordingly
  (i.e., pull 64bit RPi3 patches and OP-TEE patches into the official Raspbian
  Linux kernel build).

# 5. NFS Boot
Booting via NFS and TFTP is quite useful for several reasons, but the obvious
reason when working with Raspberry Pi is that you don't have to move the
SD-card back and forth between the host machine and the RPi itself. Below we
will describe how to setup both the TFTP part and the NFS part so we have both
ways covered. We will get kernel, optee.bin and the device tree blob from the
tftpd server and we will get the root fs from the NFS server. Note that this
guide doesn't focus on any desktop security, so eventually you would need to
harden your setup. Another thing is that this seems like a lot of steps, and it
is, but most of them is something you do once and never more and it will save
tons of time in the long run.

Note also, that this particular guide is written for the ARMv8-A setup using
OP-TEE. But, it should work on plain RPi also if you change U-boot and
filesystem accordingly.

In the description below we will use the following terminology:
```
HOST_IP=192.168.1.100   <--- This is your desktop computer
RPI_IP=192.168.1.200    <--- This is the Raspberry Pi
```

## 5.1 Configure TFTPD
There are several different servers to use, but in the description we're going
to use `atftpd`, so start by apt-get that package.
```
$ sudo apt-get install atftpd
```

Next edit the configuration file for atftpd
```
$ sudo vim /etc/default/atftpd
```

And change the files so it looks exactly like this, nothing less, nothing more!
```
USE_INETD=false
OPTIONS="--tftpd-timeout 300 --retry-timeout 5 --mcast-port 1758 --mcast-addr 239.239.239.0-255 --mcast-ttl 1 --maxthread 100 --verbose=5 /tftpboot"
```

Create the tftpboot folder and change the permissions
```
$ sudo mkdir /tftpboot
$ sudo chmod -R 777 /tftpboot
$ sudo chown -R nobody /tftpboot
```

And finally restart the daemon
```
$ sudo /etc/init.d/atftpd restart
```

## 5.2 Configure NFS
Start by installing the NFS server
```
$ sudo apt-get install nfs-kernel-server
```

Then edit the exports file,
```
$ sudo vim /etc/exports
```

In this file you shall tell where your files/folder are and what IP is allowed
to access the files. The way it's written below will make it available to every
machine on the same subnet (again, be careful about security here). Let's add
this line to the file (it's the only line necessary in the file).
```
/srv/nfs/rpi 192.168.1.0/24(rw,sync,no_root_squash,no_subtree_check)
```

Next create the folder and change some permissions.
```
$ sudo mkdir /srv/nfs/rpi
```

After this, restart the nfs kernel server
```
$ service nfs-kernel-server restart
```

## 5.3 Prepare files to be shared.
We need to prepare and put the files on the tftpd and the NFS-server. There are
several ways to do it, copy files, symlink etc.

### 5.3.1 Image, optee.bin and *.dtb
We're just going to create symlinks. By doing so you don't have to think about
copy files, just rebuild and you have the latest version available for the next
boot. On my computer I've symlinked like this:
```
$ ll
lrwxrwxrwx  1 jbech  jbech         65 jul 14 09:03 Image -> /home/jbech/devel/optee_projects/rpi3/linux/arch/arm64/boot/Image
lrwxrwxrwx  1 jbech  jbech         85 jul 14 09:03 optee.bin -> /home/jbech/devel/optee_projects/rpi3/arm-trusted-firmware/build/rpi3/debug/optee.bin
lrwxrwxrwx  1 jbech  jbech         90 Sep 13 11:19 bcm2710-rpi-3-b.dtb -> /home/jbech/devel/optee_projects/rpi3/linux/arch/arm64/boot/dts/broadcom/bcm2710-rpi-3-b.dtb
```

### 5.3.2 The root FS
We are now going to put the root fs on the location we prepare for NFS shared
in previous section (5.2). The path to the `filesystem.cpio.gz` will differ on
your machine, so update accordingly.
```
$ cd /srv/nfs/rpi
$ sudo gunzip -cd /home/jbech/devel/optee_projects/rpi3/build/../gen_rootfs/filesystem.cpio.gz | sudo cpio -idmv
$ sudo rm -rf /srv/nfs/rpi/boot/*
```

### 5.4 Update uboot.env
We need to make a couple of changes to that file to ensure that it will try to
boot using everything we have prepared. So, start by inserting the UART cable
and open up `/dev/ttyUSB0`
```
# sudo apt-get install picocom
$ picocom -b 115200 /dev/ttyUSB0
```

Power on the Raspberry Pi and almost immediately hit any key and you should see
the `U-Boot>` prompt. First add a new variable which will gather all files and
boot up the device. For simplicity I call that variable `optee`. So in the
prompt write (pay attention to the IP's used as described in the beginning of
this section):
```
U-Boot> setenv optee 'usb start; dhcp ${kernel_addr_r} 192.168.1.100:Image; dhcp ${fdt_addr_r} 192.168.1.100:${fdtfile}; dhcp ${atf_load_addr} 192.168.1.100:${atf_file}; run boot_it'
```

Also ensure that you have the variables stored that are used in the `optee`
U-Boot environment variable above. If you don't, then do:

```
U-Boot> setenv fdtfile 'bcm2710-rpi-3-b.dtb'
U-Boot> setenv atf_file 'optee.bin'
```

Next, we should update the kernel commandline to use NFS, to easier understand
what changes needs to be done I list both the unmodified command line and the
changed and correct one for NFS boot.

Original
```
setenv bootargs 'console=ttyS0,115200 root=/dev/mmcblk0p2 rw rootfs=ext4 ignore_loglevel dma.dmachans=0x7f35 rootwait 8250.nr_uarts=1 elevator=deadline fsck.repair=yes smsc95xx.macaddr=b8:27:eb:74:93:b0 bcm2708_fb.fbwidth=1920 bcm2708_fb.fbheight=1080 vc_mem.mem_base=0x3dc00000 vc_mem.mem_size=0x3f000000'
```

Updated for NFS boot
```
setenv bootargs 'console=ttyS0,115200 root=/dev/nfs rw rootfstype=nfs nfsroot=192.168.1.100:/srv/nfs/rpi,udp,vers=3  ip=dhcp ignore_loglevel dma.dmachans=0x7f35 rootwait 8250.nr_uarts=1 elevator=deadline fsck.repair=yes smsc95xx.macaddr=b8:27:eb:74:93:b0 bcm2708_fb.fbwidth=1920 bcm2708_fb.fbheight=1080 vc_mem.mem_base=0x3dc00000 vc_mem.mem_size=0x3f000000'
```

If you want those environment variables to persist between boots, then type.
```
U-Boot> saveenv
```

And don't worry about the `FAT: Misaligned buffer address ...` message, it will
still work.

## 5.5 Network boot the RPi
With all preparations done correctly above, you should now be able to boot up
the device and kernel, secure side OP-TEE and the entire root fs should be
loaded from the network shares. Power up the Raspberry, halt in U-Boot and then
type.
```
U-Boot> run optee
```

Profit!

## 5.6 Tricks
If everything works, you can simply copy paste files like xtest, the trusted
applications etc, directly from your build folder to the `/srv/nfs/rpi` folders
after rebuilding them. By doing so you don't have to reboot the device when
doing development and testing. Note that you cannot make symlinks to those like
we did with `Image`, `bcm2710-rpi-3-b.dtb` and `optee.bin`.

## 5.7 Other root filesystems than initramfs based?
The default root filesystem used for OP-TEE development is a simple CPIO archive
used as initramfs. That is small and is good enough for testing and debugging.
But sometimes you want to use a more traditional Linux filesystem, such as those
that are in distros. With such filesystem you can apt-get (if Debian based)
other useful tools, such as gdb on the device, valgrind etc to mention a few. An
example of such a rootfs is the [linaro-vivid-developer-20151215-114.tar.gz](http://releases.linaro.org/ubuntu/images/developer-arm64/15.12/linaro-vivid-developer-20151215-114.tar.gz),
which is an Ubuntu 15.04 based filesystem. The procedure to use that filesystem
with NFS is the same as for the CPIO based, you need to extract the files to a
folder which is known by the NFS server (use regular `tar -xvf ...` command).

Then you need to copy `xtest` and `tee-supplicant` to `/bin/`, copy `libtee.so*`
to `/lib/` and copy all `*.ta` files to `/lib/optee_armtz/`. Easiest here is to
write a small shell script or add a target to the make file which will do this
so the files always are up-to-date after a rebuild.

When that has been done, you can run OP-TEE tests, TA's etc.

# 6. OpenOCD
TDB (instructions how to debug OP-TEE using OpenOCD and JTAG debuggers).

## 6.1 Debug cable / UART cable
TBD
