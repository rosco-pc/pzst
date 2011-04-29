#ifndef SPINCODELEXER_H
#define SPINCODELEXER_H

namespace PZST {
	class SpinCodeLexer {
		public:
		typedef enum {
                        CHAR = 0,
                        CONDITION,
			EOI,
			RESERVED,
			TYPE,
			STRING,
			IDENTIFIER,
			NUMBER,
			COMMENT,
			PUB,
			PRI,
			DAT,
			OBJ,
			CON,
			VAR,
			WHITESPACE,
                        NL,
                        PREPRO,
                        FILE,
                } Retval;

                static Retval scan(char *start, char *end, char** next);
	};
}

#endif

