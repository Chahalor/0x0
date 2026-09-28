// Header

/* ----| Headers    |----- */
	/* Standard */
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

	/* Internal */
#include "random.h"

	/* External */
#include "../math.h"
#include "../memory.h"

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */

/* ----| Public     |----- */

int	randint(
	const int _val1,
	const int _val2
)
{
	const int	_min = min(_val1, _val2);
	const int	_max = max(_val1, _val2);
	const uint64_t span = (uint64_t)((int64_t)_max - _min) + 1;
	const uint64_t threshold = (uint64_t)(-span) % span;
	uint64_t sample;
	int i;

	do
	{
		sample = 0;
		i = 0;
		while (i++ < 5)
			sample = (sample << 15) | ((unsigned int)rand() & 0x7fffU);
	} while (sample < threshold);
	return ((int)((int64_t)_min + (int64_t)(sample % span)));
}

__attribute__((malloc))
int	*rand_range(
	const unsigned int _size,
	const int _val1,
	const int _val2
)
{
	int				*result = NULL;
	unsigned int	_i = 0;

	#if SIZE_MAX <= UINT_MAX
	if ((size_t)_size > SIZE_MAX / sizeof(int) - 1)
	{
		errno = EOVERFLOW;
		return (NULL);
	}
	#endif
	result = mem_alloc(((size_t)_size + 1) * sizeof(int));
	if (unlikely(!result))
		return (NULL);
	while (_i < _size)
		result[_i++] = randint(_val1, _val2);
	result[_size] = 0;
	return (result);
}
