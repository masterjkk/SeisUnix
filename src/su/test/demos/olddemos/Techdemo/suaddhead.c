/*********************** self documentation ******************************/
char *sdoc = "\
									\n\
SUADDHEAD - puts headers on bare traces and sets the tracl and ns fields\n\
									\n\
suaddhead <stdin >stdout ns= ftn=0					\n\
									\n\
Required parameter:							\n\
	ns = the number of samples per trace				\n\
									\n\
Optional parameter:							\n\
	ftn = 0		Fortran flag					\n\
			0 = data written unformatted from C		\n\
			1 = data written unformatted from Fortran	\n\
									\n\
The ns field and the tracl field are set.  The other fields are		\n\
zeroed out.  Sushw and suchw can then be used to set other fields.	\n\
									\n\
Caution:								\n\
	An incorrect ns field will munge subsequent processing.		\n\
									\n\
Example:								\n\
suaddhead ns=1024 <bare_traces | sushw key=dt a=4000 >segy_traces	\n\
									\n\
This command line adds headers with ns=1024 samples.  The second part	\n\
of the pipe sets dt to 4 ms.						\n\
									\n\
";
/*************************************************************************/

/* suaddhead - add SEGY headers to bare traces
 *
 * Example:
 *	suaddhead ns=1024 <bare_trace_file |
 *	sushw key=dt a=4000 >segy_trace_file
 *	(adds header with the ns field set to 1024 samples and the
 *	 tracl field set and then uses sushw to set the dt field to 4 ms)
 *
 * Credits:
 *	SEP: Einar
 *	CWP: Jack
 *
 * Caveats:
 *	Only sets the ns field (mandatory user input), and the tracl field.
 *	The other fields are zeroed out.
 *	Sushw and suchw can be used to set or change fields.
 *
 *----------------------------------------------------------------------
 * Permission to use the SU programs is granted.  Courteous users will
 * retain this acknowlegment of origins at the Stanford Exploration
 * Project, Stanford University and the Center for Wave Phenomena, 
 * Colorado School of Mines.   Inquiries/bug reports should be sent to:
 * Jack K. Cohen, Center for Wave Phenomena, Colorado School of Mines,
 * Golden, CO 80014
 *----------------------------------------------------------------------
 *
 * $Author: jkc $
 * $Source: /src/su/src/RCS/suaddhead.c,v $
 * $Revision: 2.4 $ ; $Date: 88/11/20 22:31:49 $
*/

/* Embed Revision Control System identifier strings */
static char progid[] =
	"   $Source: /src/su/src/RCS/suaddhead.c,v $";
static char revid[] =
	"   $Revision: 2.4 $ ; $Date: 88/11/20 22:31:49 $";


#include "../include/cwp.h"
#include "../include/segy.h"

segy tr;

main(argc, argv)
int argc; char **argv;
{
	ushort ns;		/* number of samples			*/
	long tracl;		/* trace number (one-based)		*/
	int ftn;		/* ftn=1 for Fortran			*/
	char junk[sizeof(int)];	/* to discard ftn junk  		*/
	int (*rdfn) ();		/* pointer to efread() or pfread()	*/
	int efread();		/* must be declared to use pointer	*/
	int pfread();		/* must be declared to use pointer	*/


	initgetpar(argc, argv); askdoc(1);


	/* Set appropriate read function for input filetype.  Here, we	*/
	/* use a buffered read with system error checks.  The arguments	*/
	/* are the same as for fread: ptr, itemsize, nitems, stream	*/
	switch(statfil(STDIN)) {
	case TTY:
		err("input can't be tty");
	break;
	case DISK:
	case TAPE:
		rdfn = efread;
	break;
	case PIPE:
		rdfn = pfread;
	break;
	default:
		err("undefined input filetype %s", statprint(STDIN));
	break;
	}

	/* Get parameters */
	MUSTUGETPAR("ns", &ns);
	ftn = 0;	igetpar("ftn", &ftn);
	if (ftn != 0 && ftn != 1) {
		err("ftn=%d must be 0 or 1", ftn);
	}

	tr.ns = ns;

	tracl = 0;
	while (TRUE) {

		/* If Fortran data, read past the record size bytes */
		if (ftn) (*rdfn) (junk, 1, sizeof(int), stdin);

		/* Do appropriate read of data for the segy */
		switch((*rdfn) ((char *) tr.data, sizeof(float),
							ns, stdin)) {
		case 0: /* finished */
			exit(0);
		default:
			tr.tracl = ++tracl;
			puttr(&tr);
		}

		/* If Fortran data, read past the record size bytes */
		if (ftn) (*rdfn) (junk, 1, sizeof(int), stdin);

	}
}
