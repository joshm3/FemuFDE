#!/bin/bash

cd ~

#Make sure everything is updated
sudo apt update

#First get FEMU and setup VM
git clone https://github.com/vtess/femu.git
cd femu
mkdir build-femu
cd build-femu
cp ../femu-scripts/femu-copy-scripts.sh .
./femu-copy-scripts.sh .
sudo ./pkgdep.sh
./femu-compile.sh

#Now setup ~/images/u20s.qcow2?
cd ~
mkdir ./images
cd ./images
wget http://people.cs.uchicago.edu/~huaicheng/femu/femu-vm.tar.xz
tar -xjvf femu-vm.tar.xz #changed this recently may not work

#Now install the changes in femu for encryption delay
cd ~/FemuFDE
./putCryptFemu.sh

cd ~/femu/build-femu
#now you can run ./run-blackbox.sh - doesn;t always put you here?
