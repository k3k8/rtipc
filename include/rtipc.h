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
        const char *name,       /**< Name of the process. The configuration
                                 * file will be called <name>.conf */
        const char *cache_dir   /**< Path to a writeable directory for
                                 * the bulletin board */
        );

/** Create a signal group
 *
 * Every signal, be it export or import, is organized in a group. Create for
 * every sample time one group.
 *
 * returns:
 *      NULL on error
 *      pointer to struct rtipc_group on success
 */
struct rtipc_group* rtipc_create_group(
        struct rtipc* rtipc,    /**< Pointer to rtipc structure */
        double sample_time      /**< Group sample time. The sample time is
                                 * used to determine whether a signal
                                 * is connected */
        );

/** Register an IPC signal to be exported
 *
 * This call registers a process output (Tx) signal which is exported to other
 * rtipc processes via shared memory (aka IPC).
 *
 * The signals are copied to shared memory from the task when rtipc_update()
 * is called
 *
 * returns:
 *      txpdo structure. NULL on error, e.g. when the name is repeated.
 */
struct txpdo* rtipc_txpdo(
        struct rtipc_group *group,/**< Pointer to rtipc group */
        const char *name,         /**< Signal's global name. The name should
                                   * be unique over ALL applications */
        enum rtipc_datatype_t datatype, /**< Signal data type */
        const void *addr,         /**< Signal source address */
        size_t n                  /**< Element count. */
        );

/** Temporarily redirect the output signal's source address
 *
 * Use this function to rewrite the source @addr of a signal passed in the
 * call to @rtipc_txpdo(). This is useful to change the source of a signal
 * while the application is running, e.g. for testing purposes.
 *
 * To reset the source address to its original value, call this function
 * with @addr set to NULL.
 *
 * NOTE: This function may be only be called after @rtipc_prepare().
 */
void rtipc_set_txpdo_addr(
        const struct txpdo*,    /**< Pointer to a TxPdo */
        const void *addr        /**< Signal source address. NULL will use
                                 * original @addr passed when calling
                                 * @rtipc_txpdo() */
        );

/** Find an exported IPC signal
 *
 * This call attempts to find a exported IPC signal with the respective
 * properties
 *
 * returns:
 *      rxpdo structure.
 */
struct rxpdo* rtipc_rxpdo(
        struct rtipc_group *group,/**< Pointer to rtipc group */
        const char *name,         /**< Signal's name */
        enum rtipc_datatype_t datatype, /**< Signal data type */
        void *addr,               /**< Signal destination address */
        size_t n,                 /**< Element count. */
        unsigned char *connected  /**< Connected status */
        );

/** Temporarily redirect a the signal's source.
 *
 * When a input signal is connected, the source address points to a region
 * in shared memory. This function can be used let the source address
 * temporarily point to another region inside the application for testing
 * purposes.
 *
 * This call can even be used on signals that are unconnected.
 *
 * To reset the source address to shared memory, call this function
 * with @addr set to NULL.
 *
 * NOTE: This function may be only be called after @rtipc_prepare().
 */
void rtipc_set_rxpdo_addr(
        const struct rxpdo*,    /**< rxpdo structure */
        const void *addr        /**< Signal address */
        );

/** Setup library structures
 *
 * Call this function when all signals are registered so that the library
 * can set up its internal structures. After this call, no more signals can
 * be registered.
 *
 * returns:
 *      0 success
 *      error code on error
 */
int rtipc_prepare(
        struct rtipc* rtipc     /**< Pointer to rtipc structure */
        );

/** Send output variables
 *
 * Use only after calling @rtipc_prepare()
 */
void rtipc_tx(
        struct rtipc_group *group /**< Pointer to group structure */
        );

/** Receive input variables
 *
 * Use only after calling @rtipc_prepare()
 */
void rtipc_rx(
        struct rtipc_group *group /**< Pointer to group structure */
        );

/** Cleanup library
 */
void rtipc_exit(
        struct rtipc* rtipc     /**< Pointer to rtipc structure */
        );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RTIPC_H */
