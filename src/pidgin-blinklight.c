/* © 2005-2007 Joachim Breitner
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

#define PURPLE_PLUGINS

#include <unistd.h>

#ifdef ENABLE_NLS
#  include <locale.h>
#  include <libintl.h>
#  define _(x) dgettext(PACKAGE, x)
#  ifdef dgettext_noop
#    define N_(String) dgettext_noop (PACKAGE, String)
#  else
#    define N_(String) (String)
#  endif
#else
#  include <locale.h>
#  define N_(String) (String)
#  define _(x) (x)
#  define ngettext(Singular, Plural, Number) ((Number == 1) ? (Singular) : (Plural))
#endif



#include <debug.h>
// #include <pidgin.h>
#include <plugin.h>
#include <conversation.h>
#include <version.h>
#include <eventloop.h>
#include <string.h>

#define OFF    0
#define ON     1
#define TOGGLE 2

struct interface {
	char *procfile;
	char *commands[2];
	char *scanline;
};

#define INTERFACES 2
static struct interface interfaces[] = {
	{"/proc/acpi/ibm/light", 	{"off","on"},	"status: %9s" },
	{"/proc/acpi/asus/mled", 	{"0","1"},	"%9s" },
};

static struct interface *interface = NULL;

struct blinky {
	int	state;
	int	time;
};

static struct blinky seq[4] = {
	{TOGGLE,	150},
	{TOGGLE,	125},
	{TOGGLE,	150},
	{TOGGLE,	0}
};

static guint
blink(struct blinky *seq) {
	FILE *file;
	char *new_state = NULL;
	char old_state[10] = "";
	int ret;

	if (interface == NULL) return FALSE;

	// purple_debug_info("pidgin-blinklight","blink called with parameter: %i\n", seq->state);
	
	if (seq->state == ON)  new_state=interface->commands[ON];
        if (seq->state == OFF) new_state=interface->commands[OFF];
	if (seq->state == TOGGLE) {
 	        // purple_debug_info("pidgin-blinklight","trying to toggle\n");
                file = fopen(interface->procfile,"r");
                      if (file == NULL) { perror ("Trying to open procfile for reading"); return FALSE;};
                ret = fscanf(file,interface->scanline,old_state);
                      if (ret == EOF)  { perror ("Trying to read from procfile"); return FALSE;};
                ret = fclose(file);
                      if (ret != 0) { perror ("Trying to close PROCFILE"); return FALSE;};
 	        // purple_debug_info("pidgin-blinklight","done reading old state %s\n", old_state);

                if (strcmp(old_state,interface->commands[ON])  == 0) new_state=interface->commands[OFF];
                if (strcmp(old_state,interface->commands[OFF]) == 0) new_state=interface->commands[ON];
        }
	
	if (new_state == NULL) {
                // purple_debug_info("pidgin-blinklight","No new state defined\n");
		return FALSE;
        }
 	// purple_debug_info("pidgin-blinklight","setting new state: %s\n", new_state);

	file = fopen(interface->procfile,"w");
                if (file == NULL) { perror ("Trying to open procfile for writing"); return FALSE;};
        ret = fprintf(file,"%s",new_state);
	        if (ret < 0)   { perror ("Trying to write to procfile"); return FALSE;};
        ret = fclose(file);
                if (ret != 0) { perror ("Trying to close procfile"); return FALSE;};

 	// purple_debug_info("pidgin-blinklight","done setting new state: %s\n", new_state);
	
	if (seq->time) 
		purple_timeout_add(seq->time,(GSourceFunc)blink,seq+1);

	return FALSE;
}

	

static void
gt_blink(PurpleAccount *account, const gchar *sender,
         const gchar *message, int flags, gpointer data)
{
	if (purple_prefs_get_bool("/plugins/core/pidgin-blinklight/focus"))
	{
		// Blink in any case
		blink(seq);
	} else {
		// Only blink when not having focus
    		PurpleConversation *conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_ANY, sender, account);
	    	if (! purple_conversation_has_focus(conv)) 
		        blink(seq);
	}
}

static PurplePluginPrefFrame *
get_plugin_pref_frame(PurplePlugin *plugin)
{
	PurplePluginPrefFrame *frame;
	PurplePluginPref *ppref;

	frame = purple_plugin_pref_frame_new();

	ppref = purple_plugin_pref_new_with_name_and_label(
		"/plugins/core/pidgin-blinklight/focus",
		_("Blink even when conversation has the _focus.")
	);

	purple_plugin_pref_frame_add(frame, ppref);
       
	return frame;
}

static PurplePluginUiInfo prefs_info = 
{
	get_plugin_pref_frame
};

static gboolean
gt_load(PurplePlugin *plugin) {
	int i;

	// Make /proc file writable
	if (!fork())
		execl("/usr/lib/pidgin-blinklight/blinklight-fixperm","blinklight-fixperm",NULL);
	
	// figure out which interface to use
	for (i=0; i< INTERFACES; i++) {
		if (! access(interfaces[i].procfile, R_OK)) {
			// File exists and is readable (not checking writable because of possible race condition)
			purple_debug_info("pidgin-blinklight","chose file %s.\n", interfaces[i].procfile);
			interface = &interfaces[i];
		}
		
	}
	if (interface == NULL) 
		purple_debug_info("pidgin-blinklight","no suitable file found, deactivating plugin.");
	else
		purple_signal_connect(
			purple_conversations_get_handle(),
			"received-im-msg",
			plugin,
			PURPLE_CALLBACK(gt_blink),
			NULL
		);

	// purple_debug_info("pidgin-blinklight","pidgin-blinklight has loaded\n");
	return TRUE;
}

static gboolean
gt_unload(PurplePlugin *plugin) {

	// purple_debug_info("pidgin-blinklight","pidgin-blinklight has unloaded\n");
	return TRUE;
}

static PurplePluginInfo gt_info = {
	PURPLE_PLUGIN_MAGIC,									/* plugin magic		*/
	PURPLE_MAJOR_VERSION,									/* major purple version	*/
	PURPLE_MINOR_VERSION,									/* minor purple version	*/
	PURPLE_PLUGIN_STANDARD,									/* type			*/
	NULL,											/* ui requirement	*/
	0,											/* flags		*/
	NULL,											/* dependencies		*/
	PURPLE_PRIORITY_DEFAULT,									/* priority		*/
	"gtk-nomeata-blinklight",								/* id			*/
	NULL,											/* name			*/
	VERSION,										/* version		*/
	NULL,											/* summary		*/
	NULL,											/* description		*/

	"Joachim Breitner <mail@joachim-breitner.de>",						/* author		*/
	"http://www.joachim-breitner.der/blog/",										/* homepage		*/

	gt_load,										/* load			*/
	gt_unload,										/* unload		*/
	NULL,											/* destroy		*/

	NULL,											/* ui info		*/
	NULL,											/* extra info		*/
	&prefs_info,                                                        		        /* plugin prefs         */
	NULL											/* plugin actions	*/
};

static void
init_plugin(PurplePlugin *plugin) {

	purple_prefs_add_none("/plugins/core/pidgin-blinklight");
	purple_prefs_add_bool("/plugins/core/pidgin-blinklight/focus", TRUE);

#ifdef ENABLE_NLS
	bindtextdomain(PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
#endif

	gt_info.name		= _("Pidgin-Thinklight");
	gt_info.summary		= _("Flickering Messages");
	gt_info.description	= _(	"Pidgin-Thinklight:\n"
					"Flashes the ThinkLight upon new messages\n"
					"To use this, you need to have the ibm-acpi kernel module loaded.");

	// purple_debug_info("pidgin-blinklight","pidgin-blinklight has init'ed");
}

PURPLE_INIT_PLUGIN(Pidgin-Thinklight, init_plugin, gt_info)
