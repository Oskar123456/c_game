#include "incl.h"
#include "c_log.h"
#include "scrape.h"
#include "mongoose.h"

/*
 * -----------------------
 * RESTfulness in C.......
 * ***********************
 * Oskar Bahner Hansen....
 * cph-oh82@cphbusiness.dk
 * 2024-10-31.............
 * -----------------------
 */

void ev_handler(struct mg_connection *c, int ev, void *ev_data);

int serve(int argc, char **argv);

