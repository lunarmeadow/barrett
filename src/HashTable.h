
/*
 * File:   HashTable.h
 * Author: Steven LeVesque
 *
 * Created on March 13, 2018, 5:10 PM
 */

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "rt_def.h"
#include "lumpy.h"

#include "w_kpfdat.h"

typedef struct listNode
{
	int isAvaliable;
	int key;
	int data;
} listNode;

typedef struct HashTable
{
	int totalSize;
	listNode** table;
} HashTable;


typedef struct patchNode
{
	int isAvaliable;
	int key;
	decode_patch_t* data;
	int size;
} patchNode;

typedef struct PatchTable
{
	int totalSize;
	patchNode** table;
} PatchTable;

// STANDARD HASH TABLE

void InitHashTable(HashTable* hashTable, int initSize);
int HashFunc(HashTable* hashTable, int key);
void Delete(HashTable* hashTable, int key);
void ClearHashTable(HashTable* hashTable);
void Insert(HashTable* hashTable, int key, int item);
int Lookup(HashTable* hashTable, int key);

// PATCH CACHING TABLE

void InitPatchTable(PatchTable* hashTable, int initSize);
int HashPatchFunc(PatchTable* hashTable, int key);
void DeletePatch(PatchTable* hashTable, int key);
void ClearPatchTable(PatchTable* hashTable);
void InsertPatch(PatchTable* hashTable, int key, decode_patch_t* item);
decode_patch_t* LookupPatch(PatchTable* hashTable, int key);


#endif /* HASHTABLE_H */
