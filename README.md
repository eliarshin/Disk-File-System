# Disk-File-System
Implement Disk FilThe program reperesent a Disk File System in size of 256.
At first we format the disk, there we initialize the deatils of Block size, and how many direct blocks will be.
The program also include a Single In Direct which is in size of block size , he refernece us to anther block size blocks.

We got vector bit which control which blocks are free.
Also a direct blocks array that hold inside a free blocks, there we insert our data.
We got single in direct block aswell, he store casted free blocks to char, and we can get refernce from him to the right blocks we need to use.

It is a simple virtual file disk system, which include creating file, writing to file , delete file , read from file , close or open file and format the disk aswell.

To indicate into the right place, we got a file descriptor vector that hold all the data, we just need his fd and we can reach everything.

How To Use:

1. Format the Disk - 2 (X) (Y) : X is to choose block size and Y is to choose how many direct blocks will be

2. Create File - 3 (filename) : 3 is the option to create a new file - instead filename write the filename you want to give.

3.Now we can use some options on our file :

-Write to file : 6 (FD) (TEXT) : FD is the file descriptor number and text is what we want to insert

-Close File : 5 (FileName) : We put the fd and than we close the file, you could not be able to do anything on it.

-Open FIle :4 (FD) : we put instead FD the number of file we want to open, when we open it we can use all functions on it.

-Delete File : 8 (FileName) : we put the file descriptor of the file and we delete him, his data and restart all data caused with him.

-Read From File : 7 (size) : we put insted size what size we want to read from file

-ALL DATA PRINT : 1 : print us everything on disk 

-CLOSE FILE : 0 : Close the program.

How To Compile:
Use Visual Studio COde
1)press CTRL+SHIFT+B To build
2)Press CTRL+F5 / Go to Run -> Run without Debugging

Input :
We Write into our "Virtual" disk file system , we get chars into the disk that reperesent data.

Output:
1)We can print all DATA DISK
2)We can print Only specific File Datae System in C++
