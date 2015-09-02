/*----------------------------------------------------------------------------
Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
Author:         Radig Ulrich
Remarks:        
known Problems: none
Version:        24.10.2007
Description:    Webserver Applikation

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
------------------------------------------------------------------------------*/
#include "config.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stack.h"
#include "base64.h"
#include "timer.h"
#include "usart.h"


#include "httpd.h"
#include "ntp.h"

#define HTTP_SERVER "marioli embedded 0.1"

struct http_table http_entry[MAX_TCP_ENTRY];

//Hier wird das codierte Passwort aus config.h gespeichert.
unsigned char http_auth_passwort[30];

char dstr[24]={"No Time...             "};

const PROGMEM char http_header[]={  
  "HTTP/1.0 200 Document follows\r\n"
  "Server: " HTTP_SERVER "\r\n"
  "Content-Type: application/javascript\r\n\r\n"
};

const PROGMEM char http_header_auth[]= {
  "HTTP/1.0 401 Unauthorized\r\n"
  "Server: " HTTP_SERVER "\r\n"
  "WWW-Authenticate: Basic realm=\"Authentication\""
  "Content-Type: text/plain\r\n\r\n"
};


//----------------------------------------------------------------------------
//Kein Zugriff Seite bei keinem Passwort
const PROGMEM char Page0[] = {"401 Unauthorized%END"};

unsigned char rx_header_end[5] = {"\r\n\r\n\0"};

//----------------------------------------------------------------------------
//Initialisierung des Httpd Testservers
void httpd_init(void)
{
  //HTTP_AUTH_STRING 
  decode_base64((unsigned char*)HTTP_AUTH_STRING,http_auth_passwort);

  //Serverport und Anwendung eintragen
  add_tcp_app (HTTPD_PORT, (void(*)(unsigned char))httpd);
}

//----------------------------------------------------------------------------
//http Testserver
void httpd (unsigned char index)
{
  //Verbindung wurde abgebaut!
  if (tcp_entry[index].status & FIN_FLAG)
  {
    return;
  }

  //Allererste Aufruf des Ports für diese Anwendung
  //HTTPD_Anwendungsstack löschen
  if(tcp_entry[index].app_status==1)
  {
    httpd_stack_clear(index);
  }

  //HTTP wurde bei dieser Verbindung zum ersten mal aufgerufen oder
  //HTML Header Retransmission!
  if (tcp_entry[index].app_status <= 2)
  {	
    httpd_header_check(index);
    return;
  }

  //Der Header wurde gesendet und mit ACK bestätigt (tcp_entry[index].app_status+1)
  //war das HTML Paket fertig, oder müssen weitere Daten gesendet werden, oder Retransmission?
  if(tcp_entry[index].app_status > 2 && tcp_entry[index].app_status < 0xFFFE && tcp_entry[index].status == ACK_FLAG)
  {
    httpd_data_send(index);
    return;
  }

  //Verbindung kann geschlossen werden! Alle HTML Daten wurden gesendet TCP Port kann
  //geschlossen werden (tcp_entry[index].app_status >= 0xFFFE)!!
  if(tcp_entry[index].app_status >= 0xFFFE)
  {
    tcp_entry[index].app_status = 0xFFFE;
    tcp_Port_close(index);
    return;
  }
  return;
}

//----------------------------------------------------------------------------
//HTTPD_STACK löschen
void httpd_stack_clear(unsigned char index)
{
  http_entry[index].first_switch = 0;
  http_entry[index].http_auth = HTTP_AUTH_DEFAULT;
  http_entry[index].auth_ptr = http_auth_passwort;
  http_entry[index].hdr_end_pointer = rx_header_end;
  HTTP_DEBUG("\r\n**** NEUE HTTP ANFORDERUNG ****\r\n\r\n");	
  return;
}

//----------------------------------------------------------------------------
//Eintreffenden Header vom Client checken
void httpd_header_check(unsigned char index)
{
  unsigned int a = 0;

  //finden der Authorization und das Ende im Header auch über mehrere Pakete hinweg!!
  if(*http_entry[index].hdr_end_pointer != 0)
  {		
    for(a = TCP_DATA_START_VAR; a < (TCP_DATA_END_VAR); a++)
    {	
      HTTP_DEBUG("%c", eth_buffer[a]);

      if(!http_entry[index].http_auth) 
      {
        if(eth_buffer[a] != *http_entry[index].auth_ptr++)
        {
          http_entry[index].auth_ptr = http_auth_passwort;
        }
        if(*http_entry[index].auth_ptr == 0) 
        {
          http_entry[index].http_auth = 1;
          HTTP_DEBUG("  <---LOGIN OK!--->\r\n");
        }
      }

      if(eth_buffer[a] != *http_entry[index].hdr_end_pointer++)
      {
        http_entry[index].hdr_end_pointer = rx_header_end;
      }

      //Das Headerende wird mit (CR+LF+CR+LF) angezeigt!
      if(*http_entry[index].hdr_end_pointer == 0) 
      {
        HTTP_DEBUG("<---HEADER ENDE ERREICHT!--->\r\n");
        break;
      }
    }
  }

#if 0
  if(!http_entry[index].new_page_pointer)
  {
    for(a = TCP_DATA_START_VAR+5;a<(TCP_DATA_END_VAR);a++)
    {
      if(eth_buffer[a] == '\r')
      {
        eth_buffer[a] = '\0';
        break;
      }
    }

    // Initialize pointer to avoid problems when communcation is not finished after webpage transmssion
    http_entry[index].new_page_pointer = 0;

  }
#endif

  //Wurde das Ende vom Header nicht erreicht
  //kommen noch weitere Stücke vom Header!
  if (*http_entry[index].hdr_end_pointer != 0)
  {
    //Der Empfang wird Quitiert und es wird auf weiteres Headerstück gewartet
    tcp_entry[index].status =  ACK_FLAG;
    create_new_tcp_packet(0, index);
    //Warten auf weitere Headerpakete
    tcp_entry[index].app_status = 1;
    return;
  }	

  //Wurde das Passwort in den ganzen Headerpacketen gefunden?
  //Wenn nicht dann ausführen und Passwort anfordern!
  if((!http_entry[index].http_auth) && tcp_entry[index].status & PSH_FLAG)
  {	
    //HTTP_AUTH_Header senden!
    memcpy_P((char*)&eth_buffer[TCP_DATA_START_VAR], http_header_auth, (sizeof(http_header_auth) - 1));
    tcp_entry[index].status =  ACK_FLAG | PSH_FLAG;
    create_new_tcp_packet(sizeof(http_header_auth) - 1, index);
    tcp_entry[index].app_status = 2;
    return;
  }

  tcp_entry[index].app_status = 2;

  //Seiten Header wird gesendet
  memcpy_P((char*)&eth_buffer[TCP_DATA_START_VAR], http_header, sizeof(http_header) - 1);
  tcp_entry[index].status =  ACK_FLAG | PSH_FLAG;
  create_new_tcp_packet(sizeof(http_header) - 1, index);

  return;
}

//----------------------------------------------------------------------------
//Daten Pakete an Client schicken
void httpd_data_send(unsigned char index)
{	
  unsigned int a;
  //Passwort wurde im Header nicht gefunden
  if(!http_entry[index].http_auth)
  {
    //http_entry[index].new_page_pointer = Page0;
  }

  //kein Paket empfangen Retransmission des alten Paketes
#if 0
  if (tcp_entry[index].status == 0) 
  {
    http_entry[index].new_page_pointer = http_entry[index].old_page_pointer;
  }
  http_entry[index].old_page_pointer = http_entry[index].new_page_pointer;
#endif

  a = snprintf(eth_buffer + TCP_DATA_START, MTU_SIZE - TCP_DATA_START - 150, "{ \"code\": 0 }");
  //Erzeugte Paket kann nun gesendet werden!
  tcp_entry[index].status =  ACK_FLAG | PSH_FLAG;
  create_new_tcp_packet(a, index);
  return;
}
