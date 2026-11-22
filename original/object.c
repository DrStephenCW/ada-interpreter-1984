static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/object.v  Revision 1.6  83/11/30  14:14:15  ada  Exp$";

/*******************************************************************
 *
 * Copyright 1983      Thomas A. Standish
 *		       Programming Environment Project
 *                     Department of Information and Computer Science
 *                     University of California
 *                     Irvine, California  92717
 *
 * All rights reserved. No part of this software may be sold or distributed
 * in any form or by any means without the prior written permission of the 
 * author.
 *
 * Report problems and direct all inquiries to ada%uci@rand-relay (ARPA net).
 */


/* $Log:	/ub/darpa/ada/RCS/object.v $
 * Revision 1.6  83/11/30  14:14:15  ada
 * same as 1.5.5.1
 * 
 * Revision 1.5.5.1  83/11/29  15:59:13  snider
 * Minor changes in hookup.
 * 
 * Revision 1.5  83/03/10  15:59:21  ada
 * same as 1.4.2.1
 * 
 * Revision 1.4.2.1  83/03/10  14:53:56  willson
 * Expanded loader tables
 * 
 * Revision 1.4  83/01/12  09:07:05  ada
 * Added copyright notice.
 * 
 * Revision 1.3  83/01/04  14:12:32  willson
 * the loader now looks in the global environment for objects
 * as well as "subr's" and links them in.  Also, objects can
 * get patched into the loaded code too.  the value pointer
 * for a : integer; (for example) will get changed to point
 * to the loaded code instead of the default little block at
 * the end of the symp.  This only works if the symp doesn't
 * go away!
 * 
 * Revision 1.2  82/12/20  15:12:51  willson
 * Fixed bug with data segment offsets (strings work o.k. now)
 * Added support for references to internal Arcturus identifiers
 * 
 * Revision 1.1  82/12/12  21:37:31  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include <stdio.h>
#include <a.out.h>
#include "macros.h" 
#include "nodes.h"
#include "int.h"     
#include "sym.h"
#include "oblist.h"
#include "tab.h"

/* Load an object file into the interpreter */
/* Compile and load the C source file like this: (it should have a "main") */
/* cc -c file.c */
/* ld -N -r file.o -o file.O -lc */

#define NO_SUCH_SYM -1

static  unsigned text_base;     /* base of text segment in memory */
static  unsigned data_base;     /* base of data segment in memory */
static  unsigned bss_base;      /* base of blank storage space */
static	unsigned sym_offset;	/* offset of object file symbol table */
static	unsigned str_offset;	/* offset of object file string table */
static  unsigned code_address;  /* address in memory where code will */
                                /* be loaded (from call to malloc) */
static  unsigned entry;		/* address of "main" routine of object code */
				/* (corresponds to main body of package, */
				/* containing initialization code) */
static	(*func)();		/* used to indirect thru "entry" to */
				/* execute main body of package */
static	boolean	symbols_resident;
				/* true if symbol and string tables loaded */
				/* from object file into memory */
static	struct nlist *lsymp _ null;	/* points to object file */
				/* symbol table in memory */
static	char *strp;		/* points to object file string table */
				/* in memory */
static	int nsyms;		/* number of symbols in object file */
static	FILE *f;		/* used to reference object file */

char *tail();			/* returns file component of file pathname */
char *remove_underscore();	/* returns an object file symbol name with */
				/* the initial underscore removed */
unsigned int in_special_table();/* used to access internal Arcturus addresses */

	/* main routine */

load_object(file)		/* given the file to load, first reads in */
				/* specificiation in file.S, then reads */
				/* the code in from file.O, then hooks the */
				/* two together.  In thesis terms, this is */
				/* the import laundry.  Also will hook in */
				/* some interpreter addresses (like malloc). */
char *file;
{
        char object_file[256];	/* with .O tagged onto "file" */
        char spec_file[256];	/* with .S tagged onto "file" */
        long magic;             /* magic number from a.out */
        unsigned text;          /* size of text segment */
        unsigned data;          /* size of data segment */
        unsigned bss;           /* size of uninitialized data */
        unsigned object_size;   /* size of text + data + bss */
        unsigned trsize;        /* size of text segment relocation info */
        unsigned drsize;        /* size of data segment relocation info */
        int i;                  /* for for loops */
        char *p;                /* used to copy bytes into memory from file */
	fixp packfix;		/* fixed part of symbol */
				/* used to hook in code with specifications */

        sprintf(object_file, "%s.O", file);
        sprintf(spec_file, "%s.S", file);
        f = fopen(object_file,"r");
        if (not f)  error("Object file %s not found in load pragma\n",
			object_file);
 
        magic = getw(f);

	/* note:  OMAGIC is the old style impure format, which has the */
	/* features that the code is all scrunched together (that is, not */
	/* aligned on page boundaries) and is basically the easiest format */
	/* to diddle with.  */

        if (magic neq 0407 /* OMAGIC */)  {
                fclose(f);
                error("Object file must have magic number 0407 (OMAGIC)");
        };

        text _ getw(f);         /* size of text segment */
        data _ getw(f);         /* size of data segment */
        bss _ getw(f);          /* size of blank storage segment */
        nsyms _ getw(f);        /* get size (in bytes) of symbol table */
        entry _ getw(f);	/* entry point corresponds to "main" */
        trsize _ getw(f);       /* size of text segment relocation info */
        drsize _ getw(f);       /* size of data segment relocation info */
                                /* object file pointer "f" is left at */
				/* beginning of text segment */

	sym_offset _ text + data + trsize + drsize + sizeof(struct exec);
				/* offset to symbol information in file */
	str_offset _ sym_offset + nsyms;
				/* strings follow symbols; getw(f) returns */
				/* the size of the symbol table */
				/* we must do this calculation before we */
				/* fudge trsize and drsize just below */

        trsize _ trsize / sizeof(struct relocation_info);
        drsize _ drsize / sizeof(struct relocation_info);
	/* note:  trsize and drsize started off in bytes; the above code */
	/* changes them into "relocation_info" units (that is, the number */
	/* of relocation items, not the size in bytes). */

        object_size _ text + data + bss;
                                /* size we need to allocate in memory */
				/* text in unix terminology is the */
				/* executable code, data is initialized data */
				/* and bss is uninitialized data. */

/*
        putf("Sizes in decimal:  text: %d  data: %d  bss: %d   total:  %d\n", 
               text, data, bss, object_size);
*/

        code_address _ (unsigned)zalloc(object_size, false);
                        /* this allocates enough space for resulting code */

        include(spec_file, true);       /* include specification */

	/* now read in the object code (both executable code and initialized
	   data). */

        p _ (char *)code_address;
        for (i _ 0; i < text; i++)
                *p++ _ getc(f);   /* read in text segment */
        for (i _ 0; i < data; i++)
                *p++ _ getc(f);   /* read in data segment */
                                  /* bss is the remaining space */
                                  /* not read into */

        /* These loops leave the file pointer pointing at the relocation */
        /* information in the object file.  We use that stuff to resolve */
        /* undefined symbols and to relocate references from the base */
        /* of the text segment or the data segment. */

        text_base _ code_address;       /* base of text segment */
        data_base _ text_base + text;   /* base of data segment */
        bss_base _ data_base + data;    /* base of blank storage segment */

        while (trsize--) relocate(text_base);   
				/* read text offsets from file and relocate */
				/* relative to the text_base */
        while (drsize--) relocate(data_base);   
				/* read data offsets from file and relocate */
				/* relative to the data_base */

        /* Finally, connect loaded code with ada environment symbols */

        file _ intern(tail(file));      /* otherwise, nothing works */
	/* Naughty business of using variable twice; this hacks the file */
	/* name down to the final component which is presumed to be the */
	/* name of the package being loaded.  Then we look it up to get */
	/* access to it's symbol tables, so we can hook in the code to the */
	/* symbols defined by the package spec. */

        packfix _ lookup(currtab, file, null);
				/* packfix now points at the package */
				/* symbol */
        if (not packfix) 
                error("Package specification went away in load pragma");

	hookup(packfix->fx.pack.int_form->nd.packagex.table);
	hookup(packfix->fx.pack.body_int_form->nd.packagex.table);
		/* for each object declaration in the package, connect */
		/* up the corresponding machine code address */

	entry _ entry + code_address;
		 	/* since entry is an offset in the object file, we */
			/* now make it a real address */
	copy_x2y(&entry,&func);
			/* defeat type checking */
	(*func)();	/* execute main body of package */
        fclose(f);

	/* free space used by symbol table */
	if (lsymp) 
		free((char *)lsymp);
	if (strp) 
		free((char *)strp);
}
                                                                
char *tail(s)
char *s;	/* returns tail of file pathname s */
{
	char *p;
	p _ s;
	while (*s)
		if (*s++ eq '/')  p _ s;
	return p;
}

unsigned int addr_in_fixp();

relocate(creloc)	/* read a text or data segment relocation datum */
			/* and patch up the object code we've read in. */
			/* ideas in this routine from load2td in ld.c */
unsigned creloc;
{                            
        struct relocation_info rel;   /* holds a relocation datum */
	register char *cp;	/* used to access location to be relocated */
				/* this is cast between long, short and char */
        unsigned base;		/* either text, data, or bss */
	fixp env_fixp;		/* fixed part of symbol */
				/* dug out of environment */
	long tw;		/* value (via *cp) to relocate and put */
				/* pack in memory */

        fread((char *)&rel, 1, sizeof(rel), f);
				/* read next relocation datum from file */
				/* (how often do you see someone use the */
				/* word "datum"?) */

 /*	putf("Address: %d ", rel.r_address);
        putf("Symbolnum: %d ", rel.r_symbolnum);
        putf("Pcrel: %d ", rel.r_pcrel);
        putf("Length: %d ", rel.r_length);
        putf("Extern: %d\n", rel.r_extern);
  */				/* debugging information */

 	if ((not rel.r_extern) and rel.r_pcrel) return;
                        /* already relocated by the program counter */
                        /* ala pdp-11 */
			/* and no modifications for external reference */
			/* so nothing to do */

	/* pick up previous value at location to be relocated */

	cp _ (char *)code_address + rel.r_address;
			/* now points into loaded code at address we want */
			/* to relocate */

	switch (rel.r_length) {
	case 0: /* byte */
		tw _ *cp;
		break;
	case 1: /* short */
		tw _ *(short *)cp;
		break;
	case 2: /* long */
		tw _ *(long *)cp;
		break;
	default: error("Bad relocation length (%d) while loading",
			rel.r_length);
	}

	/* if relative to an undefined external variable, then dig up the */
	/* value of the variable from the Ada environment, if it exists. */
	/* if it doesn't, that's an error (for now: future plan is to make */
	/* some interpreter variables (like zalloc) available). */

        if (rel.r_extern)  {

		unsigned int arcturus_code_addr;

        	if (not symbols_resident)
			getsymbols();
		env_fixp _ lookup(currtab, 
				  remove_underscore(		
				  	lsymp[rel.r_symbolnum].n_un.n_name), 
				  null);
			/* find identifier in environment */
		if (env_fixp and (arcturus_code_addr _ addr_in_fixp(env_fixp)))
			{ }
		else if (arcturus_code_addr _ in_special_table(
			 remove_underscore(
			     lsymp[rel.r_symbolnum].n_un.n_name)))
			{ }
		else {
			 putf("Object file symbol %s not hooked to anything\n",
			       remove_underscore(
				lsymp[rel.r_symbolnum].n_un.n_name));
	                  return;
		}

		tw += (int)arcturus_code_addr;
				/* tw is an offset from the external */
				/* identifer, so we add them together */
		/* note:  what that means is that there might not be an
		   external identifier that hits the exact location to be
		   relocated, so they get at it via the nearest external
		   identifier by adding in an offset.  In this case, the
		   offset is based from the address of the code for the
		   symbol that was dug up out of the Ada env.  */
        }

	/* if we get to this "else" case, then we weren't dealing with
	   an undefined external.  The object file is already "relocated"
	   relative to the start of the file, so in this case, we simply
	   add the address of the start of the code to "tw".  If we were
	   loading more than one file, then the data segment might have been
	   moved relative to the text segment, and more fudging would be
	   required.  The comment in ld.c that applies here is the one that
	   says something about the definition of "ctrel" and "cdrel" and
	   talks about "if the data segment started at 4 in the .o and was
	   going to start at 1024 in the a.out, then cdrel would be 1020..."
	*/

	else
          switch (rel.r_symbolnum & N_TYPE) {
                case N_TEXT:
                case N_DATA:
                case N_BSS:
			tw += code_address; break;
		case N_ABS: break;
		case N_UNDF: error("??? Reference to undefined symbol\n");
                default: error("??? Unknown relocation type\n");
        };

	/* Relocation on the vax is usually (always?) pc relative, so this */
	/* next if statement is almost always true.  For a while, I got away */
	/* without most of this logic, because if you are relocating one */
	/* file only, most of this is a no-op.  But relocating external */
	/* references makes it all come back */

	if (rel.r_pcrel)
		tw -= creloc;	/* decrease the relocation by the amount */
				/* the current segment is displaced. (for */
				/* references from the text segment to */
				/* itself or the data segment this is zero) */
				/* (E.g., if we are a relative reference */
				/* to a text location from data space */
				/* [like a function pointer], we added */
				/* the increase in the text address above */
				/* and we subtract the increase in our */
				/* data address here, leaving the net */
				/* change the relative change in the */
				/* positioning of our text and data segments)*/

	/* finally, put the value back into the segment, all fixed up. */
	/* Should check for overflow, but I have faith! */

	switch (rel.r_length) {
	    case 0: 
		*cp _ tw;
		break;
	    case 1: 
		*(short *) cp _ tw;
		break;
	    case 2: 
		*(long *) cp _ tw;
		break;
	}			/* default would've been taken in the last 
				   one of these */
}

unsigned int addr_in_fixp(env_fixp) fixp env_fixp;
{
	switch (env_fixp -> fxf.class) {
	case proc_id:
	case func_id:
		if (env_fixp->fx.func.code eq 0) {
		 putf("%s isn't compiled\n", env_fixp -> fxf.name);
		 return;
		}
		else return (unsigned int)env_fixp->fx.func.code;
	case obj_id:
		if (env_fixp -> fxf.dyna -> sy.var.value eq 0) {
		 putf("%s has zero value?\n", env_fixp -> fxf.name);
		 return;
		}
		else return (unsigned int)env_fixp -> fxf.dyna -> sy.var.value;
	default:
		putf("case (%d) not handled for %s while loading\n",
			env_fixp -> fxf.class,
			env_fixp -> fxf.name);
	}
}

char *remove_underscore(s)
char *s;		/* remove the leading underscore from the string */
			/* s and return a pointer to the rest of the string */
			/* if such an undercore exists.  This is because */
			/* the C compiler likes to put underscores in front */
			/* of all the symbol names.  Very silly.  */
{
	if (*s eq '_') return intern(s + 1);
	else return s;	/* intern(s) ? */
}

getsymbols()		/* read symbol table into memory */
{
	int n;		/* number of symbols in file */
	int i;		/* counts symbols as they are read */
	int strsiz;
	struct nlist sym;
	int currfspot;	/* current spot in file f */
	symbols_resident _ true;
	currfspot _ ftell(f);
	rewind(f);	/* seek to beginning of file */
	fseek(f,sym_offset,0);
			/* now point at first symbol */
	i _ 0;
	nsyms _ nsyms / sizeof(struct nlist);
			/* nsyms original is in bytes; now it is in */
			/* number of symbols */
	n _ nsyms;

	/* Most of the following code was taken */
	/* (in spirit, if not the letter) from */
	/* nm.c */
	
	while (--n >= 0) { /* read each symbol entry */
		fread((char *)&sym, 1, sizeof(sym), f);
		if (lsymp eq null) 
			lsymp _ (struct nlist *)
				zalloc(sizeof (struct nlist), false);
		else
			lsymp _ (struct nlist *)
				realloc(lsymp, (i+1)*sizeof(struct nlist));
		/* if you don't understand, you'd better read about realloc */
		if (lsymp eq null) 
			error("Out of memory in load pragma (symbols)");
		lsymp[i++] _ sym;
		/* this could only be written in C... */
	};

	if (fread((char *)&strsiz, sizeof(strsiz), 1, f) neq 1) 
		error("No string table found during load pragma");
	strp _ (char *)zalloc(strsiz, false);
	if (strp eq null) 
		error("Out of memory in load pragma (strings)");
	if
	 (fread(strp+sizeof(strsiz), strsiz-sizeof(strsiz), 1, f) neq 1) 
		error("Error reading string table in load pragma");
		/* remember, string table size includes bytes to store size */
	{ int j;
		for(j _ 0; j < i; j++) 
			if (lsymp[j].n_un.n_strx) 
				lsymp[j].n_un.n_name _
				  lsymp[j].n_un.n_strx + strp;
			else
				lsymp[j].n_un.n_name _ "";
	}; 
	rewind(f);		/* put f back where it started from */
	fseek(f, currfspot, 0);
}

unsigned getaddr(s)
char *s;	/* look into symbol table and determine address of */
		/* label s in the code; this is used for hooking up */
		/* objects in the Ada specification with the location */
		/* in memory they should reference */
{	int i;
       	if (not symbols_resident)
		getsymbols();
	for (i _ 0; i < nsyms; i++)
		if (lsymp[i].n_un.n_name &&
		    (strcmp(remove_underscore(lsymp[i].n_un.n_name), s) eq 0))
			return lsymp[i].n_value;
	return NO_SUCH_SYM;		/* symbol is undefined */
}

copy_x2y(x,y)           /* used to defeat type checking */
int *x,*y;
{       *y = *x;
}

hookup(tab)		/* take symbols in tab and change them to point into
			   the object file: this is how the spec's get 
			   connected to the stuff in the object file.
			*/
symtabp tab;
{
        register symp p;
	register fixp f;
        int tabsize = tab->symtab_size;
        int bucketnum;
	unsigned offset;

        for (bucketnum _ 0; bucketnum < tabsize; bucketnum++) {
          f _ tab->symtab_tab[bucketnum];
          while (f) {
	    p _ f->fxf.dyna;
	    switch (f -> fxf.class) {
		case proc_id:
		case func_id:
			offset _ getaddr(f -> fxf.name);

		/*	putf("Symbol: %s; address: %x\n", 
				f->fxf.name,
				offset);
		*/
			if (offset neq NO_SUCH_SYM) 
			  f->fx.func.code _ 
				(int (*)())(code_address + offset);
			else if (f->fx.func.int_form == null)
			    error("subp %s has no code or intform\n",f->fxf.name);

		/*	putf("Code_address %x\n", code_address);
			putf("Offset %x\n", offset);
			putf("f->fx.func.code %x\n", f->fx.func.code);
		*/
			break;
		case obj_id:
			offset _ getaddr(f -> fxf.name);
			if (offset neq NO_SUCH_SYM)
				if (not f->fxf.dyna) 
				  putf("No dynamic part for %s", f->fxf.name);
				else f->fxf.dyna->sy.var.value _ 
					(int *)(code_address + offset);
		/*	else putf("Warning: Object %s has no referent in object file",
				  f->fxf.name); */
			break;
	/* 	default: putf("Warning: case (%d); symbol %s not hooked to anything\n",
				f->fxf.class, f->fxf.name); */
	    }
	    f _ f->fxf.next;
          };
        };
}

/*	Add function names here to make them available to the loader.  You
	may need to declare them extern so the C compiler knows what to do
	with them.
*/

extern malloc();
extern putf();
extern errno;
extern print();
extern pipe();
extern signal();
extern fork();
extern close();
extern execvp();
/* extern fopen(); */
/* extern _iob; */
extern fprintf();
extern printf();
extern strlen();
extern strcat();
extern fputs();
extern _flsbuf;
extern free();
extern fclose();
extern strcpy();
extern malloc();
extern atoi();
extern atol();
extern atof();
extern access();
/* extern fgets(); */
extern popen();
extern pclose();
extern strcmp();
/* extern _filbuf; */
extern isatty();
extern system();
extern _ctype_;
extern realloc();
extern vfork();
extern environ;		/* ()? */
extern execle();
extern exit();
extern wait();
extern fflush();
/* extern putchar(); */
extern open();
extern read();
extern write();

struct	internal_names {
	char *names;
	int addrs;
} internal_names[] = {
	"malloc", (int)malloc,
	"putf", (int)putf,
	"errno", (int)&errno,
	"printf", (int)printf,
	"pipe", (int)pipe,
	"signal", (int)signal,
	"fork", (int)fork,
	"close", (int)close,
	"execvp", (int)execvp,
	"fopen", (int)fopen,
	"_iob", (int)_iob,
	"fprintf", (int)fprintf,
	"strlen", (int)strlen,
	"strcat", (int)strcat,
	"fputs", (int)fputs,
	"_flsbuf", (int)&_flsbuf,
	"free", (int)free,
	"fclose", (int)fclose,
	"strcpy", (int)strcpy,
	"atoi", (int)atoi,
	"atol", (int)atol,
	"atof", (int)atof,
	"access", (int)access,
	"fgets", (int)fgets,
	"popen", (int)popen,
	"pclose", (int)pclose,
	"strcmp", (int)strcmp,
	"_filbuf", (int)_filbuf,
	"isatty", (int)isatty,
	"system", (int)system,
	"_ctype_", (int)&_ctype_,
	"realloc", (int)realloc,
	"vfork", (int)vfork,
	"environ", (int)&environ,
	"execle", (int)execle,
	"exit", (int)exit,
	"wait", (int)wait,
	"fflush", (int)fflush,
/*	"putchar", (int)putchar,	*/
	"open", (int)open,
	"read", (int)read,
	"write", (int)write,
	0, 0};			/* must be terminated by these two zeros */

unsigned int in_special_table(name) char *name;
	/* returns the address of symbols that need to be known to compiled
	   code that aren't otherwise available via package specifications.
	*/
{	
	register struct internal_names *in;
	for (in _ internal_names; in -> names; in++)
		if (strcmp(name, in -> names) eq 0) break;
	return (unsigned int)in -> addrs;
}

