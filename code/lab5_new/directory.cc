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
#include <iostream>
#include <fstream>
#include <stdio.h>
using namespace std;

const int TreeSize = 10;


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
    table = new DirectoryEntry[size];
    tableSize = size;
    for (int i = 0; i < tableSize; i++){
	    table[i].inUse = FALSE;
        //table[i].root_num = 0;
        table[i].child_num = 0;
        table[i].father = 0;
        for(int j = 0; j < TreeSize; j ++){
            table[i].children[j] = -1;
        }
        //table[i].cur_root = -1;
    }
    table[0].inUse = TRUE;
    table[0].father = -1;
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
    (void) file->ReadAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
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
    (void) file->WriteAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
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
    /*
    for (int i = 0; i < tableSize; i++)
        if (table[i].inUse && !strncmp(table[i].name, name, FileNameMaxLen))
            return i;
    return -1;    
    */

    List();

    // no match
    if(table[0].child_num <= 0){
        //cout<<"No Match!"<<endl;
        return -1;
    }

    /*
    bool hasRoot = false;
    for(int i = 0; i < TreeSize; i ++){
        int idx = table[0].children[i];
        if(idx == -1){
            continue;
        }
        if(table[idx].inUse){
            hasRoot = true;
            break;
        }

    }
    if(!hasRoot){
        return -1;
    }
    */

    string name_s = name;

    //cout<<"Find "<<name_s<<endl;

    // split the name according to the tree level
    vector<string> name_list;
    string tmp = "";

    //cout<<"Name List"<<endl;
    for(int i = 0; i < name_s.length(); i ++){
        while(name_s[i] != '/' && i < name_s.length()){
            tmp += name_s[i];
            i ++;
        }
        if(tmp.length() > 0){
            name_list.push_back(tmp);
            //cout<<tmp<<" ";
        }
        tmp = "";
    }
    //cout<<endl;

    // find tree root
    string cur = name_list[0];
    int root_id = -1;
    for(int i = 0; i < table[0].child_num; i ++){
        int idx = table[0].children[i];
        if(!table[idx].inUse){
            continue;
        }
        bool sameflag = true;
        for(int j = 0; j < cur.length(); j ++){
            if(cur[j] != table[idx].name[j]){
                sameflag = false;
                break;
            }
        }
        if(sameflag){
            root_id = idx;
            break;
        }
    }

    // not match
    if(root_id == -1){
        return -1;
    }

    // find match for each tree level
    if(name_list.size() == 1){
        return root_id;
    }
    else{

        for(int i = 1; i < name_list.size(); i ++){

            bool find_flag = false;

            for(int j = 0; j < table[root_id].child_num; j ++){
                int curID = table[root_id].children[j];
                if(!table[curID].inUse){
                    continue;
                }
                bool sameflag = true;
                for(int k = 0; k < name_list[i].length(); k ++){
                    if(name_list[i][k] != table[curID].name[k]){
                        sameflag = false;
                        break;
                    }
                }
                if(sameflag){
                    find_flag = true;
                    root_id = curID;
                    break;
                }
            }

            if(!find_flag){
                return -1;
            }

        } 
        //cout<<"Successfully Find: "<<root_id<<endl;
        return root_id;    
    }

}

//----------------------------------------------------------------------
// Directory::Find
// 	Look up file name in directory, and return the disk sector number
//	where the file's header is stored. Return -1 if the name isn't 
//	in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::Find(char *name)
{
    int i = FindIndex(name);

    if (i != -1)
	   return table[i].sector;

    return -1;
}

//----------------------------------------------------------------------
// Directory::Add
// 	Add a file into the directory.  Return TRUE if successful;
//	return FALSE if the file name is already in the directory, or if
//	the directory is completely full, and has no more space for
//	additional file names.
//
//	"name" -- the name of the file being added
//	"newSector" -- the disk sector containing the added file's header
//----------------------------------------------------------------------

bool
Directory::Add(char *name, int newSector)
{ 
    // already exist
    if (FindIndex(name) != -1)
       return FALSE;

    string name_s = name;
    int s_idx = -1;
    for(int i = 0; i < name_s.length(); i ++){
        if(name_s[i] == '/'){
            s_idx = i;
        }
    }
    string before_name = "";
    string cur_name = "";
    for(int i = 0; i < s_idx; i ++){
        before_name += name_s[i];
    }
    for(int i = s_idx + 1; i < name_s.length(); i ++){
        cur_name += name_s[i];
    }

    int fatherID = 0;

    if(before_name.length() > 0){

        char new_name[50];
        strcpy(new_name, before_name.c_str());

        //char *new_name = before_name.c_str();
        //int b_len = before_name.length();
        //new_name = (char *)malloc((b_len + 1)*sizeof(char));
        //before_name.copy(new_name, b_len, 0);

        fatherID = FindIndex(new_name);     

    }


    for (int i = 1; i < tableSize; i++){

        if (!table[i].inUse) {

            table[i].inUse = TRUE;
            for(int p = 0; p < FileNameMaxLen; p ++){
                table[i].name[p] = '\0';
            }
            for(int p = 0; p < cur_name.length(); p ++){
                table[i].name[p] = cur_name[p];
            }
            table[i].sector = newSector;
            table[i].father = fatherID;
            table[i].child_num = 0;

            // update directory tree root info
            // update father info
            table[fatherID].child_num += 1;
            table[fatherID].children[table[fatherID].child_num - 1] = i;

            cout<<"Successfully Add: "<<cur_name<<endl;

            return TRUE;

       }

    }

    return FALSE;   // no space.  Fix when we have extensible files.
}

//----------------------------------------------------------------------
// Directory::Remove
// 	Remove a file name from the directory.  Return TRUE if successful;
//	return FALSE if the file isn't in the directory. 
//
//	"name" -- the file name to be removed
//----------------------------------------------------------------------

void 
Directory::RemoveSubTree(int i){
    for(int j = 0; j < table[i].child_num; j ++){
        int idx = table[i].children[j];
        if(table[idx].inUse){
            table[idx].inUse = FALSE;
            table[idx].child_num = 0;
            RemoveSubTree(idx);
        }
    }
}

bool
Directory::Remove(char *name)
{ 
    int i = FindIndex(name);

    if (i == -1)
	   return FALSE; 		// name not in directory

    table[i].inUse = FALSE;

    // update father info
    
    int fatherID = table[i].father;
    if(fatherID != -1){
        vector<int> tmp_child;
        for(int j = 0; j < table[fatherID].child_num; j ++){
            if(table[fatherID].children[j] != i){
                tmp_child.push_back(table[fatherID].children[j]);
            }
        }
        for(int j = 0; j < tmp_child.size(); j ++){
            table[fatherID].children[j] = tmp_child[j];
        }
        table[fatherID].child_num --;
    }
    

    // remove sub-tree
    for(int j = 0; j < table[i].child_num; j ++){
        int idx = table[i].children[j];
        if(table[idx].inUse){
            table[idx].inUse = FALSE;
            table[idx].child_num = 0;
            RemoveSubTree(idx);
        }
    }

    return TRUE;	
}

//----------------------------------------------------------------------
// Directory::List
// 	List all the file names in the directory. 
//----------------------------------------------------------------------

void
Directory::List()
{
    
    cout<<"Directory Info List:"<<endl;
    printf("/root\n");

    for (int i = 1; i < tableSize; i++){
        if(table[i].inUse){
            vector<string> name_list;
            int idx = i;
            while(table[idx].father != -1){
                string tmp = table[idx].name;
                name_list.push_back(tmp);
                idx = table[idx].father;
            }
            cout<<"/root";
            for(int j = name_list.size() - 1; j >= 0; j --){
                cout << "/";
                cout << name_list[j];
            }
            cout<<endl;
        }
    }

}

//----------------------------------------------------------------------
// Directory::Print
// 	List all the file names in the directory, their FileHeader locations,
//	and the contents of each file.  For debugging.
//----------------------------------------------------------------------

void
Directory::VisOutput(){
    /*
    for example:
    dot -Tpng visTree.dot -o visTree.png
    */
    ofstream out;
    out.open("visTree.dot");
    out<<"strict graph s{"<<endl;
    for(int i = 0; i < tableSize; i ++){
        if(table[i].inUse){
            vector<string> name_list;
            int idx = i;
            while(table[idx].father != -1){
                string tmp = table[idx].name;
                name_list.push_back(tmp);
                idx = table[idx].father;
            }
            string tmp_r = "root";
            name_list.push_back(tmp_r);
            for(int j = name_list.size() - 1; j >= 1; j --){
                out<<name_list[j]<<" -- "<<name_list[j-1]<<";"<<endl;
            }         
        }
    }
    out<<"}"<<endl;
    out.close();

}

void
Directory::Print()
{ 

    FileHeader *hdr = new FileHeader;

    printf("Directory contents:\n");

    for (int i = 0; i < tableSize; i++){
    	if (table[i].inUse) {
            printf("Name: ");
            vector<string> name_list;
            int idx = i;
            while(table[idx].father != -1){
                string tmp = table[idx].name;
                name_list.push_back(tmp);
                idx = table[idx].father;
            }
            cout<<"/root";
            for(int j = name_list.size() - 1; j >= 0; j --){
                cout << "/";
                cout << name_list[j];
            }
            cout<<endl;
            printf("Sector: %d\n", table[i].sector);
    	    //printf("Name: %s, Sector: %d\n", table[i].name, table[i].sector);
    	    hdr->FetchFrom(table[i].sector);
    	    hdr->Print();
    	}
    }
    printf("\n");

    delete hdr;

    VisOutput();

}
