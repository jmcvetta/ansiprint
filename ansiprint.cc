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

/*
 * separate_files -- print a cntrl-D (EOF) between each file specified
 * on the cmd line.
 *
 * 0 = do NOT separate
 * 1 = DO separate
 */
int static separate_files = 1;



/***** FUNCTIONS *****/



/**********************************************************************
 * usage -- print correct syntax
 *
 * (Printed to stderr; should it instead go to stdout?)
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
 	<< "USAGE:  ansiprint [-d] [-t] [-S] [-b<buffersize>] [-f file1 file2 ...]\n"
 	<< "\n"
   	<< "\t -d  Write an EOF (cntrl-D) after the last file (for form-feeding)\n"
	<< "\t -S  Do NOT write an EOF between each separate file specified on the command line\n"
  	<< "\t -t  Write output to /dev/tty instead of stdout\n"
 	<< "\t -b<buffersize>  Set the read/write buffer to <buffersize>.  (default = 512 bytes)\n"
 	<< "\n"
	<< "DEFAULT BEHAVIOR:\n"
	<< "Unless '-f' is specified, files on the command line are ignored and stdin is\n"
	<< "printed.  If '-f' is specified, stdin is ignored.  When multiple files are\n"
	<< "given, an EOF is printed after each file except the last, unless '-S' is\n"
	<< "specified.  If '-S' is specified, the files are seperated by only a newline\n"
	<< "character.  Independent of this, the user may elect to include an EOF after all\n"
	<< "input has been printed, using '-d'.  Ansiprint does not recognize multiple\n"
	<< "files piped to stdin, and will do nothing special for them.\n"
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
	
	// A throwaway for the while() below.
	int x;
	
	// Stays == 0 unless '-f' is specified
	int use_files = 0;
	
	/*
	 * If there are no options, process_cmd_line need not return any
	 * values.  Since, if '-f' is not specified, any files named on the 
	 * cmd line will be ignored.
	 */
	while ((x = getopt(argc, argv, "Sdtfb:")) != -1)
	{
		switch(x)
		{
			case 'S':
				separate_files = 0;
				break;
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
			case '?':
				usage();
				break;
			// Fall through: must be a syntax error
			default:
				usage();
				break;
		}
	}
	
	// The next argument should be the first file
	if (use_files == 1)
		/*
		 * The first file will be the first cmd line argument that
		 * is not an option (i.e. does not begin with a '-'.  After all
		 * option have been processed, optind is 1 higher than the
		 * position of the last option.  argv[] starts at zero, whereas
		 * optind starts at 1.  So the final optind value will be the
		 * same as the argv[] position of the first file.
		 */
		first_file = optind;
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

	/* 
	 * used to capture the size of the read; if the buffer is full,
	 * will be the same as bufsize (or else bufsize-1??)
	 */
	int read_size;
	
	read_size = read(input_file, buffer, bufsize);

	/* test to see if there is something in the buffer, and if
	 * there is, print it to stdout. */
	if ( read_size > 0 ) 
	{
		write(output_file, buffer, read_size);
	}
	
	return (read_size);
}
		


/*************************************************************
 * process_files -- print files(s) specified on command line
 *
 * PARAMETERS
 *  argc, argv (usual meanings); also uses first_file, which is 
 *  a static var.
 *
 * RETURNS
 *  number of files printed (is this necessary/useful?)
 ************************************************************/
 int process_files(int argc, char *argv[])
 {
	int counter; // Current file we are using
	
	for (counter = first_file; counter < argc; ++counter)
	{
		input_file = open(argv[counter], O_RDONLY);
		
		// Prints the file
		while (do_buffer() > 0);

#ifndef NOPRINT
		/*
		 * Unless the user has specifically requested that files not be
		 * separated by an EOF, we will insert one after each file
		 * EXCEPT the last file -- the last file will still be handled
		 * according to the cntrl_d setting.
		 */
		if ((separate_files == 1) && (counter < argc - 1))
			write(output_file,"\004",1);  // EOF
		elif (counter < argc - 1)
			write(output_file,"\012",1);  // Newline
#endif //NOPRINT
				
		close(input_file);
	}
#ifndef NOPRINT
	// Prints an EOF after the LAST file
	if (cntrl_d)
		write(output_file,"\004",1);
#endif //NOPRINT
	
	// The number of files printed
	return (counter - first_file);
		
	 	
 }



int main(int argc, char *argv[]) 
{
	// Take a look at the command line
	process_cmd_line(argc, argv);

#ifndef NOPRINT
	// Beginning ANSI print code
	write(output_file,"\033[5i",4);
#endif //NOPRINT


	/*
	 * Since first_file == 0, that means there are no files
	 * specified on the command line.  So we will take input from
	 * stdin.  (input_file = 0 by default)
	 */
	if (first_file > 0)
		process_files(argc, argv);
	else
	{
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
#endif //NOPRINT
	}	

	// Closing ANSI print code.
#ifndef NOPRINT
	write(output_file,"\033[4i",4);
#endif //NOPRINT


}
