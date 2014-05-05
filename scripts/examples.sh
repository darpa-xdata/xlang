#!/usr/bin/env bash

echo "test for" > /echo.txt
# good practics to do this before installing software
sudo apt-get update

# -y will install packages without human interaction
sudo apt-get -y install  openjdk-6-jdk
sudo apt-get -y install makepasswd

# An example of how to create a user
#PASSWORD=never_put_passwords_on_github
PASSWORD=`cat ~/.ssh/a_protected_file_with_a_secure_password_in_it`
export SHELL=/bin/bash
echo $PASSWORD | sudo tee -a /tmp/password.txt
passhash=$(makepasswd --clearfrom=/tmp/password.txt --crypt-md5 |awk '{print $2}')
sudo useradd hdfs -p $passhash
#sudo rm /tmp/password.txt

# copy hadoop files to the hdfs user home dir
pwd
sudo cp -r /vagrant/files/xdata_cdh4.2 /home/hdfs/
wget http://archive.cloudera.com/cdh4/cdh/4/hadoop-2.0.0-cdh4.2.2.tar.gz -P /home/hdfs/
sudo tar -xzf /home/hdfs/hadoop-2.0.0-cdh4.2.2.tar.gz -C /home/hdfs/
sudo ln -s /home/hdfs/hadoop-2.0.0-cdh4.2.2 /home/hdfs/hadoop
sudo chown -R hdfs:hdfs /home/hdfs/*

# Environment variables necessary to run hadoop
export HADOOP_HOME=/home/hdfs/hadoop
echo HADOOP_HOME=/home/hdfs/hadoop | sudo tee -a /etc/environment
export JAVA_HOME=/usr/lib/jvm/java-6-openjdk-amd64
echo JAVA_HOME=/usr/lib/jvm/java-6-openjdk-amd64 | sudo tee -a /etc/environment
