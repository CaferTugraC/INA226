CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g 			\
		 -I../../src						\
		 -I ..								\
		 -I ../unity/src					\

SRCS = test_mock_ina226.c					\
		../../src/ina226.c					\
		../unity/src/unity.c

BUILD_DIR = build

TARGET = ${BUILD_DIR}/test_ina226_runner

# Check for Unity submodule presence
ifeq ($(wildcard ../unity/src/unity.c),)
$(error Unity test framework not found. Please run: git submodule update --init --recursive)
endif

all: ${TARGET}
	./${TARGET}

test: all

${TARGET}: ${SRCS}
	@mkdir -p ${BUILD_DIR}
	${CC} ${CFLAGS} ${SRCS} -o ${TARGET}

clean:
	rm -rf ${BUILD_DIR}

.PHONY: all test clean