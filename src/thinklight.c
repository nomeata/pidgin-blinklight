#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROCFILE "/proc/acpi/ibm/light"

#define ON "on"
#define OFF "off"
#define TOGGLE "toggle"

int main(int argc, char* argv[]) {
	FILE *file;
	int ret;
	char *new_state = NULL;
	char old_state[10] = "";
	
	if (argc != 2) {
		fprintf (stderr,"Argument required: on, off or toggle\n");
		exit(EXIT_FAILURE);
	}

	if (strcmp(argv[1],ON) == 0)  new_state=ON;
	if (strcmp(argv[1],OFF) == 0) new_state=OFF;
	if (strcmp(argv[1],TOGGLE) == 0) {
		file = fopen(PROCFILE,"r");
		      if (file == NULL) exit(EXIT_FAILURE);
		ret = fscanf(file,"status: %9s",old_state);
		      if (ret != 0) exit(EXIT_FAILURE);
		ret = fclose(file);
		      if (ret != 0) exit(EXIT_FAILURE);

		if (strcmp(old_state,ON) == 0)  new_state=OFF;
		if (strcmp(old_state,OFF) == 0) new_state=ON;
	}

	if (new_state == NULL) {
		fprintf (stderr,"No new state defined\n");
		exit(EXIT_FAILURE);
	}
		
	file = fopen(PROCFILE,"w");
	if (file == NULL) exit(EXIT_FAILURE);
	fprintf(file,"%s",new_state);
	ret = fclose(file);
	if (ret != 0) exit(EXIT_FAILURE);

	return(0);
}
