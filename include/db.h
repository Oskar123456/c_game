#ifndef DB_H
#define DB_H

#include "../include/incl.h"
#include "../include/c_log.h"
#include <postgresql/libpq-fe.h>

/*
 * -----------------------
 * RESTfulness in C.......
 * ***********************
 * Oskar Bahner Hansen....
 * cph-oh82@cphbusiness.dk
 * 2024-11-08.............
 * -----------------------
 */

PGconn* db_connect();
void db_print_table(PGconn* conn, const char* table);
bool db_exec_script(PGconn* conn, const char* path);
bool db_print_schema_stats(PGconn* conn);

#endif
