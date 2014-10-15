
%START COMMENT 

%{

#define BUF_SIZE 1025

char char_buf[BUF_SIZE] ;
char *buf_end = NULL;

%}

TRIM	([[:space:]]*\n)?

%%

	/* omit comments */
	
"//".*(\n[[:space:]]*"{")?	ECHO;
"/*"	{ BEGIN(COMMENT); ECHO; }
<COMMENT>"*/"	{ BEGIN(INITIAL); ECHO; }

	/* calculate last reentrance */
	
^[[:blank:]]* { 
	char ch = yytext[yyleng];
	
	if (yyleng < BUF_SIZE) {
		buf_end = char_buf;
		memcpy(buf_end, yytext, yyleng );
		buf_end += yyleng ;
		*buf_end = '\0';
		ECHO;
	} else {
		error("buffer overflow");
		exit(1);
	}
}

^"{".*("{"|"}").*{TRIM}	|
^. {
	buf_end = char_buf;
	*buf_end = '\0';
	ECHO;
}

	/* omit preprocessor statements */
	
\#.* ECHO;
.*\\$ ECHO;
	
	/* omit line blocks */

.*"{".*("{"|"}").*{TRIM}	ECHO;

	/* open bracket */
	
[^[:space:]][[:blank:]]*"{"{TRIM}	printf("%c\n%s{\n", yytext[0], char_buf);

	/* else */
	
"}"[[:blank:]]*els	printf("}\n%sels", char_buf);
