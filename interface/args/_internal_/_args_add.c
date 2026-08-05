/**
 * @file _args_add.c
 * @brief Internal implementation for the interface/args module.
 */

// Header

/* ----| Headers    |----- */
	/* Standard */
#include <string.h>

	/* Internal */
#include "_args.h"

	/* External */
		//...

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */

static bool	_options_match(
	const t_args_option	*const	left,
	const t_args_option	*const	right
)
{
	if (unlikely(!left || !right))
		return (false);

	if (left->long_name && right->long_name && strcmp(left->long_name, right->long_name) == 0)
		return (true);

	return (left->short_name && right->short_name && left->short_name == right->short_name);
}

static bool	_parser_has_option(
	const t_args_parser	*const	parser,
	const t_args_option	*const	option
)
{
	const t_args_option	*this;

	if (unlikely(!parser || !option))
		return (false);

	this = parser->options;
	while (this)
	{
		if (_options_match(this, option))
			return (true);

		this = this->next;
	}

	return (false);
}

static void	_inherit_options(
	const t_args_parser	*const	parent,
	t_args_parser		*const	child
)
{
	if (unlikely(!parent || !child))
		return ;

	for (const t_args_option *option = parent->options;
		option;
		option = option->next)
	{
		if (_OPT_HAS_HERITANCE(option) && !_parser_has_option(child, option))
			_args_parser_add_option(child, option->long_name, option->short_name, option->desc, option->spec);
	}
}

static void	_add_option(
	t_args_parser	*const	parent,
	t_args_option	*const	new
)
{
	t_args_option	*last = NULL;

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
			if (!_parser_has_option(this, new))
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
	_inherit_options(_parent, new);

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