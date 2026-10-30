---
id: D-001
title: Cardinalité, ordre CLI et commandes multiples dans args
---

# Cardinalité, ordre CLI et commandes multiples

Décisions de périmètre formulées par l'utilisateur le 26 octobre 2026.
Elles remplacent les restrictions correspondantes de R-001 et R-002.
Les détails de répartition et de délimitation proposés ci-dessous sont des
précisions de l'ARCHITECT ; aucune implémentation n'est réalisée.

## Decision

1. Accepter des paramètres variables suivis d'autres paramètres et réserver
   les valeurs nécessaires pour remplir les suivants. `pack a.c b.c archive.tar`
   produit `sources = [a.c, b.c]`, `destination = [archive.tar]`.
2. Les tableaux rendus conservent l'ordre CLI. Les associations entre listes
   comme `codec` et `input` sont la responsabilité de l'application.
3. Permettre plusieurs invocations de sous-commandes sur activation explicite ;
   cette capacité est désactivée par défaut. Une hiérarchie imbriquée reste
   distincte d'une séquence d'invocations.
4. Exclure les grammaires d'expressions, opérateurs, priorités et parenthèses.

## Rationale

La réservation rend les listes positionnelles utilisables sans imposer leur
placement final. L'ordre des valeurs apporte une garantie exploitable sans
introduire des associations métier. Les commandes multiples répondent à un
besoin distinct, configurable.

## Consequences

### Répartition proposée

Un paramètre ordinaire réserve au moins une valeur. Respecter cependant ses
bornes explicites : un facultatif `min = 0` ne devient pas obligatoire ; un
tuple de minimum deux réserve deux valeurs.

```text
reserve = somme des minima des paramètres suivants
si remaining < min[i] + reserve : erreur de cardinalité
take = min(max[i], remaining - reserve)
affecter les take prochains tokens au paramètre i
```

Les sommes sont vérifiées contre les débordements. `UNBOUNDED` est une absence
de borne, jamais une taille à allouer. Tout surplus après le dernier paramètre
est une erreur. Le choix déterministe proposé est de distribuer le surplus de
gauche à droite : `<a...> <b...>` de minima un recevant `1 2 3 4` donne
`a=[1,2,3]`, `b=[4]`. Aucune recherche d'un autre découpage après échec de type.

Collecter les tokens d'un domaine avant affectation et conversion. Appliquer
la réservation séparément aux positionnels d'une commande et aux paramètres
d'une occurrence d'option. Une option variable ne réserve pas automatiquement
des valeurs pour les positionnels : les frontières lexicales de R-001 restent
en place. La sélection d'une commande après un préfixe variable demeure une
question distincte ; aucune priorité nom/valeur n'est changée implicitement.

### Ordre garanti

```text
--codec h264 --input a.mp4 --codec vp9 --input b.webm
codec = [h264, vp9]
input = [a.mp4, b.webm]
```

APPEND conserve l'ordre des occurrences puis celui des valeurs dans chacune.
Aucun tri, dédoublonnage ou appariement automatique. FIRST/LAST restent des
politiques explicitement choisies et préservent l'ordre de l'occurrence retenue.
L'accès par occurrence conserve toutes les apparitions. Les positions argv et
offsets restent disponibles pour reconstruire l'ordre relatif entre listes.
Deux listes distinctes peuvent avoir des tailles différentes : l'application
valide son éventuel appariement.

### Commandes multiples : représentation proposée

Ajouter au schéma un mode SINGLE par défaut ou SEQUENCE sur activation.
Le résultat possède une liste ordonnée de `t_args_invocation`. Chaque invocation
possède son chemin de commande, ses valeurs, ses occurrences et son remainder.
Répéter une commande crée deux invocations, sans fusion de leurs données.
Les getters et handlers portent sur une invocation. Le résultat global retient
le schéma une seule fois ; chaque invocation emprunte son résultat propriétaire.
En SINGLE, un succès contient exactement une invocation.

Syntaxe recommandée, non imposée par la décision utilisateur : séparateur
explicite configurable, par exemple `::` :

```text
image resize photo.png --width 800 :: compress --quality 85
```

Chaque segment recommence à la racine ; sa configuration, options racine
comprises, est autonome. Cette absence de propagation implicite est un choix
architectural proposé. Pas de segment vide ni séparateur initial/final.
En SEQUENCE, reconnaître le séparateur comme token entier avant affectation,
tant que la reconnaissance est active. Une occurrence incomplète échoue à la
frontière. `--value=::` reste une valeur attachée. Après `--` ou dans un remainder,
tous les tokens restants sont littéraux et la séparation est désactivée.
En SINGLE, aucun séparateur n'est réservé, et un nom de commande sœur ne provoque
pas de transition automatique.

Tout le parsing réussit avant publication et tout dispatch. Une erreur tardive
ne doit pas laisser exécuter les premiers segments. L'application dispatch les
invocations explicitement et choisit sa politique d'arrêt sur erreur métier.
Une demande HELP/VERSION reconnue nettoie les données métier des segments déjà
construits et publie seulement l'invocation de présentation ciblée ; une erreur
antérieure reste prioritaire. Aucun moteur de pipeline n'est ajouté.

### Structures

- Policy de séquence dans le schéma.
- Chemin, valeurs et occurrences dans `t_args_invocation` ; le résultat global
  possède un tableau de pointeurs vers ces objets stables.
- Une occurrence référence son invocation ; ses paramètres restent des tranches.
- Le contexte conserve des tokens non attribués et l'index d'invocation courant.
- Le schéma compilé contient des suffixes de minima pour la réservation.
- Les tableaux typés contigus et les garanties d'ownership de R-002 demeurent.

## Alternatives

Écarter la restriction « nargs doit être dernier », les associations métier
automatiques entre options et le parsing d'expressions. Ne pas fusionner les
valeurs d'invocations distinctes dans une même table globale.

## References

- R-001 : API et contrats initiaux, amendés par cette décision.
- R-002 : représentation interne initiale, amendée par cette décision.
- [Header actualisé](../reports/assets/args-internal-draft.h).

Validation future : listes suivies de scalaires/tuples, plusieurs listes,
facultatifs, valeurs insuffisantes, overflow des minima, ordre CLI, séquences
activées/désactivées, commande répétée, isolation, séparateurs littéraux,
erreur tardive sans handler, aide dans un segment ultérieur.
