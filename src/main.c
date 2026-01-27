#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mips/mips.h"

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
			c++;
			while (*c != 0) {
				switch (*c) {
					case 'h': flags |= 0x1; break;
					case 'v': flags |= 0x2; break;
					default: fprintf(stderr, "%s: unknown flag '%c'\n", argv[0], *c); flags = 0x1; exitc = 1;
				}
				c++;
			}
		}
	}
	if (argc > 3) {
		fprintf(stderr, "%s: extra operand '%s'\n", argv[0], argv[3]);
		flags = 0x1;
		exitc = 1;
	}
	if (flags & 0x1) {
		printf("usage: %s file [-hv]\n", argv[0]);
		exit(exitc);
	}

	FILE* fd = fopen(argv[1], "rb");
	if (!fd) {
		fprintf(stderr, "%s: unable to open '%s': %s\n", argv[0], argv[1], strerror(errno));
		exit(1);
	}

	void* mem; // TODO: make configurable
	mem = calloc(4, 1<<20); // 4 mb ram
	if (!mem) {
		fprintf(stderr, "%s: unable to allocate memory: %s\n", argv[0], strerror(errno));
		exit(1);
	}

	fread(mem, 1<<20, 4, fd); // 4mb max
	if (ferror(fd)) {
		fprintf(stderr, "%s: error reading file '%s': %s\n", argv[0], argv[1], strerror(errno));
		exit(1);
	}
	int psize = ftell(fd);

	fclose(fd);

	struct minimu_mips mips = mips_init(mem, psize, flags);
	while (mips.pc < mips.cpu.psize && mips.cpu.status == 0) {
		mips.cpu.execute(&mips.cpu);
	}

	// dump memory
	char outfile[256];
	snprintf(outfile, 256, "%s.memout", argv[1]);
	fd = fopen(outfile, "wb");
	fwrite(mem, 1<<20, 4, fd); // also 4mb
	if (ferror(fd)) {
		fprintf(stderr, "%s: error writing file '%s': %s\n", argv[0], outfile, strerror(errno));
		exit(1);
	}
	fclose(fd);
	snprintf(outfile, 256, "%s.regout", argv[1]);
	fd = fopen(outfile, "wb");
	fwrite(mips.registers, 4, 34, fd);
	if (ferror(fd)) {
		fprintf(stderr, "%s: error writing file '%s': %s\n", argv[0], outfile, strerror(errno));
		exit(1);
	}
	fclose(fd);

	return exitc;
}
