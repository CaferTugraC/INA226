CC ?= gcc

STD_FLAGS = -std=c99 -pedantic

WARN_FLAGS = -Wall -Wextra -Wformat=2

WARN_FLAGS += -Wconversion -Wsign-conversion
WARN_FLAGS += -Wshadow -Wundef -Wstrict-prototypes
WARN_FLAGS += -Wvla -Wdouble-promotion

DEBUG_FLAGS = -g -O0
SAN_FLAGS   = -fsanitize=address,undefined \
              -fno-sanitize-recover=all    \
              -fno-omit-frame-pointer

INC_FLAGS  = -I../../src \
             -I.. \
             -I../unity/src

CFLAGS  = $(STD_FLAGS) $(WARN_FLAGS) $(DEBUG_FLAGS) $(SAN_FLAGS) $(INC_FLAGS)
LDFLAGS = $(SAN_FLAGS)

COVERAGE ?= 0
ifeq ($(COVERAGE), 1)
    CFLAGS  += --coverage
    LDFLAGS += --coverage
endif

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
	${CC} ${CFLAGS} ${SRCS} $(LDFLAGS) -o ${TARGET}

coverage:
	$(MAKE) clean
	$(MAKE) COVERAGE=1
	@echo ""
	@echo "=== Code Coverage Report (ina226.c) ==="
	gcov -b -c -o ${BUILD_DIR}/${notdir ${TARGET}}-ina226 ../../src/ina226.c

clean:
	rm -rf ${BUILD_DIR} *.gcov

.PHONY: all test clean coverage