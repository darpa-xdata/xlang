include ../../../Makefile.inc

CPPFLAGS += 		-Wall $(shell $(R_HOME)/bin/R CMD config CPPFLAGS)
CXXFLAGS += 		$(RCPPFLAGS) $(RCPPINCL) $(RINSIDEINCL) $(shell $(R_HOME)/bin/R CMD config CXXFLAGS)
LDLIBS += 		$(RLDFLAGS) $(RRPATH) $(RBLAS) $(RLAPACK) $(RCPPLIBS) $(RINSIDELIBS)

all: example

example:	r_env_example.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) r_env_example.cpp $(LDLIBS) -o r_env_example

test:
	@echo 'No tests included =('

clean:
			rm -vf r_env_example
			rm -vrf *.dSYM
