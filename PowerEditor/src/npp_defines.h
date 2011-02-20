#ifndef NPP_DEFINES_H
#define NPP_DEFINES_H

// make sure exceptions are show at the place of occurence 
// so that we easier can follow the call stack in debug mode
#ifdef _RELEASE
#define NPP_TRY try
#define NPP_CATCH(ex) catch(ex)
#define NPP_CATCH_ALL catch(...)
#else
#define NPP_TRY if (true)
#define NPP_CATCH(ex) for(ex;false;)
#define NPP_CATCH_ALL if(false)
#endif

#endif // NPP_DEFINES_H