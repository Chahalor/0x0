// Header

/* ----| Headers    |----- */
	/* Standard */
		//...
	/* Internal */
#include "_internal_/_memory.h"
#include "memory.h"
#include <errno.h>
	/* External */
		//...

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */
	//...

/* ----| Public     |----- */

/**
 * @brief	allocate _size bit of memory (is just a malloc wrapper + garbage collector)
 * 
 * @param	_size	the allocation size
 * 
 * @return	the pointer to the area or NULL in case of error
 * 
 * @visbility	public
 * 
 * @version 1.2.0
 */
void	*mem_alloc(
	const size_t	_size
)
{
	void	*result = NULL;

	result = mem_manager_(mem_manager_access_alloc, NULL, _size);

	return (result);
}

/**
 * @brief	reallocate _area to _size
 * 
 * @param	_ptr	the memory area to reallocate
 * @param	_size	the new size of the area
 * 
 * @return	the extended pointer or NULL in case of error
 * 
 * @visibility	public
 * 
 * @version 1.1.0
*/
void	*mem_realloc(
	void			*_ptr,
	const size_t	_size
)
{
	void	*result = NULL;

	result = mem_manager_(mem_manager_access_realloc, _ptr, _size);

	return (result);
}

/**
 * @brief	duplicate the memory inside _ptr. if _size < 0 use the size stored by the manager
 * 
 * @param	_ptr	the memory area to reallocate
 * @param	_size	the new size of the area
 * 
 * @return	the newly allocated pointer or NULL in case of error
 * 
 * @visibility	public
 * 
 * @version 1.0.0
*/
void	*mem_dup(
	const void *const restrict _ptr,
	const ssize_t _size
)
{
	void	*result = NULL;

	result = mem_manager_(mem_manager_access_dup, _ptr, _size);

	return (result);
}

/**
 * @brief	free _area
 * 
 * @param	_ptr	the memory area to free
 * 
 * @return	void
 * 
 * @visibility	public
 * 
 * @version 1.0.0
*/
void	mem_free(
	void *const restrict _ptr
)
{
	int	_error = error_none;

	_error = mem_manager_(mem_manager_access_free, _ptr, 0);
}

/**
 * @brief	free all allocated memory by the program
 * 
 * @param	void
 * 
 * @return	void
 * 
 * @visibility	public
 * 
 * @version 1.0.0
*/
__attribute__((destructor))
void	mem_free_all(void)
{
	mem_manager_(mem_manager_access_free_all, NULL, 0);
}
