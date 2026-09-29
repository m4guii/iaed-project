/**
 * Entry point and main input loop for the billing system.
 * @file project.c
 * @author ist1117883 (Margarida Mineiro)
 */

#include "project.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

/**
 * Dispatches a single input line to the appropriate command handler.
 * @param sys         Pointer to the system state.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 * @param buf         Full input line.
 * @param command     The command character (first byte of buf).
 * @return 0 if the program should exit ('q'), 1 otherwise.
 */
int input(Sys* sys, int ivas_direct[], char* buf, char command) {
	/* Switches between commands */
	switch (command) {
		case 'q':
			cleanup(sys);
			return 0;
		case 'p':
			command_p(sys, ivas_direct, buf);
			return 1;
		case 'l':
			command_l(sys, buf);
			return 1;
		case 'a':
			command_a(sys, ivas_direct, buf);
			return 1;
		case 'r':
			command_r(sys, ivas_direct, buf);
			return 1;
		case 'f':
			command_f(sys, ivas_direct, buf);
			return 1;
		case 'd':
			command_d(sys, buf);
			return 1;
		case 'c':
			command_c(sys, buf);
			return 1;
		default:
			return 1;
	}
}

/**
 * Initialises the system, reads commands from stdin, and cleans up on exit.
 * Accepts an optional IVA configuration file as a command-line argument.
 * @param argc Number of command-line arguments.
 * @param argv Argument vector; argv[1] may be an IVA config file path.
 * @return Always 0.
 */
int main(int argc, char* argv[]) {
	/* Initializing command, system and ivas */
	char buf[BUFMAX], command;
	Sys sys = system_init();
	int ivas_direct[MAX_IVA];
	for (int i = 0; i < MAX_IVA; i++) {
		ivas_direct[i] = -1;
	}
	ivas_init(argc > 1 ? argv[1] : NULL, ivas_direct);

	/* Reads line of input and processes it */
	while (read_line(buf, BUFMAX)) {
		command = buf[0];
		input(&sys, ivas_direct, buf, command);
	}

	/* Cleans up system after usage */
	cleanup(&sys);
	return 0;
}