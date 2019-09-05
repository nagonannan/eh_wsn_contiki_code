/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
/*---------------------------------------------------------------------------*/
#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_


/* If using cooja to simulate set this to 1 */ 
#define COOJA_SIM 0

#define UIP_CONF_IPV6_RPL 1

/* Set contikiMAC as rdc protocol   */
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC   contikimac_driver



//SET CSMA
#undef NETSTACK_CONF_MAC 
#define NETSTACK_CONF_MAC	csma_driver    

#ifndef NESTACK_CONF_FRAMER
#if NETSTACK_CONF_WITH_IPV6
#define NESTSTACK_CONF_FRAMER framer_802154
#else //conf with ipv6
#define NETSTACK_CONF_FRAMER contikimac_framer 
#endif 
#endif 

/* Enable MCU sleep when no radio activity is occuring */ 
#undef RDC_CONF_MCU_SLEEP
#define RDC_CONF_MCU_SLEEP           0


/* Set rdc channel check rate to 4 Hz */
#undef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 4


#undef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID      0xABCD

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM          4
#endif

#undef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE       256

#ifndef UIP_CONF_RECEIVE_WINDOW
#define UIP_CONF_RECEIVE_WINDOW    60
#endif

#ifndef WEBSERVER_CONF_CFS_CONNS
#define WEBSERVER_CONF_CFS_CONNS   2
#endif

/* The following are Z1 specific */
#undef RF_CHANNEL
#define RF_CHANNEL	           26

#undef CC2420_CONF_CHANNEL
#define CC2420_CONF_CHANNEL        26

/* Test line for tx_power */ 
#ifndef EXAMPLE_TX_POWER
#define EXAMPLE_TX_POWER  31
#endif

/* The following are Zoul (RE-Mote, etc) specific */
#undef CC2538_RF_CONF_CHANNEL
#define CC2538_RF_CONF_CHANNEL     26


#define UIP_CONF_DS6_DEFAULT_PREFIX 0xfd00

/*-----------------------STOLEN FROM RPL-UDP EXAMPLE FIGURE OUT THINGS------*/
#ifndef WITH_NON_STORING
#define WITH_NON_STORING 0 /* Set this to run with non-storing mode */
#endif /* WITH_NON_STORING */

#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#undef UIP_CONF_MAX_ROUTES


#ifdef TEST_MORE_ROUTES
/* configure number of neighbors and routes */
#define NBR_TABLE_CONF_MAX_NEIGHBORS     10
#define UIP_CONF_MAX_ROUTES   30
#else
/* configure number of neighbors and routes */
#define NBR_TABLE_CONF_MAX_NEIGHBORS     10
#define UIP_CONF_MAX_ROUTES   10
#endif /* TEST_MORE_ROUTES */

//#undef NULLRDC_CONF_802154_AUTOACK
//#define NULLRDC_CONF_802154_AUTOACK       1

/* Define as minutes */
#define RPL_CONF_DEFAULT_LIFETIME_UNIT   60

/* 10 minutes lifetime of routes */
#define RPL_CONF_DEFAULT_LIFETIME        10

#define RPL_CONF_DEFAULT_ROUTE_INFINITE_LIFETIME 1

#if WITH_NON_STORING
#undef RPL_NS_CONF_LINK_NUM
#define RPL_NS_CONF_LINK_NUM 10 /* Number of links maintained at the root. Can be set to 0 at non-root nodes. */
#undef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES 0 /* No need for routes */
#undef RPL_CONF_MOP
#define RPL_CONF_MOP RPL_MOP_NON_STORING /* Mode of operation*/
#endif /* WITH_NON_STORING */

/*.-------------------------------------------------------------------------'*/

/*---------------------------------------------------------------------------*/
#endif /* PROJECT_CONF_H_ */
