#include "tclMainViewLexer.h"
#include "Common.h"

#ifdef AP_LEXER_SUPPORT
#define AP_LEXER_EXPORT __declspec(dllexport) 
#define AP_LEXER_DECL __cdecl // __stdcall
#define AP_LEXER_NAME_LEN 16
#define AP_LEXER_DESC_LEN 32

#include <tchar.h>
extern "C" {
   AP_LEXER_EXPORT void* __cdecl GetLexerFunction(unsigned int Index) {
      return 0;
   }
   AP_LEXER_EXPORT int __cdecl GetLexerCount() {
      return 1;
   }
   AP_LEXER_EXPORT void AP_LEXER_DECL GetLexerName(unsigned int Index, char *name, int buflength) {
      strcpy(name, "AnaRes");
   }
   AP_LEXER_EXPORT LexerFactoryFunction AP_LEXER_DECL GetLexerFactory(unsigned int Index) {
      return (LexerFactoryFunction)0;
   }
   AP_LEXER_EXPORT void __cdecl GetLexerStatusText(unsigned int Index, TCHAR *desc, int buflength) {
      wcscpy(desc, TEXT("Analyse Result Visualization"));
   }
}

#endif // AP_LEXER_SUPPORT
