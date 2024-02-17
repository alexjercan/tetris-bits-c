.PHONY: clean web all

MAKE = make

all: clean cli web

cli:
	$(MAKE) -f Makefile.cli

web:
	$(MAKE) -f Makefile.web

clean:
	$(MAKE) -f Makefile.cli clean
	$(MAKE) -f Makefile.web clean
