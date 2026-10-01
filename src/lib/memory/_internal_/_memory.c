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
	const void *const ptr
)
{
	return ((int)((uintptr_t)ptr % MEM_BUCKET_SIZE));
}

t_mem_node	*_mem_find_node(
	const t_mem_manager *const manager,
	const void *const ptr
)
{
	if (unlikely(!manager || !ptr))
		return (NULL);

	const int	_id = _hash(ptr);
	t_mem_node	*_node;

	_node = manager->bucket[_id];
	while (_node && _node->content != ptr)
		_node = _node->next;
	return (_node);
}

int	_mem_register_node(
	t_mem_manager *const manager,
	t_mem_node *_node
)
{
	int	_id;
	t_mem_node	*_current;

	if (unlikely(!manager || !_node))
		return (EINVAL);

	_id = _hash(_node->content);
	_current = manager->bucket[_id];
	_node->next = _current;
	manager->bucket[_id] = _node;
	return (mem_manager_error_none);
}

t_mem_node	*_mem_new_node(
	t_mem_manager *const manager,
	const size_t size
)
{
	t_mem_node *result = NULL;

	if (!manager || size == 0 || size > SIZE_MAX - sizeof(t_mem_node))
	{
		errno = size == 0 ? EINVAL : EOVERFLOW;
		return (NULL);
	}
	result = manager->allocator.alloc(size + sizeof(t_mem_node));
	if (unlikely(!result))
	{
		errno = ENOMEM;
		return (NULL);
	}
	memset(result, 0, size + sizeof(t_mem_node));
	result->content = (void *)(result + 1);
	result->size = size;
	result->next = NULL;
	return (result);
}

int	_mem_unregister_node(
	t_mem_manager *const manager,
	t_mem_node *_node,
	const int _must_free
)
{
	int	_id;
	t_mem_node	**link;

	if (unlikely(!manager || !_node))
		return (EINVAL);

	_id = _hash(_node->content);
	link = &manager->bucket[_id];
	while (*link && *link != _node)
		link = &(*link)->next;
	if (!*link)
		return (EINVAL);
	*link = _node->next;
	if (_must_free)
		manager->allocator.free(_node);
	return (mem_manager_error_none);
}

/**
 * @brief	set *_output to a pointer to a newly allocated area and register the node into the manager
 * 
 * @param	manager	the manager data struct
 * @param	size		the allocation size
 * @param	_output		the output pointer
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
int	mem_alloc__(
	t_mem_manager *const manager,
	const size_t size,
	void **_output
)
{
	t_mem_node	*_node = NULL;

	if (!_output || size == 0)
		return (EINVAL);
	*_output = NULL;
	if (size > SIZE_MAX - sizeof(t_mem_node))
		return (EOVERFLOW);
	_node = _mem_new_node(manager, size);
	if (unlikely(!_node))
		return (errno ? errno : ENOMEM);

	_mem_register_node(manager, _node);
	*_output = _node->content;
	return (mem_manager_error_none);
}

/**
 * @brief	reallocat ptr to size and register/unregister pointer.
 * 
 * @param	manager	the manager data struct
 * @param	ptr		the pointer to be reallocated
 * @param	size		the allocation size
 * @param	_output		the output pointer
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
int	mem_realloc__(
	t_mem_manager *manager,
	void *ptr,
	const size_t size,
	void **_output
)
{
	t_mem_node	*_node;
	t_mem_node	*_new;

	if (!_output)
		return (EINVAL);
	*_output = NULL;
	if (!ptr)
		return (mem_alloc__(manager, size, _output));
	_node = _mem_find_node(manager, ptr);
	if (!_node)
		return (EINVAL);
	if (size == 0)
		return (_mem_unregister_node(manager, _node, 1));
	if (size > SIZE_MAX - sizeof(t_mem_node))
		return (EOVERFLOW);
	_new = _mem_new_node(manager, size);
	if (!_new)
		return (errno ? errno : ENOMEM);
	memcpy(_new->content, _node->content,
		_node->size < size ? _node->size : size);
	_mem_register_node(manager, _new);
	_mem_unregister_node(manager, _node, 1);
	*_output = _new->content;
	return (0);
}

/**
 * @brief	duplicate ptr based on size or the stored size
 * 
 * @param	manager	the manager data struct
 * @param	ptr		the pointer to be reallocated
 * @param	size		the allocation size
 * @param	_output		the output pointer
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
int	mem_dup__(
	t_mem_manager *const manager,
	const void *const ptr,
	const ssize_t size,
	void **_output
)
{
	t_mem_node	*_target;
	t_mem_node	*_new;
	size_t copy_size;

	if (!_output)
		return (EINVAL);
	*_output = NULL;
	if (!ptr || size < -1 || size == 0)
		return (EINVAL);
	_target = _mem_find_node(manager, ptr);
	if (size == -1)
	{
		if (!_target)
			return (EINVAL);
		copy_size = _target->size;
	}
	else
	{
		if (_target && (size_t)size > _target->size)
			return (EINVAL);
		copy_size = (size_t)size;
	}
	if (copy_size > SIZE_MAX - sizeof(t_mem_node))
		return (EOVERFLOW);
	_new = _mem_new_node(manager, copy_size);
	if (!_new)
		return (errno ? errno : ENOMEM);
	memcpy(_new->content, ptr, copy_size);
	_mem_register_node(manager, _new);
	*_output = _new->content;
	return (mem_manager_error_none);
}

int	_mem_join(
	t_mem_manager *const manager,
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
	node1 = _mem_find_node(manager, _ptr1);
	node2 = _mem_find_node(manager, _ptr2);
	if ((_size1 == -1 && !node1) || (_size2 == -1 && !node2))
		return (EINVAL);
	size1 = _size1 == -1 ? node1->size : (size_t)_size1;
	size2 = _size2 == -1 ? node2->size : (size_t)_size2;
	if (size1 == 0 || size2 == 0 || (node1 && size1 > node1->size)
		|| (node2 && size2 > node2->size))
		return (EINVAL);
	if (size1 > SIZE_MAX - size2)
		return (EOVERFLOW);
	status = mem_alloc__(manager, size1 + size2, result);
	if (status != 0)
		return (status);
	memcpy(*result, _ptr1, size1);
	memcpy((char *)*result + size1, _ptr2, size2);
	return (0);
}

/**
 * @brief	free ptr and remove the linked node from the tree
 * 
 * @param	manager	the manager data struct
 * @param	ptr		the pointer to be reallocated
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
int	mem_free__(
	t_mem_manager *const manager,
	void *_target
)
{
	t_mem_node	*_node;

	if (!_target)
		return (0);
	_node = _mem_find_node(manager, _target);
	if (unlikely(!_node))
		return (EINVAL);
	else
		return (_mem_unregister_node(manager, _node, 1));
}

/**
 * @brief	free all the stored memory
 * 
 * @param	manager	the manager data struct
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
int	mem_free_all__(
	t_mem_manager *const manager
)
{
	t_mem_node	*_current;
	t_mem_node	*_next;
	int			_i = 0;

	while (_i < MEM_BUCKET_SIZE)
	{
		_current = manager->bucket[_i];
		while (_current)
		{
			_next = _current->next;
			manager->allocator.free(_current);
			_current = _next;
		}
		manager->bucket[_i] = NULL;
		_i++;
	}
	return (mem_manager_error_none);
}

# elif MEM_MANAGER_TYPE == 1

/**
 * @brief	set *_output to a pointer to a newly allocated area and register the node into the manager
 * 
 * @param	manager	the manager data struct
 * @param	size		the allocation size
 * @param	_output		the output pointer
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
void	*mem_alloc__(
	t_mem_manager *const	manager,
	const size_t			size
)
{
	rb_node	*_node = NULL;

	if (size > SIZE_MAX - sizeof(rb_node))
		return (EOVERFLOW);
	_node = rb_create_node(manager->T, size);
	if (unlikely(!_node))
		return (errno ? errno : ENOMEM);

	rb_insert(manager->T, _node);
	return (_node->key);
}

/**
 * @brief	reallocat ptr to size and register/unregister pointer.
 * 
 * @param	manager	the manager data struct
 * @param	ptr		the pointer to be reallocated
 * @param	size		the allocation size
 * @param	_output		the output pointer
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
void	*mem_realloc__(
	t_mem_manager	*manager,
	void			*ptr,
	const size_t	size
)
{
	rb_node	*_node;
	rb_node	*_new = NULL;

	if (!ptr)
		return (mem_alloc__(manager, size));

	_node = rb_search(manager->T, ptr);
	if (!_node)
		return (ptr);

	if (size == 0)
	{
		rb_delete(manager->T, _node, 1);
		return (NULL);
	}

	if (size > SIZE_MAX - sizeof(rb_node))
		return (NULL);

	_new = rb_create_node(manager->T, size);
	if (unlikely(!_new))
		return (NULL);

	memcpy(_new->key, _node->key, _node->size < size ? _node->size : size);
	rb_insert(manager->T, _new);
	rb_delete(manager->T, _node, true);
	return (_new->key);
}

/**
 * @brief	duplicate ptr based on size or the stored size
 * 
 * @param	manager	the manager data struct
 * @param	ptr		the pointer to be reallocated
 * @param	size		the allocation size
 * @param	_output		the output pointer
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
void	*mem_dup__(
	t_mem_manager	*const	manager,
	const void		*const	ptr,
	const ssize_t			size
)
{
	rb_node			*_target = NULL;
	rb_node			*_new = NULL;
	size_t			_copy_size;

	if (!ptr || size < -1 || size == 0)
		return (EINVAL);

	_target = rb_search(manager->T, (void *)ptr);
	if (size == -1)
	{
		if (unlikely(!_target))
			return (NULL);
		_copy_size = _target->size;
	}
	else
	{
		if (_target && (size_t)size > _target->size)
			return (NULL);
		_copy_size = (size_t)size;
	}

	if (_copy_size > SIZE_MAX - sizeof(rb_node))
		return (NULL);

	_new = rb_create_node(manager->T, _copy_size);
	if (!_new)
		return (errno ? errno : ENOMEM);

	memcpy(_new->key, ptr, _copy_size);
	rb_insert(manager->T, _new);
	return (_new->key);
}

/**
 * @brief	free ptr and remove the linked node from the tree
 * 
 * @param	manager	the manager data struct
 * @param	ptr		the pointer to be reallocated
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
int	mem_free__(
	t_mem_manager *const manager,
	void *_target
)
{
	rb_node	*_node = rb_search(manager->T, _target); // _mem_find_node(manager, _target);

	if (!_target)
		return (0);

	if (unlikely(!_node))
		return (EINVAL);
	else
		return (rb_delete(manager->T, _node, true), error_none);
		// return (_mem_unregister_node(manager, _node, 1));
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
static void	mem_free_all_recur__(
	rb_tree *T,
	rb_node *_node
)
{
	if (unlikely(_node == T->nil))
		return ;

	mem_free_all_recur__(T, _node->left);
	mem_free_all_recur__(T, _node->right);
	G_SETTINGS.allocator.free(_node);
}

/**
 * @brief	free all the stored memory
 * 
 * @param	manager	the manager data struct
 * 
 * @return	the error code
 * 
 * @visibility	local
 * 
 * @version 2.0.0
*/
int	mem_free_all__(
	t_mem_manager *const manager
)
{
	if (unlikely(!manager->T))
		return (0);

	mem_free_all_recur__(manager->T, manager->T->root);
	G_SETTINGS.allocator.free(manager->T->nil);
	G_SETTINGS.allocator.free(manager->T);
	manager->T = NULL;
	return (error_none);
}

void	mem_remove__(
	t_mem_manager	*const	manager,
	void			*const	ptr
)
{
	rb_node	*node = rb_search(manager->T, ptr);

	if (unlikely(!node))
		return ;

	rb_delete(manager->T, node, false);
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
void	*mem_manager_(
	const int	_access,
	void		*ptr,
	size_t		size
)
{
	static t_mem_manager	manager = {0};
	void					*out;

	#if MEM_MANAGER_TYPE == 1

	if (!manager.T && _access != mem_manager_access_free_all && _access != mem_manager_access_free)
	{
		manager.T = rb_init();
		if (!manager.T)
			return (NULL);
	}

	if (!manager.T && _access == mem_manager_access_free_all)
		return (NULL);

	if (!manager.T && _access == mem_manager_access_free)
		return (NULL);
	
	#endif

	switch (_access)
	{
		case (mem_manager_access_alloc):
		{
			out = mem_alloc__(&manager, size);
			break;
		}
		case (mem_manager_access_realloc):
		{

			out = mem_realloc__(&manager, ptr, size);
			break;
		}
		case (mem_manager_access_dup):
		{
			out = mem_dup__(&manager, ptr, size);
			break ;
		}
		case (mem_manager_access_remove):
		{
			mem_remove__(&manager, ptr);
			break ;
		}
		case (mem_manager_access_free):
		{
			out = mem_free__(&manager, ptr);
			break;
		}
		case (mem_manager_access_free_all):
			out = mem_free_all__(&manager);
			break;
		default:
			out = NULL;
	}

	return (out);
}
