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
conventient to use something that reminds of what is used in distros. For
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
reason when working with Raspberry Pi is that you don't have to moving the
SD-card back and forth between the host machine and the RPi itself. Below we
will describe how to setup both the TFTP part and the NFS part so we have both
ways covered. We will get kernel and optee.bin from the tftpd server and we will
get the root fs from the nfs server. Note that this guide doesn't focus on any
desktop security, so eventually you would need to harden your setup. Another
thing is that this seems like a lot of steps, and it is, but most of them is
something you do once and never more.

Note also, that this particular guide is written for the ARMv8-A setup using
OP-TEE. But, it should work on plain RPi also if you change U-boot and
filesystem accordingly.

In the description below we will use the following terminology:
```
HOST_IP=192.168.1.61   <--- This is your desktop computer
RPI_IP=192.168.1.190   <--- This is the Raspberry Pi
```

## 5.1 Configure TFTPD
There are several differnet servers to use, but in the description we're going
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
to access the the files. The way it's written below will make it available to
every machine on the same subnet (again, be careful about security here). Let's
add this line to the file (only line necessary in the file).
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

## 5.3 Enable the RPi network in Linux kernel
In the default setup, the network card is configured as a module. We must change
that so it's built into the kernel. To do that you need to change a couple of
Kconfig flags and the following changes I did are the ones below:

```
-# CONFIG_IP_PNP is not set
+CONFIG_IP_PNP=y
+CONFIG_IP_PNP_DHCP=y
+# CONFIG_IP_PNP_BOOTP is not set
+# CONFIG_IP_PNP_RARP is not set
-CONFIG_MII=m
+CONFIG_MII=y
-CONFIG_USB_NET_DRIVERS=m
+CONFIG_USB_NET_DRIVERS=y
-CONFIG_USB_USBNET=m
+CONFIG_USB_USBNET=y
-CONFIG_USB_NET_SMSC95XX=m
+CONFIG_USB_NET_SMSC95XX=y
+CONFIG_ROOT_NFS=y
```

After that you should rebuild the kernel again.

## 5.4 Prepare files to be shared.
We need to prepare and put the files on the tftpd and the nfs-server. There are
several ways to do it, copy files, symlink etc.

### 5.4.1 Image and optee.bin
optee.bin we're just going to create symlinks. By doing so you don't have to
think about copy files, just rebuild and you have the latest version available
for the next boot. On my computer I've symlinked like this:
```
$ ll
lrwxrwxrwx  1 jbech  jbech         65 jul 14 09:03 Image -> /home/jbech/devel/optee_projects/rpi3/linux/arch/arm64/boot/Image
lrwxrwxrwx  1 jbech  jbech         85 jul 14 09:03 optee.bin -> /home/jbech/devel/optee_projects/rpi3/arm-trusted-firmware/build/rpi3/debug/optee.bin
```

### 5.4.2 The root FS
We are now going to put the root fs on the location we prepare for nfs shared
in previous section (5.2). The path to the `filesystem.cpio.gz` will differ on
your machine, so update accordingly.
```
$ cd /srv/nfs/rpi
$ sudo gunzip -cd /home/jbech/devel/optee_projects/rpi3/build/../gen_rootfs/filesystem.cpio.gz | sudo cpio -idmv
$ sudo rm -rf /srv/nfs/rpi/boot/*
```

### 5.5 Update uboot.env
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
U-Boot> setenv optee 'usb start; dhcp ${kernel_addr_r} 192.168.1.61:Image; run load_dtb; dhcp ${atf_load_addr} 192.168.1.61:optee.bin; run boot_it'
```

Next, we should update the kernel commandline to use nfs, to easier understand
what changes needs to be done I list both the unmodified command line and the
changed and correct one for NFS boot.

Original
```
setenv bootargs 'console=ttyS0,115200 root=/dev/mmcblk0p2 rw rootfs=ext4 ignore_loglevel dma.dmachans=0x7f35 rootwait 8250.nr_uarts=1 elevator=deadline fsck.repair=yes smsc95xx.macaddr=b8:27:eb:74:93:b0 bcm2708_fb.fbwidth=1920 bcm2708_fb.fbheight=1080 vc_mem.mem_base=0x3dc00000 vc_mem.mem_size=0x3f000000'
```

Updated for NFS boot
```
setenv bootargs 'console=ttyS0,115200 root=/dev/nfs rw rootfstype=nfs nfsroot=192.168.1.61:/srv/nfs/rpi,udp,vers=3  ip=dhcp ignore_loglevel dma.dmachans=0x7f35 rootwait 8250.nr_uarts=1 elevator=deadline fsck.repair=yes smsc95xx.macaddr=b8:27:eb:74:93:b0 bcm2708_fb.fbwidth=1920 bcm2708_fb.fbheight=1080 vc_mem.mem_base=0x3dc00000 vc_mem.mem_size=0x3f000000'
```

If you want those environment variables to persist between boots, then type.
```
U-Boot> saveenv
```

And don't worry about the `FAT: Misaligned buffer address ...` message, things
will still work.

## 5.6 Network boot the RPi
With all preparations done correctly above, you should now be able to boot up
the device and kernel, secure side OP-TEE and the entire root fs should be
loaded from the network shares. Power up the Raspberry, halt in U-Boot and then
type.
```
U-Boot> run optee
```

Profit!

## 5.7 Tricks
If everything works, you can simply copy paste files like xtest, the trusted
applications etc, directly from your build folder to the `/srv/nfs/rpi` folders
after rebuilding them. By doing so you don't have to reboot the device when
doing development and testing. Note that you cannot make symlinks to those like
we did with `Image` and `optee.bin`.


# 6. OpenOCD
TDB (instructions how to debug OP-TEE using OpenOCD and JTAG debuggers).

## 5.1 Debug cable / UART cable
TBD
