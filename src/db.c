#include "../include/db.h"
#include "../include/util.h"
#include <postgresql/libpq-fe.h>

/* -----------------------
 * RESTfulness in C.......
 * ***********************
 * Oskar Bahner Hansen....
 * cph-oh82@cphbusiness.dk
 * 2024-11-08.............
 * ----------------------- */

void custom_pq_notice_processor (void *arg, const char *message)
{
    (void)arg;
    char *message_no_newline = strdup(message);
    if (strlen(message) > 0)
        message_no_newline[strlen(message) - 1] = 0;
    c_log_warn("from postgres server", -1, message_no_newline);
    free(message_no_newline);
}

static void exit_nicely(PGconn *conn, PGresult *res)
{
    if (res)
        PQclear(res);
    PQfinish(conn);
    c_log_info("exit_nicely", -1, "exiting nicely");
    exit(1);
}

PGconn* db_connect()
{
    PGconn *conn;

    sds conn_str = sdscatprintf(sdsempty(), "host=%s port=%s dbname=%s user=%s password=%s connect_timeout=10",
            getenv("DB_HOST"), getenv("DB_PORT"), getenv("DB_NAME"),
            getenv("DB_USER"), getenv("DB_PASSWORD"));
    /* Make a connection to the database */
    conn = PQconnectdb(conn_str);

    /* Check to see that the backend connection was successfully made */
    if (PQstatus(conn) != CONNECTION_OK)
    {
        c_log_error(LOG_TAG, "%s", PQerrorMessage(conn));
        exit_nicely(conn, NULL);
    }

    PQsetNoticeProcessor(conn, custom_pq_notice_processor, NULL);

    /* Set always-secure search path, so malicious users can't take control. */
    PGresult *res = PQexec(conn, "SELECT pg_catalog.set_config('search_path', '', false)");
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        c_log_error(LOG_TAG, "%s", PQerrorMessage(conn));
    }
    PQclear(res);

    /* first, print out the attribute names */
    sdsfree(conn_str);
    return conn;
}

void db_print_table(PGconn* conn, const char* table_name)
{
    PGresult *res;

    sds stmt = sdscatprintf(sdsempty(), "select * from public.%s", table_name);

    /* Set always-secure search path, so malicious users can't take control. */
    res = PQexec(conn, "SELECT pg_catalog.set_config('search_path', '', false)");
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        c_log_error(LOG_TAG, "%s", PQerrorMessage(conn));
        exit_nicely(conn, res);
    }
    PQclear(res);

    res = PQexec(conn, stmt);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        c_log_error(LOG_TAG, "%s", PQerrorMessage(conn));
        exit_nicely(conn, res);
    }

    /* better print option */
    PQprintOpt pq_printopts = { 0 };
    pq_printopts.header = true;
    pq_printopts.align = true;
    pq_printopts.fieldSep = "|";
    PQprint(stdout, res, &pq_printopts);

    sdsfree(stmt);
    PQclear(res);
}

bool db_print_schema_stats(PGconn* conn)
{
    bool      exit_code;
    sds       stmt = sdsfread(sdsempty(), "resources/sql/gen_stats_overview.sql");
    PGresult *res;

    res = PQexec(conn, stmt);
    if (PQresultStatus(res) == PGRES_FATAL_ERROR || PQresultStatus(res) == PGRES_NONFATAL_ERROR)
    {
        c_log_error(LOG_TAG, "%s", PQerrorMessage(conn));
        goto FAIL;
    }

    PQprintOpt pq_printopts = { 0 };
    pq_printopts.header = true;
    pq_printopts.align = true;
    pq_printopts.fieldSep = "|";
    PQprint(stdout, res, &pq_printopts);

SUCCESS:
    exit_code = true;
    goto CLEAN;
FAIL:
    exit_code = false;
    goto CLEAN;
CLEAN:
    sdsfree(stmt);
    PQclear(res);
    return exit_code;
}

bool db_exec_script(PGconn* conn, const char* path)
{
    c_log_info(LOG_TAG, "source: %s", path);

    PGresult *res = NULL;
    sds stmt = sdsfread(sdsempty(), path);
    bool exit_code = false;

    res = PQexec(conn, stmt);
    if (PQresultStatus(res) == PGRES_FATAL_ERROR)
    {
        c_log_error(LOG_TAG, "%s", PQerrorMessage(conn));
        goto FAIL;
    }

SUCCESS:
    exit_code = true;
    goto CLEAN;
FAIL:
    exit_code = false;
    goto CLEAN;
CLEAN:
    sdsfree(stmt);
    PQclear(res);
    return exit_code;
}
