//#ifdef CHANGED

#include "copyright.h"
#include "system.h"
#include "synchconsole.h"
#include "synch.h"
#include "console.h"

static Semaphore *readAvail;
static Semaphore *writeDone;
static Semaphore *sem_lock;
static Semaphore *char_lock;
static void ReadAviail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }


SynchConsole::SynchConsole(char *readFile, char *writeFile)
{
  readAvail = new Semaphore("read avail", 0);
  writeDone = new Semaphore("write done", 0);
  sem_lock  = new Semaphore("Lock on String",1);
  char_lock  = new Semaphore("Lock on Char",1);
  console   = new Console( readFile , writeFile , ReadAviail , WriteDone ,0);
}

SynchConsole::~SynchConsole()
{
delete console;
delete writeDone;
delete readAvail;
}

void SynchConsole::SynchPutChar(const char ch)
{
  char_lock->P();
//  fprintf(stdout,"lock acquired\n");
  console->PutChar(ch);
  writeDone->P();
  char_lock->V();
//  fprintf(stdout,"lock released\n");

}

char SynchConsole::SynchGetChar()
{
  char ch;
  readAvail->P();
  ch = console->GetChar();
  return ch;
}

void SynchConsole::SynchPutString(const char s[] )
{
    sem_lock->P();
    int i = 0 ;
    for(i = 0 ; i< MAX_STRING_SIZE ; i++)
    {
    if(s[i] == '\0') break;
    synchconsole->SynchPutChar(s[i]);
    }
//    fprintf(stdout,"buffer %s with s[0] %d\n",s,s[0]);
    sem_lock->V();
    
}
void SynchConsole::SynchGetString(char *s, int n)
{
    //sem_lock->P();
   int i = 0;
   for( i = 0 ; i< n-1 ; i++)
	{
	s[i] = synchconsole->SynchGetChar();
	if(s[i] == '\n' || s[i] == EOF ) {i++; break;}
	}
   s[i] = '\0';
   //sem_lock->V();
}

void SynchConsole::SynchPutInt(int value) {
  char * buffer = new char[MAX_STRING_SIZE];
  snprintf(buffer,MAX_STRING_SIZE, "%d", value);
  synchconsole->SynchPutString(buffer);
  delete [] buffer;
}

void SynchConsole::SynchGetInt(int* n) {
  char * buffer = new char[MAX_STRING_SIZE];
  synchconsole->SynchGetString(buffer, MAX_STRING_SIZE);
  if(sscanf(buffer, "%d", n)==0) fprintf(stderr,"Error not an integer\n");
  //fprintf(stdout,"buffer %d\n",*n);
  delete [] buffer;
}

//#endif // CHANGED
