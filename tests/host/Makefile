CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g 			\
		 -I../../inc						\
		 -I ..								\
		 -I ../unity/src					\

SRCS = test_mock_ina226.c					\
		../../src/ina226.c					\
		../unity/src/unity.c

BUILD_DIR = build

TARGET = ${BUILD_DIR}/test_ina226_runner

all: ${TARGET}
	./${TARGET}

${TARGET}: ${SRCS}
	${CC} ${CFLAGS} ${SRCS} -o ${TARGET}

clean:
	rm -f ${TARGET}

.PHONY: all clean