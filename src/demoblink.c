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

#include <glib.h>
#include <stdio.h>

#include "blink.h"


// Cheap mans timeout add function
guint timeout_interval = 0;
GSourceFunc timeout_function;
gpointer timeout_data;

guint
blinklight_timeout_add (guint interval, GSourceFunc function, gpointer data) {
	timeout_interval = interval;
	timeout_function = function;
	timeout_data = data;
	return 0;
}

int main(int argc, char **argv) {
	char *file = blinklight_init();

	if (file == NULL) {
		printf("Could not find LED file to use.\n");
		return 1;

	}
	printf("Using file %s.\n", file);

	blinklight_startblink();

	while (timeout_interval > 0) {
		usleep(timeout_interval*1000);
		timeout_interval = 0;
		(*timeout_function)(timeout_data);
	}
}

