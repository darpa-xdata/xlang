#!/usr/bin/env bash

# Install git and emacs
sudo apt-get -y install git-all emacs24-nox gfortran m4 gcc g++ make
export JULIA_VAGRANT_BUILD=1

# setup julia
if [[ ! -e "julia" ]]; then
    git clone https://github.com/JuliaLang/julia
fi
cd julia
make LLVM_CONFIG=llvm-config-3.3 USE_BLAS64=0
sudo ln -s /home/vagrant/julia/julia /usr/bin/julia


