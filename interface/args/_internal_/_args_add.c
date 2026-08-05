/**
 * @file _args_add.c
 * @brief Internal implementation for the interface/args module.
 */

// Header

/* ----| Headers    |----- */
	/* Standard */
		//...

	/* Internal */
#include "_args.h"

	/* External */
		//...

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */

static void	_add_option(
	t_args_parser	*const	parent,
	t_args_option	*const	new
)
{
	t_args_option	*last;

	if (unlikely(!parent || !new))
		return ;

	for (t_args_option	*_this = parent->options;
		_this;
		_this = _this->next
	)
	{
		last = _this;
	}

	if (!last)
		parent->options = new;
	else
		last->next = new;

	if (_OPT_HAS_HERITANCE(new) && parent->sub_parsers)
	{
		for (t_args_parser	*this = parent->sub_parsers;
			this;
			this = this->next
		)
		{
			_args_parser_add_option(this, new->long_name, new->short_name, new->desc, new->spec);
		}
		
	}
}

/* ----| Public     |----- */

void	_args_add_sub(
	t_args_parser	*const	_parent,
	t_args_parser	*const	new
)
{
	t_args_parser	*_last = NULL;

	if (unlikely(!_parent || !new))
		return ;

	for (t_args_parser	*_this = _parent->sub_parsers;
		_this;
		_this = _this->next
	)
	{
		_last = _this;
	}

	if (!_last)
		_parent->sub_parsers = new;
	else
		_last->next = new;
}

t_args_parser	*_args_parser_add_sub(
	t_args_parser *const _parent,
	const char *const _name,
	const char *const _desc
)
{
	t_args_parser	*new = NULL;

	new = _args_mem_new_parser(_name, _desc);
	if (unlikely(!new))
		goto error;

	_args_add_sub(_parent, new);

error:
	return (new);
}

t_args_option	*_args_parser_add_option(
	t_args_parser *const _parent,
	const char *const _long_name,
	const char _sname,
	const char *const _desc,
	const int _spec
)
{
	const char		*_lname = NULL;
	t_args_option	*_last = NULL;
	t_args_option	*new = NULL;

	if (_long_name && _long_name[0] == '-')
	{
		if (_long_name[1] == '-')
			_lname = _long_name + 2;
		else
			_lname = _long_name + 1;
	}
	else
		_lname = _long_name;

	new = _args_mem_new_option(_lname, _sname, _desc);
	if (unlikely(!new))
		goto error;

	new->spec = _spec;
	_add_option(_parent, new);

error:
	return (new);
}

t_args_param	*_args_parser_add_param(
	t_args_parser *const _parent,
	const char *const _name,
	const char *const _desc,
	const int _spec,
	const int _type
)
{
	t_args_param	*_last = NULL;
	t_args_param	*new = NULL;

	new = _args_mem_new_param(_name, _desc, _spec, _type);
	if (unlikely(!new))
		goto error;

	for (t_args_param	*_this = _parent->params;
		_this;
		_this = _this->next
	)
	{
		_last = _this;
	}

	if (!_last)
		_parent->params = new;
	else
		_last->next = new;

error:
	return (new);
}

t_args_param	*_args_option_add_param(
	t_args_option *const _parent,
	const char *const _name,
	const char *const _desc,
	const int _spec,
	const int _type
)
{
	t_args_param	*_last = NULL;
	t_args_param	*new = NULL;

	new = _args_mem_new_param(_name, _desc, _spec, _type);
	if (unlikely(!new))
		goto error;

	for (t_args_param	*_this = _parent->params;
		_this;
		_this = _this->next
	)
	{
		_last = _this;
	}

	if (!_last)
		_parent->params = new;
	else
		_last->next = new;

error:
	return (new);
}