#!/usr/bin/env bash
scriptdir=/vagrant/scripts

echo "test for" > /echo.txt # 2112 test, if this file exists on the VM then this script ran

# this only needs to be done once
sudo apt-get update

# Install a Java runtime enviornment
sudo apt-get -y install openjdk-6-jdk
export JAVA_HOME=/usr/lib/jvm/java-6-openjdk-amd64
echo JAVA_HOME=/usr/lib/jvm/java-6-openjdk-amd64 | sudo tee -a /etc/environment


# Add your deployment scripts here:
sudo $scriptdir/helloworld.sh
sudo $scriptdir/R.sh
sudo $scriptdir/python.sh
sudo $scriptdir/julia.sh
sudo $scriptdir/tangelo.sh

