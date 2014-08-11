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
#wget http://cran.r-project.org/src/contrib/Rcpp_0.11.2.tar.gz
#sudo R CMD INSTALL Rcpp_0.11.2.tar.gz
#rm Rcpp_0.11.1.tar.gz
sudo Rscript -e "install.packages('Rcpp', repos='http://cran.r-project.org')"

# Install RInside R package
wget http://cran.r-project.org/src/contrib/RInside_0.2.11.tar.gz
sudo R CMD INSTALL RInside_0.2.11.tar.gz
rm RInside_0.2.11.tar.gz

# Install Matrix R package
wget http://cran.r-project.org/src/contrib/Matrix_1.1-4.tar.gz
sudo R CMD INSTALL Matrix_1.1-4.tar.gz
rm Matrix_1.1-4.tar.gz

# Install iterators R package
wget http://cran.r-project.org/src/contrib/iterators_1.0.7.tar.gz
sudo R CMD INSTALL iterators_1.0.7.tar.gz
rm iterators_1.0.7.tar.gz

# Install foreach R package
wget http://cran.r-project.org/src/contrib/foreach_1.4.2.tar.gz
sudo R CMD INSTALL foreach_1.4.2.tar.gz
rm foreach_1.4.2.tar.gz

# Install itertools R package
wget http://cran.r-project.org/src/contrib/itertools_0.1-3.tar.gz
sudo R CMD INSTALL itertools_0.1-3.tar.gz
rm itertools_0.1-3.tar.gz

# Install doMC R package
wget http://cran.r-project.org/src/contrib/doMC_1.3.3.tar.gz
sudo R CMD INSTALL doMC_1.3.3.tar.gz
rm doMC_1.3.3.tar.gz

# Install devtools R package
# Note that we are using an old enough version of R that devtools is not
# available.
#Rscript -e "install.packages('devtools', repos='http://cran.r-project.org')"

# Install the IRL package
# Rscript -e "devtools::install_github('bwlewis/IRL')"
wget https://github.com/bwlewis/IRL/archive/master.zip
unzip master.zip
sudo R CMD INSTALL IRL-master
rm -rf IRL-master master.zip

# Install SparseM R package
wget http://cran.r-project.org/src/contrib/SparseM_1.05.tar.gz
sudo R CMD INSTALL SparseM_1.05.tar.gz
rm SparseM_1.05.tar.gz

# Install optparse R package
sudo Rscript -e "install.packages('optparse', repos='http://cran.r-project.org')"

# Install igraph R package
sudo Rscript -e "install.packages('igraph', repos='http://cran.r-project.org')"

# Install d3Network R package
sudo Rscript -e "install.packages('d3Network', repos='http://cran.r-project.org')"
