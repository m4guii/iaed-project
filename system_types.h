/**
 * System-wide type definitions, constants, and error messages.
 * @file system_types.h
 * @author ist1117883 (Margarida Mineiro)
 */

#ifndef SYSTEM_TYPES_H
#define SYSTEM_TYPES_H

/* Libraries */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Sizes and limits */
#define BUFMAX 65536
#define HASH_SIZE 10007
#define MAX_PRODUCTS 10000
#define MAX_IVA 26
#define MAX_DESCRIPTION 50
#define EAN_SIZE 14
#define EAN8_SIZE 8
#define EAN13_SIZE 13
#define NIF_SIZE 9
#define MIN_NIF 100000000
#define MAX_NIF 999999999
/** Initial allocated capacity for dynamic arrays. */
#define INITIAL_CAPACITY 16

/* Arithmetic constants */
/** Multiplier used in polynomial rolling hash. */
#define HASH_MULTIPLIER 31
/** Conversion factor: price stored in cents, displayed in euros. */
#define CENTS_PER_EURO 100
#define FLOAT_CENTS_PER_EURO 100.0
/** Added before integer division to achieve round-half-up. */
#define ROUND_HALF 50
/** EAN check-digit modulus. */
#define EAN_MODULUS 10
/** Multiplier applied to odd-position EAN digits. */
#define EAN_ODD_MULTIPLIER 3

/* Special characters */
#define ZERO_CHAR '0'
#define WILDCARD_1 '*'
#define WILDCARD_2 '?'
/** Minimum Unicode/UTF-8 value for accented uppercase letters. */
#define MIN_ACCENTED_CHAR 192

/** Default client name when none is supplied. */
#define CLIENT_STANDARD "Cliente final"
/** Special client name that triggers a cart rollback. */
#define ERROR_CLIENT "error"

/* Default IVA rates (used when no config file is supplied) */
#define IVA_A_DEFAULT 0
#define IVA_B_DEFAULT 6
#define IVA_C_DEFAULT 13
#define IVA_D_DEFAULT 23

/* Error messages */
#define EINVALID_EAN "invalid ean"
#define EINVALID_IVA "invalid iva"
#define EINVALID_PRICE "invalid price"
#define EINVALID_QUANTITY "invalid quantity"
#define EINVALID_DESCRIPTION "invalid description"
#define EINVALID_PRODUCT "invalid product"
#define EINVALID_NAME "invalid name"
#define EPRODUCT_IN_USE "product in use"
#define ENO_SUCH_PRODUCT "no such product"
#define ENO_SUCH_INVOICE "no such invoice"
#define ENO_SUCH_NIF "no such nif"
#define ENO_SUCH_CLIENT "no such client"
#define ENO_STOCK "no stock"
#define ENO_MEMORY "No memory."
#define ERROR -1

/* Structs */

/** Product information. */
typedef struct product {
	char ean[EAN_SIZE + 1];	   /**< EAN barcode string. */
	char iva;				   /**< VAT category letter (A-D). */
	long long price;		   /**< Price in cents. */
	int stock;				   /**< Current stock count. */
	char* description;		   /**< Heap-allocated description. */
	int sold;				   /**< Total units sold. */
	int in_cart;			   /**< Non-zero if product is in cart. */
	int creation_order;		   /**< Insertion order for sorting. */
	int array_index;		   /**< Index in sys->products array. */
	struct product* hash_next; /**< Next product in hash bucket. */
} Product;

/** A product entry inside the shopping cart. */
typedef struct product_in_cart {
	Product* product; /**< Pointer to the product. */
	int quantity;	  /**< Quantity currently in cart. */
} Product_in_cart;

/** Invoice information. */
typedef struct invoice {
	int number;				   /**< Unique invoice number. */
	int nif;				   /**< Client tax identification number. */
	char* client;			   /**< Client name (heap-allocated). */
	int items;				   /**< Number of items purchased. */
	long long total_payed;	   /**< Total amount paid in cents. */
	struct invoice* next;	   /**< Next invoice for the same client. */
	struct invoice* hash_next; /**< Next invoice in hash bucket. */
} Invoice;

/** Client information. */
typedef struct client {
	char* name;			   /**< Heap-allocated client name. */
	Invoice* invoices;	   /**< Head of this client's invoice list. */
	struct client* next;   /**< Next client in hash bucket. */
	Invoice* last_invoice; /**< Tail pointer for O(1) appends. */
} Client;

/** Global system state. */
typedef struct system {
	Product** products; /**< Array of pointers to all products. */
	int num_products;	/**< Current number of products. */
	int max_products;	/**< Allocated capacity of products array. */
	/** Hash table for fast product lookup by EAN. */
	Product* product_table[HASH_SIZE];

	Product_in_cart* cart; /**< Sorted cart array (by EAN). */
	int num_cart;		   /**< Number of distinct items in cart. */
	int max_cart;		   /**< Allocated capacity of cart. */

	int num_invoices; /**< Total invoices issued. */
	int max_invoices; /**< Reserved invoice capacity. */
	int next_invoice; /**< Next invoice number to assign. */
	/** Hash table for fast invoice lookup by number. */
	Invoice* invoice_table[HASH_SIZE];

	int total_items_sold;		/**< Running total of items sold. */
	long long total_value_sold; /**< Running total value in cents. */

	Client** client_list; /**< Sorted array of client pointers. */
	int num_clients;	  /**< Number of registered clients. */
	int max_clients;	  /**< Allocated capacity of client_list. */
	/** Hash table for fast client lookup by name. */
	Client* clients[HASH_SIZE];
} Sys;

#endif