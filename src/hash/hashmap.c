#include "hashmap.h"
#include <stdio.h>
#include <stdlib.h>

int defaultHashCode(HashMap hashMap, void *key)
{
	char *k = (char *)key;
	unsigned long h = 0;

	while(*k)
	{
		h = (h << 4) + *k++;
		unsigned long g = h & 0XF0000000L;
		if(g)
		{
			h ^= g >> 24;
		}
		h &= ~g;
	}
	
	return h % (hashMap->listSize);
}

/*
int defaultHashCode(HashMap hashMap, void *key)
{
	static u_int32_t a;

	a = *((u_int32_t*) key);
	a = (a+0x7ed55d16) + (a << 12);
	a = (a^0xc761c23c) ^ (a >> 19);
	a = (a+0x165667b1) + (a << 5);
	a = (a+0xd3a2646c) ^ (a << 9);
	a = (a+0xfd7046c5) + (a << 3);
	a = (a^0xb55a4f09) ^ (a >> 16);
									
	return a % (hashMap->listSize);;
}
*/


void resetHashMap(HashMap hashMap, int listSize)
{
	if(listSize < 8) 
		return;

	//键值对临时存储空间
	Entry tempList = newEntryList(hashMap->size);
	
	HashMapIterator iterator = creatHashMapIterator(hashMap);
	int length = hashMap->size;
	for(int index = 0; hasNextHashMapIterator(iterator); index++)
	{
		//迭代取出所有键值对
		iterator = nextHashMapIterator(iterator);
		tempList[index].key = iterator->entry->key;
		tempList[index].value = iterator->entry->value;
		tempList[index].next = NULL;
	}
	
	freeHashMapIterator(&iterator);

	//清楚原有键值数据
	hashMap->size = 0;
	for(int i = 0; i < hashMap->listSize; i++)
	{
		Entry current = &hashMap->list[i];
		current->key = NULL;
		current->value = NULL;
		if(current->next != NULL)
		{
			while(current->next != NULL)
			{
				Entry temp = current->next->next;
				free(temp);
				current->next = temp;
			}
		}
	}

	//更改内存大小
	hashMap->listSize = listSize;
	Entry relist = (Entry)realloc(hashMap->list, hashMap->listSize * sizeof(struct entry));

	if(relist != NULL)
	{
		hashMap->list = relist;
		relist = NULL;
	}

	//初始化数据
	for(int i = 0; i < hashMap->listSize; i++)
	{
		hashMap->list[i].key = NULL;
		hashMap->list[i].value = NULL;
		hashMap->list[i].next = NULL;
	}

	//将所有键值对重新写入内存
	for(int i = 0; i < length; i++)
	{
		hashMap->put(hashMap, tempList[i].key, tempList[i].value);
	}
	
	free(tempList);
}

void defaultPut(HashMap hashMap, void *key, void *value)
{
	if(hashMap->autoAssign && hashMap->size >= hashMap->listSize)
	{
		//内存扩充至原来的两倍
		//*注：扩充时考虑的是当前存储元素数量与存储空间的大小关系，而不是存储空间是否已经存储
		//例如：存储空间为10，存入10个键值对，但是全部冲突，所以存储空间空着9个，其余的全挂在一个上面
		//这样检索的时候和遍历查询没有什么区别了，可以简单这样理解，当存入第11个键值对的时候一定会发生冲突
		//这是由哈希函数本事的特性决定的，冲突就会导致检索变慢，所以这时候扩充空间
		resetHashMap(hashMap, hashMap->listSize * 2);
	}

	int index = hashMap->hashCode(hashMap, key);
	
	if(hashMap->list[index].key == NULL)
	{
		hashMap->size++;
		hashMap->list[index].key = key;
		hashMap->list[index].value = value;
		hashMap->list[index].next = NULL;
	}
	else
	{
		Entry	current = &hashMap->list[index];
		while(current != NULL)
		{
			if(hashMap->equal(key, current->key))
			{
				//对于键值已经存在的直接覆盖
				current->value = value;
				return;
			}
			current = current->next;
		}

		Entry entry = newEntry();
		entry->key = key;
		entry->value = value;
		entry->next = hashMap->list[index].next;
		hashMap->list[index].next = entry;
		hashMap->size++;

	}
}

void *defaultGet(HashMap hashMap, void *key)
{
	int index = hashMap->hashCode(hashMap, key);
	Entry entry = &hashMap->list[index];

	while(entry != NULL && !hashMap->equal(entry->key, key))
	{
		entry = entry->next;
	}
	return entry->value;
}

Boolean defaultEqual(void *key1, void *key2)
{
	return *(int*)key1 == *(int*)key2 ? True : False;
}

Boolean defaultRemove(HashMap hashMap, void *key)
{
	int index = hashMap->hashCode(hashMap,key);
	Entry entry = &hashMap->list[index];

	Boolean result = False;

	if(entry->key == NULL)
	{
		return False;
	}

	if(hashMap->equal(entry->key, key))
	{
		hashMap->size--;
		if(entry->next != NULL)
		{
			Entry temp = entry->next;
			entry->key = temp->key;
			entry->value = temp->value;
			entry->next = temp->next;
			free(temp);
		}
		else
		{
			entry->key = NULL;
			entry->value = NULL;
		}
		result = True;
	}
	else
	{
		Entry p = entry;
		entry = entry->next;
		while(entry != NULL)
		{
			if(hashMap->equal(entry->key, key))
			{
				hashMap->size--;
				p->next = entry->next;
				free(entry);
				result = True;
				break;
			}
			p = entry;
			entry = entry->next;
		}	
	}

	//如果空间占用不足一半，则释放多余内存
	if(result && hashMap->autoAssign && hashMap->size < hashMap->listSize/2)
	{
		resetHashMap(hashMap, hashMap->listSize/2);
	}
	return result;
}

Boolean defaultExists(HashMap hashMap, void *key)
{
	int index = hashMap->hashCode(hashMap, key);
	Entry entry = &hashMap->list[index];

	if(entry->key == NULL)
		return False;

	if(hashMap->equal(entry->key, key))
		return True;

	if(entry->next != NULL)
	{
		do{
			if(hashMap->equal(entry->key, key))
			{
				return True;
			}
			entry = entry->next;
		}while(entry != NULL);
		return False;
	}
	else
		return False;
}

void defaultClear(HashMap hashMap)
{
	for(int i = 0; i < hashMap->listSize; i++)
	{
		Entry entry = &hashMap->list[i];
		while(entry != NULL)
		{
			Entry next = entry->next;
			free(entry);
			entry = next;
		}
		hashMap->list[i].next = NULL;
	}

	//释放存储空间
	free(hashMap->list);
	hashMap->list = NULL;
	hashMap->size = -1;
	hashMap->listSize = 0;
}


HashMap creatHashMap(HashCode hashCode, Equal equal)
{
	HashMap hashMap = newHashMap();
	hashMap->size = 0;
	hashMap->listSize = 8;
	hashMap->hashCode = hashCode == NULL ? defaultHashCode : hashCode;
	hashMap->equal = equal== NULL ? defaultEqual : equal;
	hashMap->exists = defaultExists;
	hashMap->get = defaultGet;
	hashMap->put = defaultPut;
	hashMap->remove = defaultRemove;
	hashMap->clear = defaultClear;
	hashMap->autoAssign = True;
	//起始分配8个内存空间，溢出自动扩充
	hashMap->list = newEntryList(hashMap->listSize);
	Entry p = hashMap->list;	
	for(int i = 0; i < hashMap->listSize; i++)
	{
		p[i].key = p[i].value = p[i].next = NULL;
	}
	return hashMap;
}

//迭代器实现
HashMapIterator creatHashMapIterator(HashMap hashMap)
{
	HashMapIterator iterator = newHashMapIterator();
	iterator->hashMap = hashMap;
	iterator->count = 0;
	iterator->hashCode = -1;
	iterator->entry = NULL;
	return iterator;
}

Boolean hasNextHashMapIterator(HashMapIterator iterator)
{
	return iterator->count < iterator->hashMap->size ? True: False;
}

HashMapIterator nextHashMapIterator(HashMapIterator iterator)
{
	if(hasNextHashMapIterator(iterator))
	{
		if(iterator->entry != NULL && iterator->entry->next != NULL)
		{
			iterator->count++;
			iterator->entry = iterator->entry->next;
			return iterator;
		}
		
		while(++iterator->hashCode < iterator->hashMap->listSize)
		{
			Entry entry = &iterator->hashMap->list[iterator->hashCode];
			if(entry->key != NULL)
			{
				iterator->count++;
				iterator->entry = entry;
				break;
			}
		}
	}

	return iterator;
}

void freeHashMapIterator(HashMapIterator *iterator)
{
	free(*iterator);
	*iterator = NULL;
}





