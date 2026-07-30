#ifndef GENESIS_TIMES_TYPES_H
# define GENESIS_TIMES_TYPES_H

/**
 * @file	times.h
 * @brief	public API for the time module of the lib
 * 
 * this module contaigne function(s) based on time manipulation and conversion
 * 
 * @ingroup time
 */

# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Systems   |----- */
	//...

/* -----| Globals   |----- */
	//...

/* -----| Internals |----- */
	//...

/* -----| Modules   |----- */
	//...

/* ************************************************************************** */
/*                                 Typedefs                                   */
/* ************************************************************************** */
	//...

/* ************************************************************************** */
/*                                 Enums                                      */
/* ************************************************************************** */

/**
 * @enum	time_unity
 * @brief	all supported unity for the times module
 * 
 * @ingroup times
 */
enum e_time_unity
{
	time_unity_ns,
	time_unity_us,
	time_unity_ms,
	time_unity_sec,
	time_unity_min,
	time_unity_hour,
	time_unity_day,
};

/* ************************************************************************** */
/*                                 Unions                                     */
/* ************************************************************************** */
	//...

/* ************************************************************************** */
/*                                 Structs                                    */
/* ************************************************************************** */
	//...

#endif	// GENESIS_TIMES_TYPES_H
