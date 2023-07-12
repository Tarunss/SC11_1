# $Id: Makefile,v 1.10 2021-09-05 12:44:19-07 - - $

MKFILE      = Makefile
DEPSFILE    = ${MKFILE}.deps
NOINCL      = ci clean spotless
NEEDINCL    = ${filter ${NOINCL}, ${MAKECMDGOALS}}
GMAKE       = ${MAKE} --no-print-directory
GPPOPTS     = -std=gnu++2a -fdiagnostics-color=never
GPPWARN     = -Wall -Wextra -Wpedantic -Wshadow -Wold-style-cast
GPP         = g++ ${GPPOPTS} ${GPPWARN}
COMPILECPP  = ${GPP} -g -O0 ${GPPOPTS}
MAKEDEPSCPP = ${GPP} -MM ${GPPOPTS}

MODULES     = ubigint bigint libfns scanner debug util
CPPHEADER   = ${MODULES:=.h} iterstack.h relops.h
CPPSOURCE   = ${MODULES:=.cpp} main.cpp
EXECBIN     = ydc
OBJECTS     = ${CPPSOURCE:.cpp=.o}
MODULESRC   = ${foreach MOD, ${MODULES}, ${MOD}.h ${MOD}.cpp}
OTHERSRC    = ${filter-out ${MODULESRC}, ${CPPHEADER} ${CPPSOURCE}}
ALLSOURCES  = ${MODULESRC} ${OTHERSRC} ${MKFILE}
LISTING     = Listing.ps

export PATH := ${PATH}:/afs/cats.ucsc.edu/courses/cse110a-wm/bin

all : ${EXECBIN}

${EXECBIN} : ${OBJECTS} ${MKFILE}
	${COMPILECPP} -o $@ ${OBJECTS}

%.o : %.cpp
	- checksource $<
	- cpplint.py.perl $<
	${COMPILECPP} -c $<

ci : check
	cid -is ${ALLSOURCES}

check : ${ALLSOURCES}
	- checksource ${ALLSOURCES}
	- cpplint.py.perl ${CPPSOURCE}

lis : ${ALLSOURCES}
	mkpspdf ${LISTING} ${ALLSOURCES} ${DEPSFILE}

clean :
	- rm ${OBJECTS} ${DEPSFILE} core ${EXECBIN}.errs

spotless : clean
	- rm ${EXECBIN} ${LISTING} ${LISTING:.ps=.pdf}


deps : ${CPPSOURCE} ${CPPHEADER}
	@ echo "# ${DEPSFILE} created `LC_TIME=C date`" >${DEPSFILE}
	${MAKEDEPSCPP} ${CPPSOURCE} >>${DEPSFILE}

${DEPSFILE} :
	@ touch ${DEPSFILE}
	${GMAKE} deps

again :
	${GMAKE} spotless deps ci all lis

ifeq (${NEEDINCL}, )
include ${DEPSFILE}
endif

