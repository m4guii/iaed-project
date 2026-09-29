/**
 * Command handlers for the billing system.
 * Each function corresponds to a user-facing command (p, l, a, r, f, c, d).
 * @file commands.c
 * @author ist1117883 (Margarida Mineiro)
 */

#include "commands.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "project.h"

/**
 * Checks if a string consists only of decimal digits.
 * @param s String to check.
 * @return 1 if numeric, 0 otherwise.
 */
static int is_numeric(const char* s) {
	/* Goes through each char and checks if it is a number */
	for (int i = 0; s[i]; i++) {
		if (!isdigit((unsigned char)s[i])) return 0;
	}
	return 1;
}

/**
 * Validates a NIF followed by a client name in the 'f' command arguments.
 * Prints an error if the NIF is invalid.
 * @param first_word The first token (expected to be the NIF).
 * @param after      The remaining string after the first token (client name).
 * @param nif        Output: parsed NIF value.
 * @param client     Output: heap-allocated client name string.
 * @return 0 on success, -1 if the NIF is invalid.
 */
static int parse_f_nif_and_name(char* first_word, char* after, int* nif,
								char** client) {
	/* Initializations of vars */
	char* name_res;
	int maybe_nif = strtol(first_word, NULL, 10);

	/* Chceks if it is all digits and if len = 9 -> verifies nif */
	if (!is_numeric(first_word) || strlen(first_word) != 9 ||
		verify_nif(maybe_nif) == ERROR) {
		printf("%s: %s\n", first_word, ENO_SUCH_NIF);
		return -1;
	}

	/* Separates nif from name */
	*nif = maybe_nif;
	name_res = parse_name(after);
	*client = strdup(name_res ? name_res : CLIENT_STANDARD);
	return 0;
}

/**
 * Parses a single token that is either a NIF-only or a client name.
 * @param first_word The single token to interpret.
 * @param p          Pointer to the start of the token in the original string.
 * @param nif        Output: parsed NIF if token is a valid NIF.
 * @param client     Output: heap-allocated client name string.
 * @return 0 on success, -2 if the name is invalid.
 */
static int parse_f_single_word(char* first_word, char* p, int* nif,
							   char** client) {
	/* Initializations of vars */
	char* name_res;
	int maybe_nif;

	/* Chooses name and nif */
	if (is_numeric(first_word) && strlen(first_word) == 9) {
		maybe_nif = strtol(first_word, NULL, 10);
		if (verify_nif(maybe_nif) != ERROR) {
			*nif = maybe_nif;
			*client = strdup(CLIENT_STANDARD);
			return 0;
		}
	}
	name_res = parse_name(p);
	if (!name_res) return -2;
	*client = strdup(name_res);
	return 0;
}

/**
 * Parses the arguments of the 'f' command (invoice finalisation).
 * Supports an optional NIF and an optional client name.
 * @param p      Pointer to the argument string (after the command letter).
 * @param nif    Output: parsed NIF, or MAX_NIF if none provided.
 * @param client Output: heap-allocated client name string.
 * @return 0 on success, -1 on invalid NIF, -2 on invalid name.
 */
static int parse_f_args(char* p, int* nif, char** client) {
	/* Initializations of vars */
	char first_word[BUFMAX];
	int i = 0;
	char *start, *res;

	/* Parses f arguments -> nif and name (using previous functions) */
	if (p)
		while (isspace((unsigned char)*p)) p++;
	if (!p || *p == '\0') {
		*client = strdup(CLIENT_STANDARD);
		return 0;
	}
	start = p;

	if (*p == '"') {
		res = parse_name(p);
		if (!res) return -2;
		*client = strdup(res);
		return 0;
	}

	while (p[i] != '\0' && !isspace((unsigned char)p[i])) {
		first_word[i] = p[i];
		i++;
	}
	first_word[i] = '\0';
	p += i;
	while (isspace((unsigned char)*p)) p++;

	if (*p != '\0') return parse_f_nif_and_name(first_word, p, nif, client);
	return parse_f_single_word(first_word, start, nif, client);
}

/**
 * Validates the arguments for the 'p' (product) command.
 * @param ean         EAN string to validate.
 * @param iva         IVA category letter.
 * @param price_double Price as a double (before conversion to cents).
 * @param stock       Stock quantity.
 * @param desc        Product description string.
 * @param ivas_direct Array mapping IVA letters to rates.
 * @return 0 if all arguments are valid, ERROR otherwise.
 */
static int validate_p_args(char* ean, char iva, long long price, int stock,
						   char* desc, int* ivas_direct) {
	/* Validates all arguments of p (ean, iva, price, stock and description)*/
	if (verify_ean(ean) == ERROR) {
		puts(EINVALID_EAN);
		return ERROR;
	}
	if (verify_iva(iva, ivas_direct) == ERROR) {
		puts(EINVALID_IVA);
		return ERROR;
	}
	if (price <= 0) {
		puts(EINVALID_PRICE);
		return ERROR;
	}
	if (verify_stock(stock) == ERROR) {
		puts(EINVALID_QUANTITY);
		return ERROR;
	}
	if (verify_description(desc) == ERROR) {
		puts(EINVALID_DESCRIPTION);
		return ERROR;
	}
	return 0;
}

/**
 * Parses the 'p' command input into its fields.
 * @param in          Full input line starting with 'p'.
 * @param ean         Output: EAN string.
 * @param iva         Output: IVA category letter.
 * @param price_double Output: price as double.
 * @param stock       Output: stock quantity.
 * @param description Output: product description.
 * @return Number of fields parsed (5 on success).
 */
static int parse_p_input(char* in, char* ean, char* iva, double* price_double,
						 int* stock, char* description) {
	/* Initializations of vars */
	char price_str[BUFMAX] = "";
	int n = sscanf(in, "p %s %c %s %d %[^\n]", ean, iva, price_str, stock,
				   description);

	/* Parses p input -> 5 arguments */
	if (n == 5) *price_double = atof(price_str);
	return n;
}

/**
 * Handles the 'p' command: adds or updates a product.
 * Parses EAN, IVA, price, stock, and description from the input line.
 * @param sys         Pointer to the system state.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 * @param in          Full input line starting with 'p'.
 */
void command_p(Sys* sys, int* ivas_direct, char* in) {
	/* Initializations of vars */
	char ean[EAN_SIZE + 1] = "";
	char iva = '\0';
	double price_double = 0;
	int stock = -1;
	char description[BUFMAX] = "";
	long long price;
	Product* product;

	/* Checks description */
	if (parse_p_input(in, ean, &iva, &price_double, &stock, description) < 5) {
		puts(EINVALID_DESCRIPTION);
		return;
	}
	/* Checks arguments */
	price = (long long)(price_double * CENTS_PER_EURO + 0.5);
	if (validate_p_args(ean, iva, price, stock, description, ivas_direct) ==
		ERROR)
		return;

	/* Checks if product is in use */
	product = find_product(sys, ean);
	if (product && product->in_cart && price != product->price) {
		puts(EPRODUCT_IN_USE);
		return;
	}
	/* Checks if it is a valid product (non NULL and system is good to go)*/
	if (!product && verify_system(sys) == ERROR) {
		puts(EINVALID_PRODUCT);
		return;
	}

	/* Updates / Creates product in system */
	if (product)
		update_product(sys, product, iva, price, stock, description);
	else
		add_product(sys, ean, iva, price, stock, description);
}

/**
 * Handles the 'l' command: lists products matching wildcard patterns.
 * If no pattern is given, lists all products with stock > 0.
 * @param sys Pointer to the system state.
 * @param in  Full input line starting with 'l'.
 * @return Always 0.
 */
int command_l(Sys* sys, char* in) {
	/* Initializations of vars */
	char* wildcards;

	/* Takes wildcards */
	strtok(in, " \n");
	wildcards = strtok(NULL, "\n");

	/* Checks if there are wildcards */
	if (!wildcards || strlen(wildcards) == 0) wildcards = "*";

	/* Lists wildcards */
	list_wildcards(sys, wildcards);
	return 0;
}

/**
 * Handles the 'a' command: adds a product to the cart, or lists the cart.
 * If no argument is given, the current cart contents are printed.
 * @param sys         Pointer to the system state.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 * @param in          Full input line starting with 'a'.
 * @return Always 0.
 */
int command_a(Sys* sys, int* ivas_direct, char* in) {
	/* Initializations of vars */
	char* pending;

	/* Takes arguments */
	strtok(in, " \n");
	pending = strtok(NULL, "\n");

	/* In case there is no arguments */
	if (!pending || strlen(pending) == 0) {
		list_cart(sys, ivas_direct);  // lists cart
		return 0;
	}
	/* Adds to cart */
	add_to_cart(sys, ivas_direct, pending);
	return 0;
}

/**
 * Prints global totals: items sold, invoice count, total value, IVA rates.
 * @param sys         Pointer to the system state.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 */
static void print_r_totals(Sys* sys, int* ivas_direct) {
	/* Initializations of vars */
	long long total_value = 0;
	int items = 0;

	/* Calculates sum */
	sum_value_invoice(sys, &items, &total_value);
	/* Prints output */
	printf("%d %d %.2f\n", items, sys->next_invoice - 1,
		   sys->total_value_sold / FLOAT_CENTS_PER_EURO);
	/* Prints IVA table */
	for (int i = 0; i < MAX_IVA; i++) {
		if (ivas_direct[i] >= 0) printf("%c %d%%\n", 'A' + i, ivas_direct[i]);
	}
}

/**
 * Prints stock, units sold, and description for a single product.
 * @param sys Pointer to the system state.
 * @param ean EAN string of the product to look up.
 */
static void print_r_product(Sys* sys, char* ean) {
	/* Initializations of vars */
	Product* product;

	/* Checks EAN */
	if (verify_ean(ean) == ERROR) {
		puts(EINVALID_EAN);
		return;
	}

	/* Finds product */
	product = find_product(sys, ean);
	if (product == NULL) {
		printf("%s: %s\n", ean, ENO_SUCH_PRODUCT);	// if tehre's no product
		return;
	}
	/* Prints output */
	printf("%d %d %s\n", product->stock, product->sold, product->description);
}

/**
 * Handles the 'r' command: shows system totals or a single product's stats.
 * Without an EAN, prints total items sold, invoice count, and IVA rates.
 * With an EAN, prints the product's stock, units sold, and description.
 * @param sys         Pointer to the system state.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 * @param in          Full input line starting with 'r'.
 * @return Always 0.
 */
int command_r(Sys* sys, int* ivas_direct, char* in) {
	/* Initializations of vars */
	char* ean;

	/* Takes arguments */
	strtok(in, " \n");
	ean = strtok(NULL, "\n");

	if (ean == NULL)  // no EAN
		print_r_totals(sys, ivas_direct);
	else  // with EAN
		print_r_product(sys, ean);
	return 0;
}

/**
 * Validates the client argument of the 'f' command.
 * Prints an error if the name is invalid.
 * @param client Heap-allocated client name string to validate.
 * @return 0 on success, -1 if invalid (client is freed on error).
 */
static int validate_f_client(char* client) {
	/* Checks name of client */
	if (client == NULL || verify_name(client) == ERROR) {
		puts(EINVALID_NAME);
		free(client);
		return -1;
	}
	return 0;
}

/**
 * Creates an invoice from the current cart and prints the result.
 * @param sys         Pointer to the system state.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 * @param nif         Client NIF.
 * @param client      Heap-allocated client name (freed by this function).
 */
static void finalise_invoice(Sys* sys, int* ivas_direct, int nif,
							 char* client) {
	/* Initializations of vars */
	int items = 0;
	long long value = 0;

	/* Calculate cart */
	calculate_cart(sys, ivas_direct, &items, &value);
	/* Create invoice */
	add_invoice(sys, nif, client, items, value, sys->next_invoice);
	sys->next_invoice++;
	sys->num_invoices++;
	/* Prints output */
	printf("%d %.2f %d\n", items, value / FLOAT_CENTS_PER_EURO,
		   sys->next_invoice - 1);
	/* Clears cart for new invoice */
	clear_cart(sys);
	free(client);
}

/**
 * Handles the 'f' command: finalises the cart into an invoice.
 * Accepts an optional NIF and client name. The special name "error"
 * triggers a cart rollback instead of invoice creation.
 * @param sys         Pointer to the system state.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 * @param in          Full input line starting with 'f'.
 * @return Always 0.
 */
int command_f(Sys* sys, int* ivas_direct, char* in) {
	/* Initializations of vars */
	int nif = MAX_NIF;
	char* client = NULL;
	int res = parse_f_args(in + 1, &nif, &client);

	/* Checks how many arguments and if they are valid */
	if (res == -1) return 0;
	if (res == -2) {
		puts(EINVALID_NAME);
		return 0;
	}
	if (validate_f_client(client) == -1) return 0;

	/* "f error" -> clears */
	if (strcmp(client, ERROR_CLIENT) == 0) {
		rollback_and_clear(sys);
		free(client);
		return 0;
	}
	/* Finalises invoice */
	finalise_invoice(sys, ivas_direct, nif, client);
	return 0;
}

/**
 * Prints all invoices belonging to a single client.
 * @param c Pointer to the client whose invoices are printed.
 */
static void print_c_client(Client* c) {
	/* Initializations of vars */
	Invoice* inv = c->invoices;

	/* Prints invoices from client output */
	while (inv) {
		printf("%d %.2f %s\n", inv->number,
			   inv->total_payed / FLOAT_CENTS_PER_EURO, c->name);
		inv = inv->next;
	}
}

/**
 * Lists all invoices across all clients in alphabetical order.
 * @param sys Pointer to the system state.
 */
static void list_c_all(Sys* sys) {
	/* Lists all invoices of all clients */
	for (int i = 0; i < sys->num_clients; i++)
		print_c_client(sys->client_list[i]);
}

/**
 * Lists all invoices for a specific named client.
 * @param sys    Pointer to the system state.
 * @param client Raw (possibly quoted) client name string.
 */
static void list_c_named(Sys* sys, char* client) {
	/* Initializations of vars */
	Client* c;

	/* Lists invoices of segregated clients */
	client = parse_name(client);
	if (!client || verify_name(client) == ERROR) {
		puts(EINVALID_NAME);  // in case name is invalid
		return;
	}
	/* Finds "special" clients */
	c = find_client(sys, client);
	if (!c) {
		printf("%s: %s\n", client, ENO_SUCH_CLIENT);  // if they don't exist
		return;
	}
	/* Does the job */
	print_c_client(c);
}

/**
 * Handles the 'c' command: lists invoices, optionally filtered by client.
 * Without an argument, lists all invoices sorted by client name.
 * @param sys Pointer to the system state.
 * @param in  Full input line starting with 'c'.
 * @return Always 0.
 */
int command_c(Sys* sys, char* in) {
	/* Initializations of vars */
	char* client;

	/* tTakes arguments */
	strtok(in, " \n");
	client = strtok(NULL, "\n");

	if (client == NULL)	 // list all
		list_c_all(sys);
	else  // list some
		list_c_named(sys, client);
	return 0;
}

/**
 * Handles the 'd' command: deletes an invoice or reduces product stock.
 * With one argument: removes the invoice with that number.
 * With two arguments: removes the given quantity from the product's stock.
 * @param sys Pointer to the system state.
 * @param in  Full input line starting with 'd'.
 * @return Always 0.
 */
int command_d(Sys* sys, char* in) {
	/* Initializations of vars */
	char *arg1, *arg2;

	/* Takes arguments */
	strtok(in, " \n");
	arg1 = strtok(NULL, " \n");
	arg2 = strtok(NULL, " \n");

	if (arg1 == NULL) return 0;	 // "d" only
	if (arg2 == NULL)			 // removes invoice with humber arg1
		remove_invoice(sys, arg1);
	else  // removes quantity (arg2) from product with (arg1) ean
		remove_product(sys, arg1, arg2);
	return 0;
}