// Header

/* ----| Headers    |----- */
	/* Standard */
		//...

	/* Internal */
#include "times.h"
#include <errno.h>
#include <stdint.h>

	/* External */
		//...

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */

/**
 * @brief	static conversion table from any handeld unit to ns
 */
static const uint64_t	_g_time_to_ns[] = {
	[time_unity_ns]		= 1LL,
	[time_unity_us]		= 1000LL,
	[time_unity_ms]		= 1000000LL,
	[time_unity_sec]	= 1000000000LL,
	[time_unity_min]	= 60LL * 1000000000LL,
	[time_unity_hour]	= 3600LL * 1000000000LL,
	[time_unity_day]	= 86400LL * 1000000000LL,
};

/* ----| Public     |----- */

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
 * 
 */
int	time_convert_checked(
	const time_t src,
	const unsigned char base_src,
	const unsigned char base_dest,
	time_t *const result
)
{
	uint64_t source_factor;
	uint64_t destination_factor;
	time_t converted;
	time_t ratio;
	uint64_t ratio_u64;

	if (!result || base_src > time_unity_day || base_dest > time_unity_day)
		return (EINVAL);
	source_factor = _g_time_to_ns[base_src];
	destination_factor = _g_time_to_ns[base_dest];
	if (source_factor >= destination_factor)
	{
		ratio_u64 = source_factor / destination_factor;
		if ((uint64_t)(time_t)ratio_u64 != ratio_u64)
		{
			if (src != 0)
				return (EOVERFLOW);
			converted = 0;
			*result = converted;
			return (0);
		}
		ratio = (time_t)ratio_u64;
		if (__builtin_mul_overflow(src, ratio, &converted))
			return (EOVERFLOW);
	}
	else
	{
		ratio_u64 = destination_factor / source_factor;
		if ((uint64_t)(time_t)ratio_u64 != ratio_u64)
			converted = 0;
		else
		{
			ratio = (time_t)ratio_u64;
			converted = src / ratio;
		}
	}
	*result = converted;
	return (0);
}

time_t	time_convert(
	const time_t src,
	const unsigned char base_src,
	const unsigned char base_dest
)
{
	time_t result;
	int status;

	status = time_convert_checked(src, base_src, base_dest, &result);
	errno = status;
	if (status != 0)
		return (0);
	return (result);
}
