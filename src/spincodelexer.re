#include <QtGlobal>
#define YYCTYPE unsigned char
#define YYLIMIT end
#define YYCURSOR start
#define	YYMARKER marker
#define YYFILL(n) {if (n == 1) RET(EOI);}
#define RET(x) {if (YYCURSOR >= end) YYCURSOR = end-1; *next = YYCURSOR; return x;}
#define YYCTXMARKER ctx

#define YYDEBUG(state,  current) {qDebug("%d %c", state, current);}

#include <stdio.h>
#include <string.h>
#include "spincodelexer.h"

namespace PZST {
SpinCodeLexer::Retval SpinCodeLexer::scan(char *start_, char *end_, char** next_)
{
        unsigned char *start = (unsigned char *)start_;
        unsigned char *end = (unsigned char *)end_;
        unsigned char **next = (unsigned char **)next_;
	YYCTYPE *marker;
	YYCTYPE *ctx;
/*!re2c
	
	re2c:yyfill:enable   = 1;

	("IF_ALWAYS" | "IF_NEVER" | "IF_E" | "IF_Z" | "IF_NE" | "IF_NZ" | "IF_NC_AND_NZ" | "IF_NZ_AND_NC" | "IF_A" | "IF_B" | "IF_C" | "IF_AE" | "IF_NC" | "IF_C_OR_Z" | "IF_Z_OR_C" | "IF_BE" | "IF_C_EQ_Z" | "IF_Z_EQ_C" | "IF_C_NE_Z" | "IF_Z_NE_C" | "IF_C_AND_Z" | "IF_Z_AND_C" | "IF_C_AND_NZ" | "IF_NZ_AND_C" | "IF_NC_AND_Z" | "IF_Z_AND_NC" |  "IF_C_OR_NZ" | "IF_NZ_OR_C" | "IF_NC_OR_Z" | "IF_Z_OR_NC" | "IF_NC_OR_NZ" | "IF_NZ_OR_NC" |  "WC" | "WZ" | "WR" | "NR" | "IF" | "ELSE" | "ELSEIF" | "ELSEIFNOT" | "IFNOT" | "CASE" | "OTHER" | "REPEAT" | "FROM" | "TO" | "STEP" | "UNTIL" | "WHILE" | "NEXT" | "QUIT")/[^a-zA-Z0-9_]
		{ RET(CONDITION); }

        ("ABS" | "ABSNEG" | "ADD" | "ADDABS" | "ADDS" | "ADDSX" | "ADDX" | "AND" | "ANDN" | "BYTEFILL" | "BYTEMOVE" | "CALL" | "CHIPVER" | "CLKFREQ" | "_CLKFREQ" | "CLKMODE" | "_CLKMODE" | "CLKSET" | "CMP" | "CMPS" | "CMPSUB" | "CMPSX" | "CMPX" | "CNT" | "COGID" | "COGINIT" | "COGNEW" | "COGSTOP" | "CONSTANT" | "CTRA" | "CTRB" | "DIRA" | "DIRB" | "DJNZ" | "ENC" | "FALSE" | "FIT" | "_FREE" | "FRQA" | "FRQB" | "HUBOP" | "INA" | "INB" | "CONST" | "JMP" | "JMPRET" | "LOCKCLR" | "LOCKNEW" | "LOCKRET" | "LOCKSET" | "LONGFILL" | "LONGMOVE" | "LOOKDOWN" | "LOOKDOWNZ" | "LOOKUP" | "LOOKUPZ" | "MAX" | "MAXS" | "MIN" | "MINS" | "MOV" | "MOVD" | "MOVI" | "MOVS" | "MUL" | "MULS" | "MUXC" | "MUXNC" | "MUXNZ" | "MUXZ" | "NEG" | "NEGC" | "NEGNC" | "NEGNZ" | "NEGX" | "NEGZ" | "NOP" | "NOT" | "ONES" | "OR" | "ORG" | "OUTA" | "OUTB" | "PAR" | "PHSA" | "PHSB" | "PI" | "PLL16X" | "PLL1X" | "PLL2X" | "PLL4X" | "PLL8X" | "POSX" | "RCFAST" | "RCL" | "RCR" | "RCSLOW" | "RDBYTE" | "RDLONG" | "RDWORD" | "REBOOT" | "RES" | "RESULT" | "RET" | "REV" | "ROL" | "ROR" | "SAR" | "SHL" | "SHR" | "SPR" | "_STACK" | "STRCOMP" | "STRING" | "STRSIZE" | "SUB" | "SUBABS" | "SUBS" | "SUBSX" | "SUBX" | "SUMC" | "SUMNC" | "SUMNZ" | "SUMZ" | "TEST" | "TESTN" | "TJNZ" | "TJZ" | "TRUE" | "VCFG" | "VSCL" | "WAITCNT" | "WAITPEQ" | "WAITPNE" | "WAITVID" | "WORDFILL" | "WORDMOVE" | "WRBYTE" | "WRLONG" | "WRWORD" | "_XINFREQ" | "XINPUT" | "XOR" | "XTAL1" | "XTAL2" | "XTAL3" | "RETURN" | "ABORT")/[^a-zA-Z0-9_]
		{ RET(RESERVED); }
	
	("LONG" | "BYTE" | "WORD")/[^a-zA-Z0-9_] {RET(TYPE);}
	
	("PUB")/[^a-zA-Z0-9_] {RET(PUB);}
	
	("PRI")/[^a-zA-Z0-9_] {RET(PRI);}

	("DAT")/[^a-zA-Z0-9_] {RET(DAT);}

	("OBJ")/[^a-zA-Z0-9_] {RET(OBJ);}

	("VAR")/[^a-zA-Z0-9_] {RET(VAR);}

        ("CON")/[^a-zA-Z0-9_] {RET(CON);}
        ("FILE")/[^a-zA-Z0-9_] {RET(FILE);}

	[a-zA-Z_] [a-zA-Z0-9_]* {RET(IDENTIFIER);}

	'%'('_'*)[01][01_]* {RET(NUMBER);}
	'%%'('_'*)[0123][0123_]* {RET(NUMBER);}
        '$'('_'*)([a-fA-F0-9])([a-fA-F0-9_]*) {RET(NUMBER);}
	[0-9][0-9_]* {RET(NUMBER);}

        ("'" (.\"\x00")*)/[\n\x00] {RET(COMMENT);}

        "{" [^{}] ([^}\x00])* ("}"|"\x00") {RET(COMMENT);}
        "{}" {RET(COMMENT);}
        "{{" [^}] ([^}\x00]|"}"[^}])* ("}}"|"\x00") {RET(COMMENT);}
        "{{}}" {RET(COMMENT);}

        ["] ([^"\x00])* (["]|"\x00") {RET(STRING);}

        '#' ("ifdef"|"ifndef"|"endif"|"define"|"undef")/[^a-zA-Z0-9_] {RET(PREPRO);}


	[\n] {RET(NL);}
        ([ ] | '\t' | '\r')+ {RET(WHITESPACE);}
	[\x00] {RET(EOI);}
        [\x80-\xFF]+ {RET(CHAR);}
        [^\x80-\xFF] {RET(CHAR);}

*/
}
}
