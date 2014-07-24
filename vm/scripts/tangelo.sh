#!/usr/bin/env bash

sudo apt-get install python-pip python-dev -y
sudo pip install tangelo
rm -rf /home/vagrant/tangelo-webroot
sudo -u vagrant mkdir -p /home/vagrant/tangelo-webroot
sudo -u vagrant -H git clone https://github.com/XDATA-Year-2/internet-graph /home/vagrant/tangelo-webroot/internet-graph
sudo -u vagrant -H tangelo restart --logdir /home/vagrant --root /home/vagrant/tangelo-webroot
