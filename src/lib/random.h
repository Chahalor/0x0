/**
 * 
*/
#ifndef GENESIS_RANDOM_H
# define GENESIS_RANDOM_H

# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Systems   |----- */
	//...

/* -----| Globals   |----- */
# include "standards/lib_config.h"
# include "standards/lib_standards.h"

/* -----| Internals |----- */
// # include "types.h"

/* -----| Modules   |----- */
	//...

/* ************************************************************************** */
/*                                 Macros                                     */
/* ************************************************************************** */
	//...

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

/**
 * @brief Return a pseudorandom integer in the inclusive range, in either order.
 * @note Uses the application's rand() state and does not call srand(). The
 * caller may seed rand() if desired. Not suitable for cryptography.
 * 
 * @param	val1	first value
 * @param	val2	second value
 * 
 * @return a random value
 * 
 * @version 0.0.1
 */
int	randint(
		const int _val1,
		const int _val2
		);

/**
 * @brief Return a registered array of size values followed by a zero sentinel.
 * @note The sentinel cannot be used to find the end when zero is a valid value.
 * Release the array with mem_free().
 * 
 * @param	size	the size of the array
 * @param	val1	first value for the range
 * @param	val2	seconde vale for the range
 * 
 * @return The allocated array, or NULL with errno set on failure.
 * @retval		ptr: the array
 * @retval		NULL: some errors append
 * 
 * @version	0.0.1
*/
int	*rand_range(
		const unsigned int _size,
		const int _val1,
		const int _val2
		);

#endif	// GENESIS_RANDOM_H
