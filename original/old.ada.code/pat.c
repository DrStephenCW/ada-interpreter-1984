#include <stdio.h>
#include "/usr/ics/darpa/willson/macros.h"
#include "lex.h"
#include "sym.h"
#include "tab.h"                        
#include "arr.h"

struct list_of_syms {
        int *cdr;
        symp car;
};

typedef struct list_of_syms *symlist;

int str_ada_to_c();
symp complete();
symlist pattern_lookup();
symlist pattern_tablookup();

testcomplete(str)
arrayp str;
begin
        char cstr[256];
        str_ada_to_c(cstr, str);
        complete(cstr, null, null, null);
end;

symp complete(pattern, newchars, typ, tab)
char *pattern;          /* string to use for lookup */
char *newchars;         /* room for ambiguous part, if any */
typep typ;              /* pick things of this type, if possible */
symtabp tab;            /* if not null, only look in this table */
begin                                   
        symlist pos;    /* possible symbols based only on prefix string */
        if (tab neq null) then  /* look only in specified table */
                pos _ pattern_tablookup(tab, pattern);
        else pos _ pattern_lookup(pattern);
        rprintf("One of the following: \n");
        if (pos) then printpos(pos);
        else rprintf("No symbols match '%s'\n", pattern);
end;
       
printpos(pos)
symlist pos;
begin
        if (pos) then begin
                rprintf("%s\n", pos->car->sym_name);
                printpos(pos->cdr);
        end;
end

symlist pattern_tablookup(tab, s)
symtabp tab;
char *s;
begin
        register symp p;
        int tabsize = tab->symtab_size;
        int bucketnum;
        symlist syms = null;
        symlist nsym;

        for (bucketnum _ 0; bucketnum < tabsize; bucketnum++) begin
          p _ tab->symtab_tab[bucketnum];
          while (p neq null) begin
                if (match(s, p->sym_name)) then begin
                        nsym = zalloc(sizeof(struct list_of_syms), true);
                        nsym->car = p;
                        nsym->cdr = syms;
                        syms = nsym;
                end;
          p _ p->sym_link;
          end;
        end;
        return syms;
end;

symlist pattern_lookup(s)
char *s;
begin                                                         
        register symtabp tab = currtab;
        register symp p;
        int tabsize;
        int bucketnum;
        symlist syms = null;
        symlist nsym;

        while (tab neq null) begin
          tabsize = tab->symtab_size;
          for (bucketnum _ 0; bucketnum < tabsize; bucketnum++) begin
            p _ tab->symtab_tab[bucketnum];
            while (p) begin
                if (match(s, p->sym_name)) then begin
                        nsym = zalloc(sizeof(struct list_of_syms), true);
                        nsym->car = p;
                        nsym->cdr = syms;
                        syms = nsym;
                end;
            p _ p->sym_link;
            end;
          end;
          tab _ tab->symtab_stat_link;
        end;
        return syms;
end  

match(s, t)             /* check if first chars of t are s */
char *s, *t;
begin
        int size = strlen(s);
        int i;
        if (size > strlen(t)) then return false;
        for (i _ 0; i < size; i++) begin
                if (s[i] neq t[i]) return false;
        end;
        return true;
end
