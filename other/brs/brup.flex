
%START COMMENT

BR_DOWN	[[:space:]]*\n[[:blank:]]*"{"
ELSE	"}"[[:space:]]*else

%%

	/* omit comments */
	
"//".*({BR_DOWN}|{ELSE})? ECHO;
"/*" { BEGIN(COMMENT); ECHO; }
<COMMENT>"*/"({BR_DOWN}|{ELSE})? { BEGIN(INITIAL); ECHO; }

	/* omit preprocessor statements */
	
\#.*({BR_DOWN}|{ELSE})? ECHO;
.*\\$ ECHO;
	
	/* omit line blocks */

.*("{"|"}"){BR_DOWN}	ECHO;

	/* don't overlap : or ; or \\ */
	
[\:\;\\][[:space:]]*"{" ECHO;

{BR_DOWN}+ {
	int i;
	
	for (i = 0; i < yyleng && yytext[i] != '{'; ++i);
	putchar(' ');
	putchar('{');
	while (++i < yyleng) putchar(yytext[i]);
}

{ELSE}	printf("} else");
