/**
 * Declarations for main and input functionc.
 * @file project.h
 * @author ist1117883 (Margarida Mineiro)
 */

#ifndef PROJECT_H
#define PROJECT_H

#include "commands.h"
#include "invoices.h"
#include "products.h"
#include "system_types.h"
#include "utils.h"

/**
 * Dispatches a single input line to the appropriate command handler.
 * @param sys         Pointer to the system state.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 * @param buf         Full input line.
 * @param command     The command character (first byte of buf).
 * @return 0 if the program should exit ('q'), 1 otherwise.
 */
int input(Sys* sys, int ivas_direct[], char* buf, char command);

/**
 * Initialises the system, reads commands from stdin, and cleans up on exit.
 * Accepts an optional IVA configuration file as a command-line argument.
 * @param argc Number of command-line arguments.
 * @param argv Argument vector; argv[1] may be an IVA config file path.
 * @return Always 0.
 */
int main(int argc, char* argv[]);

#endif
