#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mips/mips.h"
#include "mips/dbg.h"

#define MINIMU_MEM_SIZE (4u << 20) // 4 MB

static void usage(const char *argv0, int exitc) {
	printf("usage: %s file [-hv] [-d] [-b addr]...\n", argv0);
	printf("  -h          show this help\n");
	printf("  -v          verbose per-instruction trace\n");
	printf("  -d, --debug run interactively under the debugger\n");
	printf("  -b addr     preset a breakpoint (hex or decimal); implies -d\n");
	exit(exitc);
}

int main(int argc, char** argv) {
	int flags = 0;
	bool debug_mode = false;
	const char *file = NULL;
	uint64_t pending_breakpoints[MINIMU_DEBUGGER_MAX_BREAKPOINTS];
	size_t pending_count = 0;

	for (int i = 1; i < argc; i++) {
		const char *a = argv[i];
		if (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0) {
			usage(argv[0], 0);
		} else if (strcmp(a, "-v") == 0 || strcmp(a, "--verbose") == 0) {
			flags |= 0x2;
		} else if (strcmp(a, "-d") == 0 || strcmp(a, "--debug") == 0) {
			debug_mode = true;
		} else if (strcmp(a, "-b") == 0 || strcmp(a, "--break") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "%s: -b requires an address argument\n", argv[0]);
				usage(argv[0], 1);
			}
			char *end = NULL;
			uint64_t addr = strtoull(argv[++i], &end, 0);
			if (end == argv[i]) {
				fprintf(stderr, "%s: invalid breakpoint address '%s'\n", argv[0], argv[i]);
				usage(argv[0], 1);
			}
			if (pending_count < MINIMU_DEBUGGER_MAX_BREAKPOINTS) {
				pending_breakpoints[pending_count++] = addr;
			}
			debug_mode = true;
		} else if (a[0] == '-') {
			fprintf(stderr, "%s: unknown flag '%s'\n", argv[0], a);
			usage(argv[0], 1);
		} else if (!file) {
			file = a;
		} else {
			fprintf(stderr, "%s: extra operand '%s'\n", argv[0], a);
			usage(argv[0], 1);
		}
	}

	if (!file) {
		fprintf(stderr, "%s: missing file operand\n", argv[0]);
		usage(argv[0], 1);
	}

	FILE* fd = fopen(file, "rb");
	if (!fd) {
		fprintf(stderr, "%s: unable to open '%s': %s\n", argv[0], file, strerror(errno));
		exit(1);
	}

	void* mem;
	mem = calloc(1, MINIMU_MEM_SIZE);
	if (!mem) {
		fprintf(stderr, "%s: unable to allocate memory: %s\n", argv[0], strerror(errno));
		exit(1);
	}

	fread(mem, 1, MINIMU_MEM_SIZE, fd);
	if (ferror(fd)) {
		fprintf(stderr, "%s: error reading file '%s': %s\n", argv[0], file, strerror(errno));
		exit(1);
	}
	long psize = ftell(fd);

	fclose(fd);

	struct minimu_mips mips = mips_init(mem, (uint32_t)psize, (uint16_t)flags);

	if (debug_mode) {
		minimu_debug_t dbg;
		minimu_debug_init(&dbg);
		for (size_t i = 0; i < pending_count; i++) {
			minimu_debug_add_breakpoint(&dbg, pending_breakpoints[i]);
		}
		minimu_debug_repl(&mips, &dbg, MINIMU_MEM_SIZE);
	} else {
		while (mips.special[0] < mips.cpu.psize && mips.cpu.status == 0) {
			mips.cpu.execute(&mips.cpu);
		}
	}

	// dump memory
	char outfile[256];
	snprintf(outfile, 256, "%s.memout", file);
	fd = fopen(outfile, "wb");
	fwrite(mem, 1, MINIMU_MEM_SIZE, fd);
	if (ferror(fd)) {
		fprintf(stderr, "%s: error writing file '%s': %s\n", argv[0], outfile, strerror(errno));
		exit(1);
	}
	fclose(fd);
	snprintf(outfile, 256, "%s.regout", file);
	fd = fopen(outfile, "wb");
	fwrite(mips.registers, sizeof(mips.registers[0]), 32, fd);
	if (ferror(fd)) {
		fprintf(stderr, "%s: error writing file '%s': %s\n", argv[0], outfile, strerror(errno));
		exit(1);
	}
	fclose(fd);

	return mips.cpu.status != 0 && mips.cpu.status != 1 ? 1 : 0;
}
