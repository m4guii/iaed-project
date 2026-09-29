/**
 * Invoice and client management: creation, lookup, and removal.
 * @file invoices.c
 * @author ist1117883 (Margarida Mineiro)
 */

#include "invoices.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "project.h"

/**
 * Searches for a client by name in the hash table.
 * @param sys  Pointer to the system state.
 * @param name Client name to search for.
 * @return Pointer to the Client if found, NULL otherwise.
 */
Client* find_client(Sys* sys, char* name) {
	/* Initializations of vars */
	unsigned int h = hash(name);
	Client* current = sys->clients[h];

	/* Finds client by using hash table generated index */
	while (current) {
		if (strcmp(current->name, name) == 0) return current;
		current = current->next;
	}
	return NULL;
}

/**
 * Allocates and registers a new client in the system.
 * Inserts the client into both the hash table and the sorted list.
 * @param sys  Pointer to the system state.
 * @param name Name of the new client.
 * @return Pointer to the newly created Client.
 */
Client* create_client(Sys* sys, char* name) {
	/* Initializations of vars */
	unsigned int h = hash(name);
	Client* new = malloc(sizeof(Client));

	/* Checks if there is memory available */
	if (!new) {
		no_memory(sys);
	}

	/* Creates new client */
	new->name = strdup(name);
	if (!new->name) {
		no_memory(sys);
	}
	new->invoices = NULL;
	new->last_invoice = NULL;
	new->next = sys->clients[h];
	sys->clients[h] = new;

	/* Inserts created client (sorted) */
	insert_client_sorted(sys, new);

	return new;
}

/**
 * Creates an invoice and appends it to the client's invoice list.
 * Creates the client if they do not yet exist.
 * Updates the system totals for items sold and value sold.
 * @param sys    Pointer to the system state.
 * @param nif    Tax identification number of the buyer.
 * @param name   Client name.
 * @param items  Number of items in the invoice.
 * @param value  Total value in cents.
 * @param number Invoice number to assign.
 */
void add_invoice(Sys* sys, int nif, char* name, int items, long long value,
				 int number) {
	/* Initializations of vars */
	Client* client = find_client(sys, name);
	Invoice* new;
	unsigned int h;

	/* Creates client who made invoice */
	if (!client) client = create_client(sys, name);

	/* Creates invoice */
	new = malloc(sizeof(Invoice));
	if (!new) {
		no_memory(sys);
	}
	new->nif = nif;
	new->client = client->name;
	new->items = items;
	new->total_payed = value;
	new->number = number;
	new->next = NULL;
	new->hash_next = NULL;
	h = new->number % HASH_SIZE;
	new->hash_next = sys->invoice_table[h];
	sys->invoice_table[h] = new;
	sys->total_items_sold += items;
	sys->total_value_sold += value;

	/* Joins invoice with its client */
	if (!client->invoices) {  // if client does not have invoices
		client->invoices = new;
		client->last_invoice = new;
	} else {  // if client has invoices already
		client->last_invoice->next = new;
		client->last_invoice = new;
	}
}

/**
 * Returns the cumulative totals for items sold and value sold.
 * @param sys          Pointer to the system state.
 * @param total_items  Output: total number of items sold.
 * @param total_value  Output: total value sold in cents.
 */
void sum_value_invoice(Sys* sys, int* total_items, long long* total_value) {
	/* Updates the values of total_items and total_value */
	*total_items = sys->total_items_sold;
	*total_value = sys->total_value_sold;
}

/**
 * Removes an invoice from the invoice hash table.
 * @param sys Pointer to the system state.
 * @param inv Pointer to the invoice to unlink.
 */
static void unlink_invoice_hash(Sys* sys, Invoice* inv) {
	/* Initializations of vars */
	unsigned int h = inv->number % HASH_SIZE;
	Invoice* prev = NULL;
	Invoice* cur = sys->invoice_table[h];

	/* Takes invoice out of hash table */
	while (cur) {
		if (cur == inv) {
			if (prev)
				prev->hash_next = cur->hash_next;
			else
				sys->invoice_table[h] = cur->hash_next;
			return;
		}
		prev = cur;
		cur = cur->hash_next;
	}
}

/**
 * Removes an invoice from a client's linked list and frees its memory.
 * Prints the invoice details before removal and updates system totals.
 * @param sys Pointer to the system state.
 * @param c   Pointer to the client that owns the invoice.
 * @param inv Pointer to the invoice to remove.
 */
static void unlink_invoice_client(Sys* sys, Client* c, Invoice* inv) {
	/* Initializations of vars */
	Invoice* prev = NULL;
	Invoice* curr = c->invoices;

	/* Removes invoice from link with its client */
	while (curr) {
		if (curr == inv) {
			printf("%.2f %d %s\n", curr->total_payed / FLOAT_CENTS_PER_EURO,
				   curr->nif, c->name);
			sys->total_items_sold -= curr->items;
			sys->total_value_sold -= curr->total_payed;
			if (prev)
				prev->next = curr->next;
			else
				c->invoices = curr->next;
			if (c->last_invoice == curr) c->last_invoice = prev;
			free(curr);
			return;
		}
		prev = curr;
		curr = curr->next;
	}
}

/**
 * Removes an invoice by its number, unlinking it from both the hash table
 * and the owning client's list.
 * @param sys Pointer to the system state.
 * @param in  String containing the invoice number to remove.
 */
void remove_invoice(Sys* sys, char* in) {
	/* Initializations of vars */
	long long num_ll = strtoll(in, NULL, 10);
	int num;
	Invoice* inv;
	Client* c;

	/* Finds if there is the invoice */
	if (num_ll <= 0 || num_ll > (long long)INT_MAX) {
		printf("%s: %s\n", in, ENO_SUCH_INVOICE);
		return;
	}
	num = (int)num_ll;
	inv = verify_invoice(sys, num);
	if (!inv) {
		printf("%d: %s\n", num, ENO_SUCH_INVOICE);
		return;
	}

	/* Removes invoice form invoice hash table */
	unlink_invoice_hash(sys, inv);

	/* Removes invoice from client */
	c = find_client(sys, inv->client);	// finds client
	unlink_invoice_client(sys, c, inv);
}