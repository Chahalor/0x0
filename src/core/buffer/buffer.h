/**
 * @file buffer.h
 * @brief Public API for owned byte buffers.
 * @details The buffer stores raw bytes without a trailing NUL byte. Its
 * integer results are zero on success, positive errno on system failures,
 * or negative BUFFER_ERROR values on module-specific failures.
 * The allocator module owns allocation; setting supplies the default.
 */
#ifndef GENESIS_CORE_BUFFER_H
# define GENESIS_CORE_BUFFER_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Internals |----- */
# include "types.h"

/* ************************************************************************** */
/*                                 Macros                                     */
/* ************************************************************************** */

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

/**
 * @brief Initialize caller-owned, uninitialized buffer storage.
 * @param buffer Buffer object to initialize.
 * @param capacity Initial byte capacity; zero is accepted.
 * @return 0, EINVAL, or ENOMEM. On failure, buffer is unchanged and must not
 * be passed to core_buffer_deinit unless it was initialized before this call.
 * @par Example
 * @code{.c}
 * t_genesis_core_buffer buffer;
 * if (core_buffer_init(&buffer, 16) == 0) core_buffer_deinit(&buffer);
 * @endcode
 */
int	core_buffer_init(
	t_genesis_core_buffer	*const	buffer,
	const size_t					capacity
);

/**
 * @brief Release byte storage and zero a caller-owned buffer.
 * @param buffer Caller-owned buffer successfully initialized by a
 * core_buffer_init variant; NULL is accepted. Do not use on objects created
 * by a core_buffer_new variant; release those with core_buffer_free.
 * @return Nothing.
 * @par Example
 * @code{.c}
 * core_buffer_deinit(&buffer);
 * @endcode
 */
void	core_buffer_deinit(
	t_genesis_core_buffer	*const	buffer
);

/**
 * @brief Allocate a buffer object with the default allocator.
 * @param capacity Initial byte capacity; zero is accepted.
 * @return Owned buffer or NULL; errno is set on allocation failure.
 * @par Example
 * @code{.c}
 * t_genesis_core_buffer *buffer = core_buffer_new(16);
 * @endcode
 */
t_genesis_core_buffer	*core_buffer_new(
	const size_t	capacity
);

/**
 * @brief Release a buffer object created by core_buffer_new variants.
 * @param buffer Object created by a core_buffer_new variant; NULL is accepted.
 * Do not use on caller-owned buffers or after core_buffer_deinit.
 * @return Nothing.
 * @par Example
 * @code{.c}
 * core_buffer_free(buffer);
 * @endcode
 */
void	core_buffer_free(
	t_genesis_core_buffer	*const	buffer
);

/**
 * @brief Ensure capacity using the captured growth policy.
 * @param buffer Initialized buffer.
 * @param min_capacity Minimum number of bytes to hold.
 * @return 0, EINVAL, or ENOMEM.
 * @par Example
 * @code{.c}
 * core_buffer_reserve(&buffer, 128);
 * @endcode
 */
int	core_buffer_reserve(
	t_genesis_core_buffer	*const	buffer,
	const size_t					min_capacity
);

/**
 * @brief Change length, zeroing newly exposed bytes.
 * @param buffer Initialized buffer.
 * @param new_length New logical byte length.
 * @return 0, EINVAL, or ENOMEM. Shrinking retains capacity.
 * @par Example
 * @code{.c}
 * core_buffer_resize(&buffer, 32);
 * @endcode
 */
int	core_buffer_resize(
	t_genesis_core_buffer	*const	buffer,
	const size_t					new_length
);

/**
 * @brief Append bytes and grow capacity if needed.
 * @param buffer Initialized buffer.
 * @param data Source bytes; may refer to existing buffer bytes.
 * @param length Number of source bytes; zero permits NULL data.
 * @return 0, EINVAL, EOVERFLOW, ENOMEM, or
 * -BUFFER_ERROR_SOURCE_OUT_OF_RANGE. Use core_buffer_strerror() to decode it.
 * @par Example
 * @code{.c}
 * core_buffer_append(&buffer, "abc", 3);
 * @endcode
 */
int	core_buffer_append(
	t_genesis_core_buffer	*const	buffer,
	const void				*const	data,
	const size_t					length
);

/**
 * @brief Append bytes without allocating more storage.
 * @param buffer Initialized buffer.
 * @param data Source bytes; may refer to existing buffer bytes.
 * @param length Number of source bytes; zero permits NULL data.
 * @return 0, EINVAL, EOVERFLOW, ENOSPC, or
 * -BUFFER_ERROR_SOURCE_OUT_OF_RANGE for an internal source outside length.
 * @par Example
 * @code{.c}
 * core_buffer_append_fixed(&buffer, "abc", 3);
 * @endcode
 */
int	core_buffer_append_fixed(
	t_genesis_core_buffer	*const	buffer,
	const void				*const	data,
	const size_t					length
);

/**
 * @brief Reset logical length while retaining allocated storage.
 * @param buffer Buffer to clear; NULL is accepted. Bytes are not erased.
 * @return Nothing.
 * @par Example
 * @code{.c}
 * core_buffer_clear(&buffer);
 * @endcode
 */
void	core_buffer_clear(
	t_genesis_core_buffer	*const	buffer
);

#endif
