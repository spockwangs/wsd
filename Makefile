all :
	$(MAKE) -C src all
	$(MAKE) -C test all

clean :
	$(RM) -f include/*~
	$(MAKE) -C test clean
	$(MAKE) -C src clean

test :
	$(MAKE) -C src all
	$(MAKE) -C test test

.PHONY : all clean test
