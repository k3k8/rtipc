/*****************************************************************************
 *
 *  $Id$
 *
 *  Copyright 2012  Richard Hacker (lerichi at gmx dot net)
 *
 *  This file is part of the rtipc library.
 *
 *  The rtipc library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or (at
 *  your option) any later version.
 *
 *  The rtipc library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the rtipc library. If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#ifndef RTIPC_H
#define RTIPC_H

#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Data type definitions.
 *
 * Let the enumeration start at 1 so that an unset data type could be
 * detected.
 */
enum rtipc_datatype_t {
    rtipc_double_T = 1, rtipc_single_T,
    rtipc_uint8_T,      rtipc_sint8_T,
    rtipc_uint16_T,     rtipc_sint16_T,
    rtipc_uint32_T,     rtipc_sint32_T,
    rtipc_uint64_T,     rtipc_sint64_T,
    rtipc_boolean_T
};

struct rtipc;
struct rtipc_group;

/** Initialise rtipc library.
 *
 * This is the first call that initialises the library. It should be called
 * before any other library calls.
 *
 * returns:
 *      NULL on error
 *      pointer to struct rtipc on success
 */
struct rtipc* rtipc_create(
        const char *name,       /**< Name of the process */
        const char *cache_dir   /**< Path to a writeable directory for cache */
        );

struct rtipc_group* rtipc_create_group(
        struct rtipc* rtipc,    /**< Pointer to rtipc structure */
        double sample_time      /**< Optional sample time */
        );

/** Register an IPC signal
 *
 * This call registers a process output signal which is exported to other
 * rtipc processes via shared memory (aka IPC).
 *
 * The signals are copied to shared memory from the task when rtipc_update()
 * is called
 *
 */
struct txpdo* rtipc_txpdo(
        struct rtipc_group *group,
        const char *name,         /**< Signal's name */
        enum rtipc_datatype_t datatype, /**< Signal data type */
        const void *addr,         /**< Signal address */
        size_t n                  /**< Element count. */
        );

void rtipc_set_txpdo_addr(
        const struct txpdo*,
        const void *addr          /**< Signal address */
        );

/** Find an IPC signal
 *
 * This call attempts to find a exported IPC signal with the respective
 * properties
 *
 */
struct rxpdo* rtipc_rxpdo(
        struct rtipc_group *group,
        const char *name,         /**< Signal's name */
        enum rtipc_datatype_t datatype, /**< Signal data type */
        void *addr,               /**< Signal address */
        size_t n,                 /**< Element count. */
        unsigned char *connected  /**< Connected status */
        );

void rtipc_set_rxpdo_addr(
        const struct rxpdo*,
        const void *addr          /**< Signal address */
        );

int rtipc_prepare(
        struct rtipc* rtipc     /**< Pointer to rtipc structure */
        );

void rtipc_tx(
        struct rtipc_group *group
        );

void rtipc_rx(
        struct rtipc_group *group
        );

void rtipc_exit(
        struct rtipc* rtipc     /**< Pointer to rtipc structure */
        );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RTIPC_H */
