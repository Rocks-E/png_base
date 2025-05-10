#include "png.hpp"

bool crc_table_computed = false;
uint64_t crc_table[256];

void make_crc_table() {
	
	for(uint16_t n = 0; n < 256; n++) {
		
		uint64_t c = (uint64_t)n;
		
		for(uint8_t k = 0; k < 8; k++)
			c = (c >> 1) ^ ((c & 1) ? 0xEDB88320L : 0);
		
		crc_table[n] = c;
		
	}
	
	crc_table_computed = true;
	
}

uint64_t update_crc(uint64_t crc, char *buffer, size_t length) {
	
	if(!crc_table_computed)
		make_crc_table();
	
	for(size_t c = 0; c < length; c++)
		crc = crc_table[(crc ^ buffer[c]) & 0xFF] ^ (crc >> 8);
	
	return crc;
	
}

uint64_t update_crc(uint64_t crc, std::fstream &file, size_t length) {
	
	if(!crc_table_computed)
		make_crc_table();
	
	char *buffer = (char *)malloc(length * sizeof(char));
	file.read(buffer, length);
	
	for(size_t c = 0; c < length; c++)
		crc = crc_table[(crc ^ buffer[c]) & 0xFF] ^ (crc >> 8);
	
	return crc;
	
}

uint64_t get_crc(char *buffer, size_t length) {
	return update_crc(0xFFFFFFFFL, buffer, length) ^ 0xFFFFFFFFL;
}

uint64_t get_crc(std::fstream &file, size_t length) {
	return update_crc(0xFFFFFFFFL, file, length) ^ 0xFFFFFFFFL;
}

/* png_chunk */

png_chunk::png_chunk() {}

png_chunk::png_chunk(char *buffer) {
	
	// First four bytes define data length
	this->data_length = ((uint32_t *)buffer)[0];
	// Next four bytes give the chunk type, just treat as one 4 byte value for copyingCracker
	*(uint32_t *)this->chunk_type = ((uint32_t *)buffer)[1];
	
	// If there is data in this chunk, allocate space for it and copy data over
	if(this->data_length) {
		
		this->chunk_data = (char *)malloc(this->data_length * sizeof(char));
		
		// buffer + 8 to account for the chunk header info
		std::memcpy(this->chunk_data, buffer + 8, this->data_length);
		
	}

	*(uint32_t *)this->chunk_crc = ((uint32_t *)(buffer + this->data_length))[2];
	
}

png_chunk::png_chunk(std::fstream &file) {
	this->get_chunk(file);
}

void png_chunk::get_chunk(std::fstream &file) {
	
#ifdef _DEBUG
	std::cout << "File loc before reading: " << file.tellg() << '\n';
#endif
	
	// Read the four data bytes, swap endianness since PNG uses big endian encoding
	file.read((char *)(&this->data_length), 4);
	this->data_length = __builtin_bswap32(this->data_length);
	
#ifdef _DEBUG
	std::cout << "File loc after length: " << file.tellg() << '\n';
#endif
	
	// No need to swap endianness for the chunk type since it's just a char string
	file.read((char *)(&this->chunk_type), 4);
	
#ifdef _DEBUG
	std::cout << "File loc after chunk: " << file.tellg() << '\n';
#endif
	
	// If there is data in this chunk, allocate space for it and copy data over
	if(this->data_length) {
		
		this->chunk_data = (char *)malloc(this->data_length * sizeof(char));
		file.read(this->chunk_data, this->data_length);
		
	}
	
#ifdef _DEBUG
	std::cout << "File loc after data: " << file.tellg() << '\n';
#endif
	
	// Read the CRC, again swap endianness
	file.read((char *)(&this->chunk_crc), 4);
	this->chunk_crc = __builtin_bswap32(this->chunk_crc);
	
#ifdef _DEBUG
	std::cout << "File loc after crc: " << file.tellg() << '\n';
#endif
	
}

std::string png_chunk::to_string() const {
	
	std::stringstream result_stream;
	
	result_stream << "Chunk data length: " << this->data_length << '\n';
	result_stream << "Chunk type: " << this->chunk_type << '\n';
	result_stream << "\t" << (this->chunk_type[0] & 0b00100000 ? "Ancillary" : "Critical") << '\n';
	result_stream << "\t" << (this->chunk_type[1] & 0b00100000 ? "Private" : "Public") << '\n';
	result_stream << "\t" << (this->chunk_type[2] & 0b00100000 ? "Non-conforming" : "Conforming") << '\n';
	result_stream << "\t" << (this->chunk_type[3] & 0b00100000 ? "Safe to copy" : "Unsafe to copy") << '\n';
	result_stream << "Chunk CRC: " << this->chunk_crc << '\n';
	
	return result_stream.str();
	
}

