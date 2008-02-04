#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>


#define PROCFILES 3
static char *procfiles[PROCFILES] = {
	"/proc/acpi/ibm/light",
	"/proc/acpi/asus/mled",
	"/sys/class/leds/asus:phone/brightness",
};

int main(int argc, char* argv[]) {
	struct stat info;
	int i;

	for (i=0; i<PROCFILES; i++) {

		if (stat(procfiles[i], &info) != 0) {
			error (0,errno,"Could not stat procfile %s",procfiles[i]);
			continue;
		}
		
		if (chmod(procfiles[i], (info.st_mode & 000777) | S_IWOTH) != 0) {
			error (0,errno,"Could not change permissions of procfile %s",procfiles[i]);
			continue;
		}
	}

	return(EXIT_SUCCESS);
}
