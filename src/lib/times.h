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
 * @return Converted value. On error returns 0 and sets errno to EINVAL or
 * EOVERFLOW; valid zero sets errno to zero. Use time_convert_checked to
 * distinguish errors without relying on errno.
 * 
 * @pre					base_src and base_dest should be from the enum `e_time_unity`
 * 
 * @ingroup times
 * @version	1.0.0
 */
time_t	time_convert(
			const time_t	_src,
			const unsigned char	_base_src,
			const unsigned char	_base_dest
			);

/**
 * @brief Convert units with explicit status and overflow detection.
 * @param src Value to convert.
 * @param base_src Source unit.
 * @param base_dest Destination unit.
 * @param result Non-NULL output; unchanged on failure.
 * @return 0 on success, EINVAL for invalid units/output, EOVERFLOW if the
 * converted value does not fit in time_t. Integer division truncates toward 0.
 * @par Example
 * @code{.c}
 * time_t hours;
 * if (time_convert_checked(7200, time_unity_sec, time_unity_hour, &hours) == 0)
 *     use_hours(hours);
 * @endcode
 */
int	time_convert_checked(
	const time_t src,
	const unsigned char base_src,
	const unsigned char base_dest,
	time_t *const result
);

#endif	// GENESIS_TIMES_H
