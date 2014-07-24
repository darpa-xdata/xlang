#!/usr/bin/env bash



# Install Python libs
apt-get -y install python-pip python-scipy python-numpy python-virtualenv \
                   cython python-pycparser python-pyparsing python-yaml \
                   python-flask python-tables python-netcdf python-cffi \
                   redis-server python-redis python-pandas \
                   python-requests python-dateutil python-werkzeug \
                   python-greenlet ipython ipython-notebook
pip install multipledispatch six

# Install Continuum Tools
pip install git+http://github.com/ContinuumIO/datashape
pip install git+http://github.com/ContinuumIO/blz

rm -rf libdynd
git clone --depth=14 https://github.com/ContinuumIO/libdynd.git
mkdir libdynd/build
pushd libdynd/build
cmake -DDYND_BUILD_TESTS=False ..
make
mkdir ../lib
cp libdynd* ../lib
chmod +x libdynd-config
export PATH=$PATH:$PWD
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD
popd

rm -rf dynd-python
git clone --depth=14 https://github.com/ContinuumIO/dynd-python.git
mkdir dynd-python/build
pushd dynd-python/build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DUSE_SEPARATE_LIBDYND=ON -DCMAKE_INSTALL_PREFIX=${PWD} -DPYTHON_PACKAGE_INSTALL_PREFIX=${PWD} ..
make
make install
export PYTHONPATH=$PYTHONPATH:$PWD
popd

rm -rf blaze
git clone --depth=14 https://github.com/ContinuumIO/blaze.git
pushd blaze
python setup.py install
popd

rm -rf bokeh
git clone --depth=14 https://github.com/ContinuumIO/bokeh
pushd bokeh
python setup.py install
popd

