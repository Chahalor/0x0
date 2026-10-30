---
id: R-002
title: Structures internes proposées pour args
---

# Structures internes proposées pour args

**Approbation :** les structures du header actualisé ont été approuvées par
l'utilisateur le 26 octobre 2026. Le nouveau fichier
[args-api-draft.h](assets/args-api-draft.h) propose uniquement les déclarations
de l'API correspondante ; ces signatures ne sont pas encore approuvées.
Il conserve des constructeurs simples, des setters transactionnels et des
getters typés à statut explicite. Les types publics sont un prérequis de ce
fichier de conception ; il n'inclut pas les définitions privées. La vérification
syntaxique préinclut le header de structures approuvé, sans intégration à src/.

**Évolution :** [D-001](../decisions/D-001-args-cardinality-order-commands.md)
ajoute les invocations multiples et la réservation pour les paramètres suivants.
Le header est actualisé. Les exemples initiaux `result.params/options/command_path`
s'appliquent désormais à `invocation.params/options/command_path`, et les getters
et handlers ciblent une invocation. Le résultat global possède ces invocations
et retient le schéma ; les explications initiales ci-dessous se lisent avec cet
amendement.

Complément de R-001, demandé le 26 octobre 2026. Proposition de représentation,
pas modification du module actuel ni approbation définitive de la refonte.
Le [header de conception](assets/args-internal-draft.h) est autonome pour vérifier
la syntaxe C11. Ses types opaques seraient déclarés dans le futur `types.h`
public ; leurs définitions et les types de stockage resteraient privés.
Ne pas inclure ce fichier avec les headers actuels : leurs contrats diffèrent.

## 1. Les propriétaires et leurs liens

| Structure | Rôle | Propriété |
| --- | --- | --- |
| `t_args_schema` | Racine de possession, registres et état mutable/figé. | Possède tous les nœuds et les index compilés. |
| `t_args_parser` | Définition d'une commande, y compris la racine. | Possède ses métadonnées/tableaux ; emprunte les nœuds référencés. |
| `t_args_option` | Noms, portée, occurrences, action et paramètres. | Même règle : tableau possédé, paramètres empruntés. |
| `t_args_param` | Type, arity, conversion, contraintes, defaults. | Possède les descriptions/tokens ; contextes de callbacks empruntés. |
| `t_args_compiled` | Résolution des noms/alias et choix typés validés au seal. | Propriété exclusive du schéma après publication. |
| `t_args_result` | Chemin sélectionné, valeurs, occurrences et remainder. | Possède les données de sortie ; retient le schéma. |
| `t_args_parse_context` | Curseurs et état d'un seul appel de parsing. | Emprunte argv et diagnostic ; possède temporairement le résultat en construction. |

Il n'y a pas un parser distinct par rôle « builder » et « runtime ». Les mêmes
nœuds stables sont modifiables jusqu'au seal. Les seuls objets supplémentaires
produits par le seal sont les index de résolution et les choix préconvertis.
Cela évite de dupliquer tout l'arbre ou de remapper les handles rendus au builder.

Les registres du schéma sont les seuls propriétaires des nœuds : une destruction
ne parcourt pas les enfants pour les libérer une deuxième fois. Le nœud commande
libère seulement ses propres textes, règles et tableaux de références. Le
schéma libère ensuite chaque option et paramètre via leur registre.

## 2. Identité stable et indexation

Un nœud est alloué séparément. L'agrandissement d'un tableau de pointeurs ne
déplace donc jamais un `t_args_parser`, `t_args_option` ou `t_args_param`.

Chaque catégorie a ses propres slots denses, attribués à l'insertion réussie :

```c
schema->params[param->slot] == param;
result->params[param->slot];
result->options[option->slot];
schema->compiled->commands[command->slot];
```

`slot` est privé et n'a aucun rapport avec `app_id`, choisi par l'application.
Les getters vérifient l'identité du schéma, le slot, l'activité et le type avant
de retourner une vue. Le socle ne propose pas de suppression de nœuds : pas de
recyclage de slots, de générations ou de handles périmés à gérer durant le build.

`owner_index` est différent de `slot` : il indique la position du paramètre
dans l'option qui le possède, ou dans les positionnels de sa commande. Ainsi,
un getter par occurrence trouve sa tranche directement, sans recherche par nom.
`param->option == NULL` identifie un positional. `param->command` désigne
toujours la commande de déclaration, y compris pour une option héritée.

L'insertion publique est transactionnelle : construire le nœud et réserver
tous les tableaux concernés avant de publier les pointeurs et incrémenter les
compteurs. Une réserve de capacité réussie suivie d'un échec est acceptable
en interne, mais ne doit modifier ni le contenu logique ni les slots existants.

## 3. Finalisation et héritage

`schema->compiled == NULL` est l'unique marqueur d'état mutable. Pas de champ
`sealed` concurrent pouvant le contredire. Construire les données compilées
dans un temporaire, valider l'ensemble, puis affecter ce pointeur une seule fois.
Cette publication se fait sans concurrent : le builder n'est pas partageable
en mutation. Les atomiques concernent la rétention, pas le seal.

Un index par commande contient des tableaux triés de noms longs, noms courts
et sous-commandes. La recherche peut être binaire ; aucun hash ni cache global
n'est nécessaire pour poser le contrat initial. Les index contiennent toutes
les options visibles, y compris les options héritées, mais uniquement des
références aux définitions originales. Des alias différents peuvent donc
pointer vers un même `t_args_option` et vers le même slot résultat.

Les tableaux de bindings sont triés, jamais les registres à slots. La grammaire
est une configuration complète par commande, initialisée explicitement par les
builders ; pas d'héritage implicite de settings qui changerait avec l'ordre des
déclarations. Seule la portée d'option `DESCENDANTS` implique un héritage.

`compiled->choices` contient un buffer par slot de paramètre : les contraintes
de choix natifs sont converties au seal et comparées dans leur type. Les types
custom utilisent leur validateur, sans comparaison arbitraire de pointeurs.
Les defaults et valeurs implicites restent stockés sous forme de tokens copiés :
ils sont validés au seal puis convertis à nouveau pour chaque résultat. Cela
garantit qu'un objet custom issu d'un default a un seul propriétaire. Les
validateurs et convertisseurs peuvent donc être appelés au seal et au parse ;
ils doivent être déterministes et sans effets métier.

Un échec de seal nettoie également les choix custom temporaires et les tokens
convertis pour validation, sans modifier la définition source. Les choix
compilés et les résultats utilisent le même conteneur typé, mais jamais les
mêmes allocations propriétaires.

## 4. Le stockage qui résout nargs et les répétitions

Un paramètre résultat possède un seul tableau homogène. L'union sélectionne
le pointeur vers un vrai tableau du type attendu : `int64_t *`, `double *`, etc.
Ce n'est pas un tableau d'unions. `t_args_scalar` n'est utilisé que pour une
conversion/validation isolée avant insertion dans le tableau typé.

Une occurrence stocke un tableau de tranches de longueur `option->param_count`.
Une tranche porte sur le buffer du paramètre correspondant. Exemple avec un
seul paramètre `coordinates`, de cardinalité exactement deux :

```text
CLI : --point 1 2 --point 3 4

result.params[coordinates.slot].values.data.i64 = [1, 2, 3, 4]
result.options[point.slot].occurrences[0].params[0] = {0, 2}
result.options[point.slot].occurrences[1].params[0] = {2, 2}

APPEND : effective = {0, 4}
FIRST  : effective = {0, 2}
LAST   : effective = {2, 2}
ERROR  : rejet de la seconde occurrence, aucun résultat publié
```

Le même principe fonctionne pour `--map SOURCE DESTINATION` : deux paramètres,
chacun son buffer et son slot ; chaque occurrence contient deux tranches.
Un getter d'occurrence utilise `param->owner_index` pour choisir la tranche.
Toutes les occurrences sont validées, même celles ignorées par FIRST/LAST.

Une tranche unique suffit aux agrégations du socle : les éléments d'une
occurrence sont contigus dans le buffer de leur paramètre ; APPEND désigne
tous les éléments, FIRST/LAST une seule occurrence. Le futur filtrage/dédoublonnage
ou la fusion de sources discontinues nécessiterait de revoir ce contrat ou de
matérialiser une autre vue ; ne pas prétendre le supporter déjà.

`origins` contient une entrée par valeur et utilise la même capacité que le
buffer. Réserver les deux allocations avant d'annoncer une nouvelle capacité ;
ne publier `count` qu'après l'installation de la valeur et de son origine.
Les objets/chaînes sont détruits une seule fois par leur buffer propriétaire,
jamais par une tranche ou par une occurrence.

Une option présente avec zéro valeur a une occurrence et une tranche vide ;
elle reste distincte d'une option absente. `has_default` et `has_implicit`
distinguent une liste vide explicitement configurée de l'absence de réglage.
Les defaults ne créent pas d'occurrence CLI. Les valeurs implicites sont
rattachées à l'occurrence réelle avec une origine `IMPLICIT`.

Pour FLAG, l'état effectif est conservé dans `effective_flag`. Pour COUNT,
`count` des occurrences est déjà le compteur CLI, y compris pour les bundles.
Pas de second compteur susceptible de diverger. Les combinaisons incohérentes
d'action/répétition/paramètres sont rejetées au seal. La négation d'alias est
une extension future : ajouter une valeur d'action au binding et à l'occurrence
lorsqu'elle sera retenue, pas un champ mort dans toutes les structures dès maintenant.

## 5. Résultat et contexte de parsing

Le résultat possède des tableaux dimensionnés par les compteurs du schéma.
Ce choix consomme O(nombre total de paramètres/options) par résultat, même
si une seule branche est active, mais permet des getters directs et simples.
Pour un schéma de milliers de commandes rarement utilisées, une représentation
par commande active serait à mesurer avant de complexifier ce modèle.

`active` signifie que la commande de déclaration appartient au chemin actif,
pas que l'option a été fournie. Une option locale de la racine fournie avant
une sous-commande reste accessible. `command_path` est un tableau exact à la
publication ; sa dernière entrée est la commande sélectionnée. Pas de champ
`selected` redondant ni de nouvel arbre de commandes dans chaque résultat.

Le contexte contient argv, les curseurs, le mode, la commande courante et
l'option en attente. Celle-ci est identifiée par un handle et un index
d'occurrence, jamais par un pointeur dans un tableau pouvant être réalloué.
Même règle pour les paramètres : les offsets sont calculés via les tranches.
`options_enabled` et `commands_enabled` sont séparés : `--` désactive les deux,
alors que le mode « options arrêtées au premier positional » peut encore
permettre la sélection d'une commande après des positionnels fixes.

Le parseur capture la configuration technique appliquée au début de l'appel.
`values_used` et `storage_bytes` permettent de faire respecter les limites sur
la construction, provenance, occurrences et remainder compris. Des limites
de schéma et de rendu d'aide restent des contrôles des phases correspondantes,
pas des champs du contexte de parsing.

Un échec détruit `draft` et laisse le diagnostic dans le stockage de l'appelant.
Un succès RUN ne publie le résultat qu'après validation et calcul des tranches
effectives. Sur HELP/VERSION, nettoyer les données métier éventuellement déjà
construites ; conserver le schéma, le kind et le chemin cible. Les getters métier
rejettent ce kind avant d'accéder aux tableaux, qui peuvent être NULL.
Le résultat public n'a donc pas besoin d'un champ « dernière erreur ».

Remainder : un paramètre positional `remainder` est un tableau de chaînes
consommées volontairement. `result->remaining` est distinct : ce sont les
tokens non consommés rendus par la policy `UNKNOWN_STOP`. Dans les deux cas,
copier le texte avant de rendre le résultat, sans conserver un pointeur argv.

## 6. Durées de vie et limites de ce livrable

Le schéma commence avec une référence pour l'appelant racine. Chaque résultat
retient une référence ; la libération finale détruit les index puis les nœuds.
Les buffers custom sont détruits avant de relâcher cette référence, car leur
destructeur réside dans la définition. Les chaînes numériques d'origine ne
sont pas conservées : leur position l'est, et le diagnostic copie les extraits
nécessaires lors de l'erreur. Aucune API de récupération du token brut n'est
promise pour une valeur convertie.

Tout bloc opaque doit être rendu à l'allocateur qui l'a créé. Les helpers privés
d'allocation conservent les callbacks correspondants par bloc ; on ne suppose
pas que `G_SETTINGS.allocator` restera identique jusqu'au free. Ces helpers ne
sont pas implémentés par ce header. Les contextes de callbacks et identités
custom restent empruntés suivant R-001.

Le header donne les structures centrales et leurs liens. Les descripteurs
publics, enums de diagnostic/erreur, settings publics et détails du rendu d'aide
restent à spécifier avant implémentation. Sa vérification C11 garantit seulement
la cohérence syntaxique des déclarations, pas la correction du parser ni ses
propriétés de concurrence ou de durée de vie à l'exécution.

Vérification effectuée : `cc -std=c11 -pedantic -Wall -Wextra -Werror
-Wstrict-prototypes -x c -fsyntax-only` sur le header, sans diagnostic. Liens
locaux et blocs Markdown vérifiés également. Aucun test runtime exécuté.
