#!/bin/sh
# Check the demo's argument order through its command-line interface.
set -eu

demo=$1
if usage_output=$("$demo" 2>&1); then
	printf 'Missing arguments were accepted\n' >&2
	exit 1
fi
case "$usage_output" in
	usage:*) ;;
	*)
		printf 'Usage was not checked before reading arguments\n' >&2
		exit 1
		;;
esac

red_output=$("$demo" RED)
if ! printf '%s\n' "$red_output" | grep -Eq 'uuid=.*[0-9a-f]{8}-[0-9a-f]{4}-7[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}'; then
	printf 'UUID output did not contain a version 7 UUID\n' >&2
	exit 1
fi
case "$red_output" in
	*' - {"bob": 67}'*) ;;
	*)
		printf 'Single-color output did not contain the input JSON object\n' >&2
		exit 1
		;;
esac

rgb_output=$("$demo" 255 16 127)
case "$rgb_output" in
	*' - {"bob": 67}'*) ;;
	*)
		printf 'JSON output did not contain the input object\n' >&2
		exit 1
		;;
esac

if "$demo" BAD >/dev/null 2>&1; then
	printf 'Invalid color was accepted\n' >&2
	exit 1
fi
