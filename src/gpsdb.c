/* Copyright 2009 Gary Briggs

This file is part of obdgpslogger.

obdgpslogger is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

obdgpslogger is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with obdgpslogger.  If not, see <http://www.gnu.org/licenses/>.
*/


/** \file
 \brief gps database logging stuff
 */

#ifdef HAVE_GPSD

#include "gpsdb.h"

#include <stdio.h>

#include "sqlite3.h"

int creategpstable(sqlite3 *db) {
	char create_sql[] = "CREATE TABLE gps (lat REAL, lon REAL, alt REAL, time INTEGER)";

	/// sqlite3 return status
	int rc;
	/// sqlite3 error message
	char *errmsg;

	if(SQLITE_OK != sqlite3_exec(db, create_sql, NULL, NULL, &errmsg)) {
		printf("sqlite error on statement %s: %s\n", create_sql, errmsg);
		sqlite3_free(errmsg);
		return 1;
	}
	return 0;
}

int creategpsinsertstmt(sqlite3 *db, sqlite3_stmt **ret_stmt) {
	char insert_sql[] = "INSERT INTO gps (lat,lon,alt,time) VALUES (?,?,?,?)";

	int rc;
	const char *zTail;

	rc = sqlite3_prepare_v2(db,insert_sql,-1,ret_stmt,&zTail);
	if(SQLITE_OK != rc) {
		fprintf(stderr, "Can't prepare statement %s: %s\n", insert_sql, sqlite3_errmsg(db));
		return 0;
	}

	return 4;

}

#endif // HAVE_GPSD
