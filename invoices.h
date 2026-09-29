/**
 * Declarations for invoice and client management functions.
 * @file invoices.h
 * @author ist1117883 (Margarida Mineiro)
 */

#ifndef INVOICES_H
#define INVOICES_H

#include "system_types.h"

/**
 * Searches for a client by name in the hash table.
 * @param sys  Pointer to the system state.
 * @param name Client name to search for.
 * @return Pointer to the Client if found, NULL otherwise.
 */
Client* find_client(Sys* sys, char* name);

/**
 * Allocates and registers a new client in the system.
 * @param sys  Pointer to the system state.
 * @param name Name of the new client.
 * @return Pointer to the newly created Client.
 */
Client* create_client(Sys* sys, char* name);

/**
 * Creates an invoice and appends it to the client's invoice list.
 * @param sys    Pointer to the system state.
 * @param nif    Tax identification number of the buyer.
 * @param name   Client name.
 * @param items  Number of items in the invoice.
 * @param value  Total value in cents.
 * @param number Invoice number to assign.
 */
void add_invoice(Sys* sys, int nif, char* name, int items, long long value,
				 int number);

/**
 * Returns the cumulative totals for items sold and value sold.
 * @param sys          Pointer to the system state.
 * @param total_items  Output: total number of items sold.
 * @param total_value  Output: total value sold in cents.
 */
void sum_value_invoice(Sys* sys, int* total_items, long long* total_value);

/**
 * Removes an invoice by its number from the system.
 * @param sys Pointer to the system state.
 * @param in  String containing the invoice number to remove.
 */
void remove_invoice(Sys* sys, char* in);

#endif