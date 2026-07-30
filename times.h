/**
 * 
*/
#ifndef GENESIS_TIMES_H
# define GENESIS_TIMES_H

# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Systems   |----- */
# include <time.h>

/* -----| Globals   |----- */
# include "standards/lib_config.h"
# include "standards/lib_standards.h"

/* -----| Internals |----- */
# include "times/types.h"

/* -----| Modules   |----- */
	//...

/* ************************************************************************** */
/*                                 Macros                                     */
/* ************************************************************************** */

#define	TIME_MINUTES_PER_DAY		1440
#define	TIME_SECONDS_PER_DAY		86400

#define TIME_SECONDS_PER_HOUR		3600

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

/**
 * @brief				convert `src` from one time unit to an other
 * 
 * @param	src			the base value to be converted
 * @param	base_src	the base unit of src
 * @param	dest_src	the unit to be converted in
 * 
 * @return				the converted value
 * 
 * @pre					base_src and base_dest should be from the enum `e_time_unity`
 * 
 * @ingroup times
 * @version	1.0.0
 */
time_t	time_convert(
			const time_t	_src,
			const __u_char	_base_src,
			const __u_char	_base_dest
			);

#endif	// GENESIS_TIMES_H
