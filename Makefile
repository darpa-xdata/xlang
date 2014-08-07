include Makefile.inc

help:
	@echo 'Top-level Makefile building the entire xlang codebase                  '
	@echo '                                                                       '
	@echo 'Usage:                                                                 '
	@echo '   make help                        prints this help message           '
	@echo '   make all                         makes the libs, tests, and apps    '
	@echo '   make libs                        makes all the libs in thunderdome  '
	@echo '                                      and webgraph.                    '
	@echo '   make tests                       makes all the tests and runs them  '
	@echo '   make apps                        makes all example apps             '
	@echo '   make clean                       deletes temporary files            '
	@echo '                                                                       '

all: libs tests apps

tests: libs
	$(MAKE) -C thunderdome test
	$(MAKE) -C webgraph/etl test
	$(MAKE) -C webgraph/cluster/r_fielder test
	$(MAKE) -C webgraph/cluster/gunrock test
	$(MAKE) -C webgraph/apps/c-fielder-igraph test
	$(MAKE) -C webgraph/apps/c-gunrock-bokeh test
	$(MAKE) -C webgraph/apps/py-c-tangelo test

apps:  libs
	$(MAKE) -C thunderdome examples
	$(MAKE) -C webgraph/apps/c-fielder-igraph app
	$(MAKE) -C webgraph/apps/c-fielder-igraph app
	$(MAKE) -C webgraph/cluster/r_fielder example
	$(MAKE) -C webgraph/apps/c-gunrock-bokeh app
	$(MAKE) -C webgraph/apps/py-c-tangelo app

libs: 
	$(MAKE) -C thunderdome libs
	$(MAKE) -C webgraph/etl c_import.o
	$(MAKE) -C webgraph/cluster/r_fielder example
	$(MAKE) -C webgraph/cluster/gunrock gunrock_clusters.o

clean:
	$(MAKE) -C thunderdome clean
	$(MAKE) -C webgraph/etl clean
	$(MAKE) -C webgraph/cluster/r_fielder clean
	$(MAKE) -C webgraph/cluster/gunrock clean
	$(MAKE) -C webgraph/apps/c-fielder-igraph clean
	$(MAKE) -C webgraph/apps/c-gunrock-bokeh clean
	$(MAKE) -C webgraph/apps/py-c-tangelo clean
