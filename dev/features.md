# Genesis - future features

## Files
- format for any of the file types (json format, yaml format)
- implemente YAML
- review/assert TOML

## Filesystem
- look at src/lib/filesystem/TODO.md

## interface
- print/format module with custom tag and printer
	- Tout les format standards
	- des format custom qui peuvent etre ajouter par un dev via une API
		- ex: format_set("JSON", json_stringify) -> call io.print("output: %{JSON}\n", json)
		- maybe un system pour passer des args   -> ex: io.print("output: %{JSON%d}\n", json, 4)
- rework the log to have a diagnostic feature
	- simple log with differente level (config per levels)
	- system de diagnostic, qui se stack puis qui peuvent etre relacher a un moment
	- system d'erreur dans une file, avec ciblage de ligne, de colone, de quantiter de mot, de char special
- update the args module
	- rework the API to be like prism one
	- for option add list of allowed args
	- imcompatible options
	- automatique generation of zsh/bach/fish autocompletion files
- finish the dotenv module

## Types
- reusable tree
- hash map
- UUID type at the v4-V7 standard

# Build
- Option to install genesis has a global lib
- posibility to build genesis has a .so lib