#include <sqlite3ext.h> /* Do not use <sqlite3.h>! */
#include "../../core/StringCompare.h"

SQLITE_EXTENSION_INIT1
/* Insert your extension code here */

#ifdef _WIN32
__declspec(dllexport)
#endif
extern "C" int sqlite3_gearcollate_init(
  sqlite3 *db, 
  char **pzErrMsg, 
  const sqlite3_api_routines *pApi
){
  int rc = SQLITE_OK;
  SQLITE_EXTENSION_INIT2(pApi);

  rc = sqlite3_create_collation(db, "Custom", SQLITE_UTF8, nullptr, &Gear::StringCompare::compareWithLength);
  return rc;
}