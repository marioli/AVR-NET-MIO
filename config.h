/*----------------------------------------------------------------------------
Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
Author:         Radig Ulrich
Remarks:        
known Problems: none
Version:        03.11.2007
Description:    Webserver Config-File

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

#ifndef _CONFIG_H_
#define _CONFIG_H_	

/* 0x8 on PORTB (PB3 = OC0) */
#define LED_GREEN 0x8
/* 0x10 on PORTD (PD4 = OC1B)  */
#define LED_BLUE 0x10
/* 0x20 on PORTD (PD5 = OC1A)  */
#define LED_RED 0x20

//Watchdog timer for the ENC2860, resets the stack if timeout occurs
#define WTT 1200 //Watchdog timer in timer interrupt

//NTP refresh before stack timeout occurs...
#define NTP_REFRESH 1140

//Umrechnung von IP zu unsigned long
#define IP(a,b,c,d) ((unsigned long)(d)<<24)+((unsigned long)(c)<<16)+((unsigned long)(b)<<8)+a

//IP des Webservers und des Routers
#define MYIP		IP(172,31,200,254)
#define ROUTER_IP	IP(172,31,255,254)

//Netzwerkmaske
#define NETMASK		IP(255,255,0,0)

//DHCP-Server
#define USE_DHCP    1 //1 = DHCP Client on

//DNS-Server
#define USE_DNS     1 //1 = DNS Client on
#define DNS_IP      IP(172,31,255,254)


//IP des NTP-Servers z.B. Server 1.de.pool.ntp.org
#define USE_NTP		1 //1 = NTP Client on
#define NTP_IP		IP(77,37,6,59)

//MAC Adresse des Webservers	
#define MYMAC1	0x00
#define MYMAC2	0x22
#define MYMAC3	0xf9
#define MYMAC4	0x01	
#define MYMAC5	0x19
#define MYMAC6	0xd4

//Taktfrequenz - besser als gcc define -D mitgegeben
#ifndef F_CPU
//#define F_CPU 20000000UL
#define F_CPU 16000000UL	
//#define F_CPU 12000000UL	
//#define F_CPU 14745600UL
//#define F_CPU 11059200UL
#endif

//Baudrate der seriellen Schnittstelle
#define BAUDRATE 9600

//AD-Wandler benutzen?
#define USE_ADC			0

//Webserver mit Passwort? (0 == mit Passwort)
#define HTTP_AUTH_DEFAULT	1

//AUTH String "USERNAME:PASSWORT" max 14Zeichen 
//für Username:Passwort
#define HTTP_AUTH_STRING "admin:uli1"
//#define HTTP_AUTH_STRING "user:pass"

/* eeprom locations */
#define IP_EEPROM_STORE         30
#define NETMASK_EEPROM_STORE    34
#define ROUTER_IP_EEPROM_STORE  38
#define DNS_IP_EEPROM_STORE     42
#define NTP_IP_EEPROM_STORE     50

#endif //_CONFIG_H


