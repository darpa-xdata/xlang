#!/usr/bin/env bash

echo "test for" > /echo.txt
# do this before installing software
sudo apt-get update

# -y will install packages without human interaction
sudo apt-get -y install  openjdk-6-jdk
sudo apt-get -y install makepasswd

# an example of how to create a user semi-securely
#PASSWORD=never_put_passwords_on_github
PASSWORD=`cat ~/.ssh/a_protected_file_with_a_secure_password_in_it`
export SHELL=/bin/bash
echo $PASSWORD | sudo tee -a /tmp/password.txt
passhash=$(makepasswd --clearfrom=/tmp/password.txt --crypt-md5 |awk '{print $2}')
sudo useradd hdfs -p $passhash
#sudo rm /tmp/password.txt

# examples of how to wget and copy files around
pwd
sudo cp -r /vagrant/files/xdata_cdh4.2 /home/hdfs/
wget http://archive.cloudera.com/cdh4/cdh/4/hadoop-2.0.0-cdh4.2.2.tar.gz -P /home/hdfs/
sudo tar -xzf /home/hdfs/hadoop-2.0.0-cdh4.2.2.tar.gz -C /home/hdfs/
sudo ln -s /home/hdfs/hadoop-2.0.0-cdh4.2.2 /home/hdfs/hadoop
sudo chown -R hdfs:hdfs /home/hdfs/*

# how to set environment variables and make them permanent
export HADOOP_HOME=/home/hdfs/hadoop
echo HADOOP_HOME=/home/hdfs/hadoop | sudo tee -a /etc/environment
export JAVA_HOME=/usr/lib/jvm/java-6-openjdk-amd64
echo JAVA_HOME=/usr/lib/jvm/java-6-openjdk-amd64 | sudo tee -a /etc/environment
