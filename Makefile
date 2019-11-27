.PHONY: default
default: build ;

help:
	@echo "Without and target, the daemon, library and tools will be built."
	@echo "The following additional makefile targets are supported:"
	@echo ""
	@echo " help	  - display this help message."
	@echo " clean	  - cleans all build files."
	@echo " build	  - builds the daemon, library and tools."
	@echo " slimbuild - builds the daemon, library and tools without ARES, MDBM & LibEvent."
	@echo " doc	  - build the doxygen."
	@echo " all	  - builds the daemon, library and tools, conducts unit tests, and generates the doxygen."
	@echo " install   - installs the daemon, library and tools."
	@echo " test 	  - build and run the unit tests."

clean:
	rm -rf build; rm -rf build_test; rm -rf api/netchasm;rm -rf proto/netchasm/*.cc; rm -rf proto/netchasm/*.h;

doc:
	doxygen DoxyFile
	
build:
	mkdir -p build; cd build; cmake .. -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc; make build; cd ..;

slimbuild:
	mkdir -p build; cd build; cmake .. -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DSKIP-MDBM=ON -DSKIP-ARES=ON -DSKIP-LIBEVENT=ON -DSKIP-KAFKA=ON -DSKIP-RAPIDXML=ON; make build
test:
	mkdir -p build_test; cd build_test; cmake .. -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DCOV=ON -DSKIP-IPV6=ON; make testbuild; cd ..;

all: build test doc

install: build
	cd build; make install; cd ..;
