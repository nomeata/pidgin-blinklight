/* © 2009 Joachim Breitner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <blink.h>

#define OFF    0
#define ON     1
#define TOGGLE 2


struct interface {
	char *sysfs;
	char *commands[2];
	char *scanline;
};

#define INTERFACES 3
static struct interface interfaces[] = {
	{"/proc/acpi/ibm/light", 	                {"off","on"},	"status: %9s" },
	{"/proc/acpi/asus/mled", 	                {"0","1"},	    "%9s" },
	{"/sys/class/leds/asus:phone/brightness",   {"0","1"},	    "%9s" }
};

static struct interface *interface = NULL;

typedef struct {
	int	state;
	int	time;
} blinky;

typedef struct {
	blinky *seq;
	int pos;
} blinkstate;

/* TODO: We should really allow an option to continue blinking indefinitely
         until user opens message */
blinky default_seq[] = {
	{TOGGLE,	150},
	{TOGGLE,	125},
	{TOGGLE,	150},
	{TOGGLE,	0}
};

static guint
blinklight_blink(blinkstate *bstate) {
	FILE *file;
	char *new_state = NULL;
	char old_state[10] = "";
	int ret;

	blinky *seq = &bstate->seq[bstate->pos];

	if (interface == NULL) return FALSE;

	// purple_debug_info("pidgin-blinklight","blink called with parameter: %i\n", seq->state);
	
	if (seq->state == ON)  new_state=interface->commands[ON];
        if (seq->state == OFF) new_state=interface->commands[OFF];
	if (seq->state == TOGGLE) {
 	        // purple_debug_info("pidgin-blinklight","trying to toggle\n");
                file = fopen(interface->sysfs,"r");
                      if (file == NULL) { perror ("Trying to open sysfs for reading"); return FALSE;};
                ret = fscanf(file,interface->scanline,old_state);
                      if (ret == EOF)  { perror ("Trying to read from sysfs"); return FALSE;};
                ret = fclose(file);
                      if (ret != 0) { perror ("Trying to close sysfs"); return FALSE;};
 	        // purple_debug_info("pidgin-blinklight","done reading old state %s\n", old_state);

                if (strcmp(old_state,interface->commands[ON])  == 0) new_state=interface->commands[OFF];
                if (strcmp(old_state,interface->commands[OFF]) == 0) new_state=interface->commands[ON];
        }
	
	if (new_state == NULL) {
                // purple_debug_info("pidgin-blinklight","No new state defined\n");
		return FALSE;
        }
 	// purple_debug_info("pidgin-blinklight","setting new state: %s\n", new_state);

	file = fopen(interface->sysfs,"w");
                if (file == NULL) { perror ("Trying to open sysfs for writing"); return FALSE;};
        ret = fprintf(file,"%s",new_state);
	        if (ret < 0)   { perror ("Trying to write to sysfs"); return FALSE;};
        ret = fclose(file);
                if (ret != 0) { perror ("Trying to close sysfs"); return FALSE;};

 	// purple_debug_info("pidgin-blinklight","done setting new state: %s\n", new_state);
	
	if (seq->time) { 
		bstate->pos++;
		blinklight_timeout_add(seq->time,(GSourceFunc)blinklight_blink,bstate);
	} else {
		free(bstate->seq);
		free(bstate);
	}
	return FALSE;
}

// From http://en.wikipedia.org/wiki/Jenkins_hash_function
static guint32
jenkins_one_at_a_time_hash(const gchar *key)
{
    guint32 hash = 0;
 
    while (*key){
        hash += *(key++);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}


void
blinklight_startblink(const gchar *seed) {
	int length = 4;
	blinkstate *bstate = malloc(sizeof(blinkstate));
	blinky *seq ;

 	if (seed == NULL) {
		seq = malloc(sizeof(default_seq));
		memcpy(seq, default_seq, sizeof(default_seq));

	} else {
		seq = calloc(sizeof(blinky),length);
		
		// Initialize to four toggle commands
		for (int i=0; i<length; i++) {
			seq[i].state = TOGGLE;
		}

		guint32 hash = jenkins_one_at_a_time_hash(seed);
		// Set timeing based on string
		// Range: 500 + [0,1]*200 ms
		for (int i=0; i<length-1; i++) {
			// Uses last 8 bits
			seq[i].time = 50 + hash % 200;
			// So remove them
			hash = (hash << 8); 
			// printf("Time %d: %d\n", i, seq[i].time);
		}
	}

	bstate->seq = seq;
	bstate->pos = 0;
	blinklight_blink(bstate);
}

char *
blinklight_init() {
	// figure out which interface to use
	for (int i=0; i< INTERFACES; i++) {
		if (! access(interfaces[i].sysfs, R_OK)) {
			// File exists and is readable (not checking writable because of possible race condition)
			interface = &interfaces[i];
			return interfaces[i].sysfs;
		}
		
	}
	return NULL;
}
