#include <stdlib.h>
#include <string.h>
#include "HashTable.h"

#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif


// ===================================================
// STANDARD HASH TABLE
// ===================================================

void InitHashTable(HashTable* hashTable, int initSize)
{
	hashTable->totalSize = initSize;

	hashTable->table = calloc(sizeof(listNode), initSize);

	listNode* emptyNode = malloc(sizeof(listNode));

	emptyNode->isAvaliable = 1;

	emptyNode->key = -1;
	emptyNode->data = -1;

	int x;

	for (x = 0; x < initSize; x++)
	{
		listNode* node = malloc(sizeof(listNode));

		memcpy(node, emptyNode, sizeof(&emptyNode));

		hashTable->table[x] = node;
	}

	free(emptyNode);
}

int HashFunc(HashTable* hashTable, int key)
{
	return abs((key)) % hashTable->totalSize;
}

void Delete(HashTable* hashTable, int key)
{
	int index = HashFunc(hashTable, key);

	while (hashTable->table[index]->key != key)
	{
		// go to next cell
		++index;

		// wrap around the table
		index %= hashTable->totalSize;
	}

	// LinkedList * list = hashTable->table[index];

	// DeleteWithKey(list, key);
	// free(&hashTable->table[index]);
	hashTable->table[index]->isAvaliable = 1;
}

void ClearHashTable(HashTable* hashTable)
{
	int x = 0;

	// free all table entries until last index.
	// do allows zero-case to be covered,
	// pre-increment ensures that last case isn't skipped
	do free(hashTable->table[x]);
		while(hashTable->table[++x] != NULL);

	free(hashTable->table);
}

void Insert(HashTable* hashTable, int key, int item)
{
	int index = HashFunc(hashTable, key);

	while (hashTable->table[index]->isAvaliable == 0)
	{
		// go to next cell
		++index;

		// wrap around the table
		index %= hashTable->totalSize;
	}

	// listNode * newNode = malloc(sizeof(listNode));

	hashTable->table[index]->data = item;
	hashTable->table[index]->key = key;
	hashTable->table[index]->isAvaliable = 0;
}

int Lookup(HashTable* hashTable, int key)
{
	int index = HashFunc(hashTable, key);

	int origIndex = index;

	while (hashTable->table[index]->key != key &&
		   hashTable->table[index]->isAvaliable == 0)
	{
		// go to next cell
		++index;

		// wrap around the table
		index %= hashTable->totalSize;

		if (index == origIndex)
		{
			return 0;
		}
	}

	return hashTable->table[index]->data;
}

// ===================================================
// PATCH CACHING TABLE
// ===================================================

void InitPatchTable(PatchTable* hashTable, int initSize)
{
	hashTable->totalSize = initSize;

	hashTable->table = calloc(sizeof(patchNode), initSize);

	patchNode* emptyNode = malloc(sizeof(patchNode));
	
	emptyNode->data->width = 0;
	emptyNode->data->height = 0;
	emptyNode->data->leftoffset = 0;
	emptyNode->data->topoffset = 0;
	emptyNode->data->origsize = 0;

	emptyNode->size = 0;

	int x;

	for (x = 0; x < initSize; x++)
	{
		patchNode* node = malloc(sizeof(patchNode));

		memcpy(node, emptyNode, sizeof(&emptyNode));

		hashTable->table[x] = node;
	}

	free(emptyNode);
}

int HashPatchFunc(PatchTable* hashTable, int key)
{
	return abs((key)) % hashTable->totalSize;
}

void PatchDelete(PatchTable* hashTable, int key)
{
	int index = HashPatchFunc(hashTable, key);

	while (hashTable->table[index]->key != key)
	{
		// go to next cell
		++index;

		// wrap around the table
		index %= hashTable->totalSize;
	}

	// LinkedList * list = hashTable->table[index];

	// DeleteWithKey(list, key);
	// free(&hashTable->table[index]);
	hashTable->table[index]->isAvaliable = 1;
}

void ClearPatchTable(PatchTable* hashTable)
{
	int x = 0;

	// free all table entries until last index.
	// do allows zero-case to be covered,
	// pre-increment ensures that last case isn't skipped
	do free(hashTable->table[x]);
		while(hashTable->table[++x] != NULL);

	free(hashTable->table);
}

void InsertPatch(PatchTable* hashTable, int key, decode_patch_t* item)
{
	int index = HashPatchFunc(hashTable, key);

	while (hashTable->table[index]->isAvaliable == 0)
	{
		// go to next cell
		++index;

		// wrap around the table
		index %= hashTable->totalSize;
	}

	// listNode * newNode = malloc(sizeof(listNode));

	hashTable->table[index]->data = item;
	hashTable->table[index]->key = key;
	hashTable->table[index]->isAvaliable = 0;
}

decode_patch_t* LookupPatch(PatchTable* hashTable, int key)
{
	int index = HashPatchFunc(hashTable, key);

	int origIndex = index;

	while (hashTable->table[index]->key != key &&
		   hashTable->table[index]->isAvaliable == 0)
	{
		// go to next cell
		++index;

		// wrap around the table
		index %= hashTable->totalSize;

		if (index == origIndex)
		{
			return 0;
		}
	}

	return hashTable->table[index]->data;
}