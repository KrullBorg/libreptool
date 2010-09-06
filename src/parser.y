%{
#define YYSTYPE char *

#include <string.h>

#include <glib.h>

#include "lexycal.yy.h"
#include "rptreport_priv.h"

void yyerror (RptReport *rpt_report, gint row, gchar **ret, char const *s);
%}

%token INTEGER
%token FLOAT
%token STRING
%token FIELD
%token SPECIAL
%token FUNCTION

%left '&'
%left '+'
%left '-'
%left '*'
%left '/'

%parse-param {RptReport *rpt_report}
%parse-param {gint row}
%parse-param {gchar **ret}

%% /* Grammar rules and actions */
input:    /* empty */
        | input string
;

string: exp      { *ret = g_strdup ($1); }
;

exp:      INTEGER           { $$ = $1; }
        | FLOAT             { $$ = $1; }
        | STRING            { $$ = g_strndup ($1 + 1, strlen ($1) - 2) }
        | FIELD             { $$ = rpt_report_get_field (rpt_report, g_strndup ($1 + 1, strlen ($1) - 2), row); }
        | SPECIAL           { $$ = rpt_report_get_special (rpt_report, $1, row); }
		| exp '+' exp		{ $$ = g_strdup_printf ("%f", strtod ($1, NULL) + strtod ($3, NULL)); }
		| exp '-' exp		{ $$ = g_strdup_printf ("%f", strtod ($1, NULL) - strtod ($3, NULL)); }
		| exp '*' exp		{ $$ = g_strdup_printf ("%f", strtod ($1, NULL) * strtod ($3, NULL)); }
		| exp '/' exp		{ $$ = g_strdup_printf ("%f", strtod ($1, NULL) / strtod ($3, NULL)); }
		| exp '&' exp		{ $$ = g_strconcat ($1, $3, NULL); }
        | '(' exp ')'       { $$ = $2; }
;
%%

/* Called by yyparse on error.  */
void
yyerror (RptReport *rpt_report, gint row, gchar **ret, char const *s)
{
	g_warning ("Bison error: %s", s);
}
