/**
 * Declarations for utility functions.
 * @file utils.h
 * @author ist1117883 (Margarida Mineiro)
 */

#ifndef UTILS_H
#define UTILS_H

#include "system_types.h"

/**
 * Initialises the IVA rates array from a file, or uses default rates.
 * Default rates: A=0%, B=6%, C=13%, D=23%.
 * @param file        Path to an IVA configuration file, or NULL for defaults.
 * @param ivas_direct Array of size MAX_IVA to populate with tax rates.
 */
void ivas_init(char* file, int* ivas_direct);

/**
 * Allocates and zero-initialises the system state, including the products
 * array, cart, and client list.
 * @return A fully initialised Sys struct ready for use.
 */
Sys system_init(void);

/**
 * Reads one non-empty line from stdin into buf, stripping the newline.
 * Skips blank lines by recursing.
 * @param buf   Buffer to read into.
 * @param bufsz Size of the buffer in bytes.
 * @return 1 if a line was read, 0 on EOF.
 */
int read_line(char* buf, int bufsz);

/**
 * Computes a polynomial hash of a null-terminated string.
 * @param str The string to hash.
 * @return An index in [0, HASH_SIZE).
 */
unsigned int hash(char* str);

/**
 * Extracts a client name, handling optional double-quote delimiters.
 * Modifies the input string in place by replacing the closing quote with '\0'.
 * @param name Pointer to the name string (may start with '"').
 * @return Pointer to the name content, or NULL if the name is empty/invalid.
 */
char* parse_name(char* name);

/**
 * Matches a wildcard pattern against a text string.
 * Supports '*' (any sequence) and '?' (any single character).
 * @param pattern The wildcard pattern.
 * @param text    The string to match against.
 * @return 1 if the text matches the pattern, 0 otherwise.
 */
int match_wildcards(const char* pattern, const char* text);

/**
 * Inserts a client into the sorted client_list array, expanding it if needed.
 * Maintains alphabetical order by client name.
 * @param sys Pointer to the system state.
 * @param new Pointer to the client to insert.
 */
void insert_client_sorted(Sys* sys, Client* new);

/**
 * Validates an EAN-8 or EAN-13 barcode string using the check-digit algorithm.
 * @param ean Null-terminated EAN string.
 * @return 0 if valid, ERROR otherwise.
 */
int verify_ean(char* ean);

/**
 * Checks that an IVA letter is uppercase and has a defined rate.
 * @param iva         IVA category letter to validate.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 * @return 0 if valid, ERROR otherwise.
 */
int verify_iva(char iva, int* ivas_direct);

/**
 * Checks that a price is strictly positive.
 * @param price Price in cents.
 * @return 0 if valid, ERROR otherwise.
 */
int verify_price(long long price);

/**
 * Checks that a stock quantity is non-negative.
 * @param stock Stock level to validate.
 * @return 0 if valid, ERROR otherwise.
 */
int verify_stock(int stock);

/**
 * Checks that a description is non-empty, starts with an uppercase or
 * accented character, and does not exceed MAX_DESCRIPTION characters.
 * @param description Null-terminated description string.
 * @return 0 if valid, ERROR otherwise.
 */
int verify_description(char* description);

/**
 * Checks that the system has not reached the maximum number of products.
 * @param sys Pointer to the system state.
 * @return 0 if space remains, ERROR otherwise.
 */
int verify_system(Sys* sys);

/**
 * Checks that a name string is non-empty and starts with an alphabetic
 * or extended-Latin character.
 * @param name Null-terminated name string.
 * @return 0 if valid, ERROR otherwise.
 */
int verify_name(char* name);

/**
 * Checks that a NIF is within the valid Portuguese NIF range.
 * @param nif NIF value to validate.
 * @return 0 if valid, ERROR otherwise.
 */
int verify_nif(int nif);

/**
 * Looks up an invoice by number in the invoice hash table.
 * @param sys    Pointer to the system state.
 * @param numero Invoice number to find.
 * @return Pointer to the Invoice if found, NULL otherwise.
 */
Invoice* verify_invoice(Sys* sys, int numero);

/**
 * Sorts the products array in-place by creation order using quicksort.
 * @param sys Pointer to the system state.
 * @param lo  Lower bound index (inclusive).
 * @param hi  Upper bound index (inclusive).
 */
void quicksort_products(Sys* sys, int lo, int hi);

/**
 * Releases all heap memory held by the system and resets counters to zero.
 * @param sys Pointer to the system state.
 */
void cleanup(Sys* sys);

/**
 * Prints an out-of-memory error, cleans up, and exits.
 * @param sys Pointer to the system state.
 */
void no_memory(Sys* sys);

#endif