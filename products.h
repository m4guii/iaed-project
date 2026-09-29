/**
 * Declarations for product and cart management functions.
 * @file products.h
 * @author ist1117883 (Margarida Mineiro)
 */

#ifndef PRODUCTS_H
#define PRODUCTS_H

#include "system_types.h"

/**
 * Searches for a product by EAN in the product hash table.
 * @param sys Pointer to the system state.
 * @param ean EAN string to look up.
 * @return Pointer to the Product if found, NULL otherwise.
 */
Product* find_product(Sys* sys, char* ean);

/**
 * Searches for a product entry in the sorted cart array using binary search.
 * @param sys     Pointer to the system state.
 * @param product The product whose EAN is used as the search key.
 * @return Pointer to the Product_in_cart entry if found, NULL otherwise.
 */
Product_in_cart* find_in_cart(Sys* sys, Product product);

/**
 * Allocates a new product and registers it in the system.
 * @param sys         Pointer to the system state.
 * @param ean         EAN string for the product.
 * @param iva         IVA category letter.
 * @param price       Price in cents.
 * @param stock       Initial stock quantity.
 * @param description Product description string.
 * @return The product's stock after insertion.
 */
int add_product(Sys* sys, char* ean, char iva, long long price, int stock,
				char* description);

/**
 * Updates the fields of an existing product, adding stock to the current value.
 * @param sys         Pointer to the system state.
 * @param product     Pointer to the product to update.
 * @param iva         New IVA category letter.
 * @param price       New price in cents.
 * @param stock       Quantity to add to existing stock.
 * @param description New description string.
 * @return The product's stock after the update.
 */
int update_product(Sys* sys, Product* product, char iva, long long price,
				   int stock, char* description);

/**
 * Parses the 'a' command arguments and adds a product to the cart.
 * @param sys         Pointer to the system state.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 * @param pending     Argument string (quantity and/or EAN).
 */
void add_to_cart(Sys* sys, int* ivas_direct, char* pending);

/**
 * Lists all products with stock > 0 matching one or more wildcard patterns.
 * @param sys       Pointer to the system state.
 * @param wildcards Space-separated list of wildcard patterns.
 */
void list_wildcards(Sys* sys, char* wildcards);

/**
 * Prints each item in the cart with IVA rate, price, quantity, and total.
 * @param sys         Pointer to the system state.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 */
void list_cart(Sys* sys, int* ivas_direct);

/**
 * Computes the rounded total cost for a given product, quantity, and tax rate.
 * @param p    Pointer to the product.
 * @param qty  Quantity of units.
 * @param rate Tax rate as an integer percentage.
 * @return Total cost in cents.
 */
long long rounds(Product* p, int qty, int rate);

/**
 * Calculates the total items and total value of the current cart.
 * @param sys         Pointer to the system state.
 * @param ivas_direct Array mapping IVA letters to tax rates.
 * @param items       Output: total number of items in the cart.
 * @param value       Output: total cart value in cents.
 */
void calculate_cart(Sys* sys, int* ivas_direct, int* items, long long* value);

/**
 * Returns all products in the cart to their original stock levels.
 * @param sys Pointer to the system state.
 */
void return_stock(Sys* sys);

/**
 * Reduces a product's stock by a given quantity, removing it if stock hits 0.
 * @param sys Pointer to the system state.
 * @param ean EAN string of the product to modify.
 * @param in  String containing the quantity to remove.
 */
void remove_product(Sys* sys, char* ean, char* in);

/**
 * Clears the cart by resetting all in_cart flags and the cart counter.
 * @param sys Pointer to the system state.
 */
void clear_cart(Sys* sys);

/**
 * Rolls back all cart stock changes and then clears the cart.
 * @param sys Pointer to the system state.
 */
void rollback_and_clear(Sys* sys);

#endif