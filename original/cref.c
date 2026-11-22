static char rcsid[] =
"Header$";

/* $Log$

*/

/****************** CROSS REFERENCER FOR C CODE ******************************
Written by David Feldman, Anne Brindle, Jerzy Rub      Rev Date: Aug.15, 1983 

This program finds all occurences of non-reserved identifiers.  For each such
occurence, the line number, function name ('---' if identifier not inside the
body of a function), and file name are stored in a sub-tree under the 
identifier. */

#include <stdio.h> 
#include <setjmp.h>

#define TRUE 1
#define FALSE 0
#define NUM_RES_WRDS  28
#define CURLEN 27
#define inputchar {ch=getchar(); if (ch==EOF) longjmp(env,1);\
                  if (ch=='\n') linecount++;}

int currentarg;
jmp_buf env;

char  rswrds [NUM_RES_WRDS] [9]  =
            {   "auto",
                "break",
                "case",
                "char",
                "continue",
                "default",
                "do",
                "double",
                "else",
                "entry",
                "extern",
                "float",
                "for",
                "goto",
                "if",
                "int",
                "long",
                "register",
                "return",
                "short",
                "sizeof",
                "static",
                "struct",
                "switch",
                "typedef",
                "union",
                "unsigned",
                "while"
             };

struct linenode 
{ struct linenode * next;
int number; };

struct fnnode 
{ struct fnnode * next;
struct linenode * lines, * lastline;
char * name; };

struct filenode 
{ struct filenode * next;
struct fnnode * fns, * lastfn;
char * name; };

struct wordnode 
{ struct wordnode * right, * left;
struct filenode * files, * lastfile;
char * name;
int macro; };   /* look under writenode for explanation of macro values */ 

struct wordnode * root = NULL;

/* global knowledge: currently active input character and identifier, 
and pointers to the currently active function name and file name.   */ 

int ch, notinfn, linecount, blevel, plevel, linefound, mac, maxnum;
char currentid[CURLEN], nullfn[5] = "---";
char * currentfn, * currentfile;

error ( num )
int num;
{ fprintf (stderr, "oops  %d \n", num);
  exit (1); }

struct wordnode * newwordnode ()
{
  struct  wordnode  * p;
  if ((p =
           (struct wordnode *) malloc
           (sizeof (struct wordnode)) ) == NULL)
    error (3);
  p -> left = NULL;
  p -> right = NULL;
  p -> files = NULL;
  p -> lastfile = NULL;
  p -> name = (char * ) malloc( strlen(currentid) + 1 );
  strcpy(p -> name, currentid); 
  p -> macro = mac;
  return  p;
}

struct  filenode  * newfilenode ()
{ struct  filenode  * p;
  if ((p =
          (struct filenode *) malloc
          (sizeof (struct filenode)) ) == NULL)
    error (3);
  p -> next = NULL;
  p -> fns = NULL;
  p -> lastfn = NULL;
  p -> name = currentfile;
  return  p; }

struct  fnnode  * newfnnode ()
{ struct  fnnode  * p;
  if ((p =
          (struct fnnode *) malloc
          (sizeof (struct fnnode)) ) == NULL)
    error (3);
  p -> next = NULL;
  p -> lines = NULL;
  p -> lastline = NULL;
  p -> name = currentfn;
  return  p; }

struct linenode  * newlinenode ()
{ struct  linenode  * p;
  if ((p =
          (struct linenode *) malloc
          (sizeof (struct linenode)) ) == NULL)
    error (3);
  p -> next = NULL;
  p -> number = linefound;
  return  p; }

enter ()    /* put an entry in the tree for the identifier. */
{
  struct fnnode * fn;
  struct filenode  * f;
  struct wordnode * ptr, * last;
  int compare;

  if ( ! root ) 
    ptr = root = newwordnode();
  else {             /* find the appropriate word node; build if necessary. */
    ptr = root;
    last = root;
    while (ptr && (compare = strcmp (currentid, ptr -> name)) ) {
      last = ptr;
      if (compare < 0) 
        ptr = ptr -> left;
      else if (compare > 0) 
        ptr = ptr -> right; }
    if (compare < 0)            /* need new node to left of 'last'. */
      ptr = last -> left = newwordnode();
    else if (compare > 0) /* need new node to right of 'last'. */
      ptr = last -> right = newwordnode(); }

/* find the appropriate file node; build if necessary. */
  f = ptr -> files;
  while (f && (f -> name != currentfile) )
    f = f -> next;
  if ( !f ) { 
    if ( ptr -> files )        
      f = ptr -> lastfile = ptr -> lastfile -> next = newfilenode();
    else
      f = ptr -> lastfile = ptr -> files = newfilenode(); }
  
/* find the appropriate function node; build if necessary. */
  fn = f -> fns;
  while (fn && (fn -> name != currentfn))
    fn = fn -> next;
  if ( !fn ) {
    if (f -> fns) 
      fn = f -> lastfn = f -> lastfn -> next = newfnnode();
    else 
      fn = f -> fns = f -> lastfn = newfnnode(); }

/* build a new line number node. */
  if ( fn -> lines) 
    fn -> lastline = fn -> lastline -> next  = newlinenode();
  else
    fn -> lines = fn -> lastline = newlinenode(); }


int notreserved ()   /* is the word a reserved command word? */
{ int lower, upper, middle, compare;
  lower = 0;   upper = NUM_RES_WRDS - 1;  middle = upper / 2;
  while ( TRUE ) {
    compare = strcmp (currentid, rswrds [middle]);
    if ( ! compare )
      return ( FALSE );
    else if ( middle == lower )
      return ( strcmp (currentid, rswrds [upper]));
    else if ( compare < 0 )
      upper = middle;
    else
      lower = middle;
    middle = ( upper + lower ) / 2; }; }

read_id ()
{ int i=0;
  while ( letter() || digit() || (ch == '_') )  {
    if ( i <= CURLEN-4 ) currentid [ i ++ ] = ch; 
    inputchar; }
  currentid [ i ] = '\0'; }

read_num ()
{ while ( letter () || digit () || (ch=='.')  ) inputchar; }

int digit ()
{ return ( (ch >= '0') && (ch <= '9')); }

int letter ()
{ return ( ( (ch >= 'A') && (ch <= 'Z') ) ||
                 ( (ch >= 'a') && (ch <= 'z') )    ); }

starttok ()  /* skip comments, quotes, blanks, and tabs */
{
  while (TRUE) {
    if ( ch == '/' ) {              /* ignore comment */
      inputchar;
      if ( ch != '*') continue;
      inputchar;
      while (TRUE) {
        while ( ch != '*') inputchar;
        inputchar;
        if ( ch == '/') break; }}
    else if ( ch == '\'' ) {        /* ignore characters within quotes */
      inputchar;
      while ( (ch != '\'')  ) {
        if (ch == '\\') inputchar;  /* treat the slash (// means /) and the
                                    character after it as one character. */  
        inputchar; }}
    else if (ch == '"' ) {
      inputchar;
      while ((ch!='"')) { 
        if (ch == '\\') inputchar;
        inputchar;}}
    else if (ch != ' ' && ch != '\t')
      break;                          

    inputchar;}}

writenode ( p )
struct wordnode * p;
{
  int i, j;
  struct  fnnode  * fnp;
  struct  filenode  * fp;
  struct  linenode  * lp;
  putchar('\n');
  fp = p -> files;
  while ( fp ) { 
                    
/* p->macro = 0 => identifier is neither a function nor a macro.
            = 1 => identifier is a macro without parameters.
                   indicate this by putting a '#' before the identifier.
            = 2 => identifier is a macro with parameters.
                   indicate this by putting a '#' before the identifier and a
                   '()' after the identifier.
            = 3 => identifier is a function.  indicate this by putting a '()'
                   after the identifier.                                 
                                                                           */
    if (p->macro==1 || p->macro==2) printf("#  ");
    else printf("   ");
    if (p->macro ==2 || p->macro==3) {
      for (j=1; j<CURLEN-2; ++j) if (*(p->name+j)=='\0' || 
                                 *(p->name+j)=='(') break;
                               /* add '()' to the end of the identifier or
               		       write over the '()' if it was already added */
      *(p ->name+j)='(';
      *(p ->name+j+1)=')';         
      *(p ->name+j+2)='\0';
      printf("%-27s %-15s", p -> name, fp -> name); }
    else printf("%-27s %-15s", p -> name, fp -> name);
    fnp = fp -> fns;
    while ( fnp ) {
      printf (" %-25s", fnp -> name);
      lp = fnp -> lines;
      while ( lp ) {
        i = 0;
        while ( (i < maxnum) && lp ) {
          printf (" %4d", lp -> number);
          lp = lp -> next;
          i ++ ; }
        if ( lp ) printf ("\n%72s", " "); }
      fnp = fnp -> next;   
      if (fnp) printf ("\n%46s", " "); }
    fp = fp -> next;
    if ( fp ) putchar ('\n'); }}

writetree (root)
struct  wordnode  * root;
{ if (root) {
    writetree (root -> left);  writenode ( root );
    writetree (root -> right); }
  else return ; }

main (argc, argv)
int argc;
char * argv [];
{ 
  int fd, i, j;
  struct wordnode * ptr;
  currentarg  = 0;
  maxnum=9;

/*   Maxnum is the maximum number of linenumbers to be printed per output 
line.  In portrait mode, the Penguin printer can only squeeze 3 numbers after
the identifier name, file name, and function name.  In landscape mode, it can
handle 9 numbers.  If the user calls CREF with the -p option, print only 3
numbers per line. */

  for (i=0;i<argc;i++) if (!strcmp(argv[i],"-p")) {
    maxnum=3;
    for (j=i;j<argc-1;j++) argv[j]=argv[j+1]; 
    argc=argc-1; }
  if (argc < 2) {
    fprintf (stderr, "usage: cref file ... or\n       cref -p file ...\n");
    exit (1);   }

/* newarg is like a goto label.  The command longjmp(env,1) will return 
control to the line following newarg */

  newarg: setjmp(env);

/* currentarg files were already processed. */ 

  if  (currentarg < argc-1) {  
    currentarg++;                       
    if (fd > 0)   close(fd);               
    if ((fd = open (argv [currentarg], 0)) < 0) {
      fprintf (stderr, "cref: can't open %s\n", argv[currentarg]);
      goto newarg;}

/* We are parsing line linecount in file argv[currentarg].  blevel indicates
how many brackets deep we are nested.  plevel indicates how many parentheses
deep we are nested.  Since we are at the very top of the file, we are
obviously not in the body of a function, so notinfn is set to true and 
currentfn in set to the default value of '---'.  mac stores the macro value of
the current identifier being processed. */
 
    linecount = 1;
    currentfn = nullfn;
    mac = 0;
    blevel = 0;
    plevel = 0;
    notinfn = TRUE;
    close (0);  dup (fd);

/*  close(0) frees the 0 file identifier that is initially assigned to the
terminal.  dup(fd) is a function that sets the smallest free file identifier
(now 0) equal to the file identifier fd.  i/o will now automatically go to
the file argv[currentarg], instead of the terminal. */

    currentfile = (char * ) malloc ( strlen (argv [currentarg]) + 1 );
    strcpy(currentfile, argv[currentarg]);
    inputchar;
    starttok();
    while ( TRUE ) {
      if ( digit() ) read_num(); 
      else if ( letter() ) {

/* record the linenumber (linefound) at the time when the first letter of
the identifier was encountered.  This may not equal the current linenumber
(linecount) at the time the identifier is to be entered, since starttok() may
have processed several lines of comments and blanks before then. */

        linefound=linecount;
        read_id();
        starttok();
        if ( notreserved() ) {

/* if identifier is followed by a '(', it is a function.  If it is a function
and the the function body follows it, reset currentfn. */

          if ( ch == '(') {
            plevel++;                     
            mac=3;
            enter();
            mac=0; 
            inputchar;
            if (plevel==1 && notinfn && !blevel) {
              if (ch!=')') {
                currentfn = (char * ) malloc( strlen(currentid) + 1 );
                strcpy(currentfn, currentid); 
                notinfn = FALSE; }
              else {
                plevel--;
                inputchar;      
                starttok();
                if ( ch != ',' && ch != ';' ) {
                  currentfn = (char * ) malloc( strlen(currentid) + 1 );
                  strcpy(currentfn, currentid); 
                  notinfn = FALSE; }}}}
         else enter(); }}
      else if ( ch == '#' ) {      /* preprocessor commands */
        inputchar;
        starttok();
        if (ch == '\n') continue;
        linefound=linecount;
        read_id();
        if (!strcmp(currentid, "include")) {
	  while (ch!='\n') {  

/*  ignore everything on this line and continuation lines (line before a cont.
line ends with a backslash and a \n), since the preprocessor will 
replace it by the included file.  */
 
            if (ch=='\\') inputchar;
            inputchar; }
          continue; }
        else if (!strcmp(currentid,"define")) {
          mac=1;
          starttok();
          read_id();
          if (ch=='(') mac=2;     /* found macro with parameters */
          enter();
          mac=0; }
  
  /* don't input preprocessor commandword unless we can't recognize it*/
        else if (strcmp(currentid,"if") && strcmp(currentid,"ifdef")
               && strcmp(currentid,"ifndef") && strcmp(currentid,"endif")
               && strcmp(currentid,"line") && strcmp(currentid,"else") 
	       && notreserved()) enter();
        while (TRUE) {
            starttok(); 
            if (ch=='\\') {         /* continuation line */
              inputchar;  
              inputchar; }
            else if (ch== '\n' ) break;         
            else if (letter()) {  /* find all non-reserved words and enter 
	                             them.  don't worry about (, {, etc... 
                                     since we can't have functions in a define
                                     macro. */
              linefound=linecount;
              read_id();
              if (notreserved()) enter(); }
            else inputchar; }} 
      else if ( ch == ')' ) {
        plevel--;
        inputchar; }
      else if ( ch == '(' ) {
        plevel++;
        inputchar; } 
      else if ( ch == '{' ) {
        blevel++;
        inputchar; }
      else if ( ch == '}' ) {
        blevel--;
        if (!blevel) {
          notinfn = TRUE;
          currentfn = nullfn; }
        inputchar; }
      else inputchar;
      starttok(); }}
  if (maxnum==3) printf ("   %-27s %-15s %-25s %-30s\n\n", "IDENTIFIER", "IN FILE","IN FUNCTION", "FILE LINE NUM.");
  else printf ("   %-27s %-15s %-25s %-30s\n\n", "IDENTIFIER", "IN FILE","IN FUNCTION", "FILE LINE NUMBER");
  writetree ( root );
  exit (0);
}
