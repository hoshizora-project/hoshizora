DEBUG_BUILD_DIR := 'cmake-build-debug'
RELEASE_BUILD_DIR := 'cmake-build-release'

.PHONY: phony
phony: ;

.PHONY: debug
debug:
	mkdir -p ${DEBUG_BUILD_DIR}
	cd ${DEBUG_BUILD_DIR} && \
		export CXX=clang++ && \
		cmake -DCMAKE_BUILD_TYPE=Debug .. && \
		make -j 2

.PHONY: release
release:
	mkdir -p ${RELEASE_BUILD_DIR}
	cd ${RELEASE_BUILD_DIR} && \
		export CXX=clang++ && \
		cmake -DCMAKE_BUILD_TYPE=Release .. && \
		make -j 2

.PHONY: all
all: release debug

.PHONY: scan-debug
scan-debug:
	mkdir -p ${DEBUG_BUILD_DIR}
	cd ${DEBUG_BUILD_DIR} && \
		export CXX=clang++ && \
		scan-build cmake -DCMAKE_BUILD_TYPE=Debug .. && \
		scan-build make

.PHONY: scan-release
scan-release:
	mkdir -p ${RELEASE_BUILD_DIR}
	cd ${RELEASE_BUILD_DIR} && \
		export CXX=clang++ && \
		scan-build cmake -DCMAKE_BUILD_TYPE=Release .. && \
		scan-build make

.PHONY: format
format:
	zsh -c 'clang-format -i -style=file src/hoshizora/**/*.h'
	zsh -c 'clang-format -i -style=file src/hoshizora/main.cpp'

.PHONY: clean-debug
clean-debug:
	rm -rf ${DEBUG_BUILD_DIR}

.PHONY: clean-release
clean-release:
	rm -rf ${BUILD_DIR}

.PHONY: clean
clean: clean-debug clean-release
