#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define DISK_SIZE 256


void  decToBinary(int n , char &c) 
{ 
   // array to store binary number 
    int binaryNum[8]; 
  
    // counter for binary array 
    int i = 0; 
    while (n > 0) { 
          // storing remainder in binary array 
        binaryNum[i] = n % 2; 
        n = n / 2; 
        i++; 
    } 
  
    // printing binary array in reverse order 
    for (int j = i - 1; j >= 0; j--) {
        if (binaryNum[j]==1)
            c = c | 1u << j;
    }
 } 

// #define SYS_CALL
// ============================================================================
class fsInode {
    int fileSize; // how many letters we wrote untill now
    int block_in_use; // how many blocks used at the moment
    int singleBlocksInUse; // how many single blocks in use
    int singleFileSize;
    int *directBlocks; // the first block that the file saved
    int singleInDirect; // to save the refernces to the other blcoks
    int num_of_direct_blocks; // number of directed blocks
    int block_size; // size of each block


    public:
    fsInode(int _block_size, int _num_of_direct_blocks) {
        fileSize = 0; 
        block_in_use = 0; 
        singleBlocksInUse = 0;
        singleFileSize = 0;
        block_size = _block_size;
        num_of_direct_blocks = _num_of_direct_blocks;
        directBlocks = new int[num_of_direct_blocks];
		assert(directBlocks);
        for (int i = 0 ; i < num_of_direct_blocks; i++) {   
            directBlocks[i] = -1;
        }
        singleInDirect = -1;

    }
    int getSingleFileSize()
    {
        return this->singleFileSize;
    }
    void setSingleFileSize(int add)
    {
        this->singleFileSize = this->singleFileSize + add;
    }
    int getSingleBlocksInUse()
    {
        return this->singleBlocksInUse;
    }
    void setSingleBlocksInUse(int add)
    {
        this->singleBlocksInUse = this->singleBlocksInUse + add;
    }
    int getSingleInDirect()
    {
        return singleInDirect;
    }

    int setSIngleInDirect(int cell)
    {
        this->singleInDirect = cell;
    }
    int getNumDirectBlocks()
    {
        return this->num_of_direct_blocks;
    }
    // used blocks
    void setBlocksInUse(int size)
    {
        this->block_in_use = block_in_use+size;
    }
    int getBLocksInUse()
    {
        return this->block_in_use;
    }
    //blocksize get set
    int getBlockSize()
    {
        return this->block_size;
    }
    void setBlockSize(int size)
    {
        this->block_size = this->block_size+size;
    }
    // filesize get/set
    int getFileSize() 
    {
        return this->fileSize;
    }
    void setFileSize(int size)
    {
        this->fileSize += size;
    }
    int *getDirectBlocks()
    {
        return directBlocks;
    }


    int updateDirectBlocks(int fSize) // update only direct blocks
    {
        if(fSize <= num_of_direct_blocks*block_size)
        {
            for(int i = 0; i < fSize; i=i+block_size)
            {
               this->directBlocks[i] = 1;
               cout << "direct BLOCKS[I]" << directBlocks[i] << endl;
               block_in_use++;
               fileSize = fSize;
            }
            return -1;   // it means that we use only direct blocks
        }
        return (fSize-(num_of_direct_blocks*block_size)); // return how many blocks going to the SingleInDirect
    }

    ~fsInode() { 
        delete directBlocks;
    }


};

// ============================================================================
class FileDescriptor {
    pair<string, fsInode*> file;
    bool inUse;

    public:
    FileDescriptor(string FileName, fsInode* fsi) {
        file.first = FileName; // the name we give to the directed block
        file.second = fsi; // the referense to the fsInode
        inUse = true; // make the file used

    }

    string getFileName() { // get file name
        return file.first;
    }

    void setFilename(string name)
    {
        file.first = name;
    }

    fsInode* getInode() { // get inode refernse
        return file.second;
    }


    bool isInUse() {  // get if this block in use or not in use
        return (inUse); 
    }
    void setInUse(bool _inUse){ // set block used or not used
        inUse = _inUse ;
    }
};
 
#define DISK_SIM_FILE "DISK_SIM_FILE.txt"
// ============================================================================
class fsDisk {
    FILE *sim_disk_fd; // the direct to our file we work on (DISK_SIM_FILE.txt)
 
    bool is_formated; // If we did format to the disk already

	// BitVector - "bit" (int) vector, indicate which block in the disk is free
	//              or not.  (i.e. if BitVector[0] == 1 , means that the 
	//             first block is occupied. 
    int BitVectorSize; // size of the bit vector ()
    int *BitVector; // notice if the block in bitVector[i] is used or no (we mark with 0 - no used 1- used)

    // Unix directories are lists of association structures, 
    // each of which contains one filename and one inode number.
    map<string, fsInode*>  MainDir ;  // to follow our used blocks with the block name

    // OpenFileDescriptors --  when you open a file, 
	// the operating system creates an entry to represent that file
    // This entry number is the file descriptor. 
    vector< FileDescriptor> OpenFileDescriptors; // will save all open files - we will open by fileDescriptor and add him to the vector

    int direct_enteris; // direct entries to the direct blcoks
    int block_size;

    public:
    // ------------------------------------------------------------------------
    fsDisk() {
        sim_disk_fd = fopen( DISK_SIM_FILE , "r+" );
        assert(sim_disk_fd);
        for (int i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd );
            assert(ret_val == 1);
        }
        fflush(sim_disk_fd);

    }
    // ~fsDisk() // I MISS SOMTHING EVERY TIME I CREATE A NEW FILE , SADLY I COULDNT FIND WHAT I MISS .. SORRY
    // {
    //     for(int i = 0; i<OpenFileDescriptors.size() ; i++)
    //     {
    //          OpenFileDescriptors.at(i).getInode()->~fsInode();
    //           MainDir.at(OpenFileDescriptors.at(i).getFileName())->~fsInode();
    //          MainDir.erase(OpenFileDescriptors.at(i).getFileName());
    //     }
        

    //     free (BitVector);
    //     // MainDir.~map();
    //     // OpenFileDescriptors.~vector();
    //     MainDir.clear();
    //     OpenFileDescriptors.clear();
    //     fclose(sim_disk_fd); 
    // }
	


    // ------------------------------------------------------------------------
    void listAll() {
        int i = 0;    
        for ( auto it = begin (OpenFileDescriptors); it != end (OpenFileDescriptors); ++it) {
            cout << "index: " << i << ": FileName: " << it->getFileName() <<  " , isInUse: " << it->isInUse() << endl; 
            i++;
        }
        char bufy;
        cout << "Disk content: '" ;
        for (i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fread(  &bufy , 1 , 1, sim_disk_fd );
             cout << bufy;              
        }
        cout << "'" << endl;


    }
 
    // ------------------------------------------------------------------------
    void fsFormat( int blockSize =4, int direct_Enteris_ = 3 )  // FORMAT THE DISK
    {
        is_formated = true; // we did format
        this->block_size = blockSize;
        this->direct_enteris = direct_Enteris_;
        this->BitVectorSize = DISK_SIZE/block_size; // 64 for 4 block size
        BitVector = new int[BitVectorSize]; // create bit vector to know what vector we use 0 - unused 1 - used

        for(int i = 0; i < BitVectorSize ; i ++)
            BitVector[i] = 0;
         cout << "FORMAT DISK : number of blocks : " << BitVectorSize << endl;
    }

    // ------------------------------------------------------------------------
    int CreateFile(string fileName) //CREATE NEW FILE 
    {
        //check if i did format
        if(is_formated == false)
        {
            cout <<"DISK IS NOT FORMATED , CANNOT CREATE FILE" << endl;
            return -1;
        }
        for(int i = 0; i<OpenFileDescriptors.size();i++) // check if there are no more same files in our disk
            if(fileName.compare(OpenFileDescriptors[i].getFileName())==0)
            {
                cout <<"CANNOT CREATE FILE - SAME FILE NAME, PLEASE CHOOSE ANOTHER FILE NAME"<<endl;
                return -1;
            }
        
        //create rest 
        fsInode *node = new fsInode(block_size,direct_enteris); //initialize with block sizeand direct entries from format
        FileDescriptor *fd = new FileDescriptor(fileName,node); // insert file name and pointer to the node
        MainDir.emplace(fileName,node); // insert to the map filename description + node options
        OpenFileDescriptors.push_back(*fd); // 
        int position = 0;
        position = OpenFileDescriptors.size()-1; // return the FD
        return position;
    
    }

    // ------------------------------------------------------------------------
    int OpenFile(string fileName) //OPEN EXICTED FILE
    {
        if(is_formated == false)
        {
            cout<<"DISK IS NOT FORMATED, CANNOT OPEN THE FILE"<<endl;
        }
        
        int i = 0;
        while(i<OpenFileDescriptors.size())
        {
            //FileDescriptor fd = OpenFileDescriptors[i];
            if((fileName.compare(OpenFileDescriptors[i].getFileName())==0) && (OpenFileDescriptors[i].isInUse()==false)) // if the name equal so we return the position
            {
                OpenFileDescriptors[i].setInUse(true);
                return i;
            }
            i++;
        }
        cout<<"COULDNT FIND THE FILE , PLEASE TRY AGAIN"<<endl;
        return -1; // in case we dont have the file here
    }  

    // ------------------------------------------------------------------------
    string CloseFile(int fd) //CLOSE EXISTED FILE
    {
        if(is_formated == false)
        {
            cout<<"DISK IS NOT FORMATED, CANNOT CLOSE THE FILE"<<endl;
        }
        if(fd > OpenFileDescriptors.size()-1)
        {
            cout <<"COULDNT CLOSE THE FILE, PLEASE TRY AGAIN"<<endl;
            return "-1";
        }

        if(OpenFileDescriptors[fd].isInUse() == true) // check if its true
        {
            OpenFileDescriptors[fd].setInUse(false);
            return OpenFileDescriptors[fd].getFileName();
        }

        return "-1";
    }

    int getVectorSlot() // fUNCTION THAT UPDATE THE VECTOR BIT AND GIVES US FREE BIT
    {
        for(int i = 0 ; i < BitVectorSize ; i++)
            if(BitVector[i] == 0)
            {
                BitVector[i]=1;
                return i;
            }
        return -1;

    }
    // ------------------------------------------------------------------------
    int WriteToFile(int fd, char *buf, int len ) //WRITE TO FILE FUNCTION
    {
        //CHECKS BEFORE START//

        if(is_formated == false)
        {
            cout <<"DISK IS NOT FORMATED, COULDNT WRITE TO FILE"<<endl;
            return -1;
        }
        if(OpenFileDescriptors.at(fd).isInUse() == false) // in case file not in use
        {
            cout<<"FILE NOT IN USE, TRY AGAIN"<<endl;
            return -1;
        }

        if(OpenFileDescriptors.at(fd).getInode()->getFileSize() + len >  // If there is not enough capacity on the file
        block_size*OpenFileDescriptors.at(fd).getInode()->getNumDirectBlocks() + block_size*block_size)
        {
            cout<<"NOT ENOUGH MEMORY IN THE FILE , MAX BUF SIZE CAN ENTERED IS :"<<
            (block_size*OpenFileDescriptors.at(fd).getInode()->getNumDirectBlocks() + block_size*block_size) - OpenFileDescriptors.at(fd).getInode()->getFileSize()<<endl;
            return -1;
        }


        fsInode *node = OpenFileDescriptors.at(fd).getInode(); // for easier use instead write all the time openfiledescriptiors
        int *dirBlocks = node->getDirectBlocks(); // to update the directBlocks array locations
        bool toCheck = true;
        
        int freeSpaceInBlocks =(node->getBLocksInUse()*block_size) - node->getFileSize(); // does there any block who got free space
        int SingleFreeSpaceInBlocks = (node->getSingleBlocksInUse()*block_size)-node->getSingleFileSize(); // single space in blocks

        //if we are in direct blocks
        if(node->getFileSize() + len <= node->getNumDirectBlocks()*block_size) // check if we are only in direct blocks
        { 
            if(freeSpaceInBlocks > 0) // check if there is free space in any direct block
            {
                int val =  fseek(sim_disk_fd,(dirBlocks[node->getBLocksInUse()-1]*block_size) +(block_size - freeSpaceInBlocks),SEEK_SET);
                val =  fwrite(buf,sizeof(char),freeSpaceInBlocks,sim_disk_fd);
                if(len <= freeSpaceInBlocks) // we need to update the size of the file
                    node->setFileSize(len);
                else
                    node->setFileSize(freeSpaceInBlocks);
            }
            
           
            //we create substring from the original buf because we already inserted the beginning
            int newBufSize = len-freeSpaceInBlocks; // size of the string
            if(newBufSize <= 0)// if there is no more data to add
                return 1;
       

            int blocksNeeded = (newBufSize/block_size); // how many blocks needed after the insert
            if(newBufSize%block_size != 0) // if there are more letters we need more block
                blocksNeeded++;
            

            char subBuff[newBufSize]; // we will copy the rest of 
            int point = 0; //I LOST 15 HOURS TO FIND THAT I NEED TO PUT SUBBUFF[POINTS] INSTEAD SUBBUFF[I]!!!!!!!!!!!
            for(int i = freeSpaceInBlocks ; i <len; i++) // we copy the substring from buf without freespaceinblocks because we already insreted it
            {
                subBuff[point] = buf[i];
                point++;
            }

            int i = 0;
            int j = 0;
            dirBlocks[node->getBLocksInUse()] = getVectorSlot(); // get free slot in vector bit
            while(i<newBufSize) // here we insert the data into the vector slot , and also update everything
            {
                
                fseek(sim_disk_fd,(dirBlocks[node->getBLocksInUse()]*block_size)+j,SEEK_SET); //find the position
                fwrite(subBuff+i,1,1,sim_disk_fd);
                //node->setFileSize(1);
                j++;
                
                if((j == block_size) && (i != newBufSize-1)) // if block ended or we in our end of the string
                {
                    node->setBlocksInUse(1); // update used blocks
                    dirBlocks[node->getBLocksInUse()] = getVectorSlot(); // give more free slot
                    j=0;
                }
                i++;
            }
            node->setBlocksInUse(1); // update data
            node->setFileSize(newBufSize);
            return 1;
           
        }

        // in case we need to use Single in direct 
        else
        {
            if(node->getFileSize() <= node->getNumDirectBlocks()*block_size && node->getSingleInDirect() == -1) // in case our direct not full yet so we need to filll it and open single
            {
                if(freeSpaceInBlocks > 0) // same we did before just in right position
                {
                    int val =  fseek(sim_disk_fd,(dirBlocks[node->getBLocksInUse()-1]*block_size) +(block_size - freeSpaceInBlocks),SEEK_SET);
                    val =  fwrite(buf,sizeof(char),freeSpaceInBlocks,sim_disk_fd);
                    if(len <= freeSpaceInBlocks)
                        node->setFileSize(len);
                    else
                        node->setFileSize(freeSpaceInBlocks);
                }
          
                // if we need more direct blocks
                if(node->getFileSize() <=node->getNumDirectBlocks()*block_size) // more direct blocks needed
                {
                    int subBufSize = ((node->getNumDirectBlocks()*block_size) - node->getFileSize()); // we take the remain if there are more space in direct
                    char dirBuf[subBufSize];
                    int l = 0;
                  //   cout <<"@ SINGLE @" << "Sub Buf Size : " << subBufSize <<endl;

                    for(int i = freeSpaceInBlocks ; i <=(node->getNumDirectBlocks()*block_size) - node->getFileSize()+2 ; i++) //@@@MAYBE <= check later @@@@@@
                    {
                        dirBuf[l]='\0';
                        dirBuf[l]=buf[i];
                //        cout<<"DirBuf[l] = " << dirBuf[l] <<endl;
                        l++;
                        
                    }

                    int i = 0;
                    int j = 0;
                    if(node->getBLocksInUse() < node->getNumDirectBlocks()) // check if we added the last block for the directs if no we got more directs
                    {
                        dirBlocks[node->getBLocksInUse()] = getVectorSlot();
                        while(i<subBufSize)
                        {
                            
                            fseek(sim_disk_fd,(dirBlocks[node->getBLocksInUse()]*block_size)+j,SEEK_SET);
                        //   cout <<"@ SINGLE @" << "Direct Location Writing : " << (dirBlocks[node->getBLocksInUse()]*block_size)+j<<" CHAR "<<buf[i]<<endl;
                            fwrite(dirBuf+i,1,1,sim_disk_fd);
                            //node->setFileSize(1);
                            j++;
                            
                            if((j == block_size) && (i != subBufSize-1))
                            {
                                node->setBlocksInUse(1);
                                dirBlocks[node->getBLocksInUse()] = getVectorSlot();
                                j=0;
                            }
                            i++;
                        }
                        node->setBlocksInUse(1);
                        node->setFileSize(subBufSize);
                       // cout <<"Blocks in use (single + direct)"<<node->getBLocksInUse() <<endl;
                    }

                //     cout <<"@ SINGLE @" <<" Direct Blocks In Use : "<<node->getBLocksInUse() <<" Direct File Size" << node->getFileSize() <<endl;

                   
                    //@@WE ARE IN SINGLE IN DIRECT@@
                    int singleSize = len - freeSpaceInBlocks - subBufSize; // the rest to know the size of single its what we fill + new directs
                    char singleBuf[singleSize];
                    int w = 0;


                    for(int i = (freeSpaceInBlocks + subBufSize) ; i<len ; i++) //@@ CHECK HERE AFTER @@
                    {
                        singleBuf[w] = buf[i];
                        w++;
                    }

                    int SingleBlocksNeeded = singleSize / block_size; // how many blocks we need in the single
                    if(singleSize % block_size != 0)
                        SingleBlocksNeeded++;

                    node->setSIngleInDirect(getVectorSlot()); // get vector slot for the single place
                    int single = node->getSingleInDirect();
                 //    cout <<"@ SINGLE @" << "SINGLE IN DIRECT MAIN INDEX :" <<single <<endl;
                    char c = 0;
                    int z = 0;
                    i=0;
                    j=0;
                    int ss = 0;
                    while(z < SingleBlocksNeeded) // fill the single
                    {
                        c=0;
                        int SingleNewBlock = getVectorSlot(); //get vector slot to initialize in the single and write to there
                        decToBinary(SingleNewBlock,c);
                        fseek(sim_disk_fd,(single*block_size)+ss,SEEK_SET);
                        fwrite(&c,1,1,sim_disk_fd);
                        // cout << " @SINGLE@ " << (int)c << " Single New Block " <<SingleNewBlock <<endl;
                        // cout <<"@ SINGLE @" <<"Single write char "<<(single*block_size)+ss<<endl;

                         while(i<singleSize && j<block_size) // write into our new block
                        {
                        
                            int val =fseek(sim_disk_fd,(SingleNewBlock*block_size)+j,SEEK_SET);
                         //    cout <<"@ SINGLE @" <<"Single Location Writing = "<< (SingleNewBlock*block_size)+j << endl;
                            val = fwrite(singleBuf+i,1,1,sim_disk_fd);
                            j++;
                            i++; 
                        }
                        j=0;
                        z++;
                        ss++;
                        node->setSingleBlocksInUse(1);
                    }
                    node->setSingleFileSize(singleSize); // update data
                    node->setFileSize(singleSize);
                    node->setBlocksInUse(SingleBlocksNeeded);
                }
                
            }
            //in case our direct blocks are full for sure and single blocks already used
            else if(node->getSingleInDirect() != -1 )
            {
               // cout << " I AM HERE " <<endl;
               int singleIndex = node->getSingleInDirect();
               char getBlock = '\0';
               fseek(sim_disk_fd,block_size*singleIndex +node->getSingleBlocksInUse() -1,SEEK_SET);
               fread(&getBlock,1,1,sim_disk_fd);
               int writingIndex ;
               writingIndex = (int)getBlock;
               if(SingleFreeSpaceInBlocks > 0) // fill the free space in single blocks
               {
                    int val =  fseek(sim_disk_fd,(writingIndex*block_size) +(block_size - SingleFreeSpaceInBlocks),SEEK_SET);
                    val =  fwrite(buf,sizeof(char),freeSpaceInBlocks,sim_disk_fd);
                    if(len <= SingleFreeSpaceInBlocks) // update data of both , single and total
                    {
                        node->setFileSize(len);
                        node->setSingleFileSize(len);
                    }
                    else
                    {
                        node->setFileSize(SingleFreeSpaceInBlocks);
                        node->setSingleFileSize(SingleFreeSpaceInBlocks);  
                    } 
                }
                    // cout<< "Single INDIRECT INDEX"<<singleIndex<<endl;
                    //  cout << " SINGLE FILE SIZE" <<node->getSingleFileSize() <<"SINGLE BLOCK IN USE" << node->getSingleBlocksInUse()<<endl;
                    //  cout << "TOTAL FILE SIZE" << node->getFileSize() << "TOTAL BLOCKS USED" <<node->getBLocksInUse()<<endl;
                   
                    int singleBufSize = len - SingleFreeSpaceInBlocks; // size of single buf
                    int blocksNeeded = (singleBufSize/block_size); // how many blocks needed after the insert
                    
                    if(singleBufSize%block_size != 0)
                        blocksNeeded++;

                    if(singleBufSize <= 0) // in case we dont go forward the len was not enough to open new block
                        return 1;

                    // cout<<"Single Buf Size = " << singleBufSize <<endl;
                    char singleSubBuf[singleBufSize];
                    int pointer = 0;

                    for(int i = SingleFreeSpaceInBlocks ; i<len ; i++)
                    {
                        singleSubBuf[pointer] = buf[i];
                        pointer++;
                    }

                    int z = 0;
                    int i=0;
                    int j=0;
                    int ss = 0;
                    char c;
                    while(z < blocksNeeded) // to fill new single blocks
                    {
                        c=0;
                        int SingleNewBlock = getVectorSlot();
                        decToBinary(SingleNewBlock,c);
                        fseek(sim_disk_fd,(singleIndex*block_size)+node->getSingleBlocksInUse(),SEEK_SET);
                        fwrite(&c,1,1,sim_disk_fd);
                        // cout << " @SINGLE@ " << (int)c << " Single New Block " <<SingleNewBlock <<endl;
                        // cout <<"@ SINGLE @" <<"Single write char "<<(single*block_size)+ss<<endl;

                         while(i<singleBufSize && j<block_size)
                        {
                        
                            int val =fseek(sim_disk_fd,(SingleNewBlock*block_size)+j,SEEK_SET);
                         //    cout <<"@ SINGLE @" <<"Single Location Writing = "<< (SingleNewBlock*block_size)+j << endl;
                            val = fwrite(singleSubBuf+i,1,1,sim_disk_fd);
                            j++;
                            i++; 
                        }
                        j=0;
                        z++;
                        ss++;
                        node->setSingleBlocksInUse(1);
                    }
                    node->setSingleFileSize(singleBufSize); // update data
                    node->setFileSize(singleBufSize);
                    node->setBlocksInUse(blocksNeeded);
                    //  cout <<"@ SINGLE @" << " Single File Size " <<node->getSingleFileSize()<<
                    //  " Single Blocks In Use " <<node->getSingleBlocksInUse() <<endl;
                    //   cout <<"@ SINGLE @" <<" Direct File Size :"<< node->getFileSize()<<
                    //   " Blocks In Use " << node->getBLocksInUse()<<endl; 
            }
        }

        
   // if(tests)
   // {
        // if(node->getFileSize() + len > node->getBlockSize() * node ->getNumDirectBlocks()//I check if single in direct need to be involved
        // || node->getFileSize() >= node->getNumDirectBlocks() * node-> getNumDirectBlocks()) //
        // {
        //     int index = node->getSingleInDirect();
        //     if(index == -1) // we check if its the first time we use single in direct
        //     {
        //         node->setSIngleInDirect(getVectorSlot());
        //         index = node->getSingleInDirect();
        //     }

        //     if(freeSpaceInBlocks > 0 && node->getFileSize() < node->getBlockSize() *node->getNumDirectBlocks()) // case one we need to add to direct block + open singleinuse
        //     {
        //         int val = fseek(sim_disk_fd,(dirBlocks[node->getBLocksInUse()-1]*block_size)+(block_size-freeSpaceInBlocks),SEEK_SET);
        //         val = fwrite(buf,sizeof(buf),freeSpaceInBlocks,sim_disk_fd);
        //         if(len<= freeSpaceInBlocks)
        //             node->setFileSize(len);
        //         else
        //             node->setFileSize(freeSpaceInBlocks);  

        //         if(node->getFileSize() < node->getBlockSize() * node->getBlockSize())
        //         {
        //             return 1;
        //         } 
                
        //         int bufSize = len - freeSpaceInBlocks; // save the size we need after inserting to direct
        //         int neededBlocks = (bufSize/block_size); /// how many blocks we need
        //         if(bufSize%block_size != 0)
        //             neededBlocks++;

        //         char subBuf[bufSize];
        //         int k = 0;
        //         for(int i = freeSpaceInBlocks ; i < len ; i++) // init our new sub buff
        //         {
        //             subBuf[k] = buf[i];
        //             k++;
        //         }
                
        //         int newBlock = getVectorSlot();
        //         char c = 0;
        //         decToBinary(newBlock,c);
        //         fseek(sim_disk_fd,block_size*(index+node->getSingleBlocksInUse()),SEEK_SET);
        //         cout<< "INDEX : "<<block_size*(index+node->getSingleBlocksInUse())<<endl;
        //         fwrite(&c,sizeof(char),1,sim_disk_fd);
        //         char singleblock;
        //         fseek(sim_disk_fd,block_size*(index+node->getSingleBlocksInUse()),SEEK_SET);
        //         fread(&singleblock,1,1,sim_disk_fd);
        //         int runIndex = (int)singleblock;
        //         cout<<" @ SINGLE IN DIRECT @" << "Single index :" << index <<"Next Block :"<<runIndex<<endl;
        //        // node->setSingleBlocksInUse(1);
        //         //node->setBlocksInUse(1);
        //         int i = 0;
        //         int j = 0;
        //         int q = 1;
        //         while( i < bufSize)
        //         {
        //             fseek(sim_disk_fd,block_size*(runIndex*block_size)+j,SEEK_SET);
        //              fwrite(subBuf+i,1,1,sim_disk_fd);
        //             cout<<" @ SINGLE IN DIRECT @" << "Single index :" << index <<"Next Block :"<<runIndex<<endl;
        //             //node->setFileSize(1);
        //             j++;
        //             q++;
        //             if((q= block_size) && (i!=bufSize-1))
        //             {
        //                 cout<<"I AM INSIDE" <<endl;
        //                 newBlock = getVectorSlot();
        //                 node->setSingleBlocksInUse(1);
        //                 node->setBlocksInUse(1);
        //                 c=0;
        //                 decToBinary(newBlock,c);
        //                 fseek(sim_disk_fd,block_size*(index+node->getSingleBlocksInUse()),SEEK_SET);
        //                 fwrite(&c,sizeof(char),1,sim_disk_fd);
        //                 fseek(sim_disk_fd,block_size*(index+node->getSingleBlocksInUse()),SEEK_SET);
        //                 fread(&singleblock,1,1,sim_disk_fd);
        //                 runIndex = (int)singleblock;
        //                 // node->setBlocksInUse(1);
        //                 // dirBlocks[node->getBLocksInUse()] = getVectorSlot();
        //                 j=0;
        //                 q=0;
                    
        //             }
        //             i++;
        //         }
        //         node->setSingleBlocksInUse(1);
        //         node->setBlocksInUse(1);
        //         node->setSingleFileSize(bufSize);
        //         node->setFileSize(bufSize);
        //     }
        // }

        // if(node->getNumDirectBlocks() * node->getBlockSize() )
        
        
        
      //   cout<< " FILE SIZE EVERY RUN "<<node->getFileSize()<<endl;
      //  return 1;

        ///////////////////////////////////////////////////////////////////////
        // char endBuf[0];
        //  int sizeEndBuf =0;
        // if(newBufSize%block_size != 0)
        // {
        //     // sizeEndBuf = (newBufSize-((blocksNeeded-1)*block_size));
        //     // endBuf[sizeEndBuf];
        //     // for(int i = (((blocksNeeded-1)*block_size)) ; i<newBufSize ; i++)
        //     // {
        //     //     endBuf[i]=subBuff[i];
        //     //     cout << endBuf[i];
        //     // }
        // }
        // for(int i = 0; i<blocksNeeded; i++)
        // {
        //     cout<<"I =" << i <<"blocks needed" <<blocksNeeded <<endl;
        //     dirBlocks[node->getBLocksInUse()+i] = getVectorSlot();
        //     node->setBlocksInUse(1);
        //     //cout<<" BLOCKS IN USE" <<node->getBLocksInUse()<<endl;
        // }
      
        
        
//////////////////////////////////////////////////
        // if(freeSpaceInBlocks != 0)
        // {

        // }
      
        // cout << "CatchedBlocks " << catchedBlocks << endl;
        

       
        // if(len > (node->getBlockSize() * (node->getNumDirectBlocks()+block_size)))
        // {
        //     cout << "NOT ENOUGH PLACE ON FILE" << endl;
        //     return -1;
        // }
        // if(OpenFileDescriptors.at(fd).isInUse()==false || is_formated == false)
        //     return -1;

        //  int freeSpaceInBlocks =(node->getBLocksInUse()*block_size) - node->getFileSize(); // how many free space i got in my blocks
        //  if(freeSpaceInBlocks != 0 )
        //  {
        //      int indexBlock = dirBlocks[node->getBLocksInUse()-1]*block_size + (block_size - freeSpaceInBlocks);
        //      fseek(sim_disk_fd,indexBlock,SEEK_SET);
        //      fwrite(buf,sizeof(char),freeSpaceInBlocks,sim_disk_fd);
        //  }

        
        //  char subBuff[freeSpaceInBlocks];
        //  char RetBuff[len - freeSpaceInBlocks];
        // //we check if we got free space in blocks
        // if(freeSpaceInBlocks != 0)
        // {
        //     int indexSpace = (dirBlocks[node->getBLocksInUse()-1]*block_size + (block_size-freeSpaceInBlocks));
        //     fseek(sim_disk_fd,indexSpace,SEEK_SET);
        //     char subBuff[freeSpaceInBlocks];
        //     for(int i = 0; i<freeSpaceInBlocks;i++)
        //         subBuff[i]=buf[i];
        //     fprintf(sim_disk_fd,subBuff);
        //     node->setBlockSize(node->getBlockSize()+strlen(subBuff));
        //     if(strlen(RetBuff)!=0)
        //     {
        //         int checker = 0;
        //         int pos = 0;
        //         for(int i = 0 ; i < BitVectorSize ; i++)
        //         {
        //             if(BitVector[i]==0)
        //             {
        //                 BitVector[i] = 1;
        //                 dirBlocks[node->getBLocksInUse()+pos] = i;
        //                 pos++;
        //             }
        //             if(pos == catchedBlocks)
        //                 break;
        //         }
           
        //         fseek(sim_disk_fd,(dirBlocks[pos]*block_size),SEEK_SET);
        //         fprintf(sim_disk_fd,RetBuff);
        //              node -> setBlockSize(node->getBLocksInUse()+strlen(RetBuff));
        //         node ->setBlocksInUse(node->getBLocksInUse()+pos);
        //     }
        // }
        // else{
        //     node->setBlocksInUse(catchedBlocks);
        //     node->setFileSize(len);
        //     char restBuff[len-freeSpaceInBlocks];
        //     for(int i = freeSpaceInBlocks;i<len;i++)
        //     {
        //         restBuff[i]=buf[i];
        //         cout<<restBuff[i];
        //     }
        //     int checker = 0;
           
        //     for(int i = 0; i < BitVectorSize ; i++)
        //     {
        //         if(BitVector[i] == 0)
        //         {
        //             BitVector[i]=1; // update vector bit
        //             dirBlocks[checker] = i; // update used blocks
        //             cout << "I =" << i<< "dirBlocks " << dirBlocks[checker]<< "checker " << checker<< "BitVector :" << BitVector[i]  << endl;  
        //             checker++; // update pointer on used blocks
        //           //  cout << "dirBlocks " << dirBlocks[checker]<< "checker " << checker<< "BitVector :" << BitVector[i]  << endl;      
        //         }
        //         if(checker == catchedBlocks) // if the size done we out
        //         {
        //             cout << "CatchedBlocks " << catchedBlocks <<"Checker "<< checker <<" BLOCKS IN USE"<<node->getBLocksInUse()<< endl;
        //             break;
        //         }
                
        //     }
        //             fseek(sim_disk_fd,(dirBlocks[0]*block_size),SEEK_SET);
        //             fprintf(sim_disk_fd,buf);
                     
        // }
            

//////////////////////////@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@////////////////////////
        
        

        // else if(node->getFileSize() > 0 &&node->getBLocksInUse() > 0) // if its not the first time we use the fd
        // {
        //     int freeSpace = (block_size*node->getNumDirectBlocks() ) - node->getFileSize();
        //     int freeSpaceInBlock = (block_size * node->getBLocksInUse()) - node->getFileSize();
        //     cout << "TOTAL FREE SPACE"<< freeSpace << "FREE SPACE IN BLOCK" << freeSpaceInBlock <<endl;

        //     if(freeSpaceInBlock == 0)
        //     {
        //         int checker = node->getBLocksInUse()+1;
        //         int pos = 0;
        //         for(int i = 0 ; i < BitVectorSize ; i++)
        //         {
        //             if(BitVector[i] == 0)
        //             {
        //                 BitVector[i] = 1;
        //                 dirBlocks[checker] = i;
        //                 cout << "DIR BLOCK [CHECKER] "<< dirBlocks[checker] <<endl;
        //                 checker++;
        //                 pos++;
        //             }
        //             if(pos == catchedBlocks)
        //                 break;
        //         }
                
                
        //         for(int i = node->getBLocksInUse()+1 ; i<catchedBlocks+node->getBLocksInUse()+1; i++)
        //         {
        //             fseek(sim_disk_fd,dirBlocks[i]*block_size,SEEK_SET);
        //             fprintf(sim_disk_fd,buf);
        //         }

        //         node->setBlocksInUse(node->getBLocksInUse()+pos);
        //         node->setFileSize(node->getFileSize()+len);
        //         cout<<"BLOCKS IN USE"<< node->getBLocksInUse() <<"TFILE SIZE UPDATED"<<node->getFileSize()<<endl;
        //     }
            // else if(freeSpaceInBlock != 0)
            // {
            //     int addSize = 0;
            //     int freeSpaceInBlock = (block_size * node->getBLocksInUse()) - node->getFileSize();
            //     char blockBuff[freeSpaceInBlock];
            //     char restBuff[len-freeSpaceInBlock];
            //     strncpy(blockBuff,buf,freeSpaceInBlock);
            //     for(int i = freeSpaceInBlock ; i<len ;i++)
            //     {
            //         restBuff[i] = buf[i];
            //        // cout<<restBuff[i];
            //     }
            //     if(len>freeSpaceInBlock)
            //         addSize = freeSpaceInBlock;
            //     else
            //         addSize = len;
                
            //     fseek(sim_disk_fd,dirBlocks[node->getBLocksInUse()-1]*block_size + (block_size-freeSpaceInBlock),SEEK_SET);
            //     fprintf(sim_disk_fd,blockBuff);
            //     node->setFileSize(node->getFileSize()+addSize);
            //     cout << "FILE SIZE" <<node->getFileSize()<<endl;

            //     int restBlocks = (len-freeSpaceInBlock)/block_size;
            //     //cout << "RESTBLOCKS :"<<restBlocks<<endl;
            //     int cnt =0;
            //     for(int i = 0 ; i <BitVectorSize ; i++)
            //     {
            //         if(BitVector[i]==0)
            //         {
            //             BitVector[i]=1;
            //             dirBlocks[i]=i;
            //             cnt++;
            //         }
            //         if(cnt == restBlocks)
            //             break;
            //     }
            //     for(int i = node->getBLocksInUse()+1 ; i<node->getBLocksInUse()+restBlocks; i++)
            //     {
            //         fseek(sim_disk_fd,dirBlocks[i]*block_size,SEEK_SET);
            //         fprintf(sim_disk_fd,buf);
            //     }

                
            // }  
      //  }
        // ////////////////////////////////////////////////////////////////////////////////////////////////////
        // else if(node->getFileSize() > 0 ) // if its not the first time we use this file and we want to add info but its still on direct blocks
        // {
        //     int TotalSpaceInFile = (block_size*node->getNumDirectBlocks()) - node->getFileSize();
        //     int SpaceInOneBlock = ((node->getBLocksInUse()*block_size)-node->getFileSize());
        //     int catchedBlocks = (len/block_size);
        //      if(len%block_size != 0)
        //         catchedBlocks++;
        //   //  cout << "catched blocks "<<catchedBlocks<<" SPCE IN ONE BLOCK"<< SpaceInOneBlock <<endl;
        //    // cout<<"space in one block:"<< SpaceInOneBlock<<endl;
        //     // int BlockIndex= dirBlocks[node->getBLocksInUse()-1] * block_size + (block_size - SpaceInOneBlock);
        //     // cout <<"BLOCK INDEX :" <<BlockIndex<<endl;
        //     // char *subIndex[BlockIndex];
        //     // strncpy(*subIndex,buf,BlockIndex-block_size);
        //     // cout << " SUBINDEX :" << subIndex <<endl;

        //    node->setBlocksInUse(catchedBlocks+node->getBLocksInUse());
        //    node->setFileSize(len+node->getFileSize());
        //     // if there are excaly blocks taken' so we need to keep our info forward
        //     if(SpaceInOneBlock == 0)
        //     {
        //         int checker = node->getBLocksInUse()+1;
        //         int towatch = 0;
        //         for(int i = 0; i < BitVectorSize ; i++)
        //         {
        //             if(BitVector[i] == 0)
        //             {
        //                 BitVector[i] =1 ;
        //                 dirBlocks[checker] = i;
        //                 cout << "I ="<< i <<" BITVECTOR ="<<BitVector[i]<<" dirBlocks="<<dirBlocks[checker]<<" checker = "<<checker<<endl;
        //                 checker++;
        //                 towatch++;
        //             }
        //             if(towatch == catchedBlocks+1)
        //                 break;
        //         }
        //          for(int i = node->getBLocksInUse()+1; i<catchedBlocks+node->getBLocksInUse()+1;i++)
        //          {

        //          //   cout<<dirBlocks[node->getBLocksInUse()+1] << "<<<< DIR BLOCKS"<<" CHECKER ==" <<endl ;
        //            fseek(sim_disk_fd,(dirBlocks[node->getBLocksInUse()+1]*block_size),SEEK_SET);
        //            fprintf(sim_disk_fd,buf);
        //         }
        //         cout << " blocks in use" << node->getBLocksInUse()<<endl;
        //         node->setBlocksInUse(catchedBlocks+node->getBLocksInUse());
        //         node->setFileSize(len+node->getFileSize());
        //     }
        //     // else if(SpaceInOneBlock != 0) // in case we got space in our free block
        //     // {
        //     //     // char subBuffer[SpaceInOneBlock];
        //     //     // char endBuffer[len-SpaceInOneBlock];
        //     //     // strncpy(subBuffer,buf,SpaceInOneBlock);
        //     //     // // if(len <= SpaceInOneBlock)
        //     //     // // {
        //     //     // //     cout << "BLOCKS IN USE :"<<node->getBLocksInUse() <<" DIRBLOCK "<<(dirBlocks[node->getBLocksInUse()-1])<<"blocksize - spinoneb"<<(block_size-SpaceInOneBlock)<<endl;
        //     //     // //     fseek(sim_disk_fd,((dirBlocks[node->getBLocksInUse()-1]*block_size) + (block_size-SpaceInOneBlock)),SEEK_SET);
        //     //     // //     fprintf(sim_disk_fd,subBuffer);
        //     //     // //     node->setFileSize(len+node->getFileSize());
        //     //     // // }
        //     //     // cout<<"IM HERE"<<endl;
        //     //     // cout << "BLOCKS IN USE :"<<node->getBLocksInUse() <<" DIRBLOCK "<<(dirBlocks[node->getBLocksInUse()-1])<<"blocksize - spinoneb"<<(block_size-SpaceInOneBlock)<<endl;
        //     //     // fseek(sim_disk_fd,((dirBlocks[node->getBLocksInUse()-1]*block_size) + (block_size-SpaceInOneBlock)),SEEK_SET);
        //     //     // fprintf(sim_disk_fd,subBuffer);
        //     //     // node->setFileSize(SpaceInOneBlock+node->getFileSize());


                
                
        //     //    // cout << "SUB BUFFER = " <<subBuffer <<endl;
        //     // }
            
            
        //     // if(SpaceInOneBlock == 0)
        //     // {
        //     //     int checker = node->getBLocksInUse()+1; // we go to the next block
        //     //     for(int i = 0; i < BitVectorSize ; i++)
        //     //     {
        //     //         if(BitVector[i] == 0)
        //     //         {
        //     //             BitVector[i] == 1;
        //     //             dirBlocks[checker] = i;
        //     //             checker ++;
        //     //         }
        //     //         if(checker == catchedBlocks)
        //     //             break;
                    
        //     //     }
        //     //     for(int i = 0; i<catchedBlocks ;i++)
        //     //     {
        //     //     fseek(sim_disk_fd,(dirBlocks[checker+i]*block_size),SEEK_SET);
        //     //     fprintf(sim_disk_fd,buf);
        //     //     }
        //     // }
           
        // }
       
    //}
    }
    // ------------------------------------------------------------------------
    int DelFile( string FileName ) 
    {
        if(is_formated == false)
        {
            cout<<"FILE IS NOT FORMATED, COULDNT DELETE FILE"<<endl;
            return -1;
        }

        int fd ;
        for(int i = 0; i<OpenFileDescriptors.size();i++) //
            if(FileName.compare(OpenFileDescriptors[i].getFileName())==0)
            {
                fd = i;
                break;
            }


        fsInode *node = OpenFileDescriptors.at(fd).getInode();
        int *dirBlocks = node->getDirectBlocks();
        char c = '\0';

        int DirBlocksInUse = node->getBLocksInUse() - node->getSingleBlocksInUse();

        for(int i = 0; i < DirBlocksInUse; i++) // clear direct blocks in use
        {
            for(int j = 0; j < block_size ; j++) // clear every direct block
            {
                fseek(sim_disk_fd,dirBlocks[i]*block_size+j,SEEK_SET);
                fwrite(&c,1,1,sim_disk_fd);
            }
        }

        char read = '\0';
        for(int i = 0; i < node->getSingleBlocksInUse() ; i++)  // clear all single blocks
        {
            read = '\0';
            fseek(sim_disk_fd,node->getSingleInDirect()*block_size+i,SEEK_SET);
            fread(&read,1,1,sim_disk_fd);
            int loc = (int) read;
            // BitVector[loc] = 0;
            // fwrite(&c,1,1,sim_disk_fd);
            for(int j = 0 ; j<block_size ; j++) // here we clear each block
            {
                fseek(sim_disk_fd,loc*block_size+j,SEEK_SET);
                fwrite(&c,1,1,sim_disk_fd);
            }
             BitVector[loc] = 0;
             fseek(sim_disk_fd,node->getSingleInDirect()*block_size+i,SEEK_SET);
             fwrite(&c,1,1,sim_disk_fd);
        }

        // update the data like it is restarted
        for(int i = 0 ; i < DirBlocksInUse; i ++)
        {
            BitVector[dirBlocks[i]] = 0;
            dirBlocks[i] = -1;
        }
        node->setSIngleInDirect(-1);
        node->setFileSize(0);
        node->setSingleFileSize(0);
        node->setBlocksInUse(0);
        node->setSingleBlocksInUse(0);
        MainDir.erase(FileName);
        OpenFileDescriptors.at(fd).setFilename("");
        OpenFileDescriptors.at(fd).setInUse(false);
        return fd;
    }
    // ------------------------------------------------------------------------
    int ReadFromFile(int fd, char *buf, int len ) 
    { 
      if(is_formated == false)
      {
          cout<<"FILE IS NOT FORMATED, COULDNT READ FROM FILE" << endl;
          memset(buf,0,sizeof(buf));
          for(int i = 0; i < (block_size*OpenFileDescriptors.at(fd).getInode()->getNumDirectBlocks() +block_size*block_size)+len; i++)
             buf[i]='\0';
          return -1;
      }

    //   if(len > OpenFileDescriptors.at(fd).getInode()->getFileSize()) // in case there is not enough data in our file
    //   {
    //       cout<<"YOU REQUEST TO READ TO MUCH DATA, OUR FILE SIZE IS : "<<OpenFileDescriptors.at(fd).getInode()->getFileSize()<<endl;
    //       memset(buf,0,sizeof(buf));
    //       for(int i = 0; i < (block_size*OpenFileDescriptors.at(fd).getInode()->getNumDirectBlocks() +block_size*block_size)+len; i++)
    //          buf[i]='\0';
    //       return -1;
    //   }

      if(OpenFileDescriptors.at(fd).isInUse() == false) // if file not in use
      {
          cout<<"FILE NOT IN USE, YOU COULDNT READ FROM FILE"<<endl;
          memset(buf,0,sizeof(buf));
          for(int i = 0; i < (block_size*OpenFileDescriptors.at(fd).getInode()->getNumDirectBlocks() +block_size*block_size)+len; i++)
             buf[i]='\0';
          return -1;
      }
      
      fsInode *node = OpenFileDescriptors.at(fd).getInode();
      int *directBlocks = node->getDirectBlocks();

      int HowManyBlocks = 0;
      if(len >= block_size * node->getNumDirectBlocks()) // how many blocks our direct block need
            HowManyBlocks =  node->getNumDirectBlocks();
        else // in case len shorter than direct blocks size
        {
         HowManyBlocks = len/block_size;
         if(len%block_size!=0)
            HowManyBlocks++;
        }
        //initialize our buf (I LOST LIKE 10 HOURS FOR GARBAGE INFORMATION BECAUSE I JUST INITIALIZE UNTIL len AND NOT ALL SIZE)
      memset(buf,0,sizeof(buf));
      for(int i = 0; i < (block_size*node->getNumDirectBlocks()+block_size*block_size)+len; i++)
        buf[i]=0;
    
        

      

        
    //first of all we read from the direct blocks
      int k = 0;
      for(int i = 0; i <HowManyBlocks ; i++)
      {
          int location = directBlocks[i]*block_size; // get position
          for(int j = 0 ; j < block_size ; j++)
          {
              
              if(k == (node->getFileSize())|| k == len  ) // if we reach the position
              {  
                break;
              }
              fseek(sim_disk_fd,location+j,SEEK_SET);  //search in the right location
              fread(buf+k,1,1,sim_disk_fd);   // read to the right place in buffer
              k++;  
          }
          
      }

        //IN CASE WE NEED TO READ FROM INGLE IN DIRECT
       char test = '\0'; 
      if(node->getSingleInDirect() != -1 && len > node->getNumDirectBlocks()*block_size)
      {
          int howManySingleBlocks = len - node->getNumDirectBlocks()*block_size; // INITIALZIE WHAT SIZE
          int blocks = howManySingleBlocks/block_size; // HOW MANY BLOCKS WE NEED
          if(howManySingleBlocks%block_size!=0)
            blocks++;
          
          int index = node->getSingleInDirect();
          int s = 0;
          int q = node->getNumDirectBlocks()*block_size;
          //Here we run on our single in direct and get the right places
          for(int i = 0 ; i < blocks; i++)
          {
              //cout<<" WE ARE IN I = "<< i << endl;
              int singleLocation;
              char c = 0;
              fseek(sim_disk_fd,block_size*index +s,SEEK_SET);
              fread(&c,1,1,sim_disk_fd);
              singleLocation=(int) c;
              for(int j = 0; j <block_size ;j++)
              {
              
                fseek(sim_disk_fd,block_size*singleLocation+j,SEEK_SET);
                 fread(&test,1,1,sim_disk_fd);
                if( q ==len ) // if we got max size before we reach block size we out!
                    break;
                fseek(sim_disk_fd,block_size*singleLocation+j,SEEK_SET);
                fread(buf+q,1,1,sim_disk_fd);
                q++;
              }
              s++;
          }
      }  
	  return 1;
    }

     ~fsDisk() // I MISS FREE NODE + FD IN CREATE BUT I CANT FREE THEM THERE BECAUSE MY PROGRAM BUILT ON IT! IF I FREE THE SIZE OF EACH ONE WILL MOVE
    {
        // free(BitVector);
        // OpenFileDescriptors.clear();
        // MainDir.clear();
        // delete (sim_disk_fd);
        for(int i = 0; i<OpenFileDescriptors.size() ; i++)
        {
             OpenFileDescriptors.at(i).getInode()->~fsInode();
             //MainDir[(OpenFileDescriptors.at(i).getFileName())]->~fsInode();
             MainDir.erase(OpenFileDescriptors.at(i).getFileName());
        }
        free (BitVector);
        // MainDir.~map();
        // OpenFileDescriptors.~vector();
        MainDir.clear();
        OpenFileDescriptors.clear();
        delete sim_disk_fd;
         
    }
};
    
int main() {
    int blockSize; 
	int direct_entries;
    string fileName;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read; 
    int _fd;

    fsDisk *fs = new fsDisk();
    int cmd_;
    while(1) {
        cin >> cmd_;
        switch (cmd_)
        {
            case 0:   // exit
				delete fs;
				exit(0);
                break;

            case 1:  // list-file
                fs->listAll(); 
                break;
          
            case 2:    // format
                cin >> blockSize;
				cin >> direct_entries;
                fs->fsFormat(blockSize, direct_entries);
                break;
          
            case 3:    // creat-file
                cin >> fileName;
                _fd = fs->CreateFile(fileName);
                cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;
            
            case 4:  // open-file
                cin >> fileName;
                _fd = fs->OpenFile(fileName);
                cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;
             
            case 5:  // close-file
                cin >> _fd;
                fileName = fs->CloseFile(_fd); 
                cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;
           
            case 6:   // write-file
                cin >> _fd;
                cin >> str_to_write;
                fs->WriteToFile( _fd , str_to_write , strlen(str_to_write) );
                break;
          
            case 7:    // read-file
                cin >> _fd;
                cin >> size_to_read ;
                fs->ReadFromFile( _fd , str_to_read , size_to_read );
                cout << "ReadFromFile: " << str_to_read << endl;
                break;
           
            case 8:   // delete file 
                 cin >> fileName;
                _fd = fs->DelFile(fileName);
                cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;
            default:
                break;
        }
    }

} 