// Header

/* ----| Headers    |----- */
	/* Standard */
#include <stdarg.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Internal */
#include "_memory.h"
	/* External */
		//...

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */

#if MEM_MANAGER_TYPE == 0

int	_hash(
	const void *const _ptr
)
{
	return ((int)((uintptr_t)_ptr % MEM_BUCKET_SIZE));
}

t_mem_node	*_mem_find_node(
	const t_mem_manager *const _manager,
	const void *const _ptr
)
{
	if (unlikely(!_manager || !_ptr))
		return (NULL);

	const int	_id = _hash(_ptr);
	t_mem_node	*_node;

	_node = _manager->bucket[_id];
	while (_node && _node->content != _ptr)
		_node = _node->next;
	return (_node);
}

int	_mem_register_node(
	t_mem_manager *const _manager,
	t_mem_node *_node
)
{
	int	_id;
	t_mem_node	*_current;

	if (unlikely(!_manager || !_node))
		return (EINVAL);

	_id = _hash(_node->content);
	_current = _manager->bucket[_id];
	_node->next = _current;
	_manager->bucket[_id] = _node;
	return (mem_manager_error_none);
}

t_mem_node	*_mem_new_node(
	t_mem_manager *const _manager,
	const size_t _size
)
{
	t_mem_node *result = NULL;

	if (!_manager || _size == 0 || _size > SIZE_MAX - sizeof(t_mem_node))
	{
		errno = _size == 0 ? EINVAL : EOVERFLOW;
		return (NULL);
	}
	result = _manager->allocator.alloc(_size + sizeof(t_mem_node));
	if (unlikely(!result))
	{
		errno = ENOMEM;
		return (NULL);
	}
	memset(result, 0, _size + sizeof(t_mem_node));
	result->content = (void *)(result + 1);
	result->size = _size;
	result->next = NULL;
	return (result);
}

int	_mem_unregister_node(
	t_mem_manager *const _manager,
	t_mem_node *_node,
	const int _must_free
)
{
	int	_id;
	t_mem_node	**link;

	if (unlikely(!_manager || !_node))
		return (EINVAL);

	_id = _hash(_node->content);
	link = &_manager->bucket[_id];
	while (*link && *link != _node)
		link = &(*link)->next;
	if (!*link)
		return (EINVAL);
	*link = _node->next;
	if (_must_free)
		_manager->allocator.free(_node);
	return (mem_manager_error_none);
}

/**
 * @brief	set *_output to a pointer to a newly allocated area and register the node into the manager
 * 
 * @param	_manager	the manager data struct
 * @param	_size		the allocation size
 * @param	_output		the output pointer
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
int	_mem_alloc(
	t_mem_manager *const _manager,
	const size_t _size,
	void **_output
)
{
	t_mem_node	*_node = NULL;

	if (!_output || _size == 0)
		return (EINVAL);
	*_output = NULL;
	if (_size > SIZE_MAX - sizeof(t_mem_node))
		return (EOVERFLOW);
	_node = _mem_new_node(_manager, _size);
	if (unlikely(!_node))
		return (errno ? errno : ENOMEM);

	_mem_register_node(_manager, _node);
	*_output = _node->content;
	return (mem_manager_error_none);
}

/**
 * @brief	reallocat _ptr to _size and register/unregister pointer.
 * 
 * @param	_manager	the manager data struct
 * @param	_ptr		the pointer to be reallocated
 * @param	_size		the allocation size
 * @param	_output		the output pointer
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
int	_mem_realloc(
	t_mem_manager *_manager,
	void *_ptr,
	const size_t _size,
	void **_output
)
{
	t_mem_node	*_node;
	t_mem_node	*_new;

	if (!_output)
		return (EINVAL);
	*_output = NULL;
	if (!_ptr)
		return (_mem_alloc(_manager, _size, _output));
	_node = _mem_find_node(_manager, _ptr);
	if (!_node)
		return (EINVAL);
	if (_size == 0)
		return (_mem_unregister_node(_manager, _node, 1));
	if (_size > SIZE_MAX - sizeof(t_mem_node))
		return (EOVERFLOW);
	_new = _mem_new_node(_manager, _size);
	if (!_new)
		return (errno ? errno : ENOMEM);
	memcpy(_new->content, _node->content,
		_node->size < _size ? _node->size : _size);
	_mem_register_node(_manager, _new);
	_mem_unregister_node(_manager, _node, 1);
	*_output = _new->content;
	return (0);
}

/**
 * @brief	duplicate _ptr based on _size or the stored size
 * 
 * @param	_manager	the manager data struct
 * @param	_ptr		the pointer to be reallocated
 * @param	_size		the allocation size
 * @param	_output		the output pointer
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
int	_mem_dup(
	t_mem_manager *const _manager,
	const void *const _ptr,
	const ssize_t _size,
	void **_output
)
{
	t_mem_node	*_target;
	t_mem_node	*_new;
	size_t copy_size;

	if (!_output)
		return (EINVAL);
	*_output = NULL;
	if (!_ptr || _size < -1 || _size == 0)
		return (EINVAL);
	_target = _mem_find_node(_manager, _ptr);
	if (_size == -1)
	{
		if (!_target)
			return (EINVAL);
		copy_size = _target->size;
	}
	else
	{
		if (_target && (size_t)_size > _target->size)
			return (EINVAL);
		copy_size = (size_t)_size;
	}
	if (copy_size > SIZE_MAX - sizeof(t_mem_node))
		return (EOVERFLOW);
	_new = _mem_new_node(_manager, copy_size);
	if (!_new)
		return (errno ? errno : ENOMEM);
	memcpy(_new->content, _ptr, copy_size);
	_mem_register_node(_manager, _new);
	*_output = _new->content;
	return (mem_manager_error_none);
}

int	_mem_join(
	t_mem_manager *const _manager,
	const void *const _ptr1,
	const void *const _ptr2,
	const ssize_t _size1,
	const ssize_t _size2,
	void **const result
)
{
	const t_mem_node *node1;
	const t_mem_node *node2;
	size_t size1;
	size_t size2;
	int status;

	if (!result)
		return (EINVAL);
	*result = NULL;
	if (!_ptr1 || !_ptr2 || _size1 < -1 || _size2 < -1)
		return (EINVAL);
	node1 = _mem_find_node(_manager, _ptr1);
	node2 = _mem_find_node(_manager, _ptr2);
	if ((_size1 == -1 && !node1) || (_size2 == -1 && !node2))
		return (EINVAL);
	size1 = _size1 == -1 ? node1->size : (size_t)_size1;
	size2 = _size2 == -1 ? node2->size : (size_t)_size2;
	if (size1 == 0 || size2 == 0 || (node1 && size1 > node1->size)
		|| (node2 && size2 > node2->size))
		return (EINVAL);
	if (size1 > SIZE_MAX - size2)
		return (EOVERFLOW);
	status = _mem_alloc(_manager, size1 + size2, result);
	if (status != 0)
		return (status);
	memcpy(*result, _ptr1, size1);
	memcpy((char *)*result + size1, _ptr2, size2);
	return (0);
}

/**
 * @brief	free _ptr and remove the linked node from the tree
 * 
 * @param	_manager	the manager data struct
 * @param	_ptr		the pointer to be reallocated
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
int	_mem_free(
	t_mem_manager *const _manager,
	void *_target
)
{
	t_mem_node	*_node;

	if (!_target)
		return (0);
	_node = _mem_find_node(_manager, _target);
	if (unlikely(!_node))
		return (EINVAL);
	else
		return (_mem_unregister_node(_manager, _node, 1));
}

/**
 * @brief	free all the stored memory
 * 
 * @param	_manager	the manager data struct
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
int	_mem_free_all(
	t_mem_manager *const _manager
)
{
	t_mem_node	*_current;
	t_mem_node	*_next;
	int			_i = 0;

	while (_i < MEM_BUCKET_SIZE)
	{
		_current = _manager->bucket[_i];
		while (_current)
		{
			_next = _current->next;
			_manager->allocator.free(_current);
			_current = _next;
		}
		_manager->bucket[_i] = NULL;
		_i++;
	}
	_manager->allocator = (t_mem_allocator){0};
	return (mem_manager_error_none);
}

# elif MEM_MANAGER_TYPE == 1

/**
 * @brief	set *_output to a pointer to a newly allocated area and register the node into the manager
 * 
 * @param	_manager	the manager data struct
 * @param	_size		the allocation size
 * @param	_output		the output pointer
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
int	_mem_alloc(
	t_mem_manager *const _manager,
	const size_t _size,
	void **_output
)
{
	rb_node	*_node = NULL;

	if (!_output || _size == 0)
		return (EINVAL);
	*_output = NULL;
	if (_size > SIZE_MAX - sizeof(rb_node))
		return (EOVERFLOW);
	_node = rb_create_node(_manager->T, _size);
	if (unlikely(!_node))
		return (errno ? errno : ENOMEM);

	rb_insert(_manager->T, _node);
	*_output = _node->key;
	return (mem_manager_error_none);
}

/**
 * @brief	reallocat _ptr to _size and register/unregister pointer.
 * 
 * @param	_manager	the manager data struct
 * @param	_ptr		the pointer to be reallocated
 * @param	_size		the allocation size
 * @param	_output		the output pointer
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
int	_mem_realloc(
	t_mem_manager *_manager,
	void *_ptr,
	const size_t _size,
	void **_output
)
{
	rb_node		*_node;
	rb_node		*_new = NULL;

	if (!_output)
		return (EINVAL);
	*_output = NULL;
	if (!_ptr)
		return (_mem_alloc(_manager, _size, _output));
	_node = rb_search(_manager->T, _ptr);
	if (!_node)
		return (EINVAL);
	if (_size == 0)
	{
		rb_delete(_manager->T, _node, 1);
		return (0);
	}
	if (_size > SIZE_MAX - sizeof(rb_node))
		return (EOVERFLOW);
	_new = rb_create_node(_manager->T, _size);
	if (unlikely(!_new))
		return (errno ? errno : ENOMEM);
	memcpy(_new->key, _node->key,
		_node->size < _size ? _node->size : _size);
	rb_insert(_manager->T, _new);
	rb_delete(_manager->T, _node, true);
	*_output = _new->key;
	return (0);
}

/**
 * @brief	duplicate _ptr based on _size or the stored size
 * 
 * @param	_manager	the manager data struct
 * @param	_ptr		the pointer to be reallocated
 * @param	_size		the allocation size
 * @param	_output		the output pointer
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
int	_mem_dup(
	t_mem_manager *const _manager,
	const void *const _ptr,
	const ssize_t _size,
	void **_output
)
{
	rb_node			*_target = NULL;
	rb_node			*_new = NULL;
	size_t			_copy_size;

	if (!_output)
		return (EINVAL);
	*_output = NULL;
	if (!_ptr || _size < -1 || _size == 0)
		return (EINVAL);
	_target = rb_search(_manager->T, (void *)_ptr);
	if (_size == -1)
	{
		if (unlikely(!_target))
			return (EINVAL);
		_copy_size = _target->size;
	}
	else
	{
		if (_target && (size_t)_size > _target->size)
			return (EINVAL);
		_copy_size = (size_t)_size;
	}
	if (_copy_size > SIZE_MAX - sizeof(rb_node))
		return (EOVERFLOW);
	_new = rb_create_node(_manager->T, _copy_size);
	if (!_new)
		return (errno ? errno : ENOMEM);
	memcpy(_new->key, _ptr, _copy_size);
	rb_insert(_manager->T, _new);
	*_output = _new->key;
	return (mem_manager_error_none);
}

/**
 * @brief	will join two memory area into one new one
 * 
 * if any size is negative, it will use the manager stored size instead
 * 
 * @param	manager	the memory manager
 * @param	ptr1	the first pointer to be joined
 * @param	ptr2	the seconde pointer to be joined
 * @param	size1	the size of the first memory area
 * @param	size2	the size of the second memory area
 * @param	result	the pointer for the resulted allocation
 * 
 * @return	error_none or the corresponding error code
 * @retval		`error_none`: no error
 * @retval		`error_invalid_arg`: invalid size with NULL pointer
 * @retval		all `_mem_alloc()` errors
 * 
 * @version	1.0.0
*/
int	_mem_join(
	t_mem_manager *const _manager,
	const void *const _ptr1,
	const void *const _ptr2,
	const ssize_t _size1,
	const ssize_t _size2,
	void **const result
)
{
	const rb_node	*_node1;
	const rb_node	*_node2;
	size_t			_s1 = 0;
	size_t			_s2 = 0;
	void			*_dummy = NULL;
	int				error = 0;

	if (!result)
		return (EINVAL);
	*result = NULL;
	if (!_ptr1 || !_ptr2 || _size1 < -1 || _size2 < -1)
		return (EINVAL);
	_node1 = rb_search(_manager->T, (void *)_ptr1);
	_node2 = rb_search(_manager->T, (void *)_ptr2);

	if (unlikely((!_node1 && _size1 < 0) || (!_node2 && _size2 < 0)))
	{
		error = EINVAL;
		goto cleannup;
	}

	if (_size1 < 0)
		_s1 = _node1->size;
	else
		_s1 = _size1;
	if (_size2 < 0)
		_s2 = _node2->size;
	else
		_s2 = _size2;
	if (_s1 == 0 || _s2 == 0 || (_node1 && _s1 > _node1->size)
		|| (_node2 && _s2 > _node2->size))
		return (EINVAL);
	if (_s1 > SIZE_MAX - _s2)
		return (EOVERFLOW);

	error = _mem_alloc(_manager, _s1 + _s2, &_dummy);
	if (unlikely(error))
		goto cleannup;

	memcpy(_dummy, _ptr1, _s1);
	memcpy((char *)_dummy + _s1, _ptr2, _s2);
	*result = _dummy;

cleannup:
	return (error);
}

/**
 * @brief	free _ptr and remove the linked node from the tree
 * 
 * @param	_manager	the manager data struct
 * @param	_ptr		the pointer to be reallocated
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
int	_mem_free(
	t_mem_manager *const _manager,
	void *_target
)
{
	rb_node	*_node = rb_search(_manager->T, _target); // _mem_find_node(_manager, _target);

	if (!_target)
		return (0);
	if (unlikely(!_node))
		return (EINVAL);
	else
		return (rb_delete(_manager->T, _node, true), error_none);
		// return (_mem_unregister_node(_manager, _node, 1));
}

/**
 * @brief	recursivly free the rb tree
 * 
 * @param	_node	the current node to free
 * 
 * @return	void
 * 
 * @visibility	local
 * 
 * @version 1.0.0
*/
static void	_mem_free_all_recur(
	rb_tree *T,
	rb_node *_node
)
{
	if (unlikely(_node == T->nil))
		return ;

	_mem_free_all_recur(T, _node->left);
	_mem_free_all_recur(T, _node->right);
	T->allocator.free(_node);
}

/**
 * @brief	free all the stored memory
 * 
 * @param	_manager	the manager data struct
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
int	_mem_free_all(
	t_mem_manager *const _manager
)
{
	if (unlikely(!_manager->T))
		return (0);

	_mem_free_all_recur(_manager->T, _manager->T->root);
	_manager->T->allocator.free(_manager->T->nil);
	_manager->T->allocator.free(_manager->T);
	_manager->T = NULL;
	return (error_none);
}

#else

#error "MEM_MANAGER_TYPE is invalide"

#endif

/* ----| Public     |----- */

/**
 * @brief	manager all the memory access for allocation, reallocation and free
 * 
 * @param	_access	the type of access
 * @param	_output	the pointer to the output value
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
int	_mem_manager(
	const int _access,
	void *_output,
	...
)
{
	static t_mem_manager	manager = {0};
	#if MEM_MANAGER_TYPE == 1
	t_mem_allocator allocator;
	#endif
	int						out;
	va_list					_list;

	#if MEM_MANAGER_TYPE == 0
	if (!manager.allocator.alloc)
		manager.allocator = (t_mem_allocator){G_SETTINGS.allocator.alloc,
			G_SETTINGS.allocator.free};
	if (!manager.allocator.alloc || !manager.allocator.free)
		return (EINVAL);
	#endif

	#if MEM_MANAGER_TYPE == 1

	if (!manager.T && _access != mem_manager_access_free_all
		&& _access != mem_manager_access_free)
	{
		allocator = (t_mem_allocator){G_SETTINGS.allocator.alloc,
			G_SETTINGS.allocator.free};
		if (!allocator.alloc || !allocator.free)
			return (EINVAL);
		manager.T = rb_init(&allocator);
		if (!manager.T)
		{
			if (_output)
				*(void **)_output = NULL;
			return (errno ? errno : ENOMEM);
		}
	}
	if (!manager.T && _access == mem_manager_access_free_all)
		return (0);
	if (!manager.T && _access == mem_manager_access_free)
	{
		va_start(_list, _output);
		out = va_arg(_list, void *) ? EINVAL : 0;
		va_end(_list);
		return (out);
	}
	
	#endif

	va_start(_list, _output);
	switch (_access)
	{
		case (mem_manager_access_alloc):
		{
			size_t	_size = va_arg(_list, size_t);

			out = _mem_alloc(&manager, _size, (void **)_output);
			break;
		}
		case (mem_manager_access_realloc):
		{
			void	*_ptr = va_arg(_list, void *);
			size_t	_size = va_arg(_list, size_t);

			out = _mem_realloc(&manager, _ptr, _size, (void **)_output);
			break;
		}
		case (mem_manager_access_join):
		{
			const void *const _ptr1 = va_arg(_list, const void *);
			const void *const _ptr2  = va_arg(_list, const void *);
			const ssize_t _size1 = va_arg(_list, ssize_t);
			const ssize_t _size2 = va_arg(_list, ssize_t);

			out = _mem_join(&manager, _ptr1, _ptr2, _size1, _size2, _output);
			break;
		}
		case (mem_manager_access_dup):
		{
			const void	*_ptr = va_arg(_list, const void *);
			ssize_t	_size = va_arg(_list, ssize_t);

			out = _mem_dup(&manager, _ptr, _size, (void **)_output);
			break ;
		}
		case (mem_manager_access_free):
		{
			void	*_target = va_arg(_list, void *);

			out = _mem_free(&manager, _target);
			break;
		}
		case (mem_manager_access_free_all):
			out = _mem_free_all(&manager);
			break;
		default:
			out = error_invalid_arg;
	}

	va_end(_list);
	return (out);
}
