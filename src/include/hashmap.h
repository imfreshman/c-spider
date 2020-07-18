#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct entry{
	void *key;	// 键
	void *value;	// 值
	struct entry *next;
}*Entry;

#define NEW(type) (type*)malloc(sizeof(type))

#define newEntry() NEW(struct entry);
#define newEntryList(length) (Entry)malloc(length * sizeof(struct entry))

enum _Boolean { True = 1, False = 0 };
typedef enum _Boolean Boolean;

//哈希结构
typedef struct hashMap* HashMap;
#define newHashMap() NEW(struct hashMap)

//哈希函数类型
typedef int(*HashCode)(HashMap hashMap, void *key);

//判断函数类型
typedef Boolean(*Equal)(void *key1, void *key2);

//添加键函数
typedef void(*Put)(HashMap hashMap, void *key, void *value);

//获取键对应值的函数类型
typedef void * (*Get)(HashMap hashMap, void *key);

//删除键的函数类型
typedef Boolean(*Remove)(HashMap hashMap, void *key);

//清空Map的函数类型
typedef void(*Clear)(HashMap hashMap);

//判断键值是否存在的函数类型
typedef Boolean(*Exists)(HashMap hashMap, void *key);

typedef struct hashMap{
	int size;		//当前大小
	int listSize;		//有效空间
	HashCode hashCode;	//哈希函数
	Equal equal;		//判断函数
	Entry list;		//存储区域
	Put put;		//添加键的函数
	Get get;		//获取对应键的函数
	Remove remove;		//删除键
	Clear	clear;		//清空map
	Exists	exists;		//判断键是否存在
	Boolean	autoAssign;	//设定是否根据当前数据量动态调整内存大小，默认开启
}*HashMap;


//默认哈希函数
static int defaultHashCode(HashMap hashMap, void *key);

//默认判断键值是否相等
static Boolean defaultEqual(void *key1, void *key2);

//默认添加键值对
static void defaultPut(HashMap hashMap, void *key, void *value);

//默认获取键值对应值
static void *defaultGet(HashMap hashMap, void *key);

//默认删除键
static Boolean defaultRemove(HashMap hashMap, void *key);

//默认判断键是否存在
static Boolean defaultExists(HashMap hashMap, void *key);

//默认清空map
static void defaultClear(HashMap hashMap);

//创建一个哈希结构
HashMap creatHashMap(HashCode hashCode, Equal equal);

//重建结构
static void resetHashMap(HashMap hashMap, int listSize);



//迭代器结构
typedef struct hashMapIterator{
	Entry entry;	//迭代器当前指向
	int count;	//迭代次数
	int hashCode;	//键值对的哈希值
	HashMap hashMap;
}*HashMapIterator;

#define newHashMapIterator() NEW(struct hashMapIterator)

//创建哈希结构迭代器
HashMapIterator creatHashMapIterator(HashMap hashMap);

//迭代器是否有下一个
Boolean hasNextHashMapIterator(HashMapIterator iterator);

//迭代到下一次
HashMapIterator nextHashMapIterator(HashMapIterator iterator);

//释放迭代器内存
void freeHashMapIterator(HashMapIterator * iterator);


#endif





