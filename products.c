/**
 * Product and cart management: lookup, insertion, listing, and removal.
 * @file products.c
 * @author ist1117883 (Margarida Mineiro)
 */

#include "products.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "project.h"

/**
 * Searches for a product by EAN in the product hash table.
 * @param sys Pointer to the system state.
 * @param ean EAN string to look up.
 * @return Pointer to the Product if found, NULL otherwise.
 */
Product* find_product(Sys* sys, char* ean) {
	/* Initializations of vars */
	unsigned int h = hash(ean);
	Product* p = sys->product_table[h];

	/* Finds products through hash index*/
	while (p) {
		if (strcmp(p->ean, ean) == 0) return p;
		p = p->hash_next;
	}
	return NULL;
}

/**
 * Searches for a product entry in the sorted cart array using binary search.
 * @param sys     Pointer to the system state.
 * @param product The product whose EAN is used as the search key.
 * @return Pointer to the Product_in_cart entry if found, NULL otherwise.
 */
Product_in_cart* find_in_cart(Sys* sys, Product product) {
	/* Initializations of vars */
	char* ean = product.ean;
	int lo = 0, hi = sys->num_cart - 1;
	int mid, cmp;

	/* Finds product by EAN in cart (cart is sorted already, so kind of bst) */
	while (lo <= hi) {
		mid = (lo + hi) / 2;
		cmp = strcmp(sys->cart[mid].product->ean, ean);
		if (cmp == 0) return &sys->cart[mid];
		if (cmp < 0)
			lo = mid + 1;
		else
			hi = mid - 1;
	}
	return NULL;
}

/**
 * Allocates a new product and registers it in the system.
 * Prints the resulting stock on success.
 * @param sys         Pointer to the system state.
 * @param ean         EAN string for the product.
 * @param iva         IVA category letter.
 * @param price       Price in cents.
 * @param stock       Initial stock quantity.
 * @param description Product description string.
 * @return The product's stock after insertion.
 */
int add_product(Sys* sys, char* ean, char iva, long long price, int stock,
				char* description) {
	/* Initializations of vars */
	Product* product = malloc(sizeof(Product));
	unsigned int h;

	/* Check if it is possible to create product in terms of memory */
	if (!product) {
		no_memory(sys);
	}

	/* Creates it */
	strcpy(product->ean, ean);
	product->iva = iva;
	product->price = price;
	product->stock = stock;
	product->description = strdup(description);
	if (!product->description) {
		no_memory(sys);
	}
	product->sold = 0;
	product->hash_next = NULL;
	product->creation_order = sys->num_products;
	product->in_cart = 0;
	product->array_index = sys->num_products;
	sys->products[sys->num_products++] = product;

	/* Puts it in correct place (hash table index) */
	h = hash(ean);
	product->hash_next = sys->product_table[h];
	sys->product_table[h] = product;

	/* Prints output */
	printf("%d\n", product->stock);
	return product->stock;
}

/**
 * Updates the fields of an existing product, adding stock to the current value.
 * Prints the resulting stock on success.
 * @param sys         Pointer to the system state.
 * @param product     Pointer to the product to update.
 * @param iva         New IVA category letter.
 * @param price       New price in cents.
 * @param stock       Quantity to add to existing stock.
 * @param description New description string.
 * @return The product's stock after the update.
 */
int update_product(Sys* sys, Product* product, char iva, long long price,
				   int stock, char* description) {
	/* Updates values */
	product->iva = iva;
	product->price = price;
	product->stock += stock;
	free(product->description);
	product->description = strdup(description);
	if (!product->description) {  // checks if there is memory for description
		no_memory(sys);
	}

	/* Prints output */
	printf("%d\n", product->stock);
	return product->stock;
}

/**
 * Inserts a product into the sorted cart array, expanding it if necessary.
 * @param sys      Pointer to the system state.
 * @param product  Pointer to the product to add.
 * @param ean      EAN used as the sort key.
 * @param quantity Number of units to add to the cart.
 */
static void insert_into_cart(Sys* sys, Product* product, char* ean,
							 int quantity) {
	/* Initializations of vars */
	int lo = 0, hi = sys->num_cart, mid;
	Product_in_cart* temp;

	/* Increases size of memory vailable for cart in case it is full */
	if (sys->num_cart == sys->max_cart) {
		sys->max_cart *= 2;
		temp = realloc(sys->cart, sys->max_cart * sizeof(Product_in_cart));
		if (!temp) {
			no_memory(sys);
		}
		sys->cart = temp;
	}

	/* Finds place where to insert in cart */
	while (lo < hi) {
		mid = (lo + hi) / 2;
		if (strcmp(sys->cart[mid].product->ean, ean) < 0)
			lo = mid + 1;
		else
			hi = mid;
	}

	/* Inserts it in place */
	for (int i = sys->num_cart; i > lo; i--) sys->cart[i] = sys->cart[i - 1];
	sys->cart[lo].product = product;
	sys->cart[lo].quantity = quantity;
	sys->num_cart++;
	product->in_cart = 1;
}

/**
 * Parses the quantity and EAN from the 'a' command argument string.
 * If only one token is given, quantity defaults to 1.
 * @param pending  Argument string to parse.
 * @param quantity Output: number of units requested.
 * @param ean      Output: pointer to the EAN token within pending.
 */
static void parse_cart_args(char* pending, int* quantity, char** ean) {
	/* Initializations of vars */
	char* arg1 = strtok(pending, " \n");
	char* arg2 = strtok(NULL, " \n");

	if (arg2 == NULL) {	 // if only has one argument
		*quantity = 1;
		*ean = arg1;
	} else {  // if it has two arguments
		*quantity = strtol(arg1, NULL, 10);
		*ean = arg2;
	}
}

/**
 * Validates the EAN and looks up the product in the system.
 * @param ean     EAN string to validate and look up.
 * @param sys     Pointer to the system state.
 * @param product Output: pointer to the found product.
 * @return 0 if valid and found, ERROR otherwise.
 */
static int validate_cart_ean(char* ean, Sys* sys, Product** product) {
	/* Checks EAN */
	if (verify_ean(ean) == ERROR) {
		puts(EINVALID_EAN);
		return ERROR;
	}

	/* Finds product */
	*product = find_product(sys, ean);
	if (!*product) {
		printf("%s: %s\n", ean, ENO_SUCH_PRODUCT);	// if there is no product
		return ERROR;
	}
	return 0;
}

/**
 * Updates the quantity of a product already in the cart.
 * Checks stock and quantity constraints before modifying.
 * @param product  Pointer to the product.
 * @param item     Pointer to the cart entry to update.
 * @param quantity Quantity delta (positive or negative).
 * @return 0 on success, ERROR if constraints are violated.
 */
static int update_existing_cart(Product* product, Product_in_cart* item,
								int quantity) {
	/* Checks if there is stick available */
	if (quantity > 0 && verify_stock(product->stock - quantity) == ERROR) {
		puts(ENO_STOCK);
		return ERROR;
	}

	/* Checks if inserted quantity is valid */
	if (quantity < 0 && -quantity > item->quantity) {
		puts(EINVALID_QUANTITY);
		return ERROR;
	}

	/* Updates cart */
	item->quantity += quantity;
	return 0;
}

/**
 * Inserts a new product into the cart after validating quantity and stock.
 * @param sys      Pointer to the system state.
 * @param product  Pointer to the product to add.
 * @param ean      EAN string used as cart sort key.
 * @param quantity Number of units to add.
 * @param item     Output: pointer to the newly created cart entry.
 * @return 0 on success, ERROR if constraints are violated.
 */
static int insert_new_cart(Sys* sys, Product* product, char* ean, int quantity,
						   Product_in_cart** item) {
	/* Checks if inserted quantity is valid */
	if (quantity <= 0) {
		puts(EINVALID_QUANTITY);
		return ERROR;
	}

	/* Checks if there is stock available */
	if (verify_stock(product->stock - quantity) == ERROR) {
		puts(ENO_STOCK);
		return ERROR;
	}

	/* Inserts new product to cart */
	insert_into_cart(sys, product, ean, quantity);
	*item = find_in_cart(sys, *product);
	return 0;
}

/**
 * Prints a single cart line: IVA, price, quantity, total, description.
 * @param product     Pointer to the product.
 * @param item        Pointer to the cart entry.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 */
static void print_cart_line(Product* product, Product_in_cart* item,
							int* ivas_direct) {
	/* Initializations of vars */
	int rate = ivas_direct[(unsigned char)(product->iva - 'A')];
	long long total_cents =
		(product->price * item->quantity * (CENTS_PER_EURO + rate) +
		 ROUND_HALF) /
		CENTS_PER_EURO;

	/* Prints output */
	printf("%c %.2f %d %.2f %s\n", product->iva,
		   product->price / FLOAT_CENTS_PER_EURO, item->quantity,
		   total_cents / FLOAT_CENTS_PER_EURO, product->description);
}

/**
 * Parses the 'a' command arguments and adds a product to the cart.
 * Validates EAN, stock, and quantity before modifying the cart.
 * @param sys         Pointer to the system state.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 * @param pending     Argument string (quantity and/or EAN).
 */
void add_to_cart(Sys* sys, int* ivas_direct, char* pending) {
	/* Initializations of vars */
	char* ean;
	int quantity;
	Product* product;
	Product_in_cart* item;

	/* Parses arguments */
	parse_cart_args(pending, &quantity, &ean);
	if (validate_cart_ean(ean, sys, &product) == ERROR) return;	 // if valid EAN

	/* Finds product in cart */
	item = find_in_cart(sys, *product);
	if (item != NULL) {	 // if already exists -> updates
		if (update_existing_cart(product, item, quantity) == ERROR) return;
	} else {  // if not -> creates
		if (insert_new_cart(sys, product, ean, quantity, &item) == ERROR)
			return;
	}

	/* Updates sold/stock values */
	product->sold += quantity;
	product->stock -= quantity;

	/* Prints output */
	print_cart_line(product, item, ivas_direct);
}

/**
 * Lists all products with stock > 0 that match one or more wildcard patterns.
 * Sorts products by creation order before printing.
 * @param sys       Pointer to the system state.
 * @param wildcards Space-separated list of wildcard patterns.
 */
void list_wildcards(Sys* sys, char* wildcards) {
	/* Initializations of vars */
	char* wildcard;
	Product* product;
	int found;

	/* Sorts every product */
	quicksort_products(sys, 0, sys->num_products - 1);
	for (int i = 0; i < sys->num_products; i++) {
		sys->products[i]->array_index = i;
	}

	/* Goes through each wildcard and finds the EANs */
	wildcard = strtok(wildcards, " \n");
	while (wildcard != NULL) {
		found = 0;
		for (int i = 0; i < sys->num_products; i++) {
			product = sys->products[i];
			if (product->stock > 0 && match_wildcards(wildcard, product->ean)) {
				printf("%s %c %.2f %d %d %s\n", product->ean, product->iva,
					   product->price / FLOAT_CENTS_PER_EURO, product->sold,
					   product->stock, product->description);
				found = 1;
			}
		}
		if (!found) printf("%s: %s\n", wildcard, ENO_SUCH_PRODUCT);
		wildcard = strtok(NULL, " \n");
	}
}

/**
 * Prints each item in the cart with its IVA rate, price, quantity, and total.
 * Skips entries with a quantity of zero.
 * @param sys         Pointer to the system state.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 */
void list_cart(Sys* sys, int* ivas_direct) {
	/* Initializations of vars */
	Product* product;
	int rate;
	long long total_cents;

	/* Lists cart */
	for (int i = 0; i < sys->num_cart; i++) {
		product = sys->cart[i].product;
		if (sys->cart[i].quantity != 0) {
			rate = ivas_direct[(unsigned char)(product->iva - 'A')];
			total_cents = (product->price * sys->cart[i].quantity *
							   (CENTS_PER_EURO + rate) +
						   ROUND_HALF) /
						  CENTS_PER_EURO;
			printf("%c %.2f %d %.2f %s\n", product->iva,
				   product->price / FLOAT_CENTS_PER_EURO, sys->cart[i].quantity,
				   total_cents / FLOAT_CENTS_PER_EURO, product->description);
		}
	}
}

/**
 * Computes the rounded total cost for a given product, quantity, and tax rate.
 * Uses banker's rounding (add 50 before integer division).
 * @param p    Pointer to the product.
 * @param qty  Quantity of units.
 * @param rate Tax rate as an integer percentage.
 * @return Total cost in cents.
 */
long long rounds(Product* p, int qty, int rate) {
	/* Rounds values according to the project guidelines */
	return (p->price * qty * (CENTS_PER_EURO + rate) + ROUND_HALF) /
		   CENTS_PER_EURO;
}

/**
 * Calculates the total number of items and total value of the current cart.
 * @param sys         Pointer to the system state.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 * @param items       Output: total number of items in the cart.
 * @param value       Output: total cart value in cents.
 */
void calculate_cart(Sys* sys, int* ivas_direct, int* items, long long* value) {
	/* Initializations of vars */
	long long cents = 0;
	Product* product;
	int quantity, rate;
	*items = 0;

	/* Sums each value for every product in the cart */
	for (int i = 0; i < sys->num_cart; i++) {
		product = sys->cart[i].product;
		quantity = sys->cart[i].quantity;
		rate = ivas_direct[product->iva - 'A'];
		cents += rounds(product, quantity, rate);
		*items += quantity;
	}
	*value = cents;
}

/**
 * Returns all products in the cart to their original stock levels
 * and reverses the units-sold counter.
 * @param sys Pointer to the system state.
 */
void return_stock(Sys* sys) {
	/* Initializations of vars */
	Product* p;
	int q;

	/* Goes through cart and returns each product */
	for (int i = 0; i < sys->num_cart; i++) {
		p = sys->cart[i].product;
		q = sys->cart[i].quantity;
		p->stock += q;
		p->sold -= q;
	}
}

/**
 * Removes a product from the hash table and the products array, and frees it.
 * Swaps the last array entry into the removed slot to keep the array compact.
 * @param sys     Pointer to the system state.
 * @param product Pointer to the product to unlink and free.
 */
static void unlink_product(Sys* sys, Product* product) {
	/* Initializations of vars */
	unsigned int h = hash(product->ean);
	Product* prev = NULL;
	Product* cur = sys->product_table[h];
	int index;

	/* Takes product out of hash table */
	while (cur) {
		if (cur == product) {
			if (prev)
				prev->hash_next = cur->hash_next;
			else
				sys->product_table[h] = cur->hash_next;
			break;
		}
		prev = cur;
		cur = cur->hash_next;
	}
	index = product->array_index;
	sys->num_products--;
	if (index != sys->num_products) {
		sys->products[index] = sys->products[sys->num_products];
		sys->products[index]->array_index = index;
	}
	free(product->description);
	free(product);
}

/**
 * Reduces a product's stock by a given quantity, removing it if stock hits 0.
 * Validates the EAN, product existence, cart status, and quantity.
 * @param sys Pointer to the system state.
 * @param ean EAN string of the product to modify.
 * @param in  String containing the quantity to remove.
 */
void remove_product(Sys* sys, char* ean, char* in) {
	/* Initializations of vars */
	int quantity = atoi(in);

	/* Checks EAN */
	if (verify_ean(ean) == ERROR) {
		puts(EINVALID_EAN);
		return;
	}

	/* Checks if product exists */
	Product* product = find_product(sys, ean);
	if (!product) {
		printf("%s: %s\n", ean, ENO_SUCH_PRODUCT);
		return;
	}
	/* Checks if product is in cart */
	if (product->in_cart) {
		puts(EPRODUCT_IN_USE);
		return;
	}
	/* Chceks if quantity is valid */
	if (quantity <= 0 || verify_stock(product->stock - quantity) == ERROR) {
		puts(EINVALID_QUANTITY);
		return;
	}

	/* Removes product */
	product->stock -= quantity;
	printf("%d %s\n", product->stock, product->description);
	if (product->stock == 0) unlink_product(sys, product);
}

/**
 * Clears the cart by resetting all in_cart flags and the cart counter.
 * Does not return stock to products.
 * @param sys Pointer to the system state.
 */
void clear_cart(Sys* sys) {
	/* Clears cart quite literally... */
	for (int i = 0; i < sys->num_cart; i++) sys->cart[i].product->in_cart = 0;
	sys->num_cart = 0;
}

/**
 * Rolls back all cart stock changes and then clears the cart.
 * @param sys Pointer to the system state.
 */
void rollback_and_clear(Sys* sys) {
	/* Returns stock and clears cart :) */
	return_stock(sys);
	clear_cart(sys);
}