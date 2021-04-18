TARGET = librecnodb

# For BSD rules
.SUFFIXES: .e .c

# File locations
PREFIX ?= /usr/local
SRC = src

# Initialize with default, non-test, value
test ?= 0

# Compiler flags
CFLAGS = -Wall -Werror -std=c99 -pedantic -m64 -ggdb
LDFLAGS = 

# For a library, add -fPIC for relocatable function addresses, and possibly
# -fvisibility=hidden to restrict access to explicitely-revealed functions
CFLAGS != echo ${CFLAGS}; if [ ${test} -ne 1 ]; then echo " -fPIC -fvisibility=hidden"; fi

# TARGETS value set according to ${test} value:
LIB_TARGETS = ${TARGET}.a ${TARGET}.so
TEST_TARGETS != ls -1 ${SRC}/*.c | grep ^${SRC}/test | sed -e 's/\.c//g'
TARGETS != if [ ${test} -eq 1 ]; then echo ${TEST_TARGETS}; else echo ${LIB_TARGETS}; fi

# default rule should come before _most_, if not all, includes:
all: ${TARGETS}

# MODULES (target prerequisites) value set according ot ${test} value:
LIB_MODULES != ls -1 ${SRC}/*.c | grep -v ^${SRC}/test | sed 's/\.c/.o/g'
MODULES != if [ ${test} -eq 1 ]; then echo ${TEST_MODULES}; else echo ${LIB_MODULES}; fi

#########
# Includes here in case they prompt changes to MODULES
#########

# Remove duplicates that may have crept in:
MODULES != echo ${MODULES} | xargs -n1 | sort -u | xargs

# Project build rules

${TARGET}.a : ${MODULES}
	ar rcs $@ ${MODULES}

${TARGET}.so : ${MODULES}
	${CC} --shared -o $@ ${MODULES} ${LDFLAGS}

# %.o: %.c
.c.o:
	${CC} ${CFLAGS} -c -o $@ $<

.c.e:
	${CC} ${CFLAGS} -o $@ $<

# Other project rules:

install:
	install -D --mode=755 ${TARGET} ${PREFIX}/bin

clean:
	rm -f ${SRC}/*.o
	rm -f ${LIB_TARGETS}
	rm -f ${TEST_TARGETS}

show:
	@echo CFLAGS is ${CFLAGS}
	@echo MODULES is ${MODULES}
	@echo TARGETS is ${TARGETS}
