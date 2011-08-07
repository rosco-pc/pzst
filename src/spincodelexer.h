#ifndef SPINCODELEXER_H
#define SPINCODELEXER_H

namespace PZST {
	class SpinCodeLexer {
		public:
                enum {
                    CON = 0,
                    PUB,
                    PRI,
                    OBJ,
                    DAT,
                    VAR,
                    CHAR,
                    CONDITION,
                    EOI,
                    RESERVED,
                    TYPE,
                    STRING,
                    IDENTIFIER,
                    NUMBER,
                    COMMENT,
                    WHITESPACE,
                    NL,
                    PREPRO,
                    FILE,
                };

                enum {
                    CG_CONDITION = 0,
                    CG_COMMENT,
                    CG_RESERVED,
                    CG_IDENTIFIER,
                    CG_NUMBER,
                    CG_TYPE,
                    CG_STRING,
                    CG_PREPRO,
                    CG_OTHER,
                };


                static int scan(char *start, char *end, char** next);
	};
}

#endif

