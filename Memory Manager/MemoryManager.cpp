#include "MemoryManager.h"

MemoryManager::MemoryManager(unsigned wordSize, std::function<int(int, void*)> allocator) {
	memoryStart = nullptr;
	isInitialized = false;
	bytesPerWord = wordSize;
	memorySizeInWords = 0;
	allocatorFunction = allocator;
}

MemoryManager::~MemoryManager() {
	shutdown();		//free any memory related to whatever block is open when the manager terminates
}

void MemoryManager::initialize(size_t sizeInWords) {
	if (sizeInWords > SIZE_LIMIT) return;	//if the requested block is larger than possible, don't make it

	shutdown();	
	//clear the already initialized memory, if there is any
	//the if is handled by shutdown, so just call it and let it do its thing

	memorySizeInWords = sizeInWords;
	memoryStart = malloc(sizeInWords * bytesPerWord);	//since we want a void pointer, no cast on malloc
	isInitialized = true;
	holes[0] = sizeInWords;
	//initialize variables related to the curernt block, not the whole manager
	//at the start we have 1 hole the size of the whole manager
}

void MemoryManager::shutdown() {
	if(isInitialized){
		std::free(memoryStart);
		allocatedMemory.clear();
		holes.clear();
		//we wipe the data structure of allocated memory, since that memory's all gone now

		memoryStart = nullptr;
		isInitialized = false;
		memorySizeInWords = 0;
		//reset vars related to the current block of memory
	}
	//only deallocate if there actually something to deallocate
}

void* MemoryManager::allocate(size_t sizeInBytes) {
	if (sizeInBytes > (memorySizeInWords * bytesPerWord)) return nullptr;
	//invalid size: more than block can hold
	
	size_t totalBytes = sizeInBytes;
	if(totalBytes % bytesPerWord != 0) totalBytes += (bytesPerWord - sizeInBytes % bytesPerWord); 
	//corrects number of bytes, as it needs to allocate a flat number of words. Pads bytes to fill a possible temporary word

	int newMemoryLength = totalBytes / bytesPerWord; //convert from bytes to words
	void* holeList = getList();

	int newOffset = allocatorFunction(newMemoryLength, holeList);
	std::free(holeList);		//deallocate hole list now that we've used it
	//make sure to specify std or it's gonna use its own free oops

	if (newOffset == -1) return nullptr;	//allocator couldn't find a valid hole, so exit

	allocatedMemory[newOffset] = newMemoryLength;
	//allocating can only add one new block of allocated memory, so just put it in
	//don't have to worry about a conflict, since that's not possible

	if (newMemoryLength == holes[newOffset]) {	//new offset is the beginning of an old hole, so it's a valid key for holes
		holes.erase(newOffset);
	}
	//first case: hole filled was exactly the size of the memory
	//just delete the hole, since it was filled entirely then there's no hole

	else {
		int newHoleSize = holes[newOffset] - newMemoryLength;	//the size of the new hole is the old size minus the space taken for memory
		int newHoleOffset = newOffset + newMemoryLength;		//the start of the new hole is the start of the old hole plus the offset for new memory
		holes.erase(newOffset);		//remove the old hole
		holes[newHoleOffset] = newHoleSize;		//add the new hole
	}
	//second case: hole was partially filled

	int bytesFromBeginning = newOffset * bytesPerWord;	//convert the new offset from words to bytes to get address
	char* pointerForArithmetic = static_cast<char*>(memoryStart);
	char* returnPointer = pointerForArithmetic + bytesFromBeginning;
	//we can't do pointer arithmetic on a void pointer, so we convert it to a char* to do arithmetic
	//char* is chosen since a char is one byte, so if we add n we move the pointer by n bytes

	return static_cast<void*>(returnPointer);
}

void MemoryManager::free(void* address) {
	char* pointerForArithmetic = static_cast<char*>(memoryStart);
	int addressByteOffset = static_cast<char*>(address) - pointerForArithmetic;
	//like with allocate, convert to char* for arithmetic

	int addressWordOffset = addressByteOffset / bytesPerWord;
	//this is division with an integer, so we're losing a decimal
	//this is intentional: if it comes back as being, say, in word offset 4.5, that means it's contained within the word that starts at 4
	//so dropping the decimal will get us the beginning of the word we need

	int memoryBegin = 0;
	int memoryEnd = 0;
	//declare these outside the for loop so we can refer back to them later

	for (auto iter = allocatedMemory.begin(); iter != allocatedMemory.end(); iter++) {
		memoryBegin = iter->first;
		memoryEnd = memoryBegin + iter->second;		//memoryEnd is the first byte not allocated
		//find out where the current tracked memory begins and ends

		if ((addressWordOffset >= memoryBegin) && (addressWordOffset < memoryEnd)) {	//bounded by begin and end means we're in this chunk
			allocatedMemory.erase(iter);
			break;
		}
		//for allocated memory, all we need to do is delete the tracker for the piece of memory we just freed
	}

	int sizeOfNewHole = memoryEnd - memoryBegin;
	holes[memoryBegin] = sizeOfNewHole;
	//memorybegin and end are still set to the start and end of the block we just removed
	//since we just removed it, there's a hole where the old memory was
	//we make the new hole, and then combine if necessary

	auto newHole = holes.find(memoryBegin);
	if (newHole != holes.begin()) {		//make sure there's a preceeding hole before we compare to it
		auto prevHole = std::prev(newHole);
		int prevHoleEnd = prevHole->first + prevHole->second;
		if (prevHoleEnd == newHole->first) {	//if the first bit after the preceeding hole equals the beginning of the new hole, they need to be combined
			int combinedHoleBegin = prevHole->first;
			int combinedHoleLength = prevHole->second + newHole->second;	//the combined hole is as long as both old ones put together
			holes.erase(newHole);
			holes.erase(prevHole);		//remove the two previous holes
			holes[combinedHoleBegin] = combinedHoleLength;	//put in the new one
			//we could really just change the value corresponding to the previous hole's offset, but this feels clearer
			newHole = holes.find(combinedHoleBegin);	//set the iterator pointing to the new hole to the combined hole, since we still need to check the following hole
		}
	}

	auto nextHole = std::next(newHole);
	if (nextHole != holes.end()) {		//make sure there is a next hole before we go in
		int newHoleEnd = newHole->first + newHole->second;
		if (newHoleEnd == nextHole->first) {		//similar to above: if the end of the new hole is directly followed by the start of the next hole, combine them
			int combinedHoleBegin = newHole->first;
			int combinedHoleLength = newHole->second + nextHole->second;
			holes.erase(newHole);
			holes.erase(nextHole);
			holes[combinedHoleBegin] = combinedHoleLength;
			//don't need to reset newHole here, since where are no more checks
		}
	}
	//that's all the functionalty and this doesn't need to reutrn anything, so just exit
}

void MemoryManager::setAllocator(std::function<int(int, void*)> allocator) {
	allocatorFunction = allocator;
}

int MemoryManager::dumpMemoryMap(char* filename) {
	int file = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0644);
	//opens the file and makes it if it doesn't exist
	//if it does exist, it wipes it

	if(file == -1)  return -1;	//if there's an error on file open, return -1
	std::string output = "";
	for(auto iter = holes.begin(); iter != holes.end(); iter++){
		output += "[";
		output += std::to_string(iter->first);
		output += ", ";
		output += std::to_string(iter->second);
		output += "]";
		//adds the hole info and container to output string

		if(std::next(iter) != holes.end()) output += " - ";
		//only adds a separator if the next iter isn't the end
	}

	int writeError = write(file, output.c_str(), output.length());
	if(writeError == -1) return -1;
	//write our output string, and then return -1 if we got an error

	return close(file);
	//last thing we do is close the file. this is a 0 on success and a -1 on fail, so we can just return it
}

void* MemoryManager::getList() {
	if (!isInitialized) {
		return nullptr;
	}
	int totalListSize = 2 * holes.size() + 1;
	//two entries per hole, plus an entry for the size

	uint16_t* outputList = (uint16_t*)malloc(sizeof(uint16_t) * totalListSize);
	//allocate and don't delete - everything that uses the list will handle deleting it
	//if we deleted in here, we couldn't use it :P

	outputList[0] = static_cast<uint16_t>(holes.size());

	int index = 1;	//tracks the current index of the list to insert into, starts at 1 since 0 is the length

	for (auto iter = holes.begin(); iter != holes.end(); iter++) {
		outputList[index++] = iter->first;
		outputList[index++] = iter->second;
		//post increment index to place the current data where we need it and move it at the same time
		//this will increment index to 1 higher than is valid, but the for loop will then exit and index will never be used in its invalid state
	}

	return static_cast<void*>(outputList);	//have to return a void pointer, so we cast
}

void* MemoryManager::getBitmap() {
	int bitmapSizeWords = memorySizeInWords;
	if (bitmapSizeWords % 8 != 0) bitmapSizeWords += (8 - bitmapSizeWords % 8);
	//correct the size to account for the fact that we need to hold in bytes

	int bitmapSize = bitmapSizeWords / 8;	//convert the size to bytes
	uint8_t* bitmap = (uint8_t*) malloc(bitmapSize + 2);	//allocate the bitmap, plus the two spaces for the size

	int lowerSizeByte = bitmapSize % 256;
	int higherSizeByte = bitmapSize / 256;
	
	bitmap[0] = static_cast<uint8_t>(lowerSizeByte);
	bitmap[1] = static_cast<uint8_t>(higherSizeByte);

	int lastHoleEndpoint = 0;
	int index = 2;		//index for inserting into the array, starts at 2 since the first 2 are for the size
	std::string bitstream = "";
	for (auto iter = holes.begin(); iter != holes.end(); iter++) {
		int currentHoleBegin = iter->first;
		if (currentHoleBegin != lastHoleEndpoint) {		//only every false if memory starts with a hole, but that's good enough to have it
			int allocatedMemorySize = currentHoleBegin - lastHoleEndpoint;		//get length of allocated memory
			std::string memoryBits = std::string(allocatedMemorySize, '1');		//fill contrusctor, makes string with allocatedmemorysize copies of '1'
			bitstream = memoryBits + bitstream;		//little endian, so concatenate the new bits to the front
		}
		std::string holeBits = std::string(iter->second, '0');
		bitstream = holeBits + bitstream;		//get the length of the hole, then add that many zeroes to the bitmap

		while (bitstream.length() >= 8) {		//trim as we go to avoid bitstream getting too large
			std::string byte = bitstream.substr(bitstream.length() - 8, 8);		//get the last 8 bits from the bitstream
			uint8_t bitmapEntry = std::stoi(byte, nullptr, 2);		//converts the byte to an integer using base 2 (binary)
			bitmap[index++] = bitmapEntry;	//put the entry in the bitmap and increment the index for next time
			bitstream = bitstream.substr(0, bitstream.length() - 8);	//trim the bits we just used from the string
		}
		//this will repeat as long as there's a valid byte to process

		lastHoleEndpoint = currentHoleBegin + iter->second;	//update the endpoint to process the next memory chunk
	}

	if (lastHoleEndpoint != memorySizeInWords) {	//if the end of the last hole wasn't the length of the bitmap, then there's allocated memory at the end to add
		int allocatedMemorySize = memorySizeInWords - lastHoleEndpoint;
		std::string memoryBits = std::string(allocatedMemorySize, '1');
		bitstream = memoryBits + bitstream;
	}

	if (memorySizeInWords != bitmapSizeWords) {		//if the size of memory didn't divide evenly into bytes, fill the remaining space with padding zeroes
		int paddingZeros = bitmapSizeWords - memorySizeInWords;
		std::string padBits = std::string(paddingZeros, '0');
		bitstream = padBits + bitstream;
	}

	while (bitstream.length() >= 8) {		
		std::string byte = bitstream.substr(bitstream.length() - 8, 8);		
		uint8_t bitmapEntry = std::stoi(byte, nullptr, 2);		
		bitmap[index++] = bitmapEntry;	
		bitstream = bitstream.substr(0, bitstream.length() - 8);	
	}
	//put everything not got during for loop

	return static_cast<void*>(bitmap);
}

unsigned MemoryManager::getWordSize() {
	return bytesPerWord;
}

void* MemoryManager::getMemoryStart() {
	return memoryStart;
}

unsigned MemoryManager::getMemoryLimit() {
	return bytesPerWord * memorySizeInWords;
}
//total number of words times bytes in each word gives total number of bytes

int bestFit(int sizeInWords, void* list) {
	int wordOffset = -1;	//initialize to return value for failure, since then we don't need any special cases (we just never change it)
	uint16_t bestHoleSize = UINT16_MAX;	//initialize to max for uint_16 so it'll always be replaced by the first hole we find
	uint16_t* holes = static_cast<uint16_t*>(list);
	uint16_t holesCount = *holes;
	uint16_t desiredSize = static_cast<uint16_t>(sizeInWords);	//cast to make sure comparisons are all above board

	for (uint16_t i = 1; i < holesCount * 2; i += 2) {		//since the 0th entry is the length, start at 1. since we consider the start and offset together, increment by 2
		if (holes[i + 1] >= desiredSize) {		//if the offset is larger than our desired size, the data will fit in the hole
			if (holes[i + 1] < bestHoleSize) {	//best fit is the smallest hole. If the new hole is smaller than the current best, it's better (since we already know it fits)
				bestHoleSize = holes[i + 1];
				wordOffset = static_cast<int>(holes[i]);
			}
		}
		//these two if statements could be combined with &&, but I feel the logic is more readable like this
		//and besides, they probably compile to the same thing anyway
	}

	return wordOffset;
}

int worstFit(int sizeInWords, void* list) {
	int wordOffset = -1;	
	uint16_t bestHoleSize = 0;	//initialize to min for uint_16 (0, since it's unsigned) so it'll always be replaced by the first hole we find
	uint16_t* holes = static_cast<uint16_t*>(list);
	uint16_t holesCount = *holes;
	uint16_t desiredSize = static_cast<uint16_t>(sizeInWords);	

	for (uint16_t i = 1; i < holesCount * 2; i += 2) {		//holes count is multiplied by 2 since holes has 2 entries per hole
		if (holes[i + 1] >= desiredSize) {		//if the offset is larger than our desired size, the data will fit in the hole
			if (holes[i + 1] > bestHoleSize) {	//worst fit is the biggest hole. If the new hole is bigger than the current best, it's worse (since we already know it fits)
				bestHoleSize = holes[i + 1];
				wordOffset = static_cast<int>(holes[i]);
			}
		}
	}

	return wordOffset;
}
