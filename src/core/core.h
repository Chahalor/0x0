/**
 * @file core.h
 * @brief Aggregate public interface for Genesis core.
 * @details Exposes allocator, byte buffer, error and settings modules.
 */
#ifndef GENESIS_CORE_H
# define GENESIS_CORE_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Modules   |----- */
# include "allocator/allocator.h"
# include "allocator/error.h"
# include "buffer/buffer.h"
# include "buffer/error.h"
# include "error/error.h"
# include "setting/setting.h"
# include "setting/error.h"

#endif
