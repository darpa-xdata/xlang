#!/usr/bin/env bash

# CUDA install
wget http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1204/x86_64/cuda-repo-ubuntu1204_6.0-37_amd64.deb
sudo dpkg -i cuda-repo-ubuntu1204_6.0-37_amd64.deb
sudo apt-get update
sudo apt-get -y install cuda
export PATH=/usr/local/cuda-6.0/bin:$PATH
export LD_LIBRARY_PATH=/usr/local/cuda-6.0/lib64:$LD_LIBRARY_PATH

# CMAKE and BOOST (needed newer boost than ubuntu gives...)
sudo apt-get -y install cmake
wget http://downloads.sourceforge.net/project/boost/boost/1.55.0/boost_1_55_0.tar.gz
tar -xzf boost_1_55_0.tar.gz

# Build Gunrock
git clone https://github.com/gunrock/gunrock
pushd gunrock
git checkout dev
git submodule init
git submodule update --recursive
mkdir build
pushd build
cmake -DBoost_INCLUDE_DIR=$PWD/../../boost_1_55_0 ..
make
popd
popd
