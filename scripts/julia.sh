#!/usr/bin/env bash

apt-get update -qq -y
apt-get install g++ git make gfortran -y

if [[ ! -e "julia" ]]; then
    git clone https://github.com/JuliaLang/julia
fi
if [[ ! -e "Stage.jl" ]]; then
    git clone https://github.com/SaltPork/Stage.jl
fi

cd julia
make
rm -f /usr/bin/julia
cd /usr/bin
ln -s /home/vagrant/julia/julia .

# install packages
cd /home/vagrant
sudo -u vagrant echo "$HOME"
sudo -u vagrant echo 'Pkg.init(); Pkg.add("GZip");' > /tmp/init.jl
sudo -u vagrant -H /usr/bin/julia /tmp/init.jl
cd /home/vagrant/.julia/v0.3
sudo -u vagrant rm -f Stage.jl
sudo -u vagrant ln -s /home/vagrant/Stage.jl .

