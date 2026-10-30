/**
 * @file args-api-draft.h
 * @brief Proposition de déclarations publiques pour les structures approuvées.
 * @details Aucun corps de fonction, aucune macro d'exécution, aucune définition
 * de structure. Ce fichier de conception suppose les types publics déclarés
 * avant lui : handles opaques, enums, t_args_grammar, t_args_custom_type,
 * t_args_scalar, t_args_subject et t_args_origin du modèle approuvé.
 * À l'intégration, ces déclarations viendront de args/types.h ; ne jamais
 * inclure les définitions privées dans le header public de production.
 * Le brouillon se vérifie avec les types approuvés préinclus par le compilateur.
 *
 * Contrats communs :
 * - Les fonctions int retournent 0, un errno positif ou -ARGS_ERROR_*.
 * - Les constructeurs rendent NULL et positionnent errno : EINVAL, ENOMEM,
 *   EOVERFLOW, EEXIST ou EBUSY (schéma figé). Aucun singleton d'erreur.
 * - Les mutateurs sont transactionnels, refusent un schéma figé et copient
 *   les textes/tableaux fournis. Les contextes de callbacks restent empruntés.
 * - Un handle est stable pendant la vie de son schéma. Seule la racine se
 *   libère explicitement ; un résultat retient ce schéma jusqu'à sa libération.
 * - Les getters métier exigent une invocation RUN et un handle du même schéma,
 *   déclaré sur son chemin actif. Sinon : erreur, jamais absence silencieuse.
 * - Les getters n'allouent pas. Toutes les références rendues sont empruntées
 *   au résultat (ou au schéma pour ses métadonnées), et ne doivent pas être
 *   libérées. Les lectures sont possibles tant que le propriétaire reste vivant.
 * - Les sorties scalaires restent inchangées sur erreur. Les sorties listes
 *   sont remises à NULL/0 si leurs adresses sont valides. Une liste absente
 *   rend 0 avec NULL/0 ; un getter scalaire exige exactement une valeur.
 * - Les listes conservent l'ordre CLI. Les getters d'invocation appliquent
 *   l'agrégation ; les getters d'occurrence rendent cette occurrence complète.
 * - Aucun parse/getter/dispatch n'imprime ni ne termine le processus.
 *
 * Types non redéfinis ici : diagnostic et settings publics, dont le contrat
 * d'utilisation est donné ci-dessous. Leur disposition publique sera déclarée
 * dans types.h/setting.h lors de l'intégration.
 */
#ifndef TA_ARGS_API_DRAFT_H
# define TA_ARGS_API_DRAFT_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>

#include "args-internal-draft.h"

/* -----| Types de callbacks : mêmes signatures que les structures |----- */
typedef int (*t_args_handler_fn)(
	const t_args_invocation *invocation,
	const t_args_parser *command,
	void *context
);

typedef int (*t_args_command_validator_fn)(
	const t_args_invocation *invocation,
	const t_args_parser *command,
	void *context,
	t_args_diagnostic *diagnostic
);

typedef int (*t_args_value_validator_fn)(
	t_args_kind kind,
	const t_args_scalar *value,
	void *context,
	t_args_diagnostic *diagnostic
);

/** @brief Écrit tous les bytes (sans NUL ajouté) ou retourne un statut d'erreur. */
typedef int (*t_args_writer_fn)(
	const char *bytes,
	size_t length,
	void *context
);

/* -----| Construction et durée de vie |----- */

/**
 * @brief Créer une racine mutable nommée.
 * @details name non vide est copié. Aucune option implicite : déclarer HELP/VERSION
 * @details avec args_parser_add_option. Grammaire : options intercalées, formes attachées
 * @details et bundles activés, noms exacts, enfants facultatifs, inconnues refusées,
 * @details commandes multiples désactivées. Paramètres et options sont initialement vides.
 * @return Racine propriétaire ou NULL avec errno.
 * @par Exemple
 * `t_args_parser *cli = args_parser_new("cat");`
 */
t_args_parser *	args_parser_new(
	const char *name
);

/**
 * @brief Relâcher la référence de l'appelant à la racine.
 * @details NULL est accepté. Les résultats déjà créés restent valides. Ne pas passer
 * @details un enfant, ni réutiliser la référence relâchée.
 * @par Exemple
 * `args_parser_free(cli);`
 */
void	args_parser_free(
	t_args_parser *root
);

/**
 * @brief Détruire le résultat et toutes ses invocations.
 * @details NULL est accepté. Libère valeurs et objets custom avant la référence au
 * @details schéma. Toutes les vues, occurrences et invocations empruntées expirent.
 * @par Exemple
 * `args_result_free(result);`
 */
void	args_result_free(
	t_args_result *result
);

/**
 * @brief Ajouter une sous-commande.
 * @details name est non vide, sans préfixe '-'. description peut être NULL. Le handle
 * @details est emprunté au schéma ; ID initial 0, aucun handler ni validateur.
 * @return Handle stable ou NULL avec errno ; aucun nœud publié sur échec.
 * @par Exemple
 * `t_args_parser *setup = args_parser_add_sub(cli, "setup", "Installer");`
 */
t_args_parser *	args_parser_add_sub(
	t_args_parser *parent,
	const char *name,
	const char *description
);

/**
 * @brief Ajouter une option, un flag, un compteur ou une action intégrée.
 * @details Au moins un nom requis : long_name sans tirets ou short_name non nul.
 * @details Noms courts numériques interdits. description peut être NULL. Portée LOCAL,
 * @details occurrences 0..SIZE_MAX. VALUES : APPEND et paramètres à ajouter avant seal ;
 * @details FLAG : LAST, défaut false ; COUNT : APPEND ; HELP/VERSION : LAST, aucune valeur.
 * @details Les actions autres que VALUES ne possèdent pas de paramètres.
 * @return Handle stable ou NULL avec errno.
 * @par Exemple
 * `t_args_option *verbose = args_parser_add_option(cli, "verbose", 'v', NULL, ARGS_ACTION_COUNT);`
 */
t_args_option *	args_parser_add_option(
	t_args_parser *command,
	const char *long_name,
	char short_name,
	const char *description,
	t_args_action action
);

/**
 * @brief Ajouter un paramètre positionnel typé.
 * @details name non vide et unique parmi les positionnels ; description facultative.
 * @details Arity initiale 1..1, aucun défaut/choix/validateur, décimal, booléens true/false
 * @details et 1/0, NaN/Inf refusés, pas de vérification de fichier, valeurs avec tiret
 * @details inconnu refusées. CUSTOM requiert un codec avant seal. Les listes suivies
 * @details d'autres paramètres réservent leurs minima selon D-001.
 * @return Handle stable ou NULL avec errno.
 * @par Exemple
 * `t_args_param *files = args_parser_add_param(cli, "FILE", NULL, ARGS_KIND_PATH);`
 */
t_args_param *	args_parser_add_param(
	t_args_parser *command,
	const char *name,
	const char *description,
	t_args_kind kind
);

/**
 * @brief Ajouter un paramètre typé à une option VALUES.
 * @details Mêmes valeurs initiales que args_parser_add_param. name doit être unique
 * @details dans cette option. Une arity obligatoire n'impose pas la présence de l'option.
 * @return Handle stable ou NULL avec errno.
 * @par Exemple
 * `t_args_param *jobs = args_option_add_param(jobs_option, "N", NULL, ARGS_KIND_I64);`
 */
t_args_param *	args_option_add_param(
	t_args_option *option,
	const char *name,
	const char *description,
	t_args_kind kind
);

/* -----| Commande : texte, identité et comportement |----- */

/**
 * @brief Remplacer la description.
 * @details Texte copié ; NULL supprime la description, sans modifier le nom.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_parser_set_description(parser, "Description");`
 */
int	args_parser_set_description(
	t_args_parser *parser,
	const char *description
);

/**
 * @brief Remplacer la description.
 * @details Texte copié ; NULL supprime la description, sans modifier le nom.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_option_set_description(option, "Description");`
 */
int	args_option_set_description(
	t_args_option *option,
	const char *description
);

/**
 * @brief Remplacer la description.
 * @details Texte copié ; NULL supprime la description, sans modifier le nom.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_set_description(param, "Description");`
 */
int	args_param_set_description(
	t_args_param *param,
	const char *description
);

/**
 * @brief Ajouter un nom alternatif de commande.
 * @details alias non vide, copié. Même identité, ID et handler. Collisions vérifiées
 * @details au seal ; aucune résolution par abréviation.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_parser_add_alias(setup, "install");`
 */
int	args_parser_add_alias(
	t_args_parser *command,
	const char *alias
);

/**
 * @brief Ajouter un alias long, court ou les deux.
 * @details Au moins un nom fourni, sans tirets pour le nom long. Ajout atomique ;
 * @details les alias désignent la même définition, avec la même action.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_option_add_alias(verbose, "verbosity", '\0');`
 */
int	args_option_add_alias(
	t_args_option *option,
	const char *long_name,
	char short_name
);

/**
 * @brief Régler l'affichage et l'indication de dépréciation.
 * @details Les deux attributs sont false initialement. N'altère ni la reconnaissance
 * @details ni l'identité ; hidden omet l'entrée de l'aide, deprecated l'annote.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_parser_set_visibility(parser, false, true);`
 */
int	args_parser_set_visibility(
	t_args_parser *parser,
	bool hidden,
	bool deprecated
);

/**
 * @brief Régler l'affichage et l'indication de dépréciation.
 * @details Les deux attributs sont false initialement. N'altère ni la reconnaissance
 * @details ni l'identité ; hidden omet l'entrée de l'aide, deprecated l'annote.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_option_set_visibility(option, false, true);`
 */
int	args_option_set_visibility(
	t_args_option *option,
	bool hidden,
	bool deprecated
);

/**
 * @brief Associer un ID applicatif à une commande.
 * @details 0 signifie aucun ID ; les IDs non nuls doivent être uniques dans le
 * @details schéma au seal. Cet ID est distinct du slot interne.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_parser_set_id(setup, CMD_SETUP);`
 */
int	args_parser_set_id(
	t_args_parser *command,
	uint32_t id
);

/**
 * @brief Associer un handler invoqué uniquement par dispatch explicite.
 * @details handler NULL supprime le handler. Le contexte reste emprunté et doit
 * @details survivre aux résultats susceptibles d'exécuter le handler.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_parser_set_handler(setup, run_setup, app);`
 */
int	args_parser_set_handler(
	t_args_parser *command,
	t_args_handler_fn handler,
	void *context
);

/**
 * @brief Associer une validation croisée de commande.
 * @details validator NULL supprime la validation. Appel après conversions et règles,
 * @details pour chaque commande active, de la racine à la feuille de chaque invocation.
 * @details Contexte emprunté ; pas de mutation du résultat ni d'effet métier.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_parser_set_validator(setup, validate_setup, app);`
 */
int	args_parser_set_validator(
	t_args_parser *command,
	t_args_command_validator_fn validator,
	void *context
);

/**
 * @brief Copier les réglages de grammaire de la commande.
 * @details grammar obligatoire, copie sans allocation, inchangée sur échec. Utiliser
 * @details cette lecture avant de modifier un seul réglage avec set_grammar.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_parser_get_grammar(cli, &grammar);`
 */
int	args_parser_get_grammar(
	const t_args_parser *command,
	t_args_grammar *grammar
);

/**
 * @brief Remplacer la grammaire de cette commande.
 * @details grammar obligatoire et copiée. Pas de propagation implicite aux enfants.
 * @details La validation du schéma au seal vérifie les incompatibilités.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_parser_set_grammar(cli, &grammar);`
 */
int	args_parser_set_grammar(
	t_args_parser *command,
	const t_args_grammar *grammar
);

/**
 * @brief Configurer les commandes multiples à la racine.
 * @details SINGLE par défaut : separator doit être NULL. SEQUENCE : séparateur
 * @details non vide copié, sans collision syntaxique au seal. Chaque segment recommence
 * @details à la racine, avec ses propres valeurs ; après -- ou remainder, plus de séparation.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_parser_set_commands(cli, ARGS_COMMANDS_SEQUENCE, "::");`
 */
int	args_parser_set_commands(
	t_args_parser *root,
	t_args_command_mode mode,
	const char *separator
);

/* -----| Options : occurrences et portée |----- */

/**
 * @brief Borner les apparitions explicites d'une option.
 * @details min <= max ; SIZE_MAX signifie sans borne maximale, jamais une allocation.
 * @details Les defaults ne comptent pas comme occurrences. min >= 1 rend l'option requise.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_option_set_occurrences(output, 1, 1);`
 */
int	args_option_set_occurrences(
	t_args_option *option,
	size_t min,
	size_t max
);

/**
 * @brief Choisir l'agrégation des occurrences.
 * @details VALUES accepte ERROR/FIRST/LAST/APPEND. FLAG accepte ERROR/FIRST/LAST.
 * @details COUNT accepte APPEND uniquement ; HELP/VERSION acceptent LAST uniquement.
 * @details ERROR interdit une seconde occurrence et exige min <= 1 au seal ;
 * @details la borne max peut rester supérieure, sans permettre la répétition.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_option_set_repeat(include, ARGS_REPEAT_APPEND);`
 */
int	args_option_set_repeat(
	t_args_option *option,
	t_args_repeat repeat
);

/**
 * @brief Rendre l'option locale ou visible dans les descendants.
 * @details LOCAL par défaut. DESCENDANTS partage la définition et ses paramètres ;
 * @details aucune copie d'option, et un seul compteur par invocation.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_option_set_scope(verbose, ARGS_SCOPE_DESCENDANTS);`
 */
int	args_option_set_scope(
	t_args_option *option,
	t_args_scope scope
);

/**
 * @brief Régler le défaut d'une option FLAG.
 * @details Refusé pour les autres actions. Ne crée pas d'occurrence ; une présence
 * @details explicite rend le flag true. Aucun alias négatif implicite.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_option_set_default(enabled, true);`
 */
int	args_option_set_default(
	t_args_option *option,
	bool value
);

/* -----| Paramètres : cardinalité, defaults et contraintes |----- */

/**
 * @brief Borner les valeurs d'un paramètre.
 * @details min <= max, SIZE_MAX = sans borne. Pour une option, borne chaque occurrence.
 * @details Pour les positionnels, réserver les minima suivants avant de prendre le surplus.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_set_arity(files, 0, SIZE_MAX);`
 */
int	args_param_set_arity(
	t_args_param *param,
	size_t min,
	size_t max
);

/**
 * @brief Définir le nom affiché pour une valeur dans l'aide.
 * @details Texte non vide copié ; NULL rétablit l'affichage du nom du paramètre.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_set_metavar(jobs, "COUNT");`
 */
int	args_param_set_metavar(
	t_args_param *param,
	const char *metavar
);

/**
 * @brief Définir les valeurs utilisées en l'absence du paramètre.
 * @details Ne complète jamais une occurrence explicite incomplète. Un défaut doit
 * @details respecter l'arity et les contraintes au seal ; il ne satisfait pas une exigence
 * @details de présence explicite d'option.
 * @details Les chaînes sont copiées. NULL avec count=0 supprime le réglage ; non-NULL
 * @details avec count=0 configure explicitement une liste vide. Tout élément utilisé
 * @details est non-NULL ; une chaîne vide reste une valeur à valider.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_set_default(param, values, count);`
 */
int	args_param_set_default(
	t_args_param *param,
	const char *const values[],
	size_t count
);

/**
 * @brief Définir les valeurs d'une option présente sans valeur explicite.
 * @details Réservé au paramètre unique d'une option VALUES, d'arity 0..1.
 * @details Sans valeur attachée, le token suivant reste libre ; l'implicit fournit zéro
 * @details ou une valeur. Refusé pour un positional ou une option à plusieurs paramètres.
 * @details Les chaînes sont copiées. NULL avec count=0 supprime le réglage ; non-NULL
 * @details avec count=0 configure explicitement une liste vide. Tout élément utilisé
 * @details est non-NULL ; une chaîne vide reste une valeur à valider.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_set_implicit(param, values, count);`
 */
int	args_param_set_implicit(
	t_args_param *param,
	const char *const values[],
	size_t count
);

/**
 * @brief Limiter les valeurs à un ensemble de choix.
 * @details Tokens convertis au seal et comparés dans leur type natif. CUSTOM utilise
 * @details un validateur, pas une égalité arbitraire de pointeurs.
 * @details Les chaînes sont copiées. count=0 supprime les choix, que values soit
 * @details NULL ou non. Pour count>0, tableau et éléments sont non-NULL ; une chaîne
 * @details vide reste un choix explicite à convertir et valider.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_set_choices(param, values, count);`
 */
int	args_param_set_choices(
	t_args_param *param,
	const char *const values[],
	size_t count
);

/**
 * @brief Configurer des bornes numériques inclusives.
 * @details Réservé à I64 ; min <= max. Pour F64, bornes NaN interdites.
 * @details Les deux bornes remplacent les précédentes ; les contrôles de conversion
 * @details et de capacité du type restent toujours actifs.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_set_i64_range(param, 1, 64);`
 */
int	args_param_set_i64_range(
	t_args_param *param,
	int64_t min,
	int64_t max
);

/**
 * @brief Configurer des bornes numériques inclusives.
 * @details Réservé à U64 ; min <= max. Pour F64, bornes NaN interdites.
 * @details Les deux bornes remplacent les précédentes ; les contrôles de conversion
 * @details et de capacité du type restent toujours actifs.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_set_u64_range(param, 1, 64);`
 */
int	args_param_set_u64_range(
	t_args_param *param,
	uint64_t min,
	uint64_t max
);

/**
 * @brief Configurer des bornes numériques inclusives.
 * @details Réservé à F64 ; min <= max. Pour F64, bornes NaN interdites.
 * @details Les deux bornes remplacent les précédentes ; les contrôles de conversion
 * @details et de capacité du type restent toujours actifs.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_set_f64_range(param, 1, 64);`
 */
int	args_param_set_f64_range(
	t_args_param *param,
	double min,
	double max
);

/**
 * @brief Borner la longueur des chaînes ou chemins en octets.
 * @details STR/PATH uniquement ; min <= max. min=1 interdit le vide.
 * @details SIZE_MAX comme maximum supprime la limite supérieure métier.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_set_length(name, 1, 128);`
 */
int	args_param_set_length(
	t_args_param *param,
	size_t min,
	size_t max
);

/**
 * @brief Supprimer les bornes numériques ou de longueur configurées.
 * @details Ne désactive jamais la validation du type ou de sa capacité de stockage.
 * @details Ne supprime pas les choix ni les validateurs.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_clear_range(param);`
 */
int	args_param_clear_range(
	t_args_param *param
);

/**
 * @brief Régler la base des entiers.
 * @details I64/U64 uniquement ; bases 2, 8, 10, 16. Défaut 10, préfixe interdit.
 * @details Préfixes reconnus si activés : 0b/0o/0x pour la base correspondante ; aucun
 * @details passage implicite en octal à cause d'un zéro initial.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_set_integer_base(mask, 16, true);`
 */
int	args_param_set_integer_base(
	t_args_param *param,
	unsigned int base,
	bool allow_prefix
);

/**
 * @brief Autoriser explicitement NaN ou les infinis.
 * @details F64 uniquement ; tous deux interdits initialement. Des bornes numériques
 * @details configurées restent applicables ; NaN avec une borne configurée est refusé.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_set_float_specials(ratio, false, true);`
 */
int	args_param_set_float_specials(
	t_args_param *param,
	bool allow_nan,
	bool allow_infinity
);

/**
 * @brief Remplacer les vocabulaires textuels booléens.
 * @details BOOL uniquement ; deux listes non vides, copiées, disjointes, sans élément
 * @details NULL. Comparaison exacte ; aucun changement de locale. Défaut true/1 et false/0.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_set_bool_literals(param, yes, 2, no, 2);`
 */
int	args_param_set_bool_literals(
	t_args_param *param,
	const char *const true_values[],
	size_t true_count,
	const char *const false_values[],
	size_t false_count
);

/**
 * @brief Activer les contrôles d'un chemin.
 * @details PATH uniquement. Constantes publiques prévues dans types.h :
 * @details ARGS_PATH_EXISTS=1, ARGS_PATH_FILE=2, ARGS_PATH_DIRECTORY=4,
 * @details ARGS_PATH_READABLE=8, ARGS_PATH_WRITABLE=16. Zéro désactive les contrôles.
 * @details FILE/DIRECTORY sont
 * @details exclusifs et impliquent EXISTS. Aucun fichier ouvert/créé ; ne garantit pas
 * @details la réussite ultérieure d'une opération métier. Bits inconnus refusés.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_set_path_checks(path, ARGS_PATH_EXISTS | ARGS_PATH_READABLE);`
 */
int	args_param_set_path_checks(
	t_args_param *param,
	unsigned int checks
);

/**
 * @brief Configurer le convertisseur d'un type CUSTOM.
 * @details CUSTOM uniquement ; descripteur copié, identity/convert/destroy non NULL,
 * @details contexte emprunté. Convertisseur : transfert d'un objet non NULL sur succès,
 * @details nettoyage et sortie NULL sur échec. Destructeur appelé une seule fois par objet.
 * @details Le code, identity et contexte doivent survivre aux parsers et résultats.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_set_custom_type(uuid, &uuid_type);`
 */
int	args_param_set_custom_type(
	t_args_param *param,
	const t_args_custom_type *type
);

/**
 * @brief Ajouter une validation après conversion et contraintes natives.
 * @details validator non NULL. Ordre d'enregistrement, arrêt à la première erreur.
 * @details Valeur empruntée et non modifiable. Peut être appelé au seal pour les defaults,
 * @details puis au parse ; aucun effet métier. Contexte emprunté.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_add_validator(port, validate_port, app);`
 */
int	args_param_add_validator(
	t_args_param *param,
	t_args_value_validator_fn validator,
	void *context
);

/**
 * @brief Autoriser les tokens inconnus commençant par un tiret.
 * @details False initialement. N'empêche pas la reconnaissance d'une option connue.
 * @details Les valeurs attachées et le token '-' restent gérés par les règles de base.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_set_dash_values(param, true);`
 */
int	args_param_set_dash_values(
	t_args_param *param,
	bool enabled
);

/**
 * @brief Terminer une liste d'option devant une commande reconnue.
 * @details False initialement. Paramètre variable terminal d'une option uniquement.
 * @details Le minimum doit être atteint et la sélection d'un enfant permise à cet endroit.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_set_stop_at_command(param, true);`
 */
int	args_param_set_stop_at_command(
	t_args_param *param,
	bool enabled
);

/**
 * @brief Capturer les tokens restants comme valeurs littérales.
 * @details Positional STR uniquement, terminal, sans borne supérieure ; false initialement.
 * @details Rejeter les combinaisons incompatibles au seal. Aucun parsing d'expression,
 * @details d'option ou de séparateur de commandes après son activation.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_param_set_remainder(param, true);`
 */
int	args_param_set_remainder(
	t_args_param *param,
	bool enabled
);

/**
 * @brief Ajouter une contrainte de présence sur des handles.
 * @details Tableau copié. REQUIRES : premier sujet implique tous les suivants (count>=2).
 * @details AT_MOST_ONE exprime aussi un conflit ; groupes non vides et sans doublons.
 * @details false = présence CLI ; true = valeur effective. Les sujets doivent être valides
 * @details dans le scope de la règle au seal. Une dépendance ne crée aucune valeur.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_parser_add_rule(cli, ARGS_RULE_EXACTLY_ONE, outputs, 2, false);`
 */
int	args_parser_add_rule(
	t_args_parser *command,
	t_args_rule_kind kind,
	const t_args_subject members[],
	size_t count,
	bool effective_presence
);

/* -----| Finalisation et parsing |----- */

/**
 * @brief Valider et figer atomiquement la définition racine.
 * @details diagnostic facultatif, effacé à l'entrée. Échec : définition encore mutable,
 * @details aucun index partiel publié. Valide alias, IDs, arités, minima cumulés, règles,
 * @details defaults, codecs et collisions d'héritage. Appel répété après succès : 0.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_parser_seal(cli, diagnostic);`
 */
int	args_parser_seal(
	t_args_parser *root,
	t_args_diagnostic *diagnostic
);

/**
 * @brief Parser toute la CLI avec une définition figée.
 * @details Racine figée, argc>=1, éléments argv[0..argc) non NULL ; argv[0] est le nom
 * @details programme. result obligatoire, initialisé à NULL ; diagnostic facultatif effacé.
 * @details Échec : aucun résultat publié, aucun handler. Succès : toutes les invocations
 * @details validées, textes copiés et schéma retenu. L'appelant peut ensuite modifier argv.
 * @details HELP/VERSION renvoie un résultat de présentation, sans valeurs métier.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_parse(cli, argc, argv, &result, diagnostic);`
 */
int	args_parse(
	const t_args_parser *root,
	int argc,
	const char *const argv[],
	t_args_result **result,
	t_args_diagnostic *diagnostic
);

/* -----| Résultat, commandes et dispatch |----- */

/**
 * @brief Lire RUN, HELP ou VERSION.
 * @details kind obligatoire ; résultat non NULL. Une présentation ne peut pas être dispatchée.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_result_get_kind(result, &kind);`
 */
int	args_result_get_kind(
	const t_args_result *result,
	t_args_result_kind *kind
);

/**
 * @brief Lire le nombre d'invocations.
 * @details Ordre CLI, toujours au moins une sur succès. SINGLE et HELP/VERSION en
 * @details contiennent exactement une. count obligatoire.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_result_count(result, &count);`
 */
int	args_result_count(
	const t_args_result *result,
	size_t *count
);

/**
 * @brief Emprunter une invocation par son index, à partir de zéro.
 * @details Valide jusqu'à args_result_free. index=0 désigne l'invocation unique en SINGLE.
 * @return Invocation ou NULL avec errno (EINVAL/ERANGE).
 * @par Exemple
 * `const t_args_invocation *call = args_result_get(result, 0);`
 */
const t_args_invocation *	args_result_get(
	const t_args_result *result,
	size_t index
);

/**
 * @brief Emprunter la commande la plus profonde de l'invocation.
 * @details La racine est une commande sélectionnable. Disponible aussi pour l'aide.
 * @details Aucune recherche par nom ; durée de vie du schéma retenu par le résultat.
 * @return Commande ou NULL avec errno=EINVAL.
 * @par Exemple
 * `const t_args_parser *command = args_selected_command(call);`
 */
const t_args_parser *	args_selected_command(
	const t_args_invocation *invocation
);

/**
 * @brief Lire l'ID applicatif d'une commande.
 * @details 0 signifie qu'aucun ID n'a été attribué. Précondition : handle vivant.
 * @details NULL est invalide : retourne 0 et positionne errno=EINVAL.
 * @return ID configuré ; aucun changement de l'ID au parsing.
 * @par Exemple
 * `uint32_t id = args_command_id(args_selected_command(call));`
 */
uint32_t	args_command_id(
	const t_args_parser *command
);

/**
 * @brief Emprunter le nom canonique d'une commande.
 * @details Renvoie le même nom quel que soit l'alias utilisé dans argv. Ne pas libérer.
 * @return Nom ou NULL avec errno=EINVAL.
 * @par Exemple
 * `const char *name = args_command_name(command);`
 */
const char *	args_command_name(
	const t_args_parser *command
);

/**
 * @brief Lire la longueur du chemin actif, racine comprise.
 * @details count obligatoire ; profondeur minimale un.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_command_depth(call, &depth);`
 */
int	args_command_depth(
	const t_args_invocation *invocation,
	size_t *count
);

/**
 * @brief Emprunter une commande du chemin, zéro désignant la racine.
 * @details Disponible pour RUN et présentation. Le dernier niveau est selected_command.
 * @return Commande ou NULL avec errno=EINVAL/ERANGE.
 * @par Exemple
 * `const t_args_parser *root = args_command_at(call, 0);`
 */
const t_args_parser *	args_command_at(
	const t_args_invocation *invocation,
	size_t depth
);

/**
 * @brief Appeler explicitement le handler de la commande sélectionnée.
 * @details RUN uniquement, handler présent, sortie obligatoire. Le statut de cette
 * @details fonction décrit le dispatch ; handler_status reçoit le code métier, même
 * @details non nul. Aucun appel implicite aux parents ni à l'invocation suivante.
 * @details Deux appels explicites peuvent exécuter deux fois le handler.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_dispatch(call, &application_status);`
 */
int	args_dispatch(
	const t_args_invocation *invocation,
	int *handler_status
);

/* -----| Options présentes et occurrences |----- */

/**
 * @brief Vérifier une présence CLI explicite.
 * @details present obligatoire. Un default ne rend pas l'option présente. Branche
 * @details inactive ou schéma étranger : erreur, pas false silencieux.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_has_option(call, verbose, &present);`
 */
int	args_has_option(
	const t_args_invocation *invocation,
	const t_args_option *option,
	bool *present
);

/**
 * @brief Lire le nombre d'occurrences CLI de l'option.
 * @details count obligatoire ; comprend les répétitions et chaque lettre d'un bundle.
 * @details Indépendant de FIRST/LAST ; sert aussi de valeur pour une action COUNT.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_option_count(call, verbose, &verbosity);`
 */
int	args_option_count(
	const t_args_invocation *invocation,
	const t_args_option *option,
	size_t *count
);

/**
 * @brief Lire la valeur effective d'une option FLAG.
 * @details Refusé pour les autres actions. Rend le défaut si absente, true si présente.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_get_flag(call, enabled_option, &enabled);`
 */
int	args_get_flag(
	const t_args_invocation *invocation,
	const t_args_option *option,
	bool *value
);

/**
 * @brief Emprunter une occurrence, dans l'ordre CLI.
 * @details Toutes les occurrences sont accessibles, indépendamment de l'agrégation.
 * @details index commence à zéro ; résultat seul propriétaire.
 * @return Occurrence ou NULL avec errno : EINVAL (scope/kind/handle), ERANGE (index).
 * @par Exemple
 * `const t_args_occurrence *point = args_get_occurrence(call, points, 1);`
 */
const t_args_occurrence *	args_get_occurrence(
	const t_args_invocation *invocation,
	const t_args_option *option,
	size_t index
);

/* -----| Valeurs typées : scalaire exact ou tableau stable |----- */

/**
 * @brief Lire exactement une valeur STR/PATH.
 * @details Erreur si absente, multiple, mauvais type, branche inactive ou autre schéma.
 * @details La chaîne reste empruntée au résultat, sans nouvelle copie.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_get_string(call, param, &text);`
 */
int	args_get_string(
	const t_args_invocation *invocation,
	const t_args_param *param,
	const char * *value
);

/**
 * @brief Emprunter le tableau effectif STR/PATH, y compris pour zéro ou un élément.
 * @details values/count obligatoires, type vérifié. Pas de sentinelle de fin : count
 * @details fait autorité. Vide = NULL/0. Ordre CLI conservé selon l'agrégation choisie.
 * @details Tableau contigu du type indiqué, emprunté jusqu'à la libération du résultat.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_get_strings(call, param, &values, &count);`
 */
int	args_get_strings(
	const t_args_invocation *invocation,
	const t_args_param *param,
	const char *const * *values,
	size_t *count
);

/**
 * @brief Lire exactement une valeur BOOL.
 * @details Erreur si absente, multiple, mauvais type, branche inactive ou autre schéma.
 * @details Copie du scalaire déjà converti ; aucune conversion au getter.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_get_bool(call, param, &enabled);`
 */
int	args_get_bool(
	const t_args_invocation *invocation,
	const t_args_param *param,
	bool *value
);

/**
 * @brief Emprunter le tableau effectif BOOL, y compris pour zéro ou un élément.
 * @details values/count obligatoires, type vérifié. Pas de sentinelle de fin : count
 * @details fait autorité. Vide = NULL/0. Ordre CLI conservé selon l'agrégation choisie.
 * @details Tableau contigu du type indiqué, emprunté jusqu'à la libération du résultat.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_get_bools(call, param, &values, &count);`
 */
int	args_get_bools(
	const t_args_invocation *invocation,
	const t_args_param *param,
	const bool * *values,
	size_t *count
);

/**
 * @brief Lire exactement une valeur I64.
 * @details Erreur si absente, multiple, mauvais type, branche inactive ou autre schéma.
 * @details Copie du scalaire déjà converti ; aucune conversion au getter.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_get_i64(call, param, &jobs);`
 */
int	args_get_i64(
	const t_args_invocation *invocation,
	const t_args_param *param,
	int64_t *value
);

/**
 * @brief Emprunter le tableau effectif I64, y compris pour zéro ou un élément.
 * @details values/count obligatoires, type vérifié. Pas de sentinelle de fin : count
 * @details fait autorité. Vide = NULL/0. Ordre CLI conservé selon l'agrégation choisie.
 * @details Tableau contigu du type indiqué, emprunté jusqu'à la libération du résultat.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_get_i64s(call, param, &values, &count);`
 */
int	args_get_i64s(
	const t_args_invocation *invocation,
	const t_args_param *param,
	const int64_t * *values,
	size_t *count
);

/**
 * @brief Lire exactement une valeur U64.
 * @details Erreur si absente, multiple, mauvais type, branche inactive ou autre schéma.
 * @details Copie du scalaire déjà converti ; aucune conversion au getter.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_get_u64(call, param, &size);`
 */
int	args_get_u64(
	const t_args_invocation *invocation,
	const t_args_param *param,
	uint64_t *value
);

/**
 * @brief Emprunter le tableau effectif U64, y compris pour zéro ou un élément.
 * @details values/count obligatoires, type vérifié. Pas de sentinelle de fin : count
 * @details fait autorité. Vide = NULL/0. Ordre CLI conservé selon l'agrégation choisie.
 * @details Tableau contigu du type indiqué, emprunté jusqu'à la libération du résultat.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_get_u64s(call, param, &values, &count);`
 */
int	args_get_u64s(
	const t_args_invocation *invocation,
	const t_args_param *param,
	const uint64_t * *values,
	size_t *count
);

/**
 * @brief Lire exactement une valeur F64.
 * @details Erreur si absente, multiple, mauvais type, branche inactive ou autre schéma.
 * @details Copie du scalaire déjà converti ; aucune conversion au getter.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_get_f64(call, param, &ratio);`
 */
int	args_get_f64(
	const t_args_invocation *invocation,
	const t_args_param *param,
	double *value
);

/**
 * @brief Emprunter le tableau effectif F64, y compris pour zéro ou un élément.
 * @details values/count obligatoires, type vérifié. Pas de sentinelle de fin : count
 * @details fait autorité. Vide = NULL/0. Ordre CLI conservé selon l'agrégation choisie.
 * @details Tableau contigu du type indiqué, emprunté jusqu'à la libération du résultat.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_get_f64s(call, param, &values, &count);`
 */
int	args_get_f64s(
	const t_args_invocation *invocation,
	const t_args_param *param,
	const double * *values,
	size_t *count
);

/**
 * @brief Lire exactement une valeur CUSTOM.
 * @details Erreur si absente, multiple, mauvais type, branche inactive ou autre schéma.
 * @details Identité custom obligatoire et égale à celle du codec ; objet emprunté,
 * @details non modifiable et non libérable par l'appelant.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_get_custom(call, param, &my_type_identity, &object);`
 */
int	args_get_custom(
	const t_args_invocation *invocation,
	const t_args_param *param,
	const void *type_identity,
	const void * *value
);

/**
 * @brief Emprunter le tableau effectif CUSTOM, y compris pour zéro ou un élément.
 * @details values/count obligatoires, type vérifié. Pas de sentinelle de fin : count
 * @details fait autorité. Vide = NULL/0. Ordre CLI conservé selon l'agrégation choisie.
 * @details Identité custom obligatoire ; tableau et objets empruntés, à ne pas modifier.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_get_customs(call, param, &my_type_identity, &values, &count);`
 */
int	args_get_customs(
	const t_args_invocation *invocation,
	const t_args_param *param,
	const void *type_identity,
	void *const * *values,
	size_t *count
);

/* -----| Valeurs d'une occurrence : mêmes règles, sans agrégation |----- */

/**
 * @brief Emprunter les valeurs STR/PATH de cette occurrence.
 * @details param doit appartenir à l'option de cette occurrence, avec le type attendu.
 * @details values/count obligatoires. Même représentation pour zéro, une ou N valeurs ;
 * @details ordre d'origine conservé. Tableau emprunté, sans allocation ni conversion.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_occurrence_get_strings(occurrence, param, &values, &count);`
 */
int	args_occurrence_get_strings(
	const t_args_occurrence *occurrence,
	const t_args_param *param,
	const char *const * *values,
	size_t *count
);

/**
 * @brief Emprunter les valeurs BOOL de cette occurrence.
 * @details param doit appartenir à l'option de cette occurrence, avec le type attendu.
 * @details values/count obligatoires. Même représentation pour zéro, une ou N valeurs ;
 * @details ordre d'origine conservé. Tableau emprunté, sans allocation ni conversion.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_occurrence_get_bools(occurrence, param, &values, &count);`
 */
int	args_occurrence_get_bools(
	const t_args_occurrence *occurrence,
	const t_args_param *param,
	const bool * *values,
	size_t *count
);

/**
 * @brief Emprunter les valeurs I64 de cette occurrence.
 * @details param doit appartenir à l'option de cette occurrence, avec le type attendu.
 * @details values/count obligatoires. Même représentation pour zéro, une ou N valeurs ;
 * @details ordre d'origine conservé. Tableau emprunté, sans allocation ni conversion.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_occurrence_get_i64s(occurrence, param, &values, &count);`
 */
int	args_occurrence_get_i64s(
	const t_args_occurrence *occurrence,
	const t_args_param *param,
	const int64_t * *values,
	size_t *count
);

/**
 * @brief Emprunter les valeurs U64 de cette occurrence.
 * @details param doit appartenir à l'option de cette occurrence, avec le type attendu.
 * @details values/count obligatoires. Même représentation pour zéro, une ou N valeurs ;
 * @details ordre d'origine conservé. Tableau emprunté, sans allocation ni conversion.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_occurrence_get_u64s(occurrence, param, &values, &count);`
 */
int	args_occurrence_get_u64s(
	const t_args_occurrence *occurrence,
	const t_args_param *param,
	const uint64_t * *values,
	size_t *count
);

/**
 * @brief Emprunter les valeurs F64 de cette occurrence.
 * @details param doit appartenir à l'option de cette occurrence, avec le type attendu.
 * @details values/count obligatoires. Même représentation pour zéro, une ou N valeurs ;
 * @details ordre d'origine conservé. Tableau emprunté, sans allocation ni conversion.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_occurrence_get_f64s(occurrence, param, &values, &count);`
 */
int	args_occurrence_get_f64s(
	const t_args_occurrence *occurrence,
	const t_args_param *param,
	const double * *values,
	size_t *count
);

/**
 * @brief Emprunter les valeurs CUSTOM de cette occurrence.
 * @details param doit appartenir à l'option de cette occurrence, avec le type attendu.
 * @details values/count obligatoires. Même représentation pour zéro, une ou N valeurs ;
 * @details ordre d'origine conservé. Tableau emprunté, sans allocation ni conversion. Identité custom vérifiée ; objets non modifiables.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_occurrence_get_customs(occurrence, param, &my_type_identity, &values, &count);`
 */
int	args_occurrence_get_customs(
	const t_args_occurrence *occurrence,
	const t_args_param *param,
	const void *type_identity,
	void *const * *values,
	size_t *count
);

/* -----| Provenance et tokens non consommés |----- */

/**
 * @brief Copier l'origine d'une valeur de la liste effective.
 * @details index suit exactement la vue rendue par args_get_*s. Origine DEFAULT avec
 * @details argv_index=-1 pour un défaut ; CLI/IMPLICIT pour une occurrence explicite.
 * @details Sortie obligatoire, index hors liste = erreur.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_get_origin(call, param, 0, &origin);`
 */
int	args_get_origin(
	const t_args_invocation *invocation,
	const t_args_param *param,
	size_t index,
	t_args_origin *origin
);

/**
 * @brief Copier la position du token d'option de cette occurrence.
 * @details L'offset distingue les flags d'un bundle. Sortie obligatoire.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_occurrence_get_origin(occurrence, &origin);`
 */
int	args_occurrence_get_origin(
	const t_args_occurrence *occurrence,
	t_args_origin *origin
);

/**
 * @brief Copier l'origine d'une valeur à l'intérieur d'une occurrence.
 * @details Même contrôle de propriétaire que les getters typés ; index local à cette
 * @details occurrence, même si l'agrégation effective en retient une autre.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_occurrence_get_value_origin(occurrence, param, 0, &origin);`
 */
int	args_occurrence_get_value_origin(
	const t_args_occurrence *occurrence,
	const t_args_param *param,
	size_t index,
	t_args_origin *origin
);

/**
 * @brief Lire le nombre de tokens non consommés par UNKNOWN_STOP.
 * @details RUN uniquement. Distinct d'un positional remainder, qui est lu avec les
 * @details getters de paramètres. count obligatoire.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_remaining_count(call, &count);`
 */
int	args_remaining_count(
	const t_args_invocation *invocation,
	size_t *count
);

/**
 * @brief Emprunter un token non consommé dans l'ordre CLI.
 * @details Texte copié depuis argv et possédé par le résultat. origin est facultatif,
 * @details inchangé sur échec. Pas d'interprétation de l'arité d'une option inconnue.
 * @return Texte ou NULL avec errno=EINVAL/ERANGE.
 * @par Exemple
 * `const char *token = args_remaining_at(call, 0, &origin);`
 */
const char *	args_remaining_at(
	const t_args_invocation *invocation,
	size_t index,
	t_args_origin *origin
);

/* -----| Diagnostic, aide et convention d'erreur Genesis |----- */

/**
 * @brief Créer un diagnostic réutilisable à stockage interne borné.
 * @details Le stockage est alloué une fois ; les écritures ultérieures n'allouent pas.
 * @details Une erreur de parsing reste rapportée par son statut si ce diagnostic est NULL.
 * @return Diagnostic propriétaire ou NULL avec errno=ENOMEM.
 * @par Exemple
 * `t_args_diagnostic *diagnostic = args_diagnostic_new();`
 */
t_args_diagnostic *	args_diagnostic_new(
	void
);

/**
 * @brief Libérer un diagnostic.
 * @details NULL est accepté. N'a aucun effet sur le résultat ou le schéma.
 * @par Exemple
 * `args_diagnostic_free(diagnostic);`
 */
void	args_diagnostic_free(
	t_args_diagnostic *diagnostic
);

/**
 * @brief Effacer un diagnostic pour réutilisation.
 * @details NULL est accepté. Rétablit code 0, message vide et aucune origine.
 * @par Exemple
 * `args_diagnostic_clear(diagnostic);`
 */
void	args_diagnostic_clear(
	t_args_diagnostic *diagnostic
);

/**
 * @brief Renseigner un message depuis un convertisseur ou validateur.
 * @details Copie bornée, sans allocation ; troncature signalée dans le diagnostic.
 * @details Le moteur ajoute ensuite son contexte (commande/paramètre/position).
 * @details NULL diagnostic autorisé : aucune écriture, retourne 0. message non NULL si
 * @details diagnostic présent. Le callback retourne séparément le statut de validation.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_diagnostic_set(diagnostic, code, "Valeur interdite");`
 */
int	args_diagnostic_set(
	t_args_diagnostic *diagnostic,
	int code,
	const char *message
);

/**
 * @brief Lire le code conservé dans le diagnostic.
 * @details Retour de donnée, pas statut d'accès : 0 si vide, sinon statut enregistré.
 * @details NULL invalide renvoie EINVAL et positionne errno=EINVAL.
 * @return Code enregistré ; indépendant de la durée de vie du résultat.
 * @par Exemple
 * `int code = args_diagnostic_code(diagnostic);`
 */
int	args_diagnostic_code(
	const t_args_diagnostic *diagnostic
);

/**
 * @brief Emprunter le message du diagnostic.
 * @details Chaîne vide si aucun message ; expire au clear, à la prochaine écriture
 * @details ou au free. Les textes ne pointent pas dans un résultat détruit.
 * @return Message ou NULL avec errno=EINVAL.
 * @par Exemple
 * `const char *message = args_diagnostic_message(diagnostic);`
 */
const char *	args_diagnostic_message(
	const t_args_diagnostic *diagnostic
);

/**
 * @brief Copier la position associée au diagnostic.
 * @details Sortie obligatoire. Erreur NO_ORIGIN si ce diagnostic ne concerne pas un
 * @details token. Le diagnostic reste lisible après libération du résultat.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_diagnostic_get_origin(diagnostic, &origin);`
 */
int	args_diagnostic_get_origin(
	const t_args_diagnostic *diagnostic,
	t_args_origin *origin
);

/**
 * @brief Indiquer si un texte du diagnostic a été tronqué.
 * @details Sortie obligatoire. Aucun accès à un tampon interne mutable.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_diagnostic_truncated(diagnostic, &truncated);`
 */
int	args_diagnostic_truncated(
	const t_args_diagnostic *diagnostic,
	bool *truncated
);

/**
 * @brief Rendre l'aide d'une commande vers le writer fourni.
 * @details Schéma figé et writer non NULL. width=0 désactive le repliement automatique.
 * @details Le writer reçoit des blocs empruntés pendant l'appel ; retourne 0 ou une erreur.
 * @details Propager la première erreur ; aucun choix implicite stdout/stderr ni exit.
 * @details Aucune version applicative stockée : l'application traite VERSION elle-même.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `args_write_help(args_selected_command(call), 80, write_stdout, NULL);`
 */
int	args_write_help(
	const t_args_parser *command,
	size_t width,
	t_args_writer_fn writer,
	void *context
);

/**
 * @brief Décrire un statut args ou un errno standard.
 * @details Accepte 0, errno positif, erreur module négative ; code module inconnu
 * @details utilise le message UNKNOWN. Chaîne empruntée, non libérable.
 * @return Message non NULL.
 * @par Exemple
 * `const char *message = args_strerror(status);`
 */
const char *	args_strerror(
	int code
);

/** @brief Descripteur stable, à déclarer dans args/error.h à l'intégration. */
typedef struct s_genesis_error_module t_genesis_error_module;
extern const t_genesis_error_module genesis_args_error_module;

/* -----| Settings de module : futur args/setting.h |----- */
/**
 * @brief Workspace public à définir dans setting.h, pas un stockage de parsing.
 * @details Champs prévus : max_tokens, max_values, max_depth, max_invocations,
 * max_storage_bytes, et int (*apply)(void). Les appels runtime lisent seulement
 * la configuration privée publiée. L'application synchronise écritures/apply.
 * La définition de cette structure publique est hors du fichier de prototypes.
 */
typedef struct s_genesis_args_settings t_genesis_args_settings;
extern t_genesis_args_settings genesis_args_settings;

/**
 * @brief Valider et publier les settings du module.
 * @details Aucun argument. Échec : configuration active inchangée. Succès : les
 * @details prochains appels capturent la nouvelle configuration ; aucun résultat déjà
 * @details publié n'est modifié. Ne sélectionne pas d'allocateur.
 * @return 0 ou statut d'erreur ; conventions communes ci-dessus.
 * @par Exemple
 * `genesis_args_settings_apply();`
 */
int	genesis_args_settings_apply(
	void
);

#endif

