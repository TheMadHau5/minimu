#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
	int flags = 0;
	int exitc = 0;
	if (argc < 2) {
		fprintf(stderr, "%s: missing file operand\n", argv[0]);
		// print help
		flags |= 0x1;
	}
	if (argc == 3) {
		if (argv[2][0] == '-') {
		char* c = argv[2];
			while (*c != 0) {
				switch (*c) {
					case 'h': flags |= 0x1; break;
					case 'v': flags |= 0x2; break;
					default: fprintf(stderr, "%s: unknown flag '%c'\n", argv[0], *c); flags = 0x1; exitc = 1;
				}
			}
		}
	}
	if (argc > 3) {
		fprintf(stderr, "%s: extra operand '%s'\n", argv[0], argv[3]);
		flags = 0x1;
		exitc = 1;
	}
	if (flags & 0x1) {
		printf("usage: %s file [-hv]", argv[0]);
		exit(exitc);
	}
	FILE* fd = fopen(argv[1], "rb");
	if (!fd) {
		fprintf(stderr, "%s: unable to open '%s': %s\n", argv[0], argv[1], strerror(errno));
	}

	return 0;
}
