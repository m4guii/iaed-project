/**
 * Utility functions: system initialisation, input parsing, hashing,
 * validation, sorting, and memory cleanup.
 * @file utils.c
 * @author ist1117883 (Margarida Mineiro)
 */

#include "utils.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "project.h"

/**
 * Populates the IVA array with the hardcoded default rates.
 * Default rates: A=0%, B=6%, C=13%, D=23%.
 * @param ivas_direct Array of size MAX_IVA to populate.
 */
static void ivas_set_defaults(int* ivas_direct) {
	/* Standard IVA values */
	ivas_direct['A' - 'A'] = IVA_A_DEFAULT;	 // 0%
	ivas_direct['B' - 'A'] = IVA_B_DEFAULT;	 // 6%
	ivas_direct['C' - 'A'] = IVA_C_DEFAULT;	 // 13%
	ivas_direct['D' - 'A'] = IVA_D_DEFAULT;	 // 23%
}

/**
 * Initialises the IVA rates array from a file, or uses default rates.
 * Default rates: A=0%, B=6%, C=13%, D=23%.
 * @param file        Path to an IVA configuration file, or NULL for defaults.
 * @param ivas_direct Array of size MAX_IVA to populate with tax rates.
 */
void ivas_init(char* file, int* ivas_direct) {
	/* Initializations of vars */
	FILE* f;
	char letter;
	int value;
	int i = 0;

	/* If there isn't a file -> puts default values */
	if (!file) {
		ivas_set_defaults(ivas_direct);
		return;
	}

	/* Opens file and reads it */
	f = fopen(file, "r");
	if (!f) {  // if file is empty
		ivas_set_defaults(ivas_direct);
		return;
	}

	/* Keeps values from file in IVAs array */
	while (fscanf(f, " %c %d", &letter, &value) == 2 && i < MAX_IVA) {
		ivas_direct[letter - 'A'] = value;
		i++;
	}

	/* Closes file */
	fclose(f);
}

/**
 * Sets all scalar fields of the system state to their initial values.
 * @param sys Pointer to the system state to initialise.
 */
static void init_sys_fields(Sys* sys) {
	/* Initializations of vars */
	sys->products = NULL;
	sys->cart = NULL;
	sys->client_list = NULL;
	sys->num_products = 0;
	sys->max_products = MAX_PRODUCTS;
	sys->num_cart = 0;
	sys->max_cart = INITIAL_CAPACITY;
	sys->next_invoice = 1;
	sys->num_invoices = 0;
	sys->max_invoices = INITIAL_CAPACITY;
	sys->total_items_sold = 0;
	sys->total_value_sold = 0;
	sys->num_clients = 0;
	sys->max_clients = INITIAL_CAPACITY;
}

/**
 * Clears all three hash tables in the system state to NULL.
 * @param sys Pointer to the system state.
 */
static void init_hash_tables(Sys* sys) {
	/* Initializes all hash tables at once */
	for (int i = 0; i < HASH_SIZE; i++) {
		sys->product_table[i] = NULL;
		sys->invoice_table[i] = NULL;
		sys->clients[i] = NULL;
	}
}

/**
 * Allocates the products, cart, and client_list arrays and zeroes them.
 * Calls no_memory and exits if any allocation fails.
 * @param sys Pointer to the system state.
 */
static void alloc_sys_arrays(Sys* sys) {
	/* Allocs memory for all arrays and initializes them */
	sys->products = malloc(sys->max_products * sizeof(Product*));
	sys->cart = malloc(sys->max_cart * sizeof(Product_in_cart));
	sys->client_list = malloc(sys->max_clients * sizeof(Client*));
	if (!sys->products || !sys->cart || !sys->client_list) no_memory(sys);
	for (int i = 0; i < sys->max_products; i++) sys->products[i] = NULL;
	for (int i = 0; i < sys->max_clients; i++) sys->client_list[i] = NULL;
}

/**
 * Allocates and zero-initialises the system state, including the products
 * array, cart, and client list.
 * @return A fully initialised Sys struct ready for use.
 */
Sys system_init(void) {
	/* Initializations of vars and full system */
	Sys sys;
	init_sys_fields(&sys);
	init_hash_tables(&sys);
	alloc_sys_arrays(&sys);
	return sys;
}

/**
 * Reads one non-empty line from stdin into buf, stripping the newline.
 * Skips blank lines by recursing.
 * @param buf   Buffer to read into.
 * @param bufsz Size of the buffer in bytes.
 * @return 1 if a line was read, 0 on EOF.
 */
int read_line(char* buf, int bufsz) {
	/* Initializations of vars */
	char* p;

	/* Reads lines of input */
	if (!fgets(buf, bufsz, stdin)) return 0;
	buf[strcspn(buf, "\n")] = '\0';
	p = strchr(buf, '\r');
	if (p) *p = '\0';
	if (buf[0] == '\0') return read_line(buf, bufsz);

	return 1;
}

/**
 * Computes a polynomial hash of a null-terminated string.
 * @param str The string to hash.
 * @return An index in [0, HASH_SIZE).
 */
unsigned int hash(char* str) {
	/* Initializations of vars */
	unsigned int h = 0;

	/* Logic to create hash indexs */
	while (*str) h = h * 31 + *str++;
	return h % HASH_SIZE;
}

/**
 * Extracts a client name, handling optional double-quote delimiters.
 * Modifies the input string in place by replacing the closing quote with '\0'.
 * @param name Pointer to the name string (may start with '"').
 * @return Pointer to the name content, or NULL if the name is empty/invalid.
 */
char* parse_name(char* name) {
	/* Initializations of vars */
	char* start;
	char* end;

	/* Parses name between "" */
	if (name[0] == '"') {
		start = name + 1;
		end = strrchr(start, '"');	// returns pointer to the last occ of "

		if (!end) return NULL;	// in case there is no final "

		*end = '\0';  // closes string (full name inside "")
		if (strlen(start) == 0) return NULL;
		return start;
	}

	if (strlen(name) == 0) return NULL;
	return name;
}

/**
 * Matches a wildcard pattern against a text string.
 * Supports '*' (any sequence) and '?' (any single character).
 * @param pattern The wildcard pattern.
 * @param text    The string to match against.
 * @return 1 if the text matches the pattern, 0 otherwise.
 */
int match_wildcards(const char* pattern, const char* text) {
	/* Initializations of vars */
	const char* star = NULL;
	const char* match = text;

	/* Finds matches for each wildcard */
	while (*text) {
		if (*pattern == WILDCARD_1) {  // *
			star = pattern++;
			match = text;
		} else if (*pattern == WILDCARD_2 || *pattern == *text) {  // ?
			pattern++;
			text++;
		} else if (star) {
			pattern = star + 1;
			text = ++match;
		} else {
			return 0;
		}
	}
	while (*pattern == WILDCARD_1) pattern++;
	return *pattern == '\0';
}

/**
 * Inserts a client into the sorted client_list array, expanding it if needed.
 * Maintains alphabetical order by client name.
 * @param sys Pointer to the system state.
 * @param new Pointer to the client to insert.
 */
void insert_client_sorted(Sys* sys, Client* new) {
	/* Initializations of vars */
	Client** temp;
	int lo = 0, hi = sys->num_clients, mid;

	/* Increases size of memory vailable for clients in case it is full */
	if (sys->num_clients == sys->max_clients) {
		sys->max_clients *= 2;
		temp = realloc(sys->client_list, sys->max_clients * sizeof(Client*));
		if (!temp) {  // if no memory is left
			no_memory(sys);
		}
		sys->client_list = temp;
	}

	/* Finds place where to insert client */
	while (lo < hi) {
		mid = (lo + hi) / 2;
		if (strcmp(sys->client_list[mid]->name, new->name) < 0)
			lo = mid + 1;
		else
			hi = mid;
	}

	/* Inserts it in place */
	for (int i = sys->num_clients; i > lo; i--)
		sys->client_list[i] = sys->client_list[i - 1];
	sys->client_list[lo] = new;
	sys->num_clients++;
}

/**
 * Validates an EAN-8 or EAN-13 barcode string using the check-digit algorithm.
 * @param ean Null-terminated EAN string.
 * @return 0 if valid, ERROR otherwise.
 */
int verify_ean(char* ean) {
	/* Initializations of vars */
	int sum, final_dig, length, dig;
	sum = final_dig = 0;

	if (!ean) return ERROR;	 // NULL EAN

	/* Check if it is either 8/13 sized */
	length = strlen(ean);
	if (length != EAN8_SIZE && length != EAN13_SIZE) return ERROR;

	for (int i = 0; i < length - 1; i++) {
		if (!isdigit(ean[i])) return ERROR;	 // all digits

		/* Sum for final digit */
		dig = ean[i] - '0';
		if (i % 2 == 0)
			sum += dig;
		else
			sum += dig * EAN_ODD_MULTIPLIER;
	}

	if (!isdigit(ean[length - 1])) return ERROR;  // if last one is a digit

	/* Checks if it has the correct final digit */
	final_dig = (EAN_MODULUS - (sum % EAN_MODULUS)) % EAN_MODULUS;
	if (final_dig != (ean[length - 1] - '0')) return ERROR;

	return 0;
}

/**
 * Checks that an IVA letter is uppercase and has a defined rate.
 * @param iva         IVA category letter to validate.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 * @return 0 if valid, ERROR otherwise.
 */
int verify_iva(char iva, int* ivas_direct) {
	/* Checks if char is uppercase and a letter */
	if (!isupper((unsigned char)iva)) return ERROR;

	/* Checks if assigned value is valid */
	if (ivas_direct[iva - 'A'] < 0) return ERROR;

	return 0;
}

/**
 * Checks that a price is strictly positive.
 * @param price Price in cents.
 * @return 0 if valid, ERROR otherwise.
 */
int verify_price(long long price) { return price > 0 ? 0 : ERROR; }

/**
 * Checks that a stock quantity is non-negative.
 * @param stock Stock level to validate.
 * @return 0 if valid, ERROR otherwise.
 */
int verify_stock(int stock) {
	/* Chcecks if stock is zero or positive */
	if (stock < 0) return ERROR;
	return 0;
}

/**
 * Checks that a description is non-empty, starts with an uppercase or
 * accented character, and does not exceed MAX_DESCRIPTION characters.
 * @param description Null-terminated description string.
 * @return 0 if valid, ERROR otherwise.
 */
int verify_description(char* description) {
	/* Initializations of vars */
	unsigned char c;

	if (!description) return ERROR;	 // if it's empty / NULL

	c = description[0];										   // first letter
	if ((c >= 'A' && c <= 'Z') || c >= MIN_ACCENTED_CHAR) {	   // if Uppercase
		if (strlen(description) <= MAX_DESCRIPTION) return 0;  // if in size
	}
	return ERROR;
}

/**
 * Checks that the system has not reached the maximum number of products.
 * @param sys Pointer to the system state.
 * @return 0 if space remains, ERROR otherwise.
 */
int verify_system(Sys* sys) {
	/* Check if system is not saturated */
	if (sys->num_products >= MAX_PRODUCTS) return ERROR;
	return 0;
}

/**
 * Checks that a name string is non-empty and starts with an alphabetic
 * or extended-Latin character.
 * @param name Null-terminated name string.
 * @return 0 if valid, ERROR otherwise.
 */
int verify_name(char* name) {
	/* Initializations of vars */
	unsigned char c;

	if (!name || !name[0]) return ERROR;		   // if it is not NULL / empty
	c = (unsigned char)name[0];					   // first letter
	return (isalpha(c) || c >= 0xC0) ? 0 : ERROR;  // checks if it is valid
}

/**
 * Checks that a NIF is within the valid Portuguese NIF range.
 * @param nif NIF value to validate.
 * @return 0 if valid, ERROR otherwise.
 */
int verify_nif(int nif) {
	if (nif < MIN_NIF || nif > MAX_NIF) {  // between 100000000 and 999999999
		return ERROR;
	}
	return 0;
}

/**
 * Looks up an invoice by number in the invoice hash table.
 * @param sys    Pointer to the system state.
 * @param numero Invoice number to find.
 * @return Pointer to the Invoice if found, NULL otherwise.
 */
Invoice* verify_invoice(Sys* sys, int number_inv) {
	/* Initializations of vars */
	unsigned int h = number_inv % HASH_SIZE;
	Invoice* inv = sys->invoice_table[h];

	/* Checks if invoice exists from its number */
	while (inv) {
		if (inv->number == number_inv) return inv;
		inv = inv->hash_next;
	}
	return NULL;
}

/**
 * Sorts the products array in-place by creation order using quicksort.
 * @param sys Pointer to the system state.
 * @param lo  Lower bound index (inclusive).
 * @param hi  Upper bound index (inclusive).
 */
void quicksort_products(Sys* sys, int lo, int hi) {
	/* Initializations of vars */
	Product *pivot, *tmp;
	int i, p;

	/* Quick SOrts */
	if (lo >= hi) return;
	pivot = sys->products[hi];
	i = lo - 1;
	for (int j = lo; j < hi; j++) {
		if (sys->products[j]->creation_order <= pivot->creation_order) {
			i++;
			tmp = sys->products[i];
			sys->products[i] = sys->products[j];
			sys->products[j] = tmp;
			sys->products[i]->array_index = i;
			sys->products[j]->array_index = j;
		}
	}
	tmp = sys->products[i + 1];
	sys->products[i + 1] = sys->products[hi];
	sys->products[hi] = tmp;
	sys->products[i + 1]->array_index = i + 1;
	sys->products[hi]->array_index = hi;
	p = i + 1;
	quicksort_products(sys, lo, p - 1);
	quicksort_products(sys, p + 1, hi);
}

/**
 * Frees all client and invoice memory in the system.
 * @param sys Pointer to the system state.
 */
static void cleanup_clients(Sys* sys) {
	/* Initializations of vars */
	Client *c, *next;
	Invoice *inv, *n;

	/* Goes through each client and frees it all */
	for (int i = 0; i < HASH_SIZE; i++) {
		c = sys->clients[i];
		while (c) {
			next = c->next;
			inv = c->invoices;
			while (inv) {
				n = inv->next;
				free(inv);
				inv = n;
			}
			free(c->name);
			free(c);
			c = next;
		}
		sys->clients[i] = NULL;
	}
}

/**
 * Frees all product memory and clears the product hash table.
 * @param sys Pointer to the system state.
 */
static void cleanup_products(Sys* sys) {
	/* Goes through all products and frees them */
	for (int i = 0; i < sys->num_products; i++) {
		if (sys->products[i]) {
			free(sys->products[i]->description);
			free(sys->products[i]);
			sys->products[i] = NULL;
		}
	}
	for (int i = 0; i < HASH_SIZE; i++) sys->product_table[i] = NULL;
}

/**
 * Releases all heap memory held by the system and resets counters to zero.
 * @param sys Pointer to the system state.
 */
void cleanup(Sys* sys) {
	/* CLEANS EVERYTHING */
	fflush(stdout);
	cleanup_clients(sys);
	cleanup_products(sys);
	free(sys->products);
	free(sys->client_list);
	free(sys->cart);
	sys->products = NULL;
	sys->client_list = NULL;
	sys->cart = NULL;
	sys->num_products = sys->num_clients = sys->num_cart = 0;
}

/**
 * Prints an out-of-memory error, cleans up, and exits.
 * @param sys Pointer to the system state.
 */
void no_memory(Sys* sys) {
	/* Cleans system after No Memory Error*/
	puts(ENO_MEMORY);
	cleanup(sys);
	exit(0);
}