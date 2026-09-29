/**
 * Declarations of the command handler functions for the billing system.
 * @file commands.h
 * @author ist1117883 (Margarida Mineiro)
 */

#ifndef COMMANDS_H
#define COMMANDS_H

#include "system_types.h"

/**
 * Handles the 'p' command: adds or updates a product.
 * @param sys         Pointer to the system state.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 * @param in          Full input line starting with 'p'.
 */
void command_p(Sys* sys, int* ivas_direct, char* in);

/**
 * Handles the 'l' command: lists products matching wildcard patterns.
 * @param sys Pointer to the system state.
 * @param in  Full input line starting with 'l'.
 * @return Always 0.
 */
int command_l(Sys* sys, char* in);

/**
 * Handles the 'a' command: adds a product to the cart, or lists the cart.
 * @param sys         Pointer to the system state.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 * @param in          Full input line starting with 'a'.
 * @return Always 0.
 */
int command_a(Sys* sys, int* ivas_direct, char* in);

/**
 * Handles the 'r' command: shows system totals or a single product's stats.
 * @param sys         Pointer to the system state.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 * @param in          Full input line starting with 'r'.
 * @return Always 0.
 */
int command_r(Sys* sys, int* ivas_direct, char* in);

/**
 * Handles the 'f' command: finalises the cart into an invoice.
 * @param sys         Pointer to the system state.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 * @param in          Full input line starting with 'f'.
 * @return Always 0.
 */
int command_f(Sys* sys, int* ivas_direct, char* in);

/**
 * Handles the 'c' command: lists invoices, optionally filtered by client.
 * @param sys Pointer to the system state.
 * @param in  Full input line starting with 'c'.
 * @return Always 0.
 */
int command_c(Sys* sys, char* in);

/**
 * Handles the 'd' command: deletes an invoice or reduces product stock.
 * @param sys Pointer to the system state.
 * @param in  Full input line starting with 'd'.
 * @return Always 0.
 */
int command_d(Sys* sys, char* in);

#endif