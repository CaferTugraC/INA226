CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g 			\
		 -I../../inc						\
		 -I ..								\
		 -I ../unity/scr					\

SRCS = test_mock_ina226.c					\
		../../scr/ina226.c					\
		../unity/scr/unity.c

TARGET = test_ina226_runner

all: ${TARGET}
	./${TARGET}

${TARGET}: ${SRCS}
	${CC} ${CFLAGS} ${SRCS} -o ${TARGET}

clean:
	rm -f ${TARGET}

.PHONY