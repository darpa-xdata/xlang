#!/usr/bin/env bash

echo "test for" > /echo.txt
sudo apt-get update
# Hadoop requires a Java runtime enviornment be available
sudo apt-get -y install  openjdk-6-jdk
# Necessary for automatically creating the hdfs user 
sudo apt-get -y install makepasswd

# Create the hdfs user
PASSWORD=xdata
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
