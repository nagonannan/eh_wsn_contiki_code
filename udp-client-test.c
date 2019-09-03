/*
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
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"

/* The following libraries add IP/IPv6 support */
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"
#include "net/ipv6/uip-ds6-route.h"

/* Library used to read the metadata in the packets */
#include "net/packetbuf.h"

/* Basic libraries for timers, random numbers*/ 
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"

/* Sensor libraries */
#include "dev/battery-sensor.h"
#include "dev/button-sensor.h"

/* Example configuration file */
#include "../example.h"

/* LQ tracking estimate file */
#include "lqt.h"

#include <stdio.h>
#include <string.h>


#ifdef WITH_COMPOWER
//#include "powertrace.h"
#endif

#define DEBUG DEBUG_FULL
#include "net/ip/uip-debug.h"





/**********************************************************************************/

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define UDP_EXAMPLE_ID  190

#define MAX_PAYLOAD_LEN   80


/* Default battery parameters */
#define LOW_BAT 3200
#define HIGH_BAT 3600 

static clock_time_t calc_interv = CLOCK_SECOND*4;


static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;

/* Create a structure and pointer to store the data to be sent as payload,
   just like my_msg_t above but with less payload */

static struct my_meddelande_t meddelande; 
static struct my_meddelande_t *meddelandePtr = &meddelande; 

//TIMERS YALL
static struct ctimer periodic; 
static struct etimer shutdown_time; 
static struct ctimer lqt_timer; 

/* Toggle shutdown mode */
static uint8_t toggleShutdown = 0;
static uint8_t toggle_radio_always_on = 0; 


/* Calculate battery level n all */
uint32_t bat_loop[10];
uint16_t bat_median;




/* LQ Parameters */
static int16_t param_vector[3] = {2000,-1000,1000}; 
static int16_t feature_vector[3] = {700, 200, -650}; //Init bat, init dc, bat target
static int16_t init_vector[3] = {2000, -1000, 1000}; 
static int16_t test[3] = {5,6,7}; 

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static int seq_id;
static int reply;
static int counter;


/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/

/* Set new send rate. If the current battery level is above or below certain
   thresholds, set some pre-defined send-rates */

static void calc_interv_time (void)
{
  ctimer_reset(&lqt_timer);  
  uint8_t i; 

  /* Read battery sensor 11 times and take median 
     in order to remove the odd incorrect value */ 
  for (i=0; i<11; i++)
  {
    bat_loop[i] = battery_sensor.value(0);  
    bat_loop[i] *= 5000;
    bat_loop[i] /= 4096;
  }

  sortArray(bat_loop, 11); //Sort array elements 
  bat_median = bat_loop[5]; //Read the median value 
  meddelande.battery = bat_median; //Update the battery level for packet  

  toggleShutdown = 0; //unless critical battery level, keep radio on
  

  if (bat_median<LOW_BAT) //Critical level --> need to save power
  {
    toggleShutdown = 1; //force radio off next loop 
    process_post(&udp_client_process,PROCESS_EVENT_CONTINUE,NULL);
    strcpy(meddelande.mode, "Lo_bat");
    calc_interv = CLOCK_SECOND*100;
  }
  else if (bat_median>HIGH_BAT)
  {
    strcpy(meddelande.mode, "Hi_bat");
    calc_interv = CLOCK_SECOND/8;
    NETSTACK_MAC.off(1); //Sets the radio on time to 100% (not tested properly) 
    toggle_radio_always_on = 1; 
  }
  else {
    if (toggle_radio_always_on) 
    {
      toggle_radio_always_on = 0; //Reset the always on-flag 
      NETSTACK_MAC.on(); 
    }
    strcpy(meddelande.mode, "Normal_op"); //
    calc_interv = get_send_rate(bat_median, param_vector, feature_vector, init_vector); //LQ Tracking
  }
}

/*---------------------------------------------------------------------------*/
static void
send_packet(void *ptr)
{
  counter++;
  seq_id++;
  meddelande.counter = seq_id; 

  /*After sending packets change sending interval */ 
  calc_interv = CLOCK_SECOND*8;


  ctimer_reset(&periodic);  
  ctimer_set(&periodic, calc_interv, send_packet, NULL);

  meddelande.data_rate = calc_interv; //data rate in ticks


  PRINTF("Sent packet to node with ID: %u \n", 
                server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1]);
  PRINTF("DATA: Battery: %d mV, Counter: %u, Mode: %s  \n", meddelandePtr->battery, 
                                         meddelandePtr->counter, meddelandePtr->mode);
  PRINTF("Current send interval: every %ld seconds (every %d software clock ticks)\n\n", calc_interv/128, calc_interv);


  uip_udp_packet_sendto(client_conn, meddelandePtr, sizeof(meddelande),
                         &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));

}

/*---------------------------------------------------------------------------*/



/*_---------------------------------------------------------------------------------*/

static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Client IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
  uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;

  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  // Hardcoded server address to a specific z1 mote with ID 151. Decimal 151 = Hexadecimal 97
  // If Cooja Simulator is used for simulating the network, set server address to 1. 
if (COOJA_SIM) uip_ip6addr(&server_ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0x00ff, 0xfe00, 1); 
else uip_ip6addr(&server_ipaddr, 0xfe80, 0, 0, 0, 0xc30c, 0, 0, 0x0097);

}


/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{

#if WITH_COMPOWER
  static int print = 0;
#endif

  PROCESS_BEGIN();

  PROCESS_PAUSE();

  set_global_address();

  PRINTF("UDP client process started nbr:%d routes:%d\n",
         NBR_TABLE_CONF_MAX_NEIGHBORS, UIP_CONF_MAX_ROUTES);

  print_local_addresses();

  /* new connection with remote host */
  client_conn = udp_new(NULL, UIP_HTONS(UDP_SERVER_PORT), NULL); 
  if(client_conn == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(client_conn, UIP_HTONS(UDP_CLIENT_PORT)); 

  PRINTF("Created a connection with the server ");
  PRINT6ADDR(&client_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n",
  UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));



#if WITH_COMPOWER
  powertrace_sniff(POWERTRACE_ON);
#endif

  SENSORS_ACTIVATE(battery_sensor);
  calc_interv_time(); 
  ctimer_set(&periodic, CLOCK_SECOND*2, send_packet, NULL);
  ctimer_set(&lqt_timer, CLOCK_SECOND*5, calc_interv_time, NULL);
/*
  meddelande.battery = battery_sensor.value(0); //init battery level
  meddelande.battery *= 5000;
  meddelande.battery /= 4096;
 */ 
  while(1) {
    PROCESS_YIELD();
    printf("Not yielded? \n");
    if (toggleShutdown==1) 
    {
      ctimer_stop(&periodic);
      NETSTACK_MAC.off(0);
      etimer_set(&shutdown_time, CLOCK_SECOND*15);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&shutdown_time));
      NETSTACK_MAC.on(); 
      ctimer_reset(&periodic);
    }

    if(ev == tcpip_event) {
     // tcpip_handler();
    }


#if WITH_COMPOWER
      if (print == 0) {
  powertrace_print("#P");
      }
      if (++print == 3) {
  print = 0;
      }
#endif

    }
  
  SENSORS_DEACTIVATE(battery_sensor);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
