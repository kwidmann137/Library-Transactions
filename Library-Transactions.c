/**********************************************************************
p5qox385c by Kyle Widmann
Purpose:
    This program reads book information and a command file.   It 
    processes the commands against the book information.
    This is the driver code for the routines written by the students.
Command Parameters:
    p3 -b bookFileName -c commandFileName
Input:
    Book	same as Programming Assignment #2 although there is different data.

    Command	This is different from the previous assignment.  The file 
            contains  text in the form of commands.  
        CUSTOMER BEGIN szBookId szCustomerId szCheckedOutDt dLateFeePerDay dMaxLateFee szTitle  
            specifies the beginning of customer request and includes all the 
            identification information from program 2.
        CUSTOMER ADDRESS szStreetAddress,szCity,szStateCd,szZipCd
            specifies the address for a customer (separated by commas)
        CUSTOMER TRANS cTransType    szBookId   szTransDt
            specifies a single book transaction.  Steps:
            -	Print the Transaction Type, Book Id, and Transaction Date 
            -	Lookup the book ID using a binary search.  If not found, print a 
                warning (but do not terminate your program) and return.
            -	If the transaction date is invalid, show a message stating "invalid date", 
                but do not terminate.  Use the validateDate  function.
            -	If the transaction type is C (meaning checkout):
                o	If the book is already checked out, show a message stating 
                    "already checked out", but do not terminate.
                o	Otherwise, check out the book to this customer, setting the book's 
                    customer ID.  The book's checked out date needs to be set to the 
                    transaction's date.
            -	If the transaction type is R (meaning return):
                o	Use dateDiff to subtract the book's szCheckOutDt from the transaction's szTransDt
                o	If the difference is more than 14:
                    --	Determine the number of days late by subtracting 14 from that difference.  
                    --	Compute the late fee by multiplying the number of days late 
                        by the book's dLateFeePerDay.
                    --	If that late fee is greater than the book's dMaxLateFee, 
                        change it to dMaxLateFee.
                    --  Print the late fee.
                    --	Add the computed late fee to the customer's dFeeBalance
                o	Set the book's customer ID to "NONE".
                o	Set the book's check out date to "0000-00-00".
        CUSTOMER COMPLETE
            specifies the completion   of a customer.  Print the total fees for this customer.
        BOOK CHANGE szBookId dLateFeePerDay dMaxLateFee
            change the Late Fee Per Day and Max Late Fee to the specified values
        BOOK SHOW szBookId    
            requests a display of a particular book.  Show all of its information.
               
Results:
    Prints the Books prior to sorting
    Prints the Books after sorting.
    Processes the commands (see above) and shows any errors.
    Prints the resulting Books
Returns:
    0  normal
    -1 invalid command line syntax
    -2 show usage only
    -3 error during processing, see stderr for more information

Notes:
    p3 -?       will provide the usage information.  In some shells,
                you will have to type p3 -\?

**********************************************************************/

// If compiling using visual studio, tell the compiler not to give its warnings
// about the safety of scanf and printf
#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cs1713p5.h"

/******************** printBooks **************************************
    void printBooks(char *pszHeading, Node *pRoot)
Purpose:
    Prints each book in a table format.
Parameters:
    I char  *pszHeading    Heading to print before printing the table of books
    I NodeT *pRoot         Pointer to the root of the binary tree of books
Returns:
    n/a
Notes:
    
**************************************************************************/
void printBooks(char *pszHeading, NodeT *pRoot)
{
    printf("\n%s\n", pszHeading);

    // output column headings
    printf("   %-9s %-40s %-8s %-10s %-8s %-8s \n"
        , "Book Id", "Title", "Customer", "Ck Out Dt", "Late Fee", "Max Late");

    //Call printInOrder
    printInOrder(pRoot);
}

/******************** printInOrder **************************************
    void printInOrder(NodeT *p)
Purpose:
    Prints each book in a table format, the heading for the table is printed
    in the calling function.
Parameters:
    I Node *p       Pointer to the root of the binary tree of books
Returns:
    n/a
Notes:
    
**************************************************************************/
void printInOrder(NodeT *p)
{
  //printf("in printInOrder.  p = %p  book = %s\n", p, p->book.szBookId);
  if(p == NULL)
    return;
  printInOrder(p->pLeft);
  printf("   %-9s %-40s %-8s %-10s %8.2lf %8.2lf \n"
            , p->book.szBookId
            , p->book.szTitle
            , p->book.szCustomerId
            , p->book.szCheckedOutDt
            , p->book.dLateFeePerDay
            , p->book.dMaxLateFee
            );
  printInOrder(p->pRight);
}

/******************** prettyPrintT **************************************
    void prettyPrintT(NodeT *p,int iIndent)
Purpose:
    Prints each book in a pretty format, with the indentation of the book
    correlating to the level on the binary tree
Parameters:
    I NodeT *p      Pointer to the root of the binary tree
    I int iIndent   Integer relating to the level of the binary tree that the
                    current p is on.  Passed an initial value of 0.
Returns:
    n/a
Notes:
    
**************************************************************************/
void prettyPrintT(NodeT *p,int iIndent)
{
  if (p==NULL)
    return;
  prettyPrintT(p->pLeft, iIndent+2);
  printf("%*s%s\n", iIndent, " ", p->book.szBookId);
  prettyPrintT(p->pRight, iIndent+2);
}


/********************processCustomerCommand *****************************
    void processCustomerCommand(NodeT *pRoot
    , char *pszSubCommand, char *pszRemainingInput
    , Customer *pCustomer)
Purpose:
    Processes the subcommands associated with the CUSTOMER command:
        CUSTOMER BEGIN szBookId szCustomerId szCheckedOutDt dLateFeePerDay dMaxLateFee szTitle  
            specifies the beginning of customer request and includes all the identification information from program 2.
        CUSTOMER ADDRESS szStreetAddress,szCity,szStateCd,szZipCd
            specifies the address for a customer (separated by commas)
        CUSTOMER TRANS cTransType    szBookId   szTransDt
            specifies a single book transaction.  Steps:
            -	Print the Transaction Type, Book Id, and Transaction Date 
            -	Lookup the book ID using a binary search.  If not found, print a warning (but do not terminate your program) and return.
            -	If the transaction date is invalid, show a message stating "invalid date", but do not terminate.  Use the validateDate  function.
            -	If the transaction type is C (meaning checkout):
                o	If the book is already checked out, show a message stating "already checked out", but do not terminate.
                o	Otherwise, check out the book to this customer, setting the book's customer ID.  The book's checked out date needs to be set to the transaction's date.
            -	If the transaction type is R (meaning return):
                o	Use dateDiff to subtract the book's szCheckOutDt from the transaction's szTransDt
                o	If the difference is more than 14:
                    --	Determine the number of days late by subtracting 14 from that difference.  
                    --	Compute the late fee by multiplying the number of days late by the book's dLateFeePerDay.
                    --	If that late fee is greater than the book's dMaxLateFee, change it to dMaxLateFee.
                    --  Print the late fee.
                    --	Add the computed late fee to the customer's dFeeBalance
                o	Set the book's customer ID to "NONE".
                o	Set the book's check out date to "0000-00-00".
        CUSTOMER COMPLETE
            specifies the completion   of a customer.  Print the total fees for this customer.


Parameters:
    I   NodeT *pRoot                  Pointer to the root of binary tree for books.
    I   char  *pszSubCommand          Should be BEGIN, ADDRESS, REQUEST or COMPLETE
    I   char  *pzRemainingInput       Points to the remaining characters in the input
                                      line (i.e., the characters that following the
                                      subcommand).
    I/O Customer *pCustomer           The BEGIN subcommand begins a new customer.  
Notes:

**************************************************************************/
void processCustomerCommand(NodeT *pRoot
    , char *pszSubCommand, char *pszRemainingInput
    , Customer *pCustomer)
{
  NodeT *pFind;
    int iscanfCnt;
    Transaction transaction;
    // Determine what to do based on the subCommand   
    if (strcmp(pszSubCommand, "BEGIN") == 0)
    {

      //assign each field to variable, reformat and print
      if (sscanf(pszRemainingInput, "%s%lf%s%[^\n]", pCustomer->szCustomerId, &pCustomer->dFeeBalance, pCustomer->szEmailAddr, pCustomer->szFullName) != 4)
	{
	  exitError(ERR_CUSTOMER_ID_DATA, pszRemainingInput);
	}
    }
    else if (strcmp(pszSubCommand, "COMPLETE") == 0)
    {
        // print the customer's total fees
        printf("\t\t\t\t\tTotal Fees: %.2lf\n\n\n", pCustomer->dFeeBalance); 

    }
    else if (strcmp(pszSubCommand, "ADDRESS") == 0)
    {
        printf("********************Library Receipt********************\n");
	//Print the customer ID info
	printf("%s %s %s (previously owed $%.2lf)\n", pCustomer->szCustomerId, pCustomer->szEmailAddr, pCustomer->szFullName, pCustomer->dFeeBalance);
        // get the postal address and print it
      if(sscanf(pszRemainingInput, "%[^','],%[^','],%[^','],%s", pCustomer->szStreetAddress, pCustomer->szCity, pCustomer->szStateCd, pCustomer->szZipCd) != 4)
	{
	  exitError(ERR_CUSTOMER_ADDRESS_DATA, pszRemainingInput);
	};
      printf("%s\n", pCustomer->szStreetAddress);
      printf("%s %s %s\n", pCustomer->szCity, pCustomer->szStateCd, pCustomer->szZipCd);

      // print the column heading for the transactions
      printf("\t\t\t\t\t%-5s  %-9s  %-10s\n", "Trans", "Book", "Date");

    }
    else if (strcmp(pszSubCommand, "TRANS") == 0)
    {
      if(sscanf(pszRemainingInput, "%c%s%s", &transaction.cTransType, transaction.szBookId, transaction.szTransDt) != 3){
	exitError(ERR_TRANSACTION_DATA, pszRemainingInput);
      }
      else
	{
	  printf("\t\t\t\t\t%-6c%-10s%-15s", transaction.cTransType, transaction.szBookId, transaction.szTransDt);
	  pFind = searchT(pRoot, transaction.szBookId);
	  if(pFind == NULL)
	    {
	      //if Book ID does not exist, display message, and break loop
	      printf("***Book ID does not exist\n");
	    }
	  else
	    {
	      //Check the if the date is valid, if not, show message and continue
	      UtsaDate date;
              //UtsaDate *pDate = NULL;
	      //pDate = &date[h];
	      if(validateDate(transaction.szTransDt, &date)!=0)
		{
		  printf("***Date is invalid");
		}
	      //if date is valid, update the fee balance
	      else
		{
		  pCustomer->dFeeBalance = pCustomer->dFeeBalance + processTransaction(pRoot, *pCustomer, transaction);
		}
	      printf("\n");
	    }
	}

        printf("\n");
    }
    else printf("   *** %s %s\n", ERR_CUSTOMER_SUB_COMMAND, pszSubCommand);
}

/********************processBookCommand *****************************
    void processBookCommand(Node **ppRoot
         , char *pszSubCommand, char *pszRemainingInput)
Purpose:
    Processes the subcommands associated with the BOOK command:
        BOOK CHANGE szBookId dLateFeePerDay dMaxLateFee
            change the Late Fee Per Day and Max Late Fee to the specified values
        BOOK SHOW szBookId    
            requests a display of a particular book.  Show all of its information.
                
Parameters:
    I/O Node **ppRoot                 points to pointer for pRoot, root of binary tree for books
    I   char  *pszSubCommand          Should be CHANGE or SHOW
    I   char  *pzRemainingInput       Points to the remaining characters in the input
                                      line (i.e., the characters that following the
                                      subcommand).
Notes:

**************************************************************************/
void processBookCommand(NodeT **ppRoot, char *pszSubCommand, char *pszRemainingInput)
{
  Book book;
  NodeT *pFind;

  int iScanfCnt;
  int iBookIndex;
  
 
  // Determine what to do based on the subCommand
  if(strcmp(pszSubCommand, "CHANGE") ==0)
    {
      //scan the string for the new late fees
      if(sscanf(pszRemainingInput, "%s%lf%lf", book.szBookId, &book.dLateFeePerDay, &book.dMaxLateFee) != 3)
	{
	  printf("   *** %s %s\n", ERR_BOOK_DATA, pszRemainingInput);
	}
      else
	{
	  //find the book
	  pFind = searchT(*ppRoot, book.szBookId);
	  if( pFind == NULL)
	    {
	      //Book was not found
	      printf("   *** %s %s\n", ERR_BOOK_NOT_FOUND, pszRemainingInput);
	    }
	  else
	    {
	      //Book was found, change data
	      pFind->book.dLateFeePerDay = book.dLateFeePerDay;
	      pFind->book.dMaxLateFee = book.dMaxLateFee;
	      printf("   *** Book successfully changed!\n\n");
	    }
	}
    }
    else if(strcmp(pszSubCommand, "SHOW") == 0)
    {
      //scan the string for the bookId to show
      if(sscanf(pszRemainingInput, "%s", book.szBookId) != 1)
	{
	  printf("   *** %s %s\n", ERR_BOOK_DATA, pszRemainingInput);
	}
	
      //find the book
      pFind =  searchT(*ppRoot, book.szBookId);
      if(pFind == NULL)
	{
	  //Book was not found
	  printf("   *** %s %s\n", ERR_BOOK_NOT_FOUND, pszRemainingInput);
	}
      else
	{
	  //Book was found, show the data
	  printf("   %-9s %-40s %-8s %-10s %-8s %-8s \n"
		 , "Book Id", "Title", "Customer", "Ck Out Dt", "Late Fee", "Max Late");

	  // output each book in a table format
	  printf("   %-9s %-40s %-8s %-10s %8.2lf %8.2lf \n\n"
		 , pFind->book.szBookId
		 , pFind->book.szTitle
		 , pFind->book.szCustomerId
		 , pFind->book.szCheckedOutDt
		 , pFind->book.dLateFeePerDay
		 , pFind->book.dMaxLateFee
		 );
	}
    }
    else if (strcmp(pszSubCommand, "NEW") == 0)
      {
	//Get the info for the new book
	if(sscanf(pszRemainingInput, "%9s %6s %10s %lf %lf %[^\n]\n"
            , book.szBookId
            , book.szCustomerId
            , book.szCheckedOutDt
            , &book.dLateFeePerDay
            , &book.dMaxLateFee
            , book.szTitle) != 6)
	  {
	    exitError(ERR_BOOK_DATA, pszRemainingInput);
	  }
	else
	  {
	    //search to see if book is already in list
	    pFind = searchT(*ppRoot, book.szBookId);
	    if(pFind == NULL)
	      {
		//insert into LL
		insertT(*ppRoot, book);
		printf("   *** Book successfully added!\n\n");
	      }
	    else
	      {
		printf("   *** %s %s\n", ERR_BOOK_ALREADY_EXISTS, pszRemainingInput);
	      }
	  }
      }
    else if(strcmp(pszSubCommand,"PPRINT") == 0)
      {
	//invoke pretty print
	prettyPrintT(*ppRoot, 0);
      }
    else
      {
	printf("   *** %s %s\n", ERR_BOOK_SUB_COMMAND, pszSubCommand);
      }
}
/******************** searchT *****************************
    NodeT *searchT(NodeT *p, char szMatchBookId[])
Purpose:
    Finds the specified book in the binary tree.
Parameters:
    I   Node *p                      pointer for pRoot, root of binary tree for books
    I   char *pszMatchBookId         bBook Id to find in the array
Returns:
    p                Position of matching Node if found
    NULL             Returned if no matching Node found
Notes:

**************************************************************************/
NodeT *searchT(NodeT *p, char szMatchBookId[])
{
  if(p==NULL)
    return NULL;
  if(strcmp(szMatchBookId, p->book.szBookId) == 0)
    return p;
  if(strcmp(szMatchBookId, p->book.szBookId) <0)
    {
      searchT(p->pLeft, szMatchBookId);
    }
  else
    {
      searchT(p->pRight, szMatchBookId);
    }
}

/******************** insertT *****************************
 NodeT *insertT(NodeT *p, Book book)
Purpose:
    To insert the Book supplied in the linked list in ascending order
Parameters:
   I/O   Node *p             pointer for pRoot, root for binary tree of books
   I     Book book           contains the information for the book to be added
Returns:
   p             returns the root for the tree, after adding the node and coming back
                 up through the recurssive calls to the appropriate level.
Notes:

**************************************************************************/
NodeT *insertT(NodeT *p, Book book)
{
  NodeT *pFind;
  if(p==NULL)
    {
      p = allocateNodeT(book);
    }
  else
    {
      pFind = searchT(p, book.szBookId);
      if(pFind == NULL)
	{
	  if(strcmp(book.szBookId, p->book.szBookId) <0)
	    {
	      p->pLeft = insertT(p->pLeft, book);
	    }
	  else
	    {
	      p->pRight = insertT(p->pRight, book);
	    }
	}
      else
	{
	  printf("Book already in list\n");
	}
    }
  return p;
}


/*** include your processTransaction code from program #2 ***/

/**********************************double processTransaction************************
  double processTransaction(NodeT *pRoot, Customer customer, Transaction transaction)
Purpose:
    To process the transaction for the customer.  If the transaction is valid, a C 
will check the book out the customer or, R will result in the book being returned, 
late fees being calculated if applicable, and returning the value of the late fee.

Paramters:
    I    Node *pRoot         pointer for root of binary tree of books
    I    customer            the structure for the customer info who  we are currently processing
    I    transaction         the structure for the trans info we are currently processing

Notes:
    
Function return value:
     dLateFee            the late fee (if any) for the transaction processed

***********************************************************************************/
double processTransaction(NodeT *pRoot, Customer customer, Transaction transaction)
{
  NodeT *p;
  double dLateFee = 0;
  int i, iBookIndex, iDaysAfterCheckOut, iDaysLate;

  //Use searchLL to find  index of book in Library
  p = searchT(pRoot, transaction.szBookId);

  //set Max Late fee for later use
  double dMaxLateFee = p->book.dMaxLateFee;
  
  //if transaction type is C
  if(transaction.cTransType == 'C')
    {
      //Check if the book is checked out
      if(strcmp(p->book.szCustomerId, "NONE") != 0)
	{
	  printf("***Already checked out");
	}
      //it is not checked out, so check it out
      else
	{
	  //Set book's customerID
	  strcpy(p->book.szCustomerId, customer.szCustomerId);
	  //Set book's checkout date
	  strcpy(p->book.szCheckedOutDt, transaction.szTransDt);
	}
    }
  //If transaction type is R
  else if(transaction.cTransType == 'R')
    {
      //Use dateDiff to subtract check out date from transaction date
      iDaysAfterCheckOut = dateDiff(transaction.szTransDt, p->book.szCheckedOutDt);
      //if diff is more than 14 days...
      if(iDaysAfterCheckOut > 14)
	{
	  //Compute number of days late
	  iDaysLate = iDaysAfterCheckOut - 14;
	  //Compute late fee
	  
	  dLateFee = iDaysLate * p->book.dLateFeePerDay;
	  //If late fee greater than max late fee change to max late fee
	  if(dLateFee > dMaxLateFee)
	    {
	      dLateFee = dMaxLateFee;
	      } 
	  //Print the late fee

	  printf ("***Late Fee: %.2lf", dLateFee);

	}
      //Set the books customer id to "NONE"
      strcpy(p->book.szCustomerId, "NONE");
      //Set teh book's check out date to 0000-00-00
      strcpy(p->book.szCheckedOutDt, "0000-00-00");
    }

  //If it is neither print erorr message
  else
    {
      printf("***Transaction type is invalid");
    }
  return dLateFee;
}
