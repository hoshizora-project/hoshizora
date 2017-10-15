DEBUG_BUILD_DIR := 'cmake-build-debug'
BUILD_DIR := 'cmake-build'

.PHONY: phony
phony: ;

.PHONY: debug
debug:
	mkdir -p ${DEBUG_BUILD_DIR}
	cd ${DEBUG_BUILD_DIR} && \
		export CXX=clang++ && \
		cmake -DCMAKE_BUILD_TYPE=Debug .. && \
		make

.PHONY: release
release:
	mkdir -p ${BUILD_DIR}
	cd ${BUILD_DIR} && \
		export CXX=clang++ && \
		cmake -DCMAKE_BUILD_TYPE=Release .. && \
		make

.PHONY: all
all: release debug

.PHONY: clean-debug
clean-debug:
	rm -rf ${DEBUG_BUILD_DIR}

.PHONY: clean-release
clean-release:
	rm -rf ${BUILD_DIR}

.PHONY: clean
clean: clean-debug clean-release
