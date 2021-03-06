// directory.cc 
//	Routines to manage a directory of file names.
//
//	The directory is a table of fixed length entries; each
//	entry represents a single file, and contains the file name,
//	and the location of the file header on disk.  The fixed size
//	of each directory entry means that we have the restriction
//	of a fixed maximum size for file names.
//
//	The constructor initializes an empty directory of a certain size;
//	we use ReadFrom/WriteBack to fetch the contents of the directory
//	from disk, and to write back any modifications back to disk.
//
//	Also, this implementation has the restriction that the size
//	of the directory cannot expand.  In other words, once all the
//	entries in the directory are used, no more files can be created.
//	Fixing this is one of the parts to the assignment.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"
#include "filehdr.h"
#include "directory.h"
#include <new>

//----------------------------------------------------------------------
// Directory::Directory
// 	Initialize a directory; initially, the directory is completely
//	empty.  If the disk is being formatted, an empty directory
//	is all we need, but otherwise, we need to call FetchFrom in order
//	to initialize it from disk.
//
//	"size" is the number of entries in the directory
//----------------------------------------------------------------------

Directory::Directory(int size)
{
    table = new(std::nothrow) DirectoryEntry[size];
    tableSize = size;
    for (int i = 0; i < tableSize; i++)
        table[i].inUse = false;
}

//----------------------------------------------------------------------
// Directory::~Directory
// 	De-allocate directory data structure.
//----------------------------------------------------------------------

Directory::~Directory()
{
    delete [] table;
}

//----------------------------------------------------------------------
// Directory::FetchFrom
// 	Read the contents of the directory from disk.
//
//	"file" -- file containing the directory contents
//----------------------------------------------------------------------

void
Directory::FetchFrom(OpenFile *file)
{
    int size = 0;           // temp to read table size

    file->Seek(0);                                  // make sure we are at the beginning of the file
    file->Read((char *) &size, sizeof(int));        // read table size
    
        
    file->Read((char *) table, tableSize * sizeof(DirectoryEntry)); // read table
    file->Seek(0);                                  // reset seek position
}

//----------------------------------------------------------------------
// Directory::WriteBack
// 	Write any modifications to the directory back to disk
//
//	"file" -- file to contain the new directory contents
//----------------------------------------------------------------------

void
Directory::WriteBack(OpenFile *file)
{
    file->Seek(0);                          // make sure we are at beggining of directory
    file->Write((char *) &tableSize, sizeof(int));  // write directory length first
    file->Write((char *) table, tableSize * sizeof(DirectoryEntry));    // write table
    file->Seek(0);                          // reset seek position to be safe
}

//----------------------------------------------------------------------
// Directory::FindIndex
// 	Look up file name in directory, and return its location in the table of
//	directory entries.  Return -1 if the name isn't in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::FindIndex(char *name)
{
    for (int i = 2; i < tableSize; i++)
        if (table[i].inUse && !strncmp(table[i].name, name, FileNameMaxLen))
            return i;
    return -1;		// name not in directory
}

//----------------------------------------------------------------------
// Directory::Find
// 	Look up file name in directory, and return the disk sector number
//	where the file's header is stored. Return -1 if the name isn't
//	in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int Directory::Find(char *name)
{
    int i = FindIndex(name);

    if (i != -1)
        return table[i].sector;
    return -1;
}

bool Directory::isDirectory(char *name) {
    int i = FindIndex(name);
    if(i != -1)
        return table[i].isDir;
    return false;
}

//----------------------------------------------------------------------
// Directory::Add
// 	Add a file into the directory.  Return true if successful;
//	return false if the file name is already in the directory, or if
//	the directory is completely full, and has no more space for
//	additional file names.
//
//	"name" -- the name of the file being added
//	"newSector" -- the disk sector containing the added file's header
//----------------------------------------------------------------------

bool Directory::Add(char *name, int newSector)
{
    if (FindIndex(name) != -1)
        return false;

    for (int i = 2; i < tableSize; i++) {
        if (!table[i].inUse) {
            table[i].inUse = true;
            strncpy(table[i].name, name, FileNameMaxLen);
            table[i].sector = newSector;
            table[i].isDir = false;
            return true;
        }
    }

    return false;
}

bool Directory::AddDirectory(char *name, int newSector) {
    if (FindIndex(name) != -1)
        return false;
    for (int i = 2; i < tableSize; i++)
        if (!table[i].inUse) {
            table[i].inUse = true;
            strncpy(table[i].name, name, FileNameMaxLen);
            table[i].sector = newSector;
            table[i].isDir = true;
            return true;
        }
    return false;
}

//----------------------------------------------------------------------
// Directory::Remove
// 	Remove a file name from the directory.  Return true if successful;
//	return false if the file isn't in the directory.
//
//	"name" -- the file name to be removed
//----------------------------------------------------------------------

bool Directory::Remove(char *name)
{
    int i = FindIndex(name);

    if (i == -1)
        return false; 		// name not in directory
    table[i].inUse = false;
    return true;
}

//----------------------------------------------------------------------
// Directory::List
// 	List all the file names in the directory.
//----------------------------------------------------------------------

void Directory::List(int tabs)
{
    FileHeader *hdr = new(std::nothrow) FileHeader();
    for (int i = 2; i < tableSize; i++) {
        if (table[i].inUse) {
            for(int j = 0; j < tabs; ++j)
                printf("\t");
            hdr->FetchFrom(table[i].sector);
            if(table[i].isDir == true){
                printf("%s\t-d\n", table[i].name);
            }
            else{
                printf("%s \t filesize : %d\n", table[i].name,hdr->FileLength());
            }


            if(table[i].isDir && strcmp(table[i].name, ".") && strcmp(table[i].name, "..")) {
                Directory *dir = new(std::nothrow) Directory(10);
                OpenFile *dirFile = new(std::nothrow) OpenFile(table[i].sector);
                dir->FetchFrom(dirFile);
                dir->List(tabs + 1);
                delete dir;
                delete dirFile;
            }
        }
    }
}

//----------------------------------------------------------------------
// Directory::Print
// 	List all the file names in the directory, their FileHeader locations,
//	and the contents of each file.  For debugging.
//----------------------------------------------------------------------

void
Directory::Print()
{
    FileHeader *hdr = new(std::nothrow) FileHeader();

    printf("Directory contents:\n");
    for (int i = 2; i < tableSize; i++)
        if (table[i].inUse) {
            printf("Name: %s, Sector: %d\n", table[i].name, table[i].sector);
            hdr->FetchFrom(table[i].sector);
            hdr->Print();
        }
    printf("\n");
    delete hdr;
}

bool Directory::isEmpty(){
    for (int i = 2; i<tableSize ; i++)
    {
        if(table[i].inUse == true) {
            return false;
        }
    }
    return true;
}

bool Directory::isEmpty(int sector){
  int i;
  for (i = 2; i<tableSize ; i++)
    {
        if(table[i].inUse == true && table[i].sector == sector) {
            Directory *dir = new Directory(10);
            OpenFile *dirFile = new OpenFile(table[i].sector);
            dir->FetchFrom(dirFile);
            return dir->isEmpty();
        }
    }

  return true;
}

void Directory::MakeHier(int sector, int parentsector){

   table[0].inUse =true;
   table[0].sector = sector;
   strcpy(table[0].name,".");
   table[1].inUse =true;
   table[1].sector = parentsector;
   strcpy(table[1].name,"..");
}
