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
static	unsigned sym_offset;	/* offset of symbol table in file */
static	unsigned str_offset;	/* offset of string table in file */
static  unsigned code_address;  /* address in memory where code will */
                                /* be loaded */
static  unsigned entry;		/* address of main body of package */
				/* (contains initialization code) */
static	(*func)();		/* used to indirect thru entry to */
				/* execute main body of package */
static	boolean	symbols_resident;
				/* true if symbol and string tables loaded */

static	struct nlist *lsymp _ null;	/* points to symbol table in memory */
static	char *strp;		/* points to string table in memory */
static	int nsyms;		/* number of symbols in file */
static	FILE *f;

char *tail();
char *remove_underscore();

load_object(file)
char *file;     
{
        char object_file[256];
        char spec_file[256];
        long magic;             /* magic number from a.out */
        unsigned text;          /* size of text segment */
        unsigned data;          /* size of data segment */
        unsigned bss;           /* size of uninitialized data */
        unsigned object_size;   /* size of text + data + bss */
        unsigned trsize;        /* size of text segment relocation info */
        unsigned drsize;        /* size of data segment relocation info */
        int i;                  /* for for loops */
        char *p;                /* used to copy bytes into memory */
        symp packsym;		/* used to hook in code with specifications */
	symp subpsym;		/* for each subp in the package */

        sprintf(object_file, "%s.O", file);
        sprintf(spec_file, "%s.S", file);
        f = fopen(object_file,"r");
        if (not f)  error("Object file not found in load pragma\n");
 
        magic = getw(f);
        if (magic neq 0407 /* OMAGIC */)  {
                fclose(f);
                error("Object file must have magic number 0407 (OMAGIC)");
        };

        text _ getw(f);         /* size of text segment */
        data _ getw(f);         /* size of data segment */
        bss _ getw(f);          /* size of black storage segment */
        nsyms _ getw(f);        /* get size (bytes) of symbol table */
        entry _ getw(f);	/* entry point corresponds to "main" part */
        trsize _ getw(f);       /* size of text segment relocation info */
        drsize _ getw(f);       /* size of data segment relocation info */
                                /* object file pointer is left at beginning */
                                /* of text segment */

	sym_offset _ text + data + trsize + drsize + sizeof(struct exec);
				/* offset of symbol information in file */
	str_offset _ sym_offset + nsyms;
				/* strings follow symbols; getw(f) returns */
				/* the size of the symbol table */
				/* we must do this calculation before we */
				/* fudge trsize and drsize just below */

        trsize _ trsize / sizeof(struct relocation_info);
        drsize _ drsize / sizeof(struct relocation_info);

        object_size _ text + data + bss;
                                /* size we need to allocate in memory */

/*
        printf("Sizes in decimal:  text: %d  data: %d  bss: %d   total:  %d\n", 
               text, data, bss, object_size);
*/

        code_address _ (unsigned)malloc(object_size);
                        /* this allocates enough space for resulting code */

        include(spec_file, true);       /* include specification */

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
        /* Actually, resolving unresolved references is impossible, */
        /* because of overloading.  So that's not done. */

        text_base _ code_address;       /* base of text segment */
        data_base _ text_base + text;   /* base of data segment */
        bss_base _ data_base + data;    /* base of blank storage segment */

        while (trsize--) relocate(text_base);   
				/* read text offsets and relocate */
        while (drsize--) relocate(data_base);   
				/* read data offsets and relocate */

        /* Finally, connect loaded code with the symbol */
        file _ intern(tail(file));      /* otherwise, nothing works */
        packsym _ lookup(file, null);   /* have to put pointer to */
                                        /* code in the symbol */
					/* packsym now points at the package */
					/* symbol table */
        if (not packsym) 
                error("Package specification went away in load pragma");
	hookup(packsym->sy.pack.visible_table);
	hookup(packsym->sy.pack.body_table);
		/* for each object declaration in the package, connect */
		/* up the corresponding machine code address */

	entry _ entry + code_address;
		 	/* since entry is an offset, we */
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

relocate(creloc)	/* read a text or data segment relocation datum */
			/* and hack up the code we've read in */
			/* ideas in this routine from load2td in ld.c */
unsigned creloc;
{                            
        struct relocation_info rel;   /* holds a relocation datum */
	register char *cp;	/* used to access location to be relocated */
				/* this is cast between long, short and char */
        unsigned base;		/* either text, data, or bss */
	symp env_symbol;/* symbol value is dug out of environment */
	long tw;		/* value to relocate */

        fread((char *)&rel, 1, sizeof(rel), f);
				/* read next relocation datum from file */

/*	printf("Address: %d ", rel.r_address);
        printf("Symbolnum: %d ", rel.r_symbolnum);
        printf("Pcrel: %d ", rel.r_pcrel);
        printf("Length: %d ", rel.r_length);
        printf("Extern: %d\n", rel.r_extern);
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
	default: error("Bad relocation length while loading");
	}

	/* if relative to an undefined external variable, then dig up the */
	/* value of the variable from the Ada environment, if it exists. */
	/* if it doesn't, that's an error */

	if (rel.r_extern) {	/* external, but maybe defined */
        	if (not symbols_resident)
			getsymbols();
		if ((lsymp[rel.r_symbolnum].n_type & N_TYPE) eq N_UNDF) {
			env_symbol _ lookup(remove_underscore(		
				lsymp[rel.r_symbolnum].n_un.n_name), null);
				/* find identifier in environment */
			if (not env_symbol) {
				putf("Unresolved external symbol %s\n",
					lsymp[rel.r_symbolnum].n_un.n_name);
	                	return;	/* should be error someday */
			}
			if ((env_symbol->syf.class neq proc_id and
			     env_symbol->syf.class neq func_id) or
			    env_symbol->sy.func.code eq 0)
			    /* symbol must be proc or func have a pointer */
			    /* to some compiled code */
			    error("%s%s%s", "Can't resolve references ",
				"to anything except ",
				"subr-like procedures and functions");
			tw += (int)env_symbol->sy.func.code;
				/* tw is an offset from the external */
				/* identifer, so we add them together */
		}
		else error("Symbol is external, not defined in environment");
        }


        /* pick segment of memory relocation datum refers to; */
	/* set "base" to the value that will be added to the bits in */
	/* the object file we loaded */

	else
          switch (rel.r_symbolnum & N_TYPE) {
                case N_TEXT: tw += text_base; break;
                case N_DATA: tw += data_base; break;
                case N_BSS: tw += bss_base; break;
		case N_ABS: break;
		case N_UNDF: break;  /* already relocated */
		case N_COMM: putf("Common symbol ignored\n"); break;
		case 0x1e:  /* N_FN ; N_FN includes low bit masked out by */
			    /* N_TYPE above, so we hack in this version */ 
			putf("File name symbol ignored\n"); break;
 		default: 
		  error("??? Unknown relocation type %x for symbol at %x",
				rel.r_symbolnum & N_TYPE,
				rel.r_address);
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
				/* positioning of our text and data segments */

	/* finally, put the value back into the segment, all fixed up */
	/* should check for overflow, but I have faith! */

	switch (rel.r_length) {
	case 0: *cp _ tw; break;
	case 1: *(short *)cp _ tw; break;
	case 2: *(long *)cp _ tw; break;
	}	/* default would've been taken in the last one of these */
}

char *remove_underscore(s)
char *s;		/* remove the leading underscore from the string */
			/* s and return a pointer to the rest of the string */
			/* if such an undercore exists.  This is because */
			/* the C compiler likes to put underscores in front */
			/* of all the symbol names.  Very silly.  */
{
	if (*s eq '_') return intern(s + 1);
	else return s;
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
				malloc(sizeof (struct nlist));
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
	strp _ (char *)malloc(strsiz);
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

hookup(tab)		/* for each object in tab, */
			/* find the machine code address where it */
			/* was loaded */
symtabp tab;
{
        register symp p;
        int tabsize = tab->symtab_size;
        int bucketnum;
	unsigned offset;

        for (bucketnum _ 0; bucketnum < tabsize; bucketnum++) {
          p _ tab->symtab_tab[bucketnum];
          while (p neq null) {
	    if (p->syf.class == proc_id ||
		p->syf.class == func_id) {	/* If the symbol is not a
						   subprogram then it is
						   impolite to modify its
						   sy.func.code field. */
		offset _ getaddr(p->syf.name);
/*
     		putf("Symbol: %s; address: %d\n", 
			p->syf.name,
			offset);
*/
		if (offset != NO_SUCH_SYM) 
			p->sy.func.code _ code_address + offset;
		else if (p->sy.func.intform == null)
		    error("%s is undefined in package spec\n",p->syf.name);
/*
		putf("Code_address %d\n", code_address);
		putf("Offset %d\n", offset);
		putf("p->sy.func.code %d\n", p->sy.func.code);
*/
	    }
	    p _ p->syf.link;
          };
        };
}


