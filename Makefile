.POSIX :

TARGET = librecnodb

# For BSD rules
.SUFFIXES: .e .c .l

# File locations
PREFIX ?= /usr/local
SRC = src

# Initialize with default, non-test, value
test ?= 0

# Compiler flags (setting _POSIX_C_SOURCE=1 for stdio.h fileno() function.)
CFLAGS = -Wall -Werror -std=c99 -pedantic -m64 -ggdb -D_POSIX_C_SOURCE=1
LDFLAGS = 

CFLAGS += -Wpadded

# For a library, add -fPIC for relocatable function addresses, and possibly
# -fvisibility=hidden to restrict access to explicitely-revealed functions
CFLAGS != echo ${CFLAGS}; if [ ${test} -ne 1 ]; then echo " -fPIC -fvisibility=hidden"; fi

# TARGETS value set according to ${test} value:
LIB_TARGETS = ${TARGET}.a ${TARGET}.so
TEST_M_TARGETS != ls -1 ${SRC}/*.c | grep ^${SRC}/test_ | sed -e 's/\.c/.e/g'
TEST_L_TARGETS != ls -1 ${SRC}/*.c | grep ^${SRC}/testl_ | sed -e 's/\.c/.l/g'
TARGETS != \
	if   [ ${test} -eq 0 ]; then echo ${LIB_TARGETS}; \
	elif [ ${test} -eq 1 ]; then echo ${TEST_M_TARGETS}; \
	elif [ ${test} -eq 2 ]; then echo ${TEST_L_TARGETS}; fi

# TARGETS != if [ ${test} -eq 1 ]; then echo ${TEST_L_TARGETS} ${TEST_M_TARGETS}; \
#     else echo ${LIB_TARGETS}; fi

# For our purposes, any changed header file triggers rules
HEADERS != ls -1 ${SRC}/*.h

# default rule should come before _most_, if not all, includes:
all: ${TARGETS}

# MODULES (target prerequisites) value set according ot ${test} value:
LIB_MODULES != ls -1 ${SRC}/*.c | grep -v ^${SRC}/test | sed 's/\.c/.o/g'
MODULES != if [ ${test} -ne 1 ]; then echo ${LIB_MODULES}; fi

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

.c.o:
	@echo Suffix match build .o from .c files
	${CC} ${CFLAGS} -c -o $@ $<

.c.e:
	@echo Suffix match build .e from .c file to build test executable
	${CC} ${CFLAGS} -o $@ $<

.c.l:
	@echo Suffix match build .l from .c file to compile with library
	@echo "Building library test file"
	${CC} ${CFLAGS} -o $@ $< ${TARGET}.a


# %.o : %.c
# 	@echo Pattern match build .o from .c files
# 	${CC} ${CFLAGS} -c -o $@ $<

# $.e : %.c
# 	@echo Pattern match build .e from .c file to build test executable
# 	${CC} ${CFLAGS} -o $@ $<

# %.l : %.c
# 	@echo Pattern match build .l from .c file to compile with library
# 	${CC} ${CFLAGS} -o $@ $< ${TARGET}.a

# Other project rules:

install:
	install -D --mode=755 ${TARGET} ${PREFIX}/bin

test: ${MODULES} ${TEST_L_TARGETS} ${TEST_M_TARGETS}

clean:
	rm -f ${SRC}/*.o
	rm -f ${LIB_TARGETS}
	rm -f ${TEST_L_TARGETS}
	rm -f ${TEST_M_TARGETS}

show:
	@echo CFLAGS is ${CFLAGS}
	@echo MODULES is ${MODULES}
	@echo TARGETS is ${TARGETS}
	@echo HEADERS is ${HEADERS}
	@echo TEST_M_TARGETS is ${TEST_M_TARGETS}
	@echo All test targets is ${MODULES} ${TEST_M_TARGETS} ${TEST_L_TARGETS}
