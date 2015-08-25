/*----------------------------------------------------------------------------
Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
Author:         Radig Ulrich
Remarks:        
known Problems: none
Version:        24.10.2007
Description:    Webserver uvm.

Dieses Programm ist freie Software. Sie können es unter den Bedingungen der 
GNU General Public License, wie von der Free Software Foundation veröffentlicht, 
weitergeben und/oder modifizieren, entweder gemäß Version 2 der Lizenz oder 
(nach Ihrer Option) jeder späteren Version. 

Die Veröffentlichung dieses Programms erfolgt in der Hoffnung, 
daß es Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE, 
sogar ohne die implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT 
FÜR EINEN BESTIMMTEN ZWECK. Details finden Sie in der GNU General Public License. 

Sie sollten eine Kopie der GNU General Public License zusammen mit diesem 
Programm erhalten haben. 
Falls nicht, schreiben Sie an die Free Software Foundation, 
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA. 
----------------------------------------------------------------------------*/

#include <avr/io.h>
#include <avr/eeprom.h>
#include "config.h"
#include "usart.h"
#include "enc28j60.h"
#include "stack.h"
#include "timer.h"
#include "httpd.h"
#include "cmd.h"
#include "telnetd.h"
#include "ntp.h"
#include "base64.h"
#include "http_get.h"
#include "dhcpc.h"
#include "dnsc.h"

//----------------------------------------------------------------------------
//Hier startet das Hauptprogramm
int main(void)
{  
  unsigned char reset_counter = 0;
  unsigned long time_update = 0;
  unsigned long a;

  DDRA = OUTA;
  DDRC = OUTC;
  DDRD = OUTD;

  usart_init(BAUDRATE);
  usart_write("\n\rSystem ready.\n\r");
  usart_write("Build "__DATE__" at "__TIME__"\r\n");
  usart_write("GCC-"__VERSION__"\r\n");

  for(a = 0; a < 1000000; a++)
  {
    asm("nop");
  }

  stack_init();
  httpd_init();
  telnetd_init();

  //Ethernetcard Interrupt enable
  ETH_INT_ENABLE;

  //Globale Interrupts einschalten
  sei(); 

  dhcp_init();
  if(dhcp() == 0)
  {
    save_ip_addresses();
  }
  else
  {
    usart_write("DHCP fail\r\n");
    read_ip_addresses();
  }

  usart_write("\r\nIP          %1i.%1i.%1i.%1i\r\n", myip[0]     , myip[1]     , myip[2]     , myip[3]);
  usart_write("MASK        %1i.%1i.%1i.%1i\r\n", netmask[0]  , netmask[1]  , netmask[2]  , netmask[3]);
  usart_write("GW          %1i.%1i.%1i.%1i\r\n", router_ip[0], router_ip[1], router_ip[2], router_ip[3]);
  usart_write("DNS         %1i.%1i.%1i.%1i\r\n", dns_server_ip[0], dns_server_ip[1], dns_server_ip[2], dns_server_ip[3]);
  usart_write("NTP_Server: %1i.%1i.%1i.%1i\r\n",ntp_server_ip[0],ntp_server_ip[1],ntp_server_ip[2],ntp_server_ip[3]);
  dns_init();
  ntp_init(); 

  for(a = 0; a < 1000000; a++)
  {
    asm("nop");
  }

  ntp_request();

  if(ntp() != 0)
  {
    usart_write("NTP Err.\r\n");
  }
  else
  {
    command_time();
  }

  while(1)
  {
    eth_get_data();

    //Terminalcommandos auswerten
    if (usart_status.usart_ready){
      usart_write("\r\n");
      if(extract_cmd(&usart_rx_buffer[0]))
      {
        usart_write("Ready\r\n\r\n");
      }
      else
      {
        usart_write("ERROR\r\n\r\n");
      }
      usart_status.usart_ready =0;
    }

    //Empfang von Zeitinformationen
    if(!ntp_timer) {
      ntp_timer = NTP_REFRESH;
      ntp_request();
    }

    if (dhcp() != 0) //check for lease timeout
    {
      usart_write("dhcp lease renewal failed\r\n");
      RESET();
    }

    //USART Daten für Telnetanwendung?
    telnetd_send_data();

    if(ping.result)
    {
      usart_write("Get PONG: %i.%i.%i.%i\r\n",ping.ip1[0],ping.ip1[1],ping.ip1[2],ping.ip1[3]); 
      ping.result = 0;
    }

    //wird jede Sekunde aufgerufen
    if(time != time_update)
    {
      time_update = time;
      if(eth.no_reset)
      {
        reset_counter = 0;
        eth.no_reset = 0;
      }
      else
      {
        if((reset_counter++)>5)
        {
          reset_counter = 0;
          enc_init();
        }
      }
    }			
  }//while (1)

  return(0);
}

