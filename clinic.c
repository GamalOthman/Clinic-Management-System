// This program is a reservation data base
// for a clinic

#include <stdio.h>
#include <stdlib.h>
#include <string.h>			// for strcmp()
#include <ctype.h>			// for tolower()
#include "typedefs.h"
#include "colors.h"

// colorize input and output text:
// if your OS is linux, you can set condition to 1.
#if 0
	#define ERR_CLR		BRI_RED		// for errors
	#define OUT_CLR		RESET		// for output
	#define IN_CLR		BRI_GREEN	// for input
	#define HED_CLR		BRI_BLUE	// for headers
	#define CRD_CLR		BRI_YELLOW 	// for output cards
#else
	#define ERR_CLR		""		
	#define OUT_CLR		""		
	#define IN_CLR		""		
	#define HED_CLR		""		
	#define CRD_CLR		""	 	
#endif

#define WORD_LEN 50			// input word length in chars
#define SLOTS_NUM 5			// number of slots
#define ADMIN_PASS 1234		// only a 4-digit number

#define MEM_ERR do {\
	puts(ERR_CLR "Cannot allocate memory!");\
	exit(EXIT_FAILURE);\
	} while(0)

enum orders {
	MODE
	, ADD
	, EDIT
	, RESERVE
	, CANCEL
	, VIEW
};

enum boolean { false = 0, true = 1 };

enum modes { USER, ADMIN };
u8 mode = USER;		// default mode

enum wordModes { ALPHA, NUM };
char word[WORD_LEN];	// public share word


// gets a line from stdin
char *
getLine( int len )
{
	char * line = malloc( len * sizeof(char) );
	if( line == NULL ) { MEM_ERR; }

	char * p = line;
	char c;
	while( (c = getchar()) != EOF && c != '\n' && len-- > 2 )
	{
		if( c == '\0' ) 
			{ return NULL; }
		else
			{ *p++ = c; }
	}
	*p++ = '\n';
	*p = '\0';

	// flush the line:
	if( c != '\n' )
		{ while( getchar() != '\n' ); }

	return line;
}

// gets a word from stdin, stores it in 
// the global array (word[])
char *
getWord( u8 wordMode, u8 len )
{
	len += 2;		// for newline and '\0'

	// receive a word:
	u8 trials = 3;
	while( trials-- > 0 )
	{		
		// switch to bright green
		fputs( IN_CLR, stdout );
		char * str = getLine( len );
		if( str == NULL )
			{ puts(ERR_CLR "error: getline() returned NULL!"); exit(EXIT_FAILURE);}
		else
			{ strncpy( word, str, WORD_LEN ); free(str);}
		
		*( word + strlen(word)-1 ) = '\0';	// delete '\n' character
		
		char * p = word;
		// skip leading spaces or tabs:
		while( isblank(*p) )
			p++;

		// check if it is as required:
		u8 flag = 0;
		for( ; *p != '\0'; p++ )
		{
			*p = tolower(*p);
			if( wordMode == ALPHA && !(isalpha(*p) || isblank(*p)) )
				flag = 1;
			else if( wordMode == NUM && !isdigit(*p) )
				flag = 2;
		}
		// quit program if word is "exit":
		if( !strcmp(word, "exit") )
			exit( EXIT_SUCCESS );

		if( !flag )
			break;
		else if( flag == 1 )
			puts(ERR_CLR "Please enter a word! (chars only)");
		else
			puts(ERR_CLR "Please enter a number!");
	}
	if( trials <= 0 )
		exit( EXIT_FAILURE );

	return word;
}

// uses getWord() to get a number from stdin
u32
getNum( u8 max )
{
	u8 trials = 3;
	while( trials-- > 0 )
	{
		getWord( NUM, max );
		if( strlen(word) > max+1 )
			puts(ERR_CLR "Please enter a valid number!");
		else
			break;
	}
	if( trials <= 0 )
		exit( EXIT_FAILURE );

	u32 num = (u32) strtol( word, 0, 0 );		// declared in stdlib.h
	return num;
}

// gets an order from the user
u8
getOrder( void )
{
	puts(HED_CLR "\n** Main Menu **");
	fputs(OUT_CLR, stdout);
	puts("Select user mode: mode");
	puts("Patient orders: add, edit, view");
	puts("Reservation orders: reserve, cancel");
	puts("* write \"exit\" anytime to quit *");

	// get an order:
	fputs( OUT_CLR "\nEnter an order: ", stdout );
	getWord( ALPHA, WORD_LEN );

	// mode:
	if( !strcmp(word, "mode") )
		return MODE;
	// add:
	else if( !strcmp(word, "add") )
		return ADD;
	// edit:
	else if( !strcmp(word, "edit") )
		return EDIT;
	// reserve:
	else if( !strcmp(word, "reserve") )
		return RESERVE;
	// cancel:
	else if( !strcmp(word, "cancel") )
		return CANCEL;
	// view:
	else if( !strcmp(word, "view") )
		return VIEW;
	
	return -1;
}

// sets the user mode (ADMIN or USER)
void
setMode()
{
	// get a mode:
	fputs(OUT_CLR "Choose a mode: user, admin? ", stdout);
	getWord( ALPHA, WORD_LEN );

	if( !strcmp(word, "admin") )
	{
		u8 i;
		for( i = 1; i <= 3; i++ )
		{
			fputs( OUT_CLR "Password: ", stdout );
			u32 num = getNum(4);
			if( num == ADMIN_PASS )
			{
				mode = ADMIN;
				puts( CRD_CLR "\nAdmin Mode Entered!" );
				return;
			}
		}
		puts( "" );
		exit( EXIT_FAILURE );
	}
	else if( !strcmp(word, "user") )
	{
		mode = USER;
		puts( CRD_CLR "\nUser Mode Entered!" );
		return;
	}
	else
		puts(ERR_CLR "Mode not defined!");
}

// checks if the order requested by yser is authorized:
_Bool
isAutorized( u8 order )
{
	if( mode == ADMIN )
		{ return true; }
	else if( order != ADD && order != EDIT 
		&& order != RESERVE && order != CANCEL )
		{ return true; }
	
	puts( ERR_CLR "Only admin can access!" );
	return false;
}

typedef struct betengan
{
	u32 id;
	char * name;
	u8 age;
	u8 gender;
	struct betengan * next;
} Patient;
typedef Patient * Patient_ptr;

Patient headStruct;
Patient_ptr const head = &headStruct;
_Bool headExists;

enum gender { MALE = 1, FEMALE };

// searches for (id) and returns pointer to 
// Patient where (id) exists, or NULL if not found. 
Patient_ptr
idExists( u32 id )
{
	Patient_ptr pp = head;
	for( ; pp != NULL; pp = pp->next )
		if( pp->id == id )
			return pp;

	return NULL;
}

// allocates memory for a new Patient,
// and returns a pointer to the new Patient.
// used by addPatient().
Patient_ptr
allocatePatient()
{
	// if head not filled yet:
	if( !headExists ) 
	{	
		headExists = true;
		return head;
	}
	else
	{	
		// allocate struct and name:
		Patient_ptr pp = malloc( sizeof(Patient) );
		if( pp == NULL ) { MEM_ERR; }
		
		// link the new struct with the last one:
		Patient_ptr lastpp = head;
		while( lastpp->next != NULL )
			{ lastpp = lastpp->next; }
		lastpp->next = pp;
		pp->next = NULL;

		return pp;
	}
}

// fills information (name, age, gender) in a Patient.
// used by addPatient() and editPatient().
void
fill( Patient_ptr pp )
{
	// name:
	fputs( OUT_CLR "Enter the name: ", stdout );
	getWord( ALPHA, WORD_LEN );
	pp->name = malloc( WORD_LEN * sizeof(char) );
	if( pp->name == NULL ) { MEM_ERR; }
	strncpy( pp->name, word, WORD_LEN );
	
	// age:
	fputs( OUT_CLR "Enter the age: ", stdout );
	pp->age = (u8) getNum(4);
	
	// gender:
	fputs( OUT_CLR "Enter the gender: ", stdout );
	_Bool flag = 1;
	while( flag )
	{	
		flag = 0;
		getWord( ALPHA, WORD_LEN );

		if( !strcmp(word, "male") )
			pp->gender = MALE;
		else if( !strcmp(word, "female") )
			pp->gender = FEMALE;
		else
			{ puts( ERR_CLR "Wrong input! Please re-enter:" ); flag = 1; }
	}
	puts( OUT_CLR "\n* Patient recorded successfully! *");
}

// adds a new Patient.
void
addPatient()
{
	puts( HED_CLR "\n** New Patient **" );

	// id:
	fputs( OUT_CLR "\nEnter the ID (4 digits max): ", stdout );
	u32 temp = getNum(4);
	if( idExists(temp) )
	{
		puts( ERR_CLR "ID already exists!" );
		return;
	}
	else
	{
		Patient_ptr pp = allocatePatient();
		pp->id = temp;
		fill(pp);
	}
}

// edits the infromation in a Patient.
// it requires the Patient ID.
void
editPatient()
{
	// if there's no head:
	if( !headExists )			
		{ puts( ERR_CLR "There's no records yet!"); return; }
	else
		puts( HED_CLR "\n** Edit Patient/s **" );

	fputs( OUT_CLR "\nEnter the ID: ", stdout );
	u32 id = getNum(4);
	Patient_ptr pp = idExists(id);
	if( pp == NULL )
		{ puts( ERR_CLR "Can't find matching ID!"); return; }
	else
		fill(pp);
}

// prints a char multiple times.
// used by printPatient(), printSlot() and welcomeMessage().
void
printMulti( char c, u32 num, char tail )
{
	while( num-- > 0 )
		{ printf("%c", c); }

	printf("%c", tail);
}

// prints the information of a Patient.
// used by viewPatient().
void
printPatient( Patient_ptr pp )
{
	u8 wid = 30;	// width
	u8 ptd;			// printed chars

	fputs( CRD_CLR, stdout );
	printMulti( '-', wid-1, '\n' );

	ptd = printf( "ID: %d", pp->id );
	printf( "%*c\n", wid-ptd-1, '|');
	
	ptd = printf( "Name: %s", pp->name );
	printf( "%*c\n", wid-ptd-1, '|');
	
	ptd = printf( "Age: %d", pp->age );
	printf( "%*c\n", wid-ptd-1, '|');
	
	ptd = printf( "Gender: %s"
		, (pp->gender == MALE)? "male" : "female" );
	printf( "%*c\n", wid-ptd-1, '|');

	printMulti( '-', wid-1, '\n' );
}

// views the information of all Patients
// or a certain Patient.
void
viewPatient()
{
	// if there's no head:
	if( !headExists )			
		{ puts(ERR_CLR "There's no records yet!"); return; }
	else
		puts( HED_CLR "\n** View Patient/s **" );

	fputs( OUT_CLR "\nEnter patient ID (enter '0' to view all): ", stdout );
	u32 idx = getNum(4);

	Patient_ptr pp;
	// view all:
	if( !idx )
	{
		puts(OUT_CLR "\nShowing all records...");
		for( pp = head; pp != NULL; pp = pp->next )
			printPatient(pp);
	}

	// view one:
	else
	{
		if( (pp = idExists(idx)) )
			{ printPatient(pp); }
		else
			{ puts(ERR_CLR "Nothing Found!"); }
	}
}

typedef struct mangah
{
	_Bool reserved;
	u32 fromHour;
	u32 toHour;
	u32 id;
} Slot;

// Because number of slots is limited,
// we'll not use malloc.
// For code to be clear and readable,
// we'll not use the zeroth element.
Slot slotArr[ SLOTS_NUM+1 ];

u8 reservedSlots;		// number of reserved slots

enum slotState { EMPTY, FULL };

// converts hh:mm to decimal value of time.
u32
hourToDec( u32 hour, u32 min )
{
	return (hour * 60 + min);
}

// converts decimal value of time to hh:mm.
void
decToHour( u32 decVal, u8 * hour, u8 * min )
{
	*hour = decVal / 60;
	decVal %= 60;			// 60 mins in hour

	*min = decVal;
}

// initializes the slots with its period and number.
// will run one time on program start.
void
initializeSlots()
{
	u32 from = hourToDec( 2, 0 );		// start from 2:00
	u32 period = hourToDec( 0, 30 );	// period of slot 00:30
	u32 to = from + period;

	u8 i;
	for( i = 1; i <= SLOTS_NUM; i++ )
	{
		slotArr[i].reserved = false;
		slotArr[i].fromHour = from;
		slotArr[i].toHour = to;

		// move to next period:
		from = to;
		to = from + period;
	}
}

// printing empty slots or reserved ones,
// according to printMode.
void
printSlot( u8 i, u8 printMode )
{
	// return if you're printing empty slots while slot is reserved,
	// or you're printing full slots while slot is empty.
	if( printMode == EMPTY && slotArr[i].reserved )
		{ return; }
	else if( printMode == FULL && !slotArr[i].reserved )
		{ return; }

	u8 fromH, fromM, toH, toM;
	decToHour( slotArr[i].fromHour, &fromH, &fromM );
	decToHour( slotArr[i].toHour, &toH, &toM );

	u8 wid = 20;		// width
	u8 ptd;				// printed chars

	fputs( CRD_CLR, stdout );
	printMulti( '-', wid-1, '\n' );
	ptd = printf( "Slot number: %d", i );
	printf( "%*c\n", wid - ptd - 1, '|');
	ptd = printf( "%d:%.2d - %d:%.2d"
		, fromH, fromM, toH, toM );
	printf( "%*c\n", wid - ptd - 1, '|');	
	printMulti( '-', wid-1, '\n' );
}

// reserves an empty slot for a Patient.
void
reserveSlot()
{
	if( reservedSlots == SLOTS_NUM )
		{ puts(ERR_CLR "No more slots to reserve!"); return; }
	else
		{ puts(HED_CLR "\n** Reserve a Slot **"); }

	// printing available slots:
	puts(OUT_CLR "\nShowing all available slots...");
	u8 i;
	for( i = 1; i <= SLOTS_NUM; i++ )
		printSlot( i, EMPTY );

	// input data:
	u8 num = 0;
	u32 idx;
	puts( OUT_CLR "\nWhich slot do you want?" );

	fputs( OUT_CLR "Enter a number: ", stdout );
	num = getNum( 1 + SLOTS_NUM/10 );
	
	if( slotArr[num].reserved )
		{ puts(ERR_CLR "Slot is already reserved!"); return; }
	if( num < 1 || num > SLOTS_NUM )
		{ puts(ERR_CLR "Incorrect input!"); return; }

	fputs( OUT_CLR "Enter the ID: ", stdout );
	idx = getNum(4);

	// check if id exists:
	Patient_ptr pp = idExists(idx);
	if( pp == NULL )
		puts(ERR_CLR "This ID does not belong to any patient!");
	else
	{
		// check if this patient already has a slot:
		u8 i;
		for( i = 1; i <= SLOTS_NUM; i++ )
			if( slotArr[i].id == idx )
				{ puts(ERR_CLR "This ID has a reservation already!"); return; }

		// reserve a slot for this ID:
		slotArr[num].reserved = true;
		slotArr[num].id = idx;
		reservedSlots++;
		printf(OUT_CLR "\nSlot number %d is now\n" 
			"reserved for this patient:\n", (u32) num);
		printPatient(pp);
	}
}

// sets a certain slot as empty.
void
cancelReservation()
{
	if( !reservedSlots )
		{ puts(ERR_CLR "There's no reserved slots yet!"); return; }
	else
		puts(HED_CLR "\n** Cancel a Reservation **");

	puts(OUT_CLR "\nShowing reserved slots...");
	u8 i;
	for( i = 1; i <= SLOTS_NUM; i++ )
		printSlot( i, FULL );

	u8 num;
	puts( OUT_CLR "\nWhich slot do you want to cancel?");

	fputs( OUT_CLR "Enter a number: ", stdout );
	num = getNum( 1 + SLOTS_NUM/10 );

	if( slotArr[num].reserved == false )
		{ puts(ERR_CLR "This slot is not reserved!"); return; }
	if( num < 1 || num > SLOTS_NUM )
		{ puts(ERR_CLR "Incorrect input!"); return; }

	fputs(OUT_CLR "Are you sure you want to cancel [y/n]? ", stdout);
	getWord( ALPHA, 1 );
	if( strcmp(word, "y") )		// if input is not 'y'.	
		{ puts(OUT_CLR "Nothing cancelled!"); return; }
	
	slotArr[num].reserved = false;
	slotArr[num].id = 0;
	reservedSlots--;

	printf(OUT_CLR "Reservation for slot %d has been canceled!\n", num);
}

// displays a welcome message.
void
welcomeMessage()
{
	char * str = "Welcome To The Clinic Program!";
	fputs( HED_CLR, stdout );
	puts("");
	printMulti( '*', strlen(str) , '\n');
	puts(str);
	printMulti( '*', strlen(str) , '\n');
}

// the CEO function.
int main( void )
{	
	welcomeMessage();
	initializeSlots();

	while( 1 )
	{
		// get an order
		u8 order = getOrder();

		// check if user has access to that order:
		if( !isAutorized(order) )
			continue;

		switch( order )
		{
			case MODE:
			setMode();
			break;

			case ADD:
			addPatient(); 
			break;

			case EDIT:
			editPatient();
			break;

			case RESERVE:
			reserveSlot();
			break;

			case CANCEL:
			cancelReservation();
			break;

			case VIEW:
			viewPatient();
			break;

			default:
			puts( ERR_CLR "Order not defined!" );
			break;
		}	
	}
}
