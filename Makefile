.PHONY: clean web all

MAKE = make

all: clean cli web

cli:
	$(MAKE) -f Makefile.cli

web:
	$(MAKE) -f Makefile.web

atmega.build:
	$(MAKE) -f Makefile.atmega

atmega.copy:
	$(MAKE) -f Makefile.atmega copy

clean:
	$(MAKE) -f Makefile.cli clean
	$(MAKE) -f Makefile.web clean
