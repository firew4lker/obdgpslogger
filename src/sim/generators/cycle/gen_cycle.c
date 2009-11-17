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
 \brief Generate cyclic data
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "datasource.h"
#include "obdservicecommands.h"

#define DEFAULT_CYCLE_US 10000000
#define DEFAULT_CYCLE_GEARS 6
#define US_TO_SEC 1000000

/// The void * generator for cycle
struct cycle_gen {
	long cycle_length; // Time for a complete cycle
	struct timeval firsttime; // The last first that we pulled values
	unsigned char gears; // Time through current cycle
};

const char *cycle_simgen_name() {
	return "Cycle";
}

const char *cycle_simgen_longdesc() {
	return "Cycle through a wide range of valid OBDII values\n"
		"Seed: [cycle-length-in-seconds[,number-of-gears]]";
}

int cycle_simgen_create(void **gen, const char *seed) {
	struct cycle_gen *g = (struct cycle_gen *)malloc(sizeof(struct cycle_gen));
	if(NULL == g) {
		fprintf(stderr, "Couldn't allocate memory for cycle generator\n");
		return 1;
	}

	g->cycle_length = DEFAULT_CYCLE_US;
	gettimeofday(&g->firsttime, NULL);
	g->gears = DEFAULT_CYCLE_GEARS;

	if(NULL != seed && '\0' != *seed) {
		char *seedcpy = strdup(seed);
		
		char *tok = strtok(seedcpy, ", ");
		if(NULL != tok) {
			int t = atoi(tok);
			if(0 < t) {
				g->cycle_length = US_TO_SEC * t;
				printf("Setting cycle length to %i seconds\n", t);
			}

			tok = strtok(NULL, ", ");
			if(NULL != tok) {
				int gears = atoi(tok);
				if(0 < gears) {
					g->gears = gears;
					printf("Setting gears to %i\n", g->gears);
				}
			}
		}
		free(seedcpy);
	}

	*gen = g;
	return 0;
}

void cycle_simgen_destroy(void *gen) {
	free(gen);
}

int cycle_simgen_getvalue(void *gen, unsigned int mode, unsigned int PID, unsigned int *A, unsigned int *B, unsigned int *C, unsigned int *D) {

	struct cycle_gen *g = (struct cycle_gen *)gen;

	if(0x00 == PID) {
		*A = 0xFF;
		*B = 0xFF;
		*C = 0xFF;
		*D = 0xFE;
		return 4;
	} else {
		struct timeval newtime;
		gettimeofday(&newtime, NULL);

		long dt = (US_TO_SEC * ((long)newtime.tv_sec - (long)g->firsttime.tv_sec)
					+ ((long)newtime.tv_usec - (long)g->firsttime.tv_usec)) % g->cycle_length;

		if(dt < 0) dt = 0; // Kluuuuudge

		struct obdservicecmd *cmd = obdGetCmdForPID(PID);
		float min = cmd->min_value;
		float max = cmd->max_value;
		OBDConvRevFunc conv = cmd->convrev;

		float cyclefraction = (float)dt/(float)g->cycle_length;
		float val = min + cyclefraction * (max-min);

		// RPM gets special treatment
		if(0 != strcmp(cmd->db_column, "rpm")) {
			return conv(val, A, B, C, D);
		} else {
			int rpm_min = 500;
			int rpm_range = 6000;
			int curr_gear = (int)(g->gears * cyclefraction);

			float revs = (g->gears * rpm_range) * cyclefraction;
			while(revs > rpm_range) {
				revs -= rpm_range;
			}
			revs += rpm_min;

			// fprintf(stderr, "rpm=%f, dt=%li\n", revs, dt);
			return conv(revs, A, B, C, D);
		}
	}
	return -1; // Shouldn't be able to get here
}

int cycle_simgen_idle(void *gen, int idlems) {
	return 0;
}

// Declare our obdsim_generator. This is pulled in as an extern in obdsim.c
struct obdsim_generator obdsimgen_cycle = {
	cycle_simgen_name,
	cycle_simgen_longdesc,
	cycle_simgen_create,
	cycle_simgen_destroy,
	cycle_simgen_getvalue,
	cycle_simgen_idle
};

