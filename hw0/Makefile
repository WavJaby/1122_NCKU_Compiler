CC := gcc
CFLAGS := -Wall -O0 -ggdb
LEX_SRC := ./compiler.l
BUILD := ./build
COMPILER := compiler
TEST := ../input

all: create_build_folder ${COMPILER} run

create_build_folder:
	mkdir -p ${BUILD}

${COMPILER}: lex.yy.c
	$(info ########## Create compiler ##########)
	${CC} ${CFLAGS} -o ${BUILD}/$@ -iquote ./ -iquote ../ ${BUILD}/lex.yy.c

lex.yy.c: ${LEX_SRC}
	lex -o ${BUILD}/lex.yy.c $<

run:
	$(info ########## Run program ##########)
	@./${BUILD}/${COMPILER} ${TEST}/subtask03-precedenc/testcase01.cpp output

clean:
	rm -f ${BUILD}/${COMPILER} ${BUILD}/lex.* output
