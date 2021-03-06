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

#ifndef _HTTPD_H
#define _HTTPD_H

#define HTTPD_PORT              80
#define MAX_VAR_ARRAY           10
#define CONVERSION_BUFFER_LEN   10

//#define HTTP_DEBUG	usart_write 
#define HTTP_DEBUG(...)	


extern unsigned int var_array[MAX_VAR_ARRAY];

struct http_table
{
  unsigned char *auth_ptr;
  unsigned char *hdr_end_pointer;
  unsigned char http_auth 		: 1;
  unsigned char first_switch		: 1;
};

//Prototypen
extern void httpd (unsigned char);
extern void httpd_init (void);
extern void httpd_stack_clear (unsigned char);
extern void httpd_header_check (unsigned char);
extern void httpd_data_send (unsigned char);

#endif //_HTTPD_H




