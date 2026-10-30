---
id: R-001
title: Proposition de refonte du module args
---

# Refonte de `interface/args`

**Amendement :** [D-001](../decisions/D-001-args-cardinality-order-commands.md)
fait autorité pour la réservation des valeurs suivantes, l'ordre CLI, les
commandes multiples activables et l'exclusion des expressions. Les signatures
ci-dessous décrivent la première proposition ; getters et handlers ciblent
désormais une invocation du résultat global.

Proposition de l'ARCHITECT du 26 octobre 2026, fondée sur le dépôt au commit
`299528d`. Ce document décrit une API cible, pas une API déjà disponible ni une
décision produit acceptée. Aucune modification de l'implémentation dans cette
mission. Les extraits C sont des esquisses de contrat, pas un header exhaustif.

## 1. Besoin et contraintes

Demandes : valider réellement les types, repenser `nargs` et sa lecture,
supprimer la sélection des commandes par comparaison de chaînes côté appelant,
et permettre la composition de CLI variées.

Hypothèse proposée : autoriser une rupture de l'API `args` pour cette refonte.
L'autorisation de proposer cette rupture ne vaut pas validation de tous les
choix ci-dessous ni demande d'implémentation immédiate. Les utilisateurs externes
de Genesis ne sont pas connus ; ne pas annoncer une migration exhaustive.

Contraintes du dépôt : C11 pour les contrats testés, types opaques, façades
publiques minces et implémentation dans `_internal_`. Suivre `docs/norm.md` et
`docs/settings.md` pour les allocations, settings et erreurs. La proposition
reste dans tA : `docs/` décrit le comportement effectif et approuvé.

## 2. Constats vérifiables dans le code

| Sujet | Constat | Référence |
| --- | --- | --- |
| Types | `int`, `uint`, `file`, `str` sont explicitement des métadonnées ; les valeurs de sortie sont des chaînes. | `src/lib/interface/args/types.h:36`, `_internal_/_types.h` |
| Lecture | Une valeur est copiée en `char *` ; plusieurs valeurs donnent un tableau alloué de pointeurs empruntés. Type et durée de vie changent avec la cardinalité. | `src/lib/interface/args/_internal_/_args_extract.c:116`, `:147`, `Args.md` |
| Consommateur | Le Manager branche sur `n != 1` et caste le même `void *` en deux types. | `dev/Manager/src/sub-command/setup.c:109` |
| Répétitions | Les occurrences réutilisent le même nœud option et les paramètres de même nom ; un compteur est incrémenté, mais les frontières entre groupes ne sont pas conservées. | `src/lib/interface/args/_internal_/_args_parser.c:209`, `:242`, `:388` |
| Commandes | Le parseur interne descend dans les enfants, mais l'extraction publique expose essentiellement le premier sous-nœud et son nom. Le Manager refait le dispatch par `strcmp`. | `src/lib/interface/args/_internal_/_args_extract.c:244`, `dev/Manager/src/cli.c:160` |
| Héritage | Lors de la création d'un enfant, les options héritées sont recréées avec noms, description et flags, sans leurs paramètres. Les ajouts ultérieurs au parent ne suivent pas. | `src/lib/interface/args/_internal_/_args_add.c:59` |
| Validation | `_check_parser` est marqué inutilisé. `_args_check_output` vérifie la racine ou son premier enfant, sans parcourir tout le chemin sélectionné. | `src/lib/interface/args/_internal_/_args_checks.c:114`, `:418` |
| Aide | L'implémentation appelle `exit(0)` ; le test de `exit_on_help` est commenté. | `src/lib/interface/args/_internal_/_args_builtin.c:141` |
| Configuration | Des intentions de contrôle existent dans `_s_args_config`, mais pas d'API publique complète ; l'erreur courante est stockée dans un singleton. | `src/lib/interface/args/_internal_/_types.h`, `_args_config.c:29` |
| Tests | Aucune suite `args` trouvée dans `tests/`, et aucun raccordement `args` aux suites strictes existantes. | `tests/Makefile:192` |

Ces constats proviennent d'une lecture statique. Aucun bug n'est présenté ici
comme reproduit par un test d'exécution.

## 3. Modèle recommandé

Conserver les commandes, options et paramètres, mais séparer leurs responsabilités :

1. **Définition** : commandes, options, paramètres, contraintes et policies.
2. **Finalisation** : `args_parser_seal()` valide la définition et la fige.
3. **Parsing** : reconnaissance des tokens, conversions, validation ; produit
   un résultat indépendant de toute exécution métier.
4. **Lecture ou exécution explicite** : vues typées, commande sélectionnée,
   puis éventuellement `args_dispatch()`.

Pas de callback métier durant `args_parse()`, pas d'impression et pas d'`exit()`.
Les convertisseurs et validateurs sont appelés pendant le parsing ; ils doivent
être sans effet métier et ne pas modifier le schéma.

Un parser mutable devient figé après un `seal` réussi. Un échec le laisse
modifiable et sans publication partielle. Tout mutateur sur un parser figé
renvoie `-ARGS_ERROR_FROZEN`. Parser non figé : parsing refusé avec
`-ARGS_ERROR_NOT_SEALED`. Cette phase explicite évite de revalider la définition
à chaque invocation et rend les erreurs de déclaration distinctes des erreurs CLI.

`seal` vérifie notamment : noms/alias en collision, IDs de commande dupliqués,
bornes incohérentes, type inconnu, defaults invalides, dépendances étrangères au
schéma, ordre des positionnels ambigu et collisions d'options héritées.

Les builders acceptent des descripteurs C avec champs nommés. Exemples de
signatures cibles, à détailler en Doxygen avant implémentation :

```c
int args_parser_add_sub(t_args_parser *parent,
    const t_args_command_spec *spec, t_args_parser **command);
int args_parser_add_option(t_args_parser *command,
    const t_args_option_spec *spec, t_args_option **option);
int args_option_add_param(t_args_option *option,
    const t_args_param_spec *spec, t_args_param **param);
int args_parser_add_param(t_args_parser *command,
    const t_args_param_spec *spec, t_args_param **param);
int args_parser_seal(t_args_parser *parser, t_args_diagnostic *diagnostic);
```

Chaque handle appartient à une seule définition racine. Les getters rejettent
un handle vivant provenant d'un autre schéma ; comme toute API C, ils ne peuvent
pas rendre sûr un pointeur déjà libéré. Échec de création : sortie mise à `NULL`,
arbre inchangé. Descripteurs, noms, alias et defaults sont copiés ; les contextes
de callbacks restent empruntés. Le préfixe existant `args_` est conservé.

## 4. Types et validation

Types natifs recommandés : `STR`, `PATH`, `BOOL`, `I64`, `U64`, `F64`.
Des bornes permettent d'exprimer un port, un `int` de la plateforme ou une taille
métier sans ambiguïté sur la largeur du stockage. `PATH` reste une chaîne typée ;
vérifier l'existence d'un chemin est une contrainte distincte.

Pipeline pour chaque valeur : reconnaissance du token → conversion complète →
contraintes du paramètre → stockage typé. Puis validation des occurrences,
positionnels et relations sur tout le chemin de commandes actif.

Contrats proposés :

- Entiers : décimal par défaut, signe `+` permis, signe `-` uniquement pour
  `I64`. Rejeter chaîne vide, espaces, suffixes (`12abc`), décimaux (`1.2`),
  dépassement de capacité et entier négatif pour `U64`, y compris `-0`.
- Bases supplémentaires explicites dans le paramètre ; aucune détection
  automatique de l'octal par défaut. Un préfixe hexadécimal ne fonctionne que
  si le paramètre le permet.
- Flottants : point décimal et exposant, lecture indépendante de la locale
  globale ; fin de chaîne obligatoire, dépassement et sous-flux signalés.
  `NaN` et infinis refusés par défaut, activation explicite possible.
  Ne pas modifier la locale du processus pour convertir.
- Booléens textuels : `true`/`false` et `1`/`0`, exacts par défaut ; autres
  vocabulaires explicitement configurables. Un flag sans valeur est une action
  booléenne, pas une conversion implicite d'une chaîne vide.
- Contraintes natives : min/max inclusifs, choix autorisés, longueur en octets,
  vide permis/interdit. Les contraintes de chemins sont explicites : existence,
  fichier/répertoire, accès souhaité. Aucun fichier ouvert, créé ou réservé ;
  l'opération métier doit encore gérer ses erreurs d'accès.
- Chaque élément de `nargs`, occurrence répétée, valeur par défaut ou source
  injectée suit les mêmes validations. Une occurrence remplacée par `LAST`
  doit elle aussi être valide ; une valeur invalide n'est jamais dissimulée.

Extension : un convertisseur spécifique peut produire un objet opaque avec
identité de type, destructeur et contexte. Sur succès, transfert de l'objet au
résultat ; sur échec, le convertisseur nettoie son travail et laisse la sortie
vide. Son destructeur est appelé exactement une fois, y compris en rollback.
L'accès exige la même identité de type et renvoie un pointeur `const` emprunté.
Un validateur spécifique peut ensuite inspecter la valeur convertie ; un
validateur de commande peut vérifier des relations métier entre paramètres.
Ne pas imposer un moteur de regex, un module UUID ou du JSON au noyau `args`.

## 5. Cardinalité, répétition et extraction

Trois axes distincts :

| Axe | Exemple | Contrat |
| --- | --- | --- |
| Arity d'un paramètre | `--point X Y` | Nombre de valeurs consommées par ce paramètre dans une occurrence. |
| Occurrences d'une option | `-I inc -I vendor` | Minimum et maximum d'apparitions de l'option ; distinct d'une valeur obligatoire si l'option est présente. |
| Agrégation | `--color red --color blue` | `ERROR`, `FIRST`, `LAST`, `APPEND` ; action `COUNT` pour `-vvv`. |

`arity = {min, max}`, avec `ARGS_UNBOUNDED` comme sentinelle. Exemples :
`{1,1}`, `{0,1}`, `{2,2}`, `{0,ARGS_UNBOUNDED}`, `{1,ARGS_UNBOUNDED}`.
Une option `{occurrences: 0..1}` avec un paramètre `{arity: 1..1}` est facultative,
mais exige sa valeur lorsqu'elle apparaît. Le défaut ne complète jamais une
occurrence explicitement incomplète.

Chaque occurrence conserve ses propres paramètres. Pour
`--point 1 2 --point 3 4`, on peut lire `[1,2]` et `[3,4]` séparément, ou la vue
aplatie `[1,2,3,4]` avec `APPEND`. Plusieurs paramètres nommés restent possibles
dans une option, par exemple `--map SOURCE DESTINATION`. Les vues sont par
paramètre ; elles ne fusionnent jamais des paramètres de types différents.

API de lecture recommandée : vues typées, toujours tableau + taille.

```c
typedef struct s_args_str_view
{
    const char *const *data;
    size_t count;
} t_args_str_view;

typedef struct s_args_i64_view
{
    const int64_t *data;
    size_t count;
} t_args_i64_view;

int args_get_strings(const t_args_result *result,
    const t_args_param *param, t_args_str_view *view);
int args_get_i64s(const t_args_result *result,
    const t_args_param *param, t_args_i64_view *view);
int args_get_i64(const t_args_result *result,
    const t_args_param *param, int64_t *value);
int args_get_occurrence(const t_args_result *result,
    const t_args_option *option, size_t index, const t_args_occurrence **out);
int args_occurrence_get_i64s(const t_args_occurrence *occurrence,
    const t_args_param *param, t_args_i64_view *view);
```

Même famille pour `U64`, `F64`, `BOOL` et chemins/chaînes. Une vue `I64` pointe
réellement sur des `int64_t` contigus, jamais sur un tableau d'unions recasté.
La sortie est préparée au parsing : aucune allocation ni conversion au getter.

- Paramètre valide mais absent : succès et vue `{NULL, 0}`.
- Un élément : même type de vue que pour dix éléments.
- Getter scalaire : exige exactement un élément ; absence = `MISSING_VALUE`,
  plusieurs = `CARDINALITY`, mauvais type = `TYPE_MISMATCH`.
- Getter de liste en échec : vue remise à `{NULL, 0}`. Getter scalaire en échec :
  destination inchangée. Pointeur de sortie invalide : `EINVAL`.
- Handle d'une commande non sélectionnée : `INACTIVE_COMMAND`, pas « absent ».
- Les vues aplaties suivent l'agrégation. Les accès aux occurrences conservent
  toutes les occurrences CLI acceptées, même sous `FIRST` ou `LAST`.
- Exposer `args_option_count()` et une lecture booléenne de flags. Un défaut ne
  compte pas comme apparition CLI. Une présence sans valeur reste observable.
- Une API de provenance distingue `CLI`, `DEFAULT` et, si ajoutées, les sources
  externes ; elle fournit aussi les indices argv par occurrence/élément.

Exemple cible pour les `target` du Manager :

```c
t_args_str_view targets;
int status;

status = args_get_strings(result, target_param, &targets);
if (status != 0)
    return (status);
for (size_t i = 0; i < targets.count; ++i)
    use_target(targets.data[i]);
```

Aucun branchement spécial pour une valeur. La vue et ses chaînes sont empruntées
au résultat. Pour une configuration devant survivre au résultat, copier les
chaînes avec l'API mémoire publique et les libérer avec `mem_free`.

## 6. Sous-commandes et composition

Chaque commande possède un handle stable, des alias, un ID applicatif optionnel
et un handler optionnel. Les noms servent à interpréter argv et afficher l'aide.
La logique métier utilise les handles, IDs ou handlers.

IDs : entiers non nuls, uniques dans toute la définition lorsqu'ils sont
fournis ; `0` signifie « aucun ID applicatif ». Des plugins peuvent simplement
conserver leurs handles, sans coordonner des enum globaux.

Deux modes complémentaires, fondés sur la même résolution :

```c
/* Déclaration cible : erreurs de builders à contrôler dans le code final. */
t_args_command_spec setup_spec = {
    .name = "setup",
    .id = CMD_SETUP,
    .handler = run_setup,
    .userdata = app
};

/* Mode 1 : l'application possède son dispatch. */
switch (args_command_id(args_selected_command(result)))
{
    case CMD_SETUP: return (run_setup(result, setup_command, app));
    case CMD_LIST:  return (run_list(result, list_command, app));
    default:       return (app_usage_error());
}

/* Mode 2 : le module invoque explicitement le handler déjà enregistré. */
status = args_dispatch(result, &handler_status);
```

Handler cible :
`int (*)(const t_args_result *, const t_args_parser *selected, void *userdata)`.
`args_dispatch()` retourne le statut du dispatch ; le code métier est écrit
séparément dans `handler_status`. Un résultat HELP/VERSION ou un handler absent
produit une erreur de dispatch sans invocation. Un appel réussi invoque une
fois le handler de la commande la plus profonde ; les handlers parents ne sont
pas implicitement appelés. Deux appels explicites peuvent exécuter deux fois.

Le résultat conserve le chemin complet : racine → `remote` → `add`, par exemple.
Exposer commande sélectionnée, profondeur et commande à chaque niveau. La racine
est elle-même une commande sélectionnable, pas un cas `NULL` ambigu. Un alias
résout vers exactement la même identité et le même handler.

Composition : `register_setup(root, app, &setup_handles)` déclare un sous-arbre
et rend ses handles au module appelant ; même principe pour des groupes d'options
communs. La création peut se faire par modules C sans singleton de registre.

Héritage recommandé : `LOCAL` ou `DESCENDANTS`. Résoudre les options héritées
dans les ancêtres, sans copier leurs définitions. Une option globale avant ou
après une sous-commande garde la même identité et le même compteur. Les
collisions sont des erreurs au `seal` par défaut ; un mode explicite de masquage
par l'enfant peut être ajouté. Toutes les contraintes des ancêtres actifs
restent vérifiées.

## 7. Grammaire et ambiguïtés : règles indispensables

La modularité exige des règles prévisibles. Un parseur ne peut pas deviner si
`build` est une valeur de `--files` ou une commande.

Socle proposé :

1. Résolution exacte des noms et alias ; pas d'abréviation implicite des options.
2. `--name=value`, `--name value`, `-n value`, `-nVALUE` et bundles `-abc`
   configurables. Dans un bundle, la première option prenant une valeur consomme
   tout le suffixe comme valeur ; les lettres suivantes ne sont plus des flags.
3. Un token attaché est toujours une valeur : `--text=--help`, `--text=` et
   `--numbers=-3`. Le vide est validé comme tel, pas assimilé à une absence.
4. En attente d'une valeur séparée, un token reconnu comme option interrompt la
   consommation. Si le minimum n'est pas atteint : `MISSING_VALUE`.
   Un nombre négatif syntaxiquement numérique est candidat pour un paramètre
   numérique, puis validé ; `U64 -3` donne donc `INVALID_TYPE`. Les options
   courtes numériques sont interdites dans le profil initial.
5. Un token inconnu commençant par `-` produit `UNKNOWN_OPTION` par défaut,
   même dans une liste variable. L'autoriser comme valeur est une policy du
   paramètre ; les valeurs attachées permettent déjà de lever l'ambiguïté.
   Le token `-` seul reste une valeur ordinaire.
6. `--` termine la reconnaissance des options ET des sous-commandes pour le
   reste de l'invocation. Il clôt une option en cours ; minimum insatisfait =
   erreur. Pour passer littéralement `--` comme valeur d'option, utiliser la
   forme attachée. Le reste alimente les positionnels ou le remainder.
7. `nargs` d'option consomme de façon gloutonne jusqu'à sa borne ou une frontière
   syntaxique. Il ne réserve pas silencieusement des valeurs aux positionnels.
   `--files a b destination` est donc une liste de trois valeurs ; préférer une
   borne fixe, des occurrences `-f a -f b`, ou `--files a b -- destination`.
8. Selon D-001, un paramètre variable peut précéder d'autres paramètres.
   Réserver la somme de leurs minima et distribuer le surplus de gauche à
   droite, dans les bornes de chacun. Appliquer cette règle séparément aux
   positionnels et aux paramètres d'une occurrence d'option, avant conversion.
9. Une commande peut avoir des positionnels fixes obligatoires avant le choix
   d'un enfant. Une fois ceux-ci remplis, le prochain token ordinaire est un
   candidat commande. Si les enfants sont obligatoires : nom inconnu =
   `UNKNOWN_COMMAND`, aucun nom = `MISSING_COMMAND`. Si les enfants sont
   facultatifs, le nom exact gagne sur un positional terminal éventuel ; un
   autre token alimente ce positional. `--` force l'interprétation littérale.
10. Pendant une liste d'option, un nom de commande reste une valeur par défaut.
    Une policy explicite `stop_at_command` peut arrêter la liste lorsque le
    minimum est satisfait et que la sélection d'un enfant est permise à cet
    endroit ; sinon le token reste une valeur. Documenter ce choix dans l'aide.

Un mode `STOP_AT_FIRST_POSITIONAL` peut coexister avec l'intercalage par défaut.
Un mode `REMAINDER`, explicitement déclaré, capture les tokens restants sans
interprétation, utile pour lancer un outil enfant.

Le parsing partiel doit rendre les tokens non consommés dans l'ordre, avec leur
indice original ; il ne devine jamais combien de valeurs appartient à une
option inconnue. Le mode le plus sûr pour un wrapper est d'arrêter à la première
option inconnue et de transmettre toute la suite, ou d'exiger `--`.

## 8. Configuration utile, organisée par portée

| Portée | Capacités proposées | Priorité |
| --- | --- | --- |
| Paramètre | Type, arity, min/max, choix, longueur, vide, metavar, default, convertisseur/validateur. | Socle |
| Option | Alias courts/longs, occurrences min/max, agrégation, flag booléen, compteur, portée locale/héritée. | Socle |
| Commande | Alias, ID/handler, enfants obligatoires/facultatifs, groupes d'aide, validateurs croisés. | Socle |
| Relations | `requires(A,B)`, `conflicts(A,B)`, groupe au plus/un exactement/au moins un ; références par handles. | Socle |
| Syntaxe | Formes attachées, bundles, options intercalées, `--`, remainder, arrêt à la première option inconnue. | Socle |
| Présentation | Aide/version comme résultats, largeur et writer contrôlés par l'appelant, hidden, deprecated. | Socle |
| Option | `--feature` / `--no-feature`, valeur implicite lorsqu'une option à valeur facultative est présente. | Extension proche |
| Sources | Valeurs injectées par l'appelant, provenance et priorité explicites ; adaptateurs environnement/config. | Extension proche |
| Outils | Export du schéma et complétion shell, adaptateur de définition JSON versionné. | Après stabilisation |
| Commandes multiples | Invocations isolées sur activation ; désactivées par défaut. | Socle, D-001 |
| Syntaxe avancée | Fichiers `@response`, autres préfixes, séparateur CSV avec échappement. | À justifier par usages |

Présence dans les relations : CLI explicite par défaut ; un default ne satisfait
pas un `requires`. Une variante « valeur effective » peut être déclarée.
Les règles sont attachées à un scope actif ; leur référence doit être disponible
dans ce scope ou héritée d'un ancêtre. Rejeter les contraintes entre commandes
sœurs mutuellement exclusives. Un groupe exactement-un se vérifie après toutes
les valeurs du chemin sélectionné.

Valeur facultative d'option : par défaut, la forme séparée ne consomme pas le
token suivant, pour éviter de voler un positional. `--color` utilise une valeur
implicite déclarée ; `--color=never` fournit une valeur explicite. `default`
(option absente) et `implicit` (option présente sans valeur) sont distincts.

Sources ultérieures : `CLI > environnement > config > default`, sélection de
la source gagnante pour un paramètre entier par défaut, sans fusion implicite
de listes. La source gagnante est validée et ne retombe pas sur une source
inférieure si sa valeur est invalide. Les adaptateurs fournissent des tokens
déjà découpés ; le noyau n'invente ni shell splitting ni lecture de fichiers.
Les répétitions et la fusion entre sources devront avoir un contrat séparé.

Les réglages de grammaire sont des propriétés de la définition figée et peuvent
différer entre deux parsers. Les réglages techniques globaux (limite de tokens,
de valeurs, profondeur, taille de diagnostic/aide) suivent le modèle Genesis :
`genesis_args_settings`, `.apply()` sans argument, publication d'une configuration
privée validée. Les appels runtime ne prennent pas un struct settings global en
argument. Le parsing capture la configuration appliquée au début de l'appel ;
un échec d'`apply` conserve la précédente. Toutes les tailles et multiplications
sont vérifiées avant allocation, y compris avec `ARGS_UNBOUNDED`.

## 9. Erreurs, aide et résultat atomique

Signature proposée :

```c
int args_parse(const t_args_parser *parser, int argc,
    const char *const argv[], t_args_result **result,
    t_args_diagnostic *diagnostic);
```

`argc >= 1`, `argv[0]` = nom du programme ; arguments dans `[0, argc)` non NULL.
`result` obligatoire ; diagnostic facultatif et fourni par l'appelant.
Succès : `0` et résultat complet. Échec : statut non nul, `*result == NULL`,
aucun résultat partiellement exploitable. Tous les temporaires sont nettoyés.
La définition n'est jamais modifiée par le parsing.

Convention Genesis : erreurs système positives (`EINVAL`, `ENOMEM`,
`EOVERFLOW` pour une taille de stockage), erreurs sémantiques négatives
(`-ARGS_ERROR_INVALID_TYPE`, `OUT_OF_RANGE`, `MISSING_VALUE`, `CARDINALITY`,
`UNKNOWN_OPTION`, `UNKNOWN_COMMAND`, `CONFLICT`, etc.). Énumération avec
`ARGS_ERROR_SUCCESS = 0` et `ARGS_ERROR_UNKNOWN` final ; `args_strerror` utilise
une table indexée et expose un descripteur `genesis_args_error_module`.
Pas de singleton « dernière erreur ». Un entier trop grand fourni en CLI est
`OUT_OF_RANGE`, pas un dépassement de taille d'allocation.

Diagnostic à stockage borné fourni par l'appelant : code, étape
(définition/token/conversion/contrainte), ID de commande, index argv et offset
dans le token si applicable, index d'occurrence, noms attendus et copie bornée
du token/message. Marquer les troncatures. Il reste lisible après libération
des données temporaires, même sur manque de mémoire. Aucune dépendance à un
pointeur vers un résultat détruit.

Exemple : `setup: --jobs: attendu entier entre 1 et 64, reçu "3x" (argv[3])`.
Un seul diagnostic primaire déterministe au socle ; ne pas cumuler des erreurs
dont la récupération rendrait la grammaire incertaine.

Le résultat a un kind `RUN`, `HELP` ou `VERSION`, séparé des codes d'erreur.
Une aide reconnue court-circuite les exigences métier et valeurs obligatoires
manquantes de la commande ciblée, mais ne gomme pas une erreur déjà rencontrée.
Elle n'est reconnue ni dans une valeur attachée ni après `--`. Les tokens après
cette action ne sont pas exécutés ni convertis ; le résultat HELP n'expose pas
de valeurs métier et n'est pas dispatchable. L'aide cible la commande active à
l'emplacement du token. Les builtins sont désactivables ou renommables ; leurs
collisions sont vérifiées comme celles des autres options.

Le rendu de l'aide est une fonction séparée sur le schéma/commande et un writer
fourni. Les erreurs d'écriture remontent à l'appelant ; le choix stdout/stderr
et du code de sortie appartient à l'application.

## 10. Mémoire, durée de vie et concurrence

- Le parser racine possède toute sa définition. Les handles des descendants
  sont empruntés ; seul le propriétaire racine se libère explicitement.
- Chaque résultat retient une référence interne à la définition figée ;
  `args_parser_free()` relâche la référence de l'appelant. Les résultats et leurs
  handles restent donc valides jusqu'à leur propre libération.
- Le résultat possède les chaînes copiées depuis argv, les tableaux convertis,
  les occurrences et les objets personnalisés. argv peut être libéré/modifié
  après le retour. Les vues expirent à `args_result_free()`.
- Le handler et les convertisseurs sont retenus comme pointeurs de fonctions,
  leurs contextes restent empruntés. L'appelant maintient code et contextes en
  vie tant qu'un parser ou résultat peut les utiliser, destructeurs compris.
- Stockage opaque interne : allocations via l'allocateur global Genesis,
  conservation des callbacks de libération/réallocation correspondants pour
  ces blocs. Valeurs explicitement copiées et rendues à l'appelant : mémoire
  ordinaire Genesis, contrat `mem_free` et règles globales de changement
  d'allocateur de `docs/settings.md`. Pas de second système d'allocateurs public.
- Un builder n'est pas partageable en mutation. Un schéma figé peut servir
  plusieurs parsings avec sorties indépendantes ; rétention/libération interne
  synchronisée par atomiques C11. L'application conserve une référence vivante
  pendant chaque appel et synchronise mutations de settings et allocateurs.
  Les callbacks et allocateurs personnalisés doivent supporter la concurrence
  si l'application parse en parallèle. Un résultat immutable se lit en
  parallèle ; sa destruction exige la fin des lecteurs.

## 11. Alternatives et compromis

| Alternative | Pourquoi ne pas la choisir comme socle |
| --- | --- |
| Valider les chaînes puis garder l'extraction actuelle | L'appelant reconvertit et conserve le problème `void *`/ownership. |
| Retourner toujours `char **` | Améliore `nargs`, mais ne fournit pas de lecture typée ni les frontières d'occurrences. |
| Écrire directement dans les variables utilisateur | Confortable, mais effets partiels et durées de vie plus difficiles. Un binding ultérieur doit appliquer un résultat entièrement validé. |
| Imposer les callbacks de commande | Couplage inutile pour les applications ayant leur dispatch ; IDs et handles restent disponibles. |
| Garder des noms comme seules clés | Couplage au texte public, erreurs de renommage et recherches répétées côté client. |
| Copier les options héritées | Risque de divergence et dépendance à l'ordre des déclarations ; référence à la définition ancêtre préférable. |
| Un parseur acceptant toute grammaire | Ambiguïtés et heuristiques cachées ; profils explicites et refus des schémas ambigus préférables. |

Coûts assumés : rupture source, stockage typé avec provenance et références au
schéma, étape de finalisation supplémentaire. Ne pas promettre de gain de
performance sans mesure ; prévoir les vues contiguës pour une API fiable et
des lectures sans allocation, pas sur la base d'un benchmark absent.

## 12. Plan de réalisation et validation

1. **Contrats publics** : définir `args.h`, `args/types.h`, `args/error.h`,
   `args/setting.h`, compléter les Doxygen et les valeurs par défaut des
   descripteurs. Définir les initialiseurs pour éviter qu'un struct zéro
   signifie accidentellement « zéro valeur autorisée ».
2. **Template de tests** : créer `tests/lib/args.c`, raccorder `test-args`,
   strict, sanitizers, Valgrind et Cppcheck avant l'implémentation.
3. **Schéma et stockage** : builders transactionnels, handles, seal,
   héritage par références, ownership et chemins d'erreur d'allocation.
4. **Parsing typé** : conversion, cardinalités, occurrences, politiques de
   grammaire, vues typées et diagnostics atomiques.
5. **Commandes et capacités du socle** : chemin, IDs, dispatch explicite,
   relations, aide sans exit, settings.
6. **Migration du Manager** : remplacer `arsg_extract()` et le `strcmp` dispatch
   dans `dev/Manager/src/cli.c`, simplifier `setup_setup()` et définir la durée
   de vie de `Config` face aux vues. Mettre à jour `main.c`, handlers et cleanup.
   Les options racine `-v`/`-c` et locales `list -v`/`list -c` exigent une portée
   locale conservée ou un choix explicite de noms/masquage, pas un héritage aveugle.
7. **Adaptateur JSON et documentation** : porter le loader sur les mêmes builders
   publics, versionner le format et convertir `required`/`nargs` en bornes.
   Les fichiers JSON ne désignent pas des adresses de callbacks ; leur liaison
   est faite par l'application. Le noyau C reste utilisable sans JSON.
8. **Extensions** : sources, complétion et syntaxes avancées après validation
   des usages, avec contrats et tests propres.

Ne pas maintenir silencieusement les deux sens de `args_get_param`. Si une
période de compatibilité est exigée, isoler un adaptateur legacy explicitement
déprécié ; il ne doit pas dicter le stockage du nouveau noyau. Documenter les
changements de sémantique des erreurs, des nombres et de `--` en plus des signatures.

Matrice minimale de scénarios à tester :

| Domaine | Scénarios et résultat attendu |
| --- | --- |
| Types | `12`, `-12`, `12x`, vide, espaces, bornes I64/U64, `U64 -1`, overflow, float/locales, NaN/Inf selon policy. |
| Arity | 0/1/N valeurs, bornes min/max, tuple incomplet, optional sans valeur, default distinct d'implicit. |
| Répétition | `--point 1 2 --point 3 4` garde deux groupes ; FIRST/LAST/APPEND/ERROR ; `-vvv` compte trois. |
| Lexique | `--x=`, `--x=-3`, valeur `--help`, bundle avec option prenant une valeur, `-`, `--`, inconnues et remainder. |
| Commandes | alias même identité ; trois niveaux ; racine sélectionnée ; enfant requis manquant ; ID dupliqué ; option globale de part et d'autre du nom. |
| Ambiguïtés | valeur égale au nom d'une commande ; liste d'option avant positional ; `--` force le littéral ; réservation des minima après une liste variable (D-001). |
| Relations | required, conflit, dépendance, groupe exactement-un ; contraintes ancêtres validées ; defaults exclus de la présence CLI. |
| Échecs | injections d'échec à chaque allocation ; résultat NULL ; diagnostic vivant ; aucun handler ; destructeurs une seule fois. |
| Lecture | types et schémas incompatibles ; vue vide ; getter scalaire sur liste ; occurrence hors limites ; branche inactive. |
| Lifetime | argv libéré après parse ; référence parser relâchée avant résultat ; lectures multiples sans allocation ; copie autonome survivant au résultat. |
| Aide | retour HELP sans sortie/exit, commande cible correcte, erreurs antérieures conservées, aucun callback métier ni dispatch. |
| Settings | apply valide/invalide, configuration précédente préservée, limites de stockage et profondeur respectées. |

Cette mission vérifie la cohérence documentaire et les références au code.
Les suites runtime, la compilation d'un nouveau header et la revue indépendante
restent à réaliser lors de l'implémentation ; aucune validation indépendante
n'est revendiquée ici.
