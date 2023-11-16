#pragma once

#include <iostream>
#include <functional>
#include <map>
#include <string>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//POSIX file IO include

#define SIZE_LIMIT 65535

class MemoryManager {
	private:
		std::map<uint16_t, uint16_t> allocatedMemory;	
		std::map<uint16_t, uint16_t> holes;
		//data structure to track allocated memory, and one to track the holes
		//track allocated memory explicitly to easily solve situations where we free memory directly next to another reserved chunk
		//while we could derive the holes from the allocated memory, the spec makes it sound like we have to track it explicity
		//so we do

		//the first uint is the offset, the second is the length

		void* memoryStart;
		bool isInitialized;	//redundant, but easier and clearer than checking if memoryStart is null
		unsigned bytesPerWord;
		size_t memorySizeInWords;	//can get total size by multiplying wordSize and size in words
		std::function<int(int, void*)> allocatorFunction;

	public:
		MemoryManager(unsigned wordSize, std::function<int(int, void*)> allocator);
		~MemoryManager();
		void initialize(size_t sizeInWords);
		void shutdown();
		void* allocate(size_t sizeInBytes);
		void free(void* address);
		void setAllocator(std::function<int(int, void*)> allocator);
		int dumpMemoryMap(char* filename);
		void* getList();
		void* getBitmap();
		unsigned getWordSize();
		void* getMemoryStart();
		unsigned getMemoryLimit();
};

int bestFit(int sizeInWords, void* list);
int worstFit(int sizeInWords, void* list);

//alocator functions, declared outside of class
