/*----------------------------------------------------------------------+
  | 									|
  |	FILE :		histogram.c					|
  | 									|
  |	DESCRIPTION :	contains functions to build histogram in which	|
  |			values are automatically sorted	and archived	|
  |			according to a paramtizable interval lenght	|
  | 									|
  |     AUTHORS :       Marc LEROY					|
  | 									|
  |	CREATION :	16/06/94			 		|
  | 									|
  |	UPDATES :	20/12/94 Bruno Moisan				|
  |			interval lenght is parametrizable		|
  |			number of intervals is parametrizable		|
  | 									|
  |			1/04/2004 Yves Dufrenne				|
  |			No more error_msg.h, more sample, some comments	|
  | 									|
  +----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
  | 									|
  |	STANDARD INCLUDES						|
  | 									|
  +----------------------------------------------------------------------*/

#include <stdio.h>

/*----------------------------------------------------------------------+
  | 									|
  |	PROJECT INCLUDES						|
  | 									|
  +----------------------------------------------------------------------*/
#include "histogram.h"

/*----------------------------------------------------------------------+
  | 									|
  |	CONSTANTS and MACRO-FUNCTIONS					|
  | 									|
  +----------------------------------------------------------------------*/

#define HISTO_MAX	1000
#define HISTO_DELTA	100  /* Default step is 100 us */

#ifndef error_msg
#define error_msg(func,level,txt)\
	fprintf(stderr, "%s: <%s> %s\n", err_msg[level], func, txt); 
#define MSG_DEBUG	0
#define MSG_INFO	1
#define MSG_WARNING	2
#define MSG_ERROR	3
static char *err_msg[]={"DEBUG", "INFO", "WARNING", "ERROR"};
#endif

/*----------------------------------------------------------------------+
  | 									|
  |	STATIC and GLOBAL VARIABLES					|
  | 									|
  +----------------------------------------------------------------------*/

static 	unsigned long	histo[HISTO_MAX];

typedef struct {
  double		date;  /* Store the date of the evenement in seconds */
  unsigned long		delta; /* Store the width of this step in us */
} info_evt_t;

/*Use to store when some overrange delta happend */
info_evt_t	list_evt[HISTO_MAX];

static	unsigned int	max	= HISTO_MAX;
static	unsigned int	delta	= HISTO_DELTA;
static  unsigned int	nb_sample;
static  unsigned int	nb_evt;  /* remeber the number of overrange values */
static	double		sum_sample;

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
  |	MSG_RETURN :	0 if ok, -1 if nb > HISTO_MAX			|
  | 									|
  +----------------------------------------------------------------------*/

int histo_nb_interval( unsigned int nb )
{
  if ( (nb >= 0) && (nb <= HISTO_MAX ) )
    {
      max = nb;
      return 0;
    }
  else
    {
      error_msg("histo_nb_interval", MSG_WARNING,
		"invalid interval number requested");
      return -1;
    }
}

/*----------------------------------------------------------------------+
  | 									|
  |	FUNCTION :	histo_set_delta					|
  | 									|
  |	DESCRIPTION :	set the width of the intervals			|
  | 									|
  |	PARAMETERS :	width	requested width	in us			|
  | 									|
  |	GLOBAL VAR :	histo, delta				 	|
  | 									|
  |	MSG_RETURN :	0 if ok, -1 if width <= 0			|
  | 									|
  +----------------------------------------------------------------------*/

int histo_set_delta( unsigned int width )
{
  if ( width > 0 )
    {
      delta = width;
      return 0;
    }
  else
    {
      error_msg("histo_set_delta", MSG_WARNING,
		"invalid interval width");
      return -1;
    }
}

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
  |	MSG_RETURN :	none						|
  | 									|
  +----------------------------------------------------------------------*/

void histo_init()
{
  int i;

  for(i=0;i<max;i++)
    histo[i]=0;

  nb_evt  = nb_sample = 0;
  sum_sample = 0.0;
}

/*----------------------------------------------------------------------+
  | 									|
  |	FUNCTION :	histo_enter					|
  | 									|
  |	DESCRIPTION :	outputs the histogram				|
  | 									|
  |	PARAMETERS :	value	value to put in the histogram	in us	|
  | 									|
  |	GLOBAL VAR :	histo, max, delta			 	|
  | 									|
  |	MSG_RETURN :	none						|
  | 									|
  +----------------------------------------------------------------------*/

void histo_enter( unsigned long value )
{
  sum_sample += value;
  nb_sample ++;
  if( value < ( max -1 ) * delta )
    histo[value/delta]++;
  else
    histo[max -1]++;
}

void histo_enter_with_date( unsigned long value, double date_evt )
{
  histo_enter (value);
  /* Only try to store value if more than the width step */
  if (value >= delta)
    {
      /* Guess if witdh is much bigger than mean, that this evt is over */
      if ( (value >= sum_sample/nb_sample*2) && (nb_evt < HISTO_MAX) )
	{
	  list_evt[nb_evt].date = date_evt;
	  list_evt[nb_evt++].delta = value;
	}
    }
}



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
  |	MSG_RETURN :	none						|
  | 									|
  +----------------------------------------------------------------------*/
 
void histo_dump( FILE *f, char *title )
{
  int i;
  double mean=0;
  fprintf(f, "=============== %s ==================\n", title);
	
  if (nb_sample!=0)  mean=sum_sample/nb_sample;

  fprintf(f, "DURATION HISTOGRAM (micro seconds) : nb_sample=%d  mean=%g us\n", nb_sample, mean);
  for(i=0;i<max -1;i++)
    {
      if (histo[i] !=0)
	fprintf(f,"%10u <= dt < %10u \t: %5ld\n", delta*i, delta*(i+1), histo[i]);
		
    }

  fprintf(f,"             dt >= %4d \t: %5ld\n", delta*(max-1), histo[max-1]);

  /* Guess if witdh is twice bigger than mean , that this evt is interisting */
  if (nb_evt !=0)
    {
      fprintf(f, "DATE Overrange (date in seconde, delta in micro seconds) : nb_evt=%d\n", nb_evt);
      for(i=0;i<nb_evt;i++)
	{
	  fprintf(f,"\t%10.6fs : \t %5lu\n", list_evt[i].date, list_evt[i].delta);
	}
    }

  fprintf(f, "===========================================\n");

}

