/***********************************************************************
 * ansiprint.cc
 *
 * Takes test from either stdin or specified file(s) and outputs it
 * wrapped in ANSI escape sequences, to allow printing from a terminal. 
 *
 * Files specified on the command line will take precedence over stdin.
 *
 * I also plan to add a choice of printing to stdout or to /dev/tty;
 * pine's implimentation does only /dev/tty, but that seems messy and
 * only of occassional use.
 *
 * To begin with I will impliment only the stdin part, since that is
 * slightly easier, then add the file support later.  Also, I will add
 * support for the option of a cntrl-D at the end of the output (I don't
 * know why, but pine's has one, so mine will too).  (The cntrl-D is
 * probably for printing a form-feed at the end.)
 * 
 * A respected friend says there is too much commenting in this program.
 * I'm not so sure I agree -- I like a lot of comments.  He says it's
 * insulting to the intelligence of other programmers who might modify
 * this.. well, here's my thoughts: better to make it easy to understand
 * at the risk of being insulting, than to inflate the egos of other
 * programmers at the risk of making my code hard to understand.  If you
 * have strong feelings either way, goddamnit, email me with 'em.
 *
 * Jason R. McVetta, mcvetta.3@osu.edu
 *
 * Begun: Nov. 21, 2000
 *
 * Last updated: Dec. 1, 2000
 *
 **********************************************************************/


/***** PREPROCESSOR STUFF *****/

// Supresses the escape sequences
#define NOPRINT
// duh
#undef DEBUG

// INCLUDES
#include <iostream.h>
#include <string.h> // Check to be sure we need this
// below are the includes for raw I/O
#include <fcntl.h>
#include <unistd.h>


/***** GLOBAL VARIABLE DECLARATIONS *****/

// CONSTANTS

/*
 * Size of buffer that will be used if user does not specify otherwise
 * on the command line.
 */
const int DEF_BUFSIZE = 512;


// COMMAND LINE OPTIONS

/*
 * input_type -- where the input we plan to print is coming from
 *	1 = stdin
 *	2 = command line
 *
 * *** N.B. It may be possible to do away with this variable entirely,
 * since the first_file variable (probably) makes input_type
 * redundant. 
 */	
static int input_type = 1; 

/*
 * output_file -- file descriptor to send output to
 *	1 = stdout
 *	otherwise, will be set using open() to /dev/tty
 */
static int output_file = 1;

/*
 * cntrl_d -- write and EOF character at the end of the file; used to
 * generate a form-feed at the end when that is desirable.
 *	0 = do NOT write an EOF
 *	1 = DO write an EOF
 */
static int cntrl_d = 0;

/*
 * chosen_bufsize -- the actual size of (read) buffer that will be
 * used.  Defaults to DEF_BUFSIZE, but can be specified on the
 * command line. 
 */
static int chosen_bufsize = DEF_BUFSIZE;

/*
 * first_file -- gives the number of the first argument that is a
 * file name.  
 *
 * For example, with a command line that looks like this:
 *
 *		ansiprint -B512 -D foo.txt bar.txt
 *
 * first_file would be 3
 *
 * Zero is the default value, and indicates that there are no files
 * specifed on the command line.
 */
static int first_file = 0;



/**********************************************************************
 * process_cmd_line
 *
 * Looks at the command line options, if there are any, and sets the
 * global option variables appropriately.
 *
 * PARAMETERS
 *	argc -- number of cmd line arguments
 *	argv[] -- pointers to the command line arguments
 *
 * RETURNS
 *	(none)
 **********************************************************************/
void process_cmd_line (int argc, char *argv[]) 
{
	/*
	 * This function is only called if there are arguments on the
	 * command line, so we will start out assuming that first_file = 1
	 */
	
	while ((argc > 1) && (argv[1][0] == '-'))
	{
		switch(argv[1][1])
		{
			// -d insert an EOF file at end
			case 'd':
				cntrl_d = 1;
				break;
			case 't':
				output_file=open("/dev/tty",O_WRONLY);
#ifdef DEBUG				
				cout << "\n\n" << output_file << "\n";
#endif //DEBUG				
				break;
			// more!!!!
		}
	++argv;
	--argc;
	}
}


/**********************************************************************
 * do_buffer
 * 
 * Gets one bufferload of input from a designated input file (which may
 * be stdin).  If this buffer is not empty, prints it to a
 * specified output file (which will be either stdout or /dev/tty).
 * Returns the number of characters read (and thus printed).
 *
 * PARAMETERS
 *	input_file -- the file-descriptor to read (0 is stdin)
 *	output_file -- the file-descriptor to write to (1 is stdout)
 *	bufsize buffer size to use
 *
 * RETURNS
 *	size of buffer read (which is useful principally to see if it read
 *		anything at all
 **********************************************************************/
int do_buffer(int input_file, int output_file, int bufsize)
{
	char buffer[bufsize];

	/* used to capture the size of the read; if the buffer is full,
	 * will be the same as bufsize (or else bufsize-1??) */
	int read_size;
	
	read_size = read(input_file, buffer, bufsize);

	/* test to see if there is something in the buffer, and if
	 * there is, print it to stdout. */
	if ( read_size > 0 ) 
	{
#ifndef DEBUG
		write(output_file, buffer, read_size);
#endif //DEBUG
	}
	
	return (read_size);
}
		

/*************************************************************************
N.B. one can make a function just like above, but instead of cout
using write to /dev/tty.
**************************************************************************/




int main(int argc, char *argv[]) 
{
	// Take a look at the command line
	process_cmd_line(argc, argv);
	
	if (first_file > 0)
		/***** Not yet implimented *****/
		cerr << "File input not yet implimented.\n";
	else
		/*
		 * Since first_file == 0, that means there are no files
		 * specified on the command line.  So we will take input from
		 * stdin.
		 */
		while (do_buffer(0, output_file, chosen_bufsize) > 0);

}
