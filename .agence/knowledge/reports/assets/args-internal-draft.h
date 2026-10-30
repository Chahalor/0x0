/**
 * @file args-internal-draft.h
 * @brief Standalone C11 architecture sketch for R-002, not production API.
 * @details Layouts are proposals. Public handles are opaque in the eventual
 * public types.h; their definitions and all storage helpers stay private.
 * No implementation, error enum or public function declarations are provided.
 */
#ifndef TA_R002_ARGS_INTERNAL_DRAFT_H
# define TA_R002_ARGS_INTERNAL_DRAFT_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include <stdatomic.h>

/* -----| Opaque handles and private owners |----- */
typedef struct s_args_schema		t_args_schema;
typedef struct s_args_parser		t_args_parser;
typedef struct s_args_option		t_args_option;
typedef struct s_args_param		t_args_param;
typedef struct s_args_result		t_args_result;
typedef struct s_args_invocation	t_args_invocation;
typedef struct s_args_occurrence	t_args_occurrence;
typedef struct s_args_diagnostic	t_args_diagnostic;
typedef struct s_args_compiled		t_args_compiled;

typedef enum e_args_kind
{
	ARGS_KIND_STR,
	ARGS_KIND_PATH,
	ARGS_KIND_BOOL,
	ARGS_KIND_I64,
	ARGS_KIND_U64,
	ARGS_KIND_F64,
	ARGS_KIND_CUSTOM
} t_args_kind;

typedef enum e_args_repeat
{
	ARGS_REPEAT_ERROR,
	ARGS_REPEAT_FIRST,
	ARGS_REPEAT_LAST,
	ARGS_REPEAT_APPEND
} t_args_repeat;

typedef enum e_args_action
{
	ARGS_ACTION_VALUES,
	ARGS_ACTION_FLAG,
	ARGS_ACTION_COUNT,
	ARGS_ACTION_HELP,
	ARGS_ACTION_VERSION
} t_args_action;

typedef enum e_args_scope
{
	ARGS_SCOPE_LOCAL,
	ARGS_SCOPE_DESCENDANTS
} t_args_scope;

typedef enum e_args_result_kind
{
	ARGS_RESULT_RUN,
	ARGS_RESULT_HELP,
	ARGS_RESULT_VERSION
} t_args_result_kind;

typedef enum e_args_source
{
	ARGS_SOURCE_CLI,
	ARGS_SOURCE_DEFAULT,
	ARGS_SOURCE_IMPLICIT
} t_args_source;

typedef enum e_args_unknown_policy
{
	ARGS_UNKNOWN_ERROR,
	ARGS_UNKNOWN_STOP
} t_args_unknown_policy;

typedef enum e_args_command_mode
{
	ARGS_COMMANDS_SINGLE,
	ARGS_COMMANDS_SEQUENCE
} t_args_command_mode;

/** @brief SINGLE by default; SEQUENCE owns a configured delimiter string. */
typedef struct s_args_sequence_policy
{
	t_args_command_mode	mode;
	char				*separator;
} t_args_sequence_policy;

/* -----| Common descriptors |----- */
/** @brief SIZE_MAX is the unbounded maximum, never an allocation size. */
typedef struct s_args_bounds
{
	size_t	min;
	size_t	max;
} t_args_bounds;

typedef struct s_args_slice
{
	size_t	first;
	size_t	count;
} t_args_slice;

/** @brief Exact-sized owned list of copied strings. */
typedef struct s_args_literals
{
	char	**items;
	size_t	count;
} t_args_literals;

/** @brief Owned definition text; aliases exclude the canonical name. */
typedef struct s_args_names
{
	char			*name;
	char			*description;
	t_args_literals	aliases;
	bool			hidden;
	bool			deprecated;
} t_args_names;

/** @brief argv_index is -1 for defaults; offset identifies bundled flags. */
typedef struct s_args_origin
{
	t_args_source	source;
	int				argv_index;
	size_t			token_offset;
	size_t			token_length;
} t_args_origin;

/** @brief One conversion temporary, never the layout of a numeric array. */
typedef union u_args_scalar
{
	const char	*string;
	bool		boolean;
	int64_t		i64;
	uint64_t	u64;
	double		f64;
	void		*custom;
} t_args_scalar;

/** @brief Only the member selected by the parameter kind is active. */
typedef union u_args_array
{
	const char	**strings;
	bool		*booleans;
	int64_t		*i64;
	uint64_t	*u64;
	double		*f64;
	void		**custom;
} t_args_array;

/**
 * @brief Owns the typed allocation and each string/custom element once.
 * @details The owning parameter supplies kind and custom destructor.
 * Count covers only fully initialized elements; capacity is internal.
 */
typedef struct s_args_value_buffer
{
	t_args_array	data;
	size_t			count;
	size_t			capacity;
} t_args_value_buffer;

/** @brief Copied callback descriptor; identity/context remain borrowed. */
typedef struct s_args_custom_type
{
	const void	*identity;
	int			(*convert)(const char *text, void **out, void *context,
					t_args_diagnostic *diagnostic);
	void		(*destroy)(void *value, void *context);
	void		*context;
} t_args_custom_type;

typedef struct s_args_validator
{
	int		(*check)(t_args_kind kind, const t_args_scalar *value,
				void *context, t_args_diagnostic *diagnostic);
	void	*context;
} t_args_validator;

/** @brief Type-selected bounds; flags distinguish absent bounds from zero. */
typedef struct s_args_constraints
{
	bool	has_min;
	bool	has_max;
	union
	{
		struct { int64_t min; int64_t max; }	i64;
		struct { uint64_t min; uint64_t max; }	u64;
		struct { double min; double max; }		f64;
		struct { size_t min; size_t max; }		bytes;
	} range;
	unsigned int		path_checks;
	t_args_literals		choices;
	t_args_validator	*validators;
	size_t				validator_count;
} t_args_constraints;

typedef struct s_args_conversion
{
	unsigned int	integer_base;
	bool			allow_base_prefix;
	bool			allow_nan;
	bool			allow_infinity;
	t_args_literals	true_literals;
	t_args_literals	false_literals;
} t_args_conversion;

/** @brief Complete command grammar; builders initialize defaults explicitly. */
typedef struct s_args_grammar
{
	bool					long_attached;
	bool					short_attached;
	bool					short_bundles;
	bool					intermixed_options;
	bool					require_subcommand;
	t_args_unknown_policy	unknown_options;
} t_args_grammar;

/* -----| Stable definition nodes, individually allocated |----- */
struct s_args_param
{
	t_args_schema		*schema;
	size_t				slot;
	t_args_parser		*command;
	t_args_option		*option;
	size_t				owner_index;
	char				*name;
	char				*description;
	char				*metavar;
	t_args_kind			kind;
	t_args_bounds		arity;
	t_args_conversion	conversion;
	t_args_constraints	constraints;
	t_args_literals		defaults;
	t_args_literals		implicit;
	bool				has_default;
	bool				has_implicit;
	bool				allow_dash_values;
	bool				stop_at_command;
	bool				remainder;
	t_args_custom_type	custom;
};

struct s_args_option
{
	t_args_schema	*schema;
	size_t			slot;
	t_args_parser	*command;
	t_args_names	names;
	char			*short_names;
	t_args_scope	scope;
	t_args_action	action;
	t_args_repeat	repeat;
	t_args_bounds	occurrences;
	bool			flag_default;
	t_args_param	**params;
	size_t			param_count;
	size_t			param_capacity;
};

typedef enum e_args_rule_kind
{
	ARGS_RULE_REQUIRES,
	ARGS_RULE_AT_MOST_ONE,
	ARGS_RULE_EXACTLY_ONE,
	ARGS_RULE_AT_LEAST_ONE
} t_args_rule_kind;

typedef enum e_args_subject_kind
{
	ARGS_SUBJECT_OPTION,
	ARGS_SUBJECT_PARAM
} t_args_subject_kind;

typedef struct s_args_subject
{
	t_args_subject_kind	kind;
	union
	{
		const t_args_option	*option;
		const t_args_param	*param;
	} node;
} t_args_subject;

/** @brief REQUIRES uses members[0] as antecedent, all others as consequents. */
typedef struct s_args_rule
{
	t_args_rule_kind	kind;
	bool			use_effective_presence;
	t_args_subject	*members;
	size_t			member_count;
} t_args_rule;

struct s_args_parser
{
	t_args_schema	*schema;
	size_t			slot;
	t_args_parser	*parent;
	uint32_t		app_id;
	t_args_names	names;
	t_args_grammar	grammar;
	t_args_parser	**children;
	size_t			child_count;
	size_t			child_capacity;
	t_args_option	**options;
	size_t			option_count;
	size_t			option_capacity;
	t_args_param	**positionals;
	size_t			positional_count;
	size_t			positional_capacity;
	t_args_rule		*rules;
	size_t			rule_count;
	int				(*validate)(const t_args_invocation *invocation,
						const t_args_parser *command, void *context,
						t_args_diagnostic *diagnostic);
	int				(*handler)(const t_args_invocation *invocation,
						const t_args_parser *command, void *context);
	void			*validate_context;
	void			*handler_context;
};

/* -----| Transactionally published seal data |----- */
typedef struct s_args_long_binding
{
	const char		*name;
	const t_args_option	*option;
} t_args_long_binding;

typedef struct s_args_short_binding
{
	unsigned char		name;
	const t_args_option	*option;
} t_args_short_binding;

typedef struct s_args_command_binding
{
	const char		*name;
	const t_args_parser	*command;
} t_args_command_binding;

/** @brief count + 1 checked suffix sums; min_suffix[count] == 0. */
typedef struct s_args_param_plan
{
	size_t	*min_suffix;
	size_t	count;
} t_args_param_plan;

/** @brief Sorted visible bindings; inherited entries reference original nodes. */
typedef struct s_args_command_index
{
	t_args_long_binding		*long_options;
	size_t					long_count;
	t_args_short_binding	*short_options;
	size_t					short_count;
	t_args_command_binding	*children;
	size_t					child_count;
	t_args_param_plan		positionals;
} t_args_command_index;

struct s_args_compiled
{
	t_args_command_index	*commands;
	t_args_value_buffer		*choices;
	t_args_param_plan		*option_params;
};

/**
 * @brief Sole definition owner. Registries own nodes; hierarchy borrows them.
 * @details Dense slots are assigned at successful insertion, never reordered.
 * compiled == NULL means mutable; non-NULL means permanently sealed.
 */
struct s_args_schema
{
	atomic_size_t		references;
	t_args_parser		*root;
	t_args_parser		**commands;
	size_t				command_count;
	size_t				command_capacity;
	t_args_option		**options;
	size_t				option_count;
	size_t				option_capacity;
	t_args_param		**params;
	size_t				param_count;
	size_t				param_capacity;
	t_args_compiled		*compiled;
	t_args_sequence_policy	sequence;
};

/* -----| Result: dense slots, contiguous values and occurrence slices |----- */
typedef struct s_args_param_result
{
	bool				active;
	t_args_value_buffer	values;
	t_args_origin		*origins;
	t_args_slice		effective;
} t_args_param_result;

struct s_args_occurrence
{
	const t_args_invocation	*invocation;
	const t_args_option	*option;
	t_args_origin		origin;
	t_args_slice		*params;
};

typedef struct s_args_option_result
{
	bool				active;
	bool				effective_flag;
	t_args_occurrence	*occurrences;
	size_t				count;
	size_t				capacity;
} t_args_option_result;

typedef struct s_args_remaining_token
{
	char			*text;
	t_args_origin	origin;
} t_args_remaining_token;

/** @brief One execution scope; owns its values, borrows the parent result. */
struct s_args_invocation
{
	const t_args_result		*result;
	const t_args_parser		**command_path;
	size_t					command_count;
	t_args_param_result		*params;
	t_args_option_result	*options;
	t_args_remaining_token	*remaining;
	size_t					remaining_count;
};

/** @brief Ordered stable invocations; retains schema once; immutable on return. */
struct s_args_result
{
	t_args_schema		*schema;
	t_args_result_kind	kind;
	t_args_invocation	**invocations;
	size_t				invocation_count;
	size_t				invocation_capacity;
};

/* -----| Per-call mutable engine; never stored in definitions |----- */
typedef struct s_args_runtime_config
{
	size_t	max_tokens;
	size_t	max_values;
	size_t	max_depth;
	size_t	max_invocations;
	size_t	max_storage_bytes;
} t_args_runtime_config;

typedef enum e_args_parse_mode
{
	ARGS_PARSE_COMMAND,
	ARGS_PARSE_OPTION_VALUES,
	ARGS_PARSE_REMAINDER
} t_args_parse_mode;

/** @brief Borrowed argv text, held only until cardinality assignment. */
typedef struct s_args_pending_token
{
	const char		*text;
	t_args_origin	origin;
} t_args_pending_token;

typedef struct s_args_token_buffer
{
	t_args_pending_token	*items;
	size_t					count;
	size_t					capacity;
} t_args_token_buffer;

/** @brief Keep tokens unassigned until this occurrence's boundary is known. */
typedef struct s_args_pending_option
{
	const t_args_option	*option;
	size_t				occurrence_index;
	t_args_token_buffer	values;
} t_args_pending_option;

typedef struct s_args_parse_context
{
	const t_args_schema		*schema;
	t_args_runtime_config	config;
	int						argc;
	const char *const		*argv;
	int						argv_index;
	size_t					token_offset;
	const t_args_parser		*command;
	size_t					invocation_index;
	t_args_token_buffer		positionals;
	t_args_pending_option	pending;
	t_args_parse_mode		mode;
	bool					options_enabled;
	bool					commands_enabled;
	size_t					values_used;
	size_t					storage_bytes;
	t_args_result			*draft;
	t_args_diagnostic		*diagnostic;
} t_args_parse_context;

#endif
