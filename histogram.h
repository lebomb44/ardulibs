#ifndef _H_HISTO
#define _H_HISTO

/*----------------------------------------------------------------------+
 | 									|
 |	FILE :		histogram.h					|
 | 									|
 |	DESCRIPTION :	contains functions to build histogram in which	|
 |			values are automatically sorted	and archived	|
 |			according to a paramtizable interval lenght	|
 | 									|
 |      AUTHORS :       Marc LEROY					|
 | 									|
 |	CREATION :	16/06/94			 		|
 | 									|
 |	UPDATES :	20/12/94 Bruno Moisan				|
 |			interval lenght is parametrizable		|
 |			number of intervals is parametrizable		|
 | 									|
 +----------------------------------------------------------------------*/

#include <stdio.h> 

/*----------------------------------------------------------------------+
 | 									|
 |	FUNCTION :	histo_nb_interval				|
 | 									|
 |	DESCRIPTION :	set the number of used intervals		|
 | 									|
 |	PARAMETERS :	nb	number of requested interval		|
 | 									|
 |	GLOBAL VAR :	histo, max				 	|
 | 									|
 |	RETURN :	0 if ok, -1 if nb > HISTO_MAX			|
 | 									|
 +----------------------------------------------------------------------*/

int histo_nb_interval( unsigned int nb );

/*----------------------------------------------------------------------+
 | 									|
 |	FUNCTION :	histo_set_delta					|
 | 									|
 |	DESCRIPTION :	set the width of the intervals			|
 | 									|
 |	PARAMETERS :	width	requested width				|
 | 									|
 |	GLOBAL VAR :	histo, delta				 	|
 | 									|
 |	RETURN :	0 if ok, -1 if width <= 0			|
 | 									|
 +----------------------------------------------------------------------*/

int histo_set_delta( unsigned int width /*us*/);

/*----------------------------------------------------------------------+
 | 									|
 |	FUNCTION :	histo_init					|
 | 									|
 |	DESCRIPTION :	clear the histogram				|
 | 									|
 |	PARAMETERS :	none						|
 | 									|
 |	GLOBAL VAR :	histo, max				 	|
 | 									|
 |	RETURN :	none						|
 | 									|
 +----------------------------------------------------------------------*/

void histo_init(void);

/*----------------------------------------------------------------------+
 | 									|
 |	FUNCTION :	histo_enter					|
 | 									|
 |	DESCRIPTION :	outputs the histogram				|
 | 									|
 |	PARAMETERS :	value	value to put in the histogram		|
 | 									|
 |	GLOBAL VAR :	histo, max, delta			 	|
 | 									|
 |	RETURN :	none						|
 | 									|
 +----------------------------------------------------------------------*/

void histo_enter( unsigned long value /*us*/);
void histo_enter_with_date( unsigned long value, double date_evt );



/*----------------------------------------------------------------------+
 | 									|
 |	FUNCTION :	histo_dump					|
 | 									|
 |	DESCRIPTION :	outputs the histogram				|
 | 									|
 |	PARAMETERS :	f	stream on which histogram has to be	|
 |				displayed				|
 | 									|
 |			title	title of the histogram			|
 | 									|
 |	GLOBAL VAR :	histo, max, delta			 	|
 | 									|
 |	RETURN :	none						|
 | 									|
 +----------------------------------------------------------------------*/

void histo_dump( FILE *f, char *title );

#endif

