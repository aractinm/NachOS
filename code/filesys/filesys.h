#ifndef FS_H
#define FS_H

#include "copyright.h"
#include "openfile.h"
#include <new>

// these variables are no longer apart of the FileSystem class because of
// needed changes to make extendable files work
extern OpenFile* freeMapFile;   // Bit map of free disk blocks,
// represented as a file
extern OpenFile* directoryFile;   // "Root" directory -- list of
// file names, represented as a file

OpenFile *GetFreeMapFile();   // getter method
OpenFile *GetDirectoryFile(); // getter method

class FileSystem {
public:
    FileSystem(bool format);		// Initialize the file system.
    // Must be called *after* "synchDisk"
    // has been initialized.
    // If "format", there is nothing on
    // the disk, so initialize the directory
    // and the bitmap of free blocks.

    bool Create(char *name, int initialSize, int wdSector);
    // Create a file (UNIX creat)

    bool MakeDirectory(char *name, int initialSize, int wdSector);

    int ChangeDirectory(char *name, int wdSector);

    OpenFile* Open(char *name, int wdSector); 	// Open a file (UNIX open)

    bool Remove(char *name, int wdSector);  		// Delete a file (UNIX unlink)

    void List(int dirSector);			// List all the files in the file system

    void Print();			// List all the files and their contents

};

#endif // FS_H