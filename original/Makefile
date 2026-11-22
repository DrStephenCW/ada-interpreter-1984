# $Header:   /ub/darpa/ada/RCS/Makefile.v  Revision 1.20  83/11/30  15:08:32  ada  Exp$
#
# $Log:	/ub/darpa/ada/RCS/Makefile.v $
# Revision 1.20  83/11/30  15:08:32  ada
# Updated revision number.  Added files in subdirectory lib to STANDARDS.
# 
# Revision 1.19  83/09/21  09:45:00  ada
# same as 1.18.5.1.  Updated revision number.
# 
# Revision 1.18.5.1  83/09/20  16:03:24  snider
# ada and uui now have their own time stamps.  This ends the problem of
# restoring from a core file for the wrong linkage of the system.
# 
# Revision 1.18  83/09/08  12:55:57  ada
# same as 1.17.5.1.  Updated revision number.
# 
# Revision 1.17.5.1  83/09/08  12:49:52  snider
# added c cross-referencing tool c_cref.  'make Arcturus_cref' and
# 'make uui_cref' will do extensive cross references of the files 
# involved in those subsystems, and send the results to the line-printer.
# 'make c_cref' will generate the object code for the tool.
# 
# Revision 1.17  83/08/08  16:26:53  ada
# same as 1.16.5.1
# 
# Revision 1.16.5.1  83/08/08  15:20:11  snider
# alphabetized the uui sources, got rid of backend.h 
# 
# Revision 1.16  83/07/11  11:37:48  ada
# Changed config.o to config.c in CSOURCES.
# 
# Revision 1.15  83/06/28  15:18:04  ada
# merged 1.14 and 1.12.6.1
# 
# Revision 1.14  83/06/14  16:55:07  ada
# Updated revision to 1.5a.
# 
# Revision 1.13  83/06/14  16:26:48  ada
# Same as 1.12.4.1.
# 
# Revision 1.12.4.1  83/06/14  13:41:57  tadman
# paraux.o depends on arr.h
# 
# Revision 1.12.6.1  83/06/23  14:09:59  klefstad
# I eliminated the use of the 'jobs' library functions which are non-standart
# Unix library functions.
# 
# Revision 1.12  83/05/23  12:13:32  ada
# merged versions 1.11 and 1.9.6.1
# 
# Revision 1.11  83/05/06  14:49:37  ada
# updated release number.
# 
# Revision 1.10  83/05/04  15:45:44  ada
# added time_stamp.c compilation dependency for version.c
# 
# Revision 1.9.6.1  83/03/29  12:24:47  klefstad
# Added feature for printing all of the sources of the ADASH front end.
# Instead of typing "make print" as one does to print the sources to ADA,
# type "make uuiprint"
# 
# Revision 1.9  83/03/10  15:00:18  ada
# Same as 1.8.2.1
# 
# Revision 1.8.2.1  83/03/10  14:40:44  willson
# Little fixups
# 
# Revision 1.7  83/01/14  11:42:28  ada
# updated release number to 1.4-D
# 
# Revision 1.6  83/01/12  09:39:22  ada
# Added coyright notice.
# 
# Revision 1.5  83/01/11  15:14:00  ada
# Merged 1.4.2.1 (willson) and 1.1.6.1 (klefstad, should really be
# 1.3.6.1 since was derived from 1.3).  Changed Arcturus revision to 1.4B.
# 
#
#*******************************************************************
#*
#* Copyright 1983      Thomas A. Standish
#*		       Programming Environment Project
#*                     Department of Information and Computer Science
#*                     University of California
#*                     Irvine, California  92717
#*
#* All rights reserved. No part of this software may be sold or distributed
#* in any form or by any means without the prior written permission of the 
#* author.
#*
#* Report problems and direct all inquiries to ada%uci@rand-relay (ARPA net).
#*

MAJOR = 1
MINOR = 6
FIX = A
SYSTEM = Arcturus
OBJECTS = attach.o call.o case.o casfns.o casint.o cassym.o \
	catch.o config.o conv.o count.o \
	cref.o crefaux.o creftab.o dir.o dump.o error.o except.o\
	goto.o ifp.o init.o \
	int.o intaux.o intform.o \
	loop.o maptree.o misc.o ntabs.o \
	object.o oblist.o  \
	paraux.o parerr.o parser.o pp.o ppattach.o \
	ppaux.o ppcolor.o ppcref.o \
	pragint.o rsw.o scanner.o scope.o stack.o symtab.o \
	tokens.o toplev.o tyint.o typ.o typeonly.o version.o
CSOURCES = adacref.c attach.c case.c casfns.c casint.c cassym.c \
	catch.c config.c conv.c count.c \
	cref.c crefaux.c creftab.c dir.c dump.c \
	error.c except.c goto.c \
	ifp.c \
	init.c int.c intaux.c intform.c \
	loop.c maptree.c misc.c ntabs.c \
	object.c oblist.c paraux.c parerr.c \
	pp.c ppattach.c ppaux.c ppcolor.c ppcref.c \
	pragint.c rsw.c scope.c stack.c symtab.c \
	tokens.c toplev.c tyint.c typ.c \
	typeonly.c version.c
ASOURCES = call.s
CDEFS = adaio.h arr.h case.h castor.h color.h count.h cref.h eval.h \
	handler.h int.h macros.h nodes.h oblist.h \
	parser.h pp.h scan.h sym.h tab.h version.h
STANDARDS = standard pp_functions text_io standard.str uci_ada_fns \
	user_package internal_fns lib/internal.S lib/internal.c lib/castor
CFLAGS = -c -g -p
LDFLAGS = -g -n -z
YFLAGS = -d
YCFLAGS = -DYYDEBUG
LXFLAGS =
LXCFLAGS = 
LIBES = -ll


UUISOURCES = adacomp.h adaio.h color.h compscan.h fns.h globals.h \
	mpx.h std.h term.h trie.h \
	achar.c aggcomp.c attrcomp.c box.c buffer.c \
	color.c comcom.c comhelp.c context.c context_fns.c \
	defargs.c display.c editfns.c exprcomp.c exprhelp.c \
	fedit.c fieldcomp.c formfill.c finit.c fsave.c \
	high.c indexcomp.c initcomp.c io.c intrin.c \
	letter_case.c lisp.c literalcomp.c low.c \
	makecomp.c marks.c menus.c move.c movefield.c \
	movetoken.c mpx.c \
	packcomp.c paramcomp.c repeat.c \
	scan.c screen_pos.c shell.c strings.c subbackend.c \
	subpcomp.c sysmsg.c \
	tableau.c term.c termsetup.c trie.c typecomp.c typein.c \
	utilcomp.c uui.c varcomp.c

UUIOTHERSOURCES = ash.c forks.c main.c

EDITOBJ = achar.o mpx.o box.o buffer.o comcom.o comhelp.o \
	defargs.o display.o editfns.o fedit.o finit.o fsave.o high.o \
	intrin.o lisp.o low.o marks.o menus.o move.o repeat.o scan.o shell.o \
	trie.o typein.o io.o movetoken.o sysmsg.o letter_case.o \
	uui.o screen_pos.o strings.o color.o term.o termsetup.o tableau.o \
	movefield.o formfill.o

CONTEXTOBJ = context.o context_fns.o

COMRECOBJ = attrcomp.o literalcomp.o makecomp.o subpcomp.o \
	varcomp.o aggcomp.o\
	indexcomp.o exprcomp.o packcomp.o typecomp.o fieldcomp.o utilcomp.o \
	initcomp.o paramcomp.o exprhelp.o

EDITOROBJECTS = $(EDITOBJ) subbackend.o

FRONTOBJECTS = $(EDITOBJ) $(CONTEXTOBJ) $(COMRECOBJ)

HEADERS = globals.h

ada:	$(OBJECTS) command.o 
	@echo updating version
	@csh -f version.gen $(SYSTEM) $(MAJOR) $(MINOR) $(FIX)
	@ifwrite version.o cc $(CFLAGS) version.c
	@echo linking ada
	@cc $(LDFLAGS) $(OBJECTS) command.o $(LIBES) -o ada

uui:	adash adashbase ash ashbase
	touch uui

adacref: adacref.o creftab.o
	cc $(LDFLAGS) adacref.o creftab.o -o adacref

adacref.o: adacref.c cref.h
	ifwrite adacref.o cc $(CFLAGS) adacref.c

version.o: version.c
	csh -f version.gen $(SYSTEM) $(MAJOR) $(MINOR) $(FIX)
	ifwrite version.o cc $(CFLAGS) version.c

time_stamp: time_stamp.c
	ifwrite time_stamp cc time_stamp.c -o time_stamp

version.c: time_stamp
	touch version.c

attach.o: attach.c macros.h nodes.h int.h
	ifwrite attach.o cc $(CFLAGS) attach.c

count.o: count.c macros.h adaio.h nodes.h int.h sym.h tab.h eval.h count.h
	ifwrite count.o cc $(CFLAGS) count.c

maptree.o: maptree.c macros.h nodes.h int.h
	ifwrite maptree.o cc $(CFLAGS) maptree.c

creftab.o: creftab.c
	ifwrite creftab.o cc $(CFLAGS) creftab.c

object.o: object.c int.h nodes.h macros.h oblist.h
	ifwrite object.o cc $(CFLAGS) object.c

call.o: call.s
	ifwrite call.o cc -c call.s

init.o: init.c nodes.h int.h sym.h tab.h arr.h adaio.h macros.h oblist.h
	ifwrite init.o cc $(CFLAGS) init.c

ifp.o: ifp.c int.h nodes.h macros.h
	ifwrite ifp.o cc $(CFLAGS) ifp.c

stack.o: stack.c macros.h handler.h
	ifwrite stack.o cc $(CFLAGS) stack.c

case.o: case.c eval.h int.h nodes.h sym.h tab.h eval.h macros.h case.h
	ifwrite case.o cc $(CFLAGS) case.c

oblist.o: oblist.c macros.h oblist.h
	ifwrite oblist.o cc $(CFLAGS) oblist.c

symtab.o: symtab.c sym.h nodes.h int.h tab.h eval.h color.h macros.h oblist.h
	ifwrite symtab.o cc $(CFLAGS) symtab.c

typeonly.o: macros.h nodes.h int.h sym.h tab.h eval.h typeonly.c
	ifwrite typeonly.o cc $(CFLAGS) typeonly.c

typ.o: typ.c sym.h nodes.h int.h tab.h eval.h color.h macros.h oblist.h
	ifwrite typ.o cc $(CFLAGS) typ.c

dump.o: dump.c sym.h nodes.h int.h tab.h eval.h color.h macros.h oblist.h
	ifwrite dump.o cc $(CFLAGS) dump.c

dir.o: dir.c sym.h nodes.h int.h tab.h eval.h color.h macros.h oblist.h pp.h
	ifwrite dir.o cc $(CFLAGS) dir.c

error.o: error.c macros.h version.h
	ifwrite error.o cc $(CFLAGS) error.c

int.o: int.c handler.h int.h nodes.h sym.h eval.h tab.h adaio.h macros.h oblist.h
	ifwrite int.o cc $(CFLAGS)  int.c

intaux.o: intaux.c int.h nodes.h sym.h eval.h tab.h arr.h\
		 count.h macros.h oblist.h
	ifwrite intaux.o cc $(CFLAGS)  intaux.c

scope.o: scope.c macros.h nodes.h int.h sym.h tab.h eval.h
	ifwrite scope.o cc $(CFLAGS) scope.c

tyint.o: tyint.c int.h nodes.h sym.h eval.h tab.h macros.h
	ifwrite tyint.o cc $(CFLAGS)  tyint.c

except.o: except.c macros.h nodes.h int.h sym.h tab.h eval.h oblist.h
	ifwrite except.o cc $(CFLAGS) except.c

goto.o: goto.c macros.h nodes.h int.h sym.h tab.h eval.h
	ifwrite goto.o cc $(CFLAGS) goto.c

loop.o: loop.c macros.h nodes.h int.h sym.h tab.h eval.h
	ifwrite loop.o cc $(CFLAGS) loop.c

misc.o: misc.c eval.h int.h nodes.h sym.h macros.h oblist.h case.h
	ifwrite misc.o cc $(CFLAGS) misc.c

ppcref.o: ppcref.c macros.h nodes.h int.h sym.h tab.h \
	adaio.h count.h arr.h pp.h
	ifwrite ppcref.o cc $(CFLAGS) ppcref.c

pp.o: pp.c int.h nodes.h count.h macros.h adaio.h pp.h
	ifwrite pp.o cc $(CFLAGS) pp.c

ppattach.o: ppattach.c macros.h nodes.h int.h
	ifwrite ppattach.o cc $(CFLAGS) ppattach.c

ppaux.o: ppaux.c nodes.h int.h macros.h color.h pp.h
	ifwrite ppaux.o cc $(CFLAGS) ppaux.c

ppcolor.o: macros.h nodes.h int.h count.h color.h pp.h
	ifwrite ppcolor.o cc $(CFLAGS) ppcolor.c

cref.o: cref.c nodes.h int.h macros.h sym.h tab.h eval.h cref.h
	ifwrite cref.o cc $(CFLAGS) cref.c

crefaux.o: crefaux.c nodes.h int.h macros.h sym.h tab.h eval.h cref.h
	ifwrite crefaux.o cc $(CFLAGS) crefaux.c

ntabs.c: nodes.h
	csh ntabs.gen

ntabs.o: ntabs.c macros.h nodes.h int.h
	ifwrite ntabs.o cc $(CFLAGS) ntabs.c

toplev.o: toplev.c handler.h nodes.h int.h macros.h sym.h tab.h version.h
	ifwrite toplev.o cc $(CFLAGS) toplev.c

catch.o: catch.c macros.h
	ifwrite catch.o cc $(CFLAGS) catch.c

pragint.o: pragint.c int.h nodes.h sym.h macros.h
	ifwrite pragint.o cc $(CFLAGS) pragint.c

intform.o: intform.c macros.h arr.h nodes.h int.h parser.h sym.h tab.h
	ifwrite intform.o cc $(CFLAGS) intform.c

rsw.o: rsw.c tokens.h macros.h
	ifwrite rsw.o cc $(CFLAGS) rsw.c

paraux.o: paraux.c macros.h nodes.h int.h parser.h arr.h
	ifwrite paraux.o cc $(CFLAGS) paraux.c

parerr.o: parerr.c macros.h oblist.h
	ifwrite parerr.o cc $(CFLAGS) parerr.c

config.o: config.c macros.h
	ifwrite config.o cc $(CFLAGS) config.c

conv.o: conv.c macros.h
	ifwrite conv.o cc $(CFLAGS) conv.c

tokens.o: tokens.c 
	ifwrite tokens.o cc $(CFLAGS) tokens.c

parser.c: parser.y 
	byacc $(YFLAGS) parser.y
	ifwrite parser.c mvif y.tab.c parser.c
	ifwrite tokens.h mvif y.tab.h tokens.h

parser.o: parser.c nodes.h int.h scan.h macros.h oblist.h parser.h
	ifwrite parser.o cc $(CFLAGS) $(YCFLAGS) parser.c

scanner.c: scanner.l
	lex scanner.l
	ifwrite scanner.c mvif lex.yy.c scanner.c

scanner.o: scanner.c nodes.h int.h tokens.h scan.h macros.h oblist.h
	ifwrite scanner.o cc $(CFLAGS) $(LXCFLAGS) scanner.c

casfns.o: casfns.c macros.h nodes.h int.h sym.h tab.h eval.h oblist.h castor.h
	ifwrite casfns.o cc $(CFLAGS) casfns.c

casint.o: casint.c macros.h nodes.h int.h sym.h tab.h eval.h oblist.h \
	   arr.h count.h
	ifwrite casint.o cc $(CFLAGS) casint.c

cassym.o: cassym.c macros.h nodes.h int.h sym.h tab.h eval.h oblist.h
	ifwrite cassym.o cc $(CFLAGS) cassym.c

tags: $(CDEFS) $(CSOURCES)
	ctags $(CDEFS) $(CSOURCES)

standards: $(STANDARDS)
	ls $(STANDARDS)

c_cref: c_cref.c
	ifwrite c_cref cc -o c_cref c_cref.c

Arcturus_cref: c_cref $(CDEFS) $(CSOURCES) command.c parser.y scanner.l
	c_cref $(CDEFS) $(CSOURCES) command.c parser.y scanner.l | pr -h '            Arcturus version $(MAJOR).$(MINOR)$(FIX)        Cross Reference                ' -f > listing
	csh -f -c 'lpr -r -m -n 1 listing'


uui_cref: c_cref $(UUISOURCES) $(UUIOTHERSOURCES)
	c_cref $(UUISOURCES) $(UUIOTHERSOURCES) | pr -h '            Arcturus UUI version $(MAJOR).$(MINOR)$(FIX)        Cross Reference                ' -f > listing
	csh -f -c 'lpr -r -m -n 1 listing'


print: Makefile $(STANDARDS) $(CDEFS) $(ASOURCES) $(CSOURCES) \
		command.c parser.y scanner.l
	ctags -x $(CDEFS) $(CSOURCES) parser.y scanner.l | pr -h 'Cross Reference   Arcturus version $(MAJOR).$(MINOR)$(FIX)' -f > listing
	clist $? >> listing
	csh -f -c 'lpr -r -m -n 1 listing'
	touch print

uuiprint: $(UUISOURCES) $(UUIOTHERSOURCES)
	ctags -x $(UUISOURCES) $(UUIOTHERSOURCES) | pr -h 'Cross Reference   Arcturus UUI version $(MAJOR).$(MINOR)$(FIX)' -f > uuilisting
	clist $? >> uuilisting
	csh -f -c 'lpr -r -m -n 1 uuilisting'
	touch uuiprint

lint: $(CDEFS) $(CSOURCES)
	lint $(CSOURCES) parser.c scanner.c
	
command.o: command.c macros.h adaio.h
	ifwrite command.o cc $(CFLAGS) command.c

frontend: $(FRONTOBJECTS)
	@echo linking frontend
	@ld -r $(FRONTOBJECTS) -o frontend

ashbase: $(EDITOROBJECTS) config.o 
	cc -g $(EDITOROBJECTS) config.o -o ashbase

ash: ash.c config.o
	cc -g ash.c config.o termsetup.o -o ash

subbackend.o: subbackend.c
	ifwrite subbackend.o cc $(CFLAGS) subbackend.c

adashbase: frontend backend
	cc -g frontend backend -o adashbase

adaio.h: color.h
	touch adaio.h

globals.h: std.h adaio.h
	touch globals.h


backend: $(OBJECTS)
	@echo updating version
	@csh -f version.gen $(SYSTEM) $(MAJOR) $(MINOR) $(FIX)
	@ifwrite version.o cc $(CFLAGS) version.c
	@echo linking backend
	@ld -r $(OBJECTS) -ll -o backend

achar.o: achar.c color.h std.h
	ifwrite achar.o cc $(CFLAGS) achar.c

box.o: box.c $(HEADERS)
	ifwrite box.o cc $(CFLAGS) box.c

buffer.o: buffer.c $(HEADERS)
	ifwrite buffer.o cc $(CFLAGS) buffer.c

color.o: color.c color.h
	ifwrite color.o cc $(CFLAGS) color.c

comcom.o: comcom.c $(HEADERS) macros.h globals.h adacomp.h nodes.h int.h sym.h tab.h
	ifwrite comcom.o cc $(CFLAGS) comcom.c

comhelp.o: comhelp.c $(HEADERS)
	ifwrite comhelp.o cc $(CFLAGS) comhelp.c

context.o: context.c $(HEADERS)
	ifwrite context.o cc $(CFLAGS) context.c

context_fns.o: context_fns.c $(HEADERS) forks.h
	ifwrite context_fns.o cc $(CFLAGS) context_fns.c

defargs.o: defargs.c $(HEADERS)
	ifwrite defargs.o cc $(CFLAGS) defargs.c

display.o: display.c $(HEADERS)
	ifwrite display.o cc $(CFLAGS) display.c

editfns.o: editfns.c $(HEADERS) fns.h term.h
	ifwrite editfns.o cc $(CFLAGS) editfns.c

fedit.o: fedit.c $(HEADERS)
	ifwrite fedit.o cc $(CFLAGS) fedit.c

finit.o: finit.c $(HEADERS)
	ifwrite finit.o cc $(CFLAGS) finit.c

formfill.o: formfill.c std.h
	ifwrite formfill.o cc $(CFLAGS) formfill.c

fsave.o: fsave.c $(HEADERS)
	ifwrite fsave.o cc $(CFLAGS) fsave.c

high.o: high.c $(HEADERS)
	ifwrite high.o cc $(CFLAGS) high.c

intrin.o: intrin.c $(HEADERS)
	ifwrite intrin.o cc $(CFLAGS) intrin.c

io.o: io.c $(HEADERS)
	ifwrite io.o cc $(CFLAGS) io.c

letter_case.o: letter_case.c $(HEADERS)
	ifwrite letter_case.o cc $(CFLAGS) letter_case.c

lisp.o: lisp.c std.h trie.h
	ifwrite lisp.o cc $(CFLAGS) lisp.c

low.o: low.c $(HEADERS)
	ifwrite low.o cc $(CFLAGS) low.c

main.o: main.c $(HEADERS)
	ifwrite main.o cc $(CFLAGS) main.c

marks.o: marks.c $(HEADERS)
	ifwrite marks.o cc $(CFLAGS) marks.c

menus.o: menus.c trie.h globals.h
	ifwrite menus.o cc $(CFLAGS) menus.c

move.o: move.c $(HEADERS)
	ifwrite move.o cc $(CFLAGS) move.c

movefield.o: movefield.c $(HEADERS)
	ifwrite movefield.o cc $(CFLAGS) movefield.c

movetoken.o: movetoken.c $(HEADERS)
	ifwrite movetoken.o cc $(CFLAGS) movetoken.c

mpx.o: mpx.c mpx.h
	ifwrite mpx.o cc $(CFLAGS) mpx.c

repeat.o: repeat.c $(HEADERS)
	ifwrite repeat.o cc $(CFLAGS) repeat.c

scan.o: scan.c $(HEADERS) compscan.h std.h
	ifwrite scan.o cc $(CFLAGS) scan.c

screen_pos.o: screen_pos.c $(HEADERS)
	ifwrite screen_pos.o cc $(CFLAGS) screen_pos.c

shell.o: shell.c $(HEADERS) trie.h
	ifwrite shell.o cc $(CFLAGS) shell.c

strings.o: strings.c std.h color.h
	ifwrite strings.o cc $(CFLAGS) strings.c

sysmsg.o: sysmsg.c $(HEADERS) trie.h nodes.h int.h sym.h tab.h eval.h macros.h
	ifwrite sysmsg.o cc $(CFLAGS) sysmsg.c

tableau.o: tableau.c $(HEADERS)
	ifwrite tableau.o cc $(CFLAGS) tableau.c

term.o: term.c $(HEADERS) term.h
	ifwrite term.o cc $(CFLAGS) term.c

trie.o: trie.c $(HEADERS) trie.h
	ifwrite trie.o cc $(CFLAGS) trie.c

typein.o: typein.c $(HEADERS) fns.h
	ifwrite typein.o cc $(CFLAGS) typein.c

uui.o: uui.c $(HEADERS)
	ifwrite uui.o cc $(CFLAGS) uui.c



adacomp.h: compscan.h std.h trie.h 
	touch adacomp.h

attrcomp.o: attrcomp.c macros.h nodes.h int.h sym.h tab.h adacomp.h
	ifwrite attrcomp.o cc $(CFLAGS) attrcomp.c

literalcomp.o: literalcomp.c macros.h adacomp.h nodes.h int.h sym.h
	ifwrite literalcomp.o cc $(CFLAGS) literalcomp.c

varcomp.o: varcomp.c macros.h adacomp.h nodes.h int.h sym.h
	ifwrite varcomp.o cc $(CFLAGS) varcomp.c

aggcomp.o: aggcomp.c macros.h adacomp.h nodes.h int.h sym.h tab.h
	ifwrite aggcomp.o cc $(CFLAGS) aggcomp.c

makecomp.o: makecomp.c macros.h adacomp.h nodes.h int.h sym.h tab.h 
	ifwrite makecomp.o cc $(CFLAGS) makecomp.c

subpcomp.o: subpcomp.c macros.h adacomp.h nodes.h int.h sym.h
	ifwrite subpcomp.o cc $(CFLAGS) subpcomp.c

indexcomp.o: indexcomp.c macros.h adacomp.h nodes.h int.h sym.h
	ifwrite indexcomp.o cc $(CFLAGS) indexcomp.c

exprcomp.o: exprcomp.c macros.h adacomp.h color.h nodes.h int.h sym.h tab.h
	ifwrite exprcomp.o cc $(CFLAGS) exprcomp.c

packcomp.o: packcomp.c macros.h adacomp.h nodes.h int.h sym.h
	ifwrite packcomp.o cc $(CFLAGS) packcomp.c

typecomp.o: typecomp.c macros.h adacomp.h nodes.h int.h sym.h
	ifwrite typecomp.o cc $(CFLAGS) typecomp.c

fieldcomp.o: fieldcomp.c macros.h adacomp.h nodes.h int.h sym.h tab.h
	ifwrite fieldcomp.o cc $(CFLAGS) fieldcomp.c

utilcomp.o: utilcomp.c macros.h adacomp.h nodes.h int.h sym.h tab.h
	ifwrite utilcomp.o cc $(CFLAGS) utilcomp.c

initcomp.o: initcomp.c macros.h adacomp.h nodes.h int.h sym.h
	ifwrite initcomp.o cc $(CFLAGS) initcomp.c

paramcomp.o: paramcomp.c macros.h adacomp.h nodes.h int.h sym.h tab.h
	ifwrite paramcomp.o cc $(CFLAGS) paramcomp.c

exprhelp.o: exprhelp.c macros.h adacomp.h color.h nodes.h int.h sym.h
	ifwrite exprhelp.o cc $(CFLAGS) exprhelp.c

termsetup.o: termsetup.c
	ifwrite termsetup.o cc $(CFLAGS) termsetup.c

forks.o: forks.c std.h forks.h
	ifwrite forks.o cc $(CFLAGS) forks.c

adash: forks.o trie.o lisp.o termsetup.o tableau.o config.o
	cc forks.o trie.o lisp.o termsetup.o tableau.o config.o -o adash
