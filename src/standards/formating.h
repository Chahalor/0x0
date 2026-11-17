/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   formating.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nduvoid <nduvoid@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 10:16:49 by nduvoid           #+#    #+#             */
/*   Updated: 2026/11/17 08:43:57 by nduvoid          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GENESIS_STANDARDS_FORMATING_H
# define GENESIS_STANDARDS_FORMATING_H

# pragma once

// colors

# define BLACK			"\033[30m"
# define GREY			"\033[90m"
# define GRAY			GREY
# define RED			"\033[31m"
# define GREEN			"\033[32m"
# define YELLOW			"\033[33m"
# define BLUE			"\033[34m"
# define MAGENTA		"\033[35m"
# define CYAN			"\033[36m"
# define WHITE			"\033[37m"

# define BRIGHT_BLACK	"\033[90m"
# define BRIGHT_RED		"\033[91m"
# define BRIGHT_GREEN	"\033[92m"
# define BRIGHT_YELLOW	"\033[93m"
# define BRIGHT_BLUE	"\033[94m"
# define BRIGHT_MAGENTA	"\033[95m"
# define BRIGHT_CYAN	"\033[96m"
# define BRIGHT_WHITE	"\033[97m"

# define ORANGE			"\033[38;2;255;165;0m"
# define PINK			"\033[38;2;255;105;180m"
# define PURPLE			"\033[38;2;128;0;128m"
# define VIOLET			"\033[38;2;138;43;226m"
# define BROWN			"\033[38;2;139;69;19m"
# define GOLD			"\033[38;2;255;215;0m"
# define SILVER			"\033[38;2;192;192;192m"
# define BEIGE			"\033[38;2;245;245;220m"
# define CORAL			"\033[38;2;255;127;80m"
# define SALMON			"\033[38;2;250;128;114m"
# define LIME			"\033[38;2;50;205;50m"
# define OLIVE			"\033[38;2;128;128;0m"
# define TEAL			"\033[38;2;0;128;128m"
# define TURQUOISE		"\033[38;2;64;224;208m"
# define AQUA			"\033[38;2;0;255;255m"
# define NAVY			"\033[38;2;0;0;128m"
# define INDIGO			"\033[38;2;75;0;130m"
# define MAROON			"\033[38;2;128;0;0m"
# define CRIMSON		"\033[38;2;220;20;60m"
# define KHAKI			"\033[38;2;240;230;140m"

// formatting

# define RESET			"\033[0m"
# define BOLD			"\033[1m"
# define FAINT			"\033[2m"
# define ITALIC			"\033[3m"
# define UNDERLINE		"\033[4m"
# define BLINK			"\033[5m"
# define REVERSE		"\033[6m"
# define REVERSED		"\033[7m"
# define HIDDEN			"\033[8m"
# define STRIKETHROUGH	"\033[9m"

// background colors
//...

// cmd management

# define FULL_CLEAN		"\033[H\033[2J\033[3J"
# define CLEAR_SCREEN	"\033[2J\033[1;1H"
# define CURSOR_UP		"\033[1A"
# define CURSOR_DOWN	"\033[1B"
# define CURSOR_LEFT	"\033[1D"
# define CURSOR_RIGHT	"\033[1C"
# define CURSOR_HOME	"\033[H"

// string

// # define ERROR	RED "Error: " RESET
# define USAGE	"Usage: "
/* "└──", "├──", "│" */
// ─ │ ┌ ┐ └ ┘ ├ ┤ ┬ ┴ ┼

# ifndef HEADER
#  define HEADER	"BIG FAT HEADER\n"
# endif


#endif	/* GENESIS_STANDARDS_FORMATING_H */