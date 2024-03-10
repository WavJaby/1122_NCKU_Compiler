CC := gcc
CFLAGS := -Wall -O0 -ggdb
YFLAG := -d -v
LEX_SRC := ./compiler.l
YAC_SRC := ./compiler.y
BUILD := ./build
HEADER := ./compiler_common.h
COMPILER := compiler
EXEC := Main
v := 0

all: create_build_folder ${COMPILER} compile debug run

.PHONY: main.c

create_build_folder:
	mkdir -p ${BUILD}

${COMPILER}: lex.yy.c y.tab.c main.c
	$(info ########## Create compiler ##########)
	${CC} ${CFLAGS} -o ${BUILD}/$@ -iquote ./ -iquote ../ ${BUILD}/lex.yy.c ${BUILD}/y.tab.c ${BUILD}/main.o

lex.yy.c: ${LEX_SRC} ${HEADER}
	lex -o ${BUILD}/lex.yy.c $<

y.tab.c: ${YAC_SRC} ${HEADER}
	yacc ${YFLAG} -o ${BUILD}/y.tab.c $<

main.c:
	gcc -g -c main.c -o ${BUILD}/main.o

compile:
	$(info ########## Compile script ##########)
	./${BUILD}/${COMPILER} test.cpp ${EXEC}

debug:
	$(info ########## Check output class file ##########)
	@javap -verbose ${EXEC}.class

run:
	$(info ########## Run program ##########)
	@java ${EXEC}

judge: all
	@judge -v ${v}

clean:
	rm -f ${BUILD}/${COMPILER} ${BUILD}/y.tab.* ${BUILD}/y.output ${BUILD}/lex.* ${EXEC}.class
