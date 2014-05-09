#!/usr/bin/env bash

# Add CRAN Ubuntu repository
echo "deb http://cran.rstudio.com/bin/linux/ubuntu saucy/" | sudo tee -a /etc/apt/sources.list
gpg --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys E084DAB9
gpg -a --export E084DAB9 | sudo apt-key add -
sudo apt-get -y update

# Install R
sudo apt-get -y install r-base-dev

# sudo su - -c "R -e \"install.packages('RInside', repos='http://cran.rstudio.com/')\""

# Install Rcpp R package
wget http://cran.r-project.org/src/contrib/Rcpp_0.11.1.tar.gz
sudo R CMD INSTALL Rcpp_0.11.1.tar.gz
rm Rcpp_0.11.1.tar.gz

# Install RInside R package
wget http://cran.r-project.org/src/contrib/RInside_0.2.11.tar.gz
sudo R CMD INSTALL RInside_0.2.11.tar.gz
rm RInside_0.2.11.tar.gz

