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
 * For latest updates, see the CVS tree on ansiprint's Sourceforge
 * project site at:  sourceforge.net/projects/ansiprint
 *
 **********************************************************************/


/***** PREPROCESSOR STUFF *****/

// Supresses the escape sequences
#define NOPRINT

#define DEBUG

// INCLUDES
#include <iostream.h>
#include <string.h> // Check to be sure we need this
#include <stdlib.h>
// below are the includes for raw I/O
#include <fcntl.h>
#include <unistd.h>


/***** GLOBAL VARIABLE DECLARATIONS *****/

// CONSTANTS

/*
 * Size of buffer that will be used if user does not specify otherwise
 * on the command line.  *** Is this a good size??? ***
 */
const int DEF_BUFSIZE = 512;


// COMMAND LINE OPTIONS

/*
 * input_file -- the file being input
 */	
static int input_file = 0; 

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
 * bufsize -- the actual size of (read) buffer that will be
 * used.  Defaults to DEF_BUFSIZE, but can be specified on the
 * command line. 
 */
static int bufsize = DEF_BUFSIZE;

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




/***** FUNCTIONS *****/



/**********************************************************************
 * usage
 *
 * Prints the correct syntax for the program to stderr.  (Should it
 * instead go to stdout?)
 *
 * PARAMETERS
 * 	[none]
 *
 * RETURNS
 * 	[none]
 *********************************************************************/
void usage(void)
 {
 	cerr
 	<< "\n"
 	<< "USAGE:  ansiprint [-d] [-t] [-b<buffersize>] [file1] [file2] ...\n"
 	<< "	-d  Write an EOF (cntrl-D) after each file (for form-feeding)\n"
 	<< "	-t  Write output to /dev/tty instead of stdout\n"
 	<< "	-b<buffersize>  Set the read/write buffer to <buffersize>\n"
 	<< "\n"
 	<< "N.B. If file(s) are specified on the command line, stdin is ignored.\n"
 	<< "\n"
 	<< "File input not yet supported!\n"
 	<< "\n";
 	exit(-1);
 }



/**********************************************************************
 * process_cmd_line
 *
 * Looks at the command line options, if there are any, and sets the
 * global option variables appropriately.
 *
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
	 * command line.  However, it may be the case either that there are
	 * only options, only files, or both, on the line.
	 */
	
	extern char *optarg;
	extern int optind;
	
	// The output of getopt
	int x;
	
	x = getopt(argc, argv, "dtfb:");
	cout << x << endl;
	
	while (x != -1)
	{
		// Unless '-f' is specified as an option, use_files = 0
		int use_files = 0;
		while (optind < argc - 1)
		{
			cout << "argc == " << argc << "\n";
			cout << "optind == " << optind << "\n";
			cout << "x == " << x << "\n";
			switch(x)
			{
				case 'd':
					cntrl_d = 1;
					break;
				case 't':
					output_file=open("/dev/tty",O_WRONLY);
					break;
				case 'f':
					use_files = 1;
					break;
				case 'b':
					bufsize = atoi(optarg);
					break;
				// Fall through: must be a syntax error
				default:
					usage();
			}
			x = getopt(argc, argv, "dtfb:");
		}
		// The next argument should be the first file
		if (use_files == 1)
			/*
			 * The first file will be the first cmd line argument that
			 * is not an option (i.e. does not begin with a '-'.
			 */
			first_file = optind + 1;
	}
	return;
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
 *	all of do_buffers parameters are static variables
 *
 * RETURNS
 *	size of buffer read (which is useful principally to see if it read
 *		anything at all
 **********************************************************************/
int do_buffer(void)
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
		




int main(int argc, char *argv[]) 
{
	// Take a look at the command line
	process_cmd_line(argc, argv);

	cout << "first file == " << first_file << "\n";
		
	if (first_file > 0)
		/***** Not yet implimented *****/
		cerr << "File input not yet implimented.\n";
	else
	{
		/*
		 * ***** The print codes should be moved to encompas the entire
		 * if/else statement once file-input is supported. *****
		 */
#ifndef NOPRINT
		write(output_file,"\033[5i",4);
#endif //NOPRINT
		
		/*
		 * Since first_file == 0, that means there are no files
		 * specified on the command line.  So we will take input from
		 * stdin.  (input_file = 0 by default)
		 */
		while (do_buffer() > 0);
		
		/*
		 * unlike the initial and closing print codes, the cntrl-d code
		 * should not be moved outside of the buffer section.  This is
		 * because, if cntrl-d is specified for files on the command
		 * line, we will want to print one after each file (since it
		 * causes a form-feed).
		 */
#ifndef NOPRINT
		if (cntrl_d)
			write(output_file,"\004",1);
		// see first note regarding print codes
		write(output_file,"\033[4i",4);
#endif //NOPRINT
	
	}	
}
