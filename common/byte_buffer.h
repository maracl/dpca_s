/*
 * byte_buffer.h
 *
 *  Created on: 2013-10-9
 *      Author: ltmit
 */

#ifndef BYTE_BUFFER_H_
#define BYTE_BUFFER_H_
#include "common_interface.h"
//#include "malloc_utils.h"
#include <string>
#include <stdarg.h>
#include "General_exception2.h"
#include "basic_error.h"
#include <stdlib.h>
#include "basic_error.h"

class byte_buffer;

//used for Pointer/string-stream Wrapper
class _bb_ptr_wrap{
	friend class byte_buffer;
	friend _bb_ptr_wrap BBPW(const char*);
	friend _bb_ptr_wrap BBPW(const void*,size_t);
protected:
	const void* data;
	size_t size;
	_bb_ptr_wrap(const void* _dt,size_t _sz):data(_dt),size(_sz) {}
public:
	~_bb_ptr_wrap() {}
};

_bb_ptr_wrap BBPW(const char* str);
_bb_ptr_wrap BBPW(const void* dt,size_t sz);

class byte_buffer:no_copy {
	char* _ptr;
	size_t _bufsz;
	size_t loff,roff;

	void check_memsize(size_t newsz);
public:
	//friend class EngCore;
	void* detach();
public:
	byte_buffer();
	explicit byte_buffer(size_t init_sz);
	~byte_buffer();

	void pre_alloc(size_t sz);

	size_t buf_size() const {
		return _bufsz;
	}
	char* raw_data() {
		return _ptr;
	}

	char* data_ptr() {
		return _ptr+loff;
	}

	const char* data_ptr() const {
		return _ptr+loff;
	}

	size_t data_size() const{
		return roff-loff;
	}

	size_t lpos() const {
		return loff;
	}

	size_t rpos() const {
		return roff;
	}

	//left-ptr set
	size_t lset(size_t off) {
		size_t old = loff;
		if((loff=off)>roff) loff=roff;
		return old;
	}
	//right-ptr set
	size_t rset(size_t off) {
		size_t old = roff;
		if((roff=off)<loff) roff=loff;
		else if(roff>_bufsz) roff=_bufsz;
		return old;
	}

	//both set left-ptr and right-ptr
	void bset(size_t off) {
		if(off>_bufsz) off=_bufsz;
		loff=roff=off;
	}

	byte_buffer& push_data(const void* ptr,size_t sz);

	bool pop_data(void* buf,size_t bufsz);

	byte_buffer& operator << (const _bb_ptr_wrap& wrap) {
		int siz=(int)wrap.size;
		push_data(&siz,sizeof(int));
		return push_data(wrap.data,wrap.size);
	}

	byte_buffer& operator << (const std::string& str) {
		int strsz=(int)str.size();
		push_data(&strsz,sizeof(int));
		return push_data(str.data(),strsz);
	}

	byte_buffer& operator << (const byte_buffer& bb) {
		int datasz=(int)bb.data_size();
		push_data(&datasz,sizeof(int));
		return push_data(bb.data_ptr(),datasz);
	}

	template<class T>
	byte_buffer& operator << (const T& obj) {
		return push_data(&obj,sizeof(obj));
	}

	void swap(byte_buffer& b) {
		std::swap(_ptr, b._ptr);
		std::swap(_bufsz, b._bufsz);
		std::swap(loff, b.loff);
		std::swap(roff, b.roff);
	}// 	byte_buffer& operator >> (std::string& str) {
// 		int strsz=0;
// 		if(!pop_data(&strsz,sizeof(int)))
// 			throw "byte_buffer: invalid op >>, buffer data isn't available";
// 		if(strsz<0||strsz>10000000000)
// 			throw "byte_buffer: invalid string size!";
// 		str.resize(strsz);
// 		if(!pop_data((char*)str.data(),strsz))
// 			throw "byte_buffer: invalid op >>, buffer data isn't available";
// 		return *this;
// 	}
// 
// 	template<class T>
// 	byte_buffer& operator >> (T& obj) {
// 		if(!pop_data(&obj,sizeof(obj))) {
// 			throw "byte_buffer: invalid op >>, buffer data isn't available";
// 		}
// 		return *this;
// 	}
	///----------------------------------------///
// 	static void* operator new(size_t sz) throw(){
// 		return g_malloc(sz);
// 	}
// 	static void operator delete (void* ptr) throw() {
// 		return g_free(ptr);
// 	}
};

#if defined(__linux) || defined(__APPLE__)
inline static int format_string(std::string& str,const char* format, ...) {
	va_list args;
	va_start( args, format );
	char* ps=0;
	int rt=vasprintf(&ps,format,args);
	if(ps!=0) {str=ps; free(ps); }
	va_end(args);
	return rt;
}
#else//windows
inline static int format_string(std::string& str,const char* format, ...) {
	va_list args;
	va_start( args, format );
	str.resize(_vscprintf( format, args )); // _vscprintf doesn't count terminating '\0'
	return vsprintf_s((char*)str.data(),str.size()+1, format, args ); 
}
#endif



class CharSeqReader{
	const char* ptr;
	size_t off,size;
public:
	CharSeqReader(const char* p,size_t sz):ptr(p),off(0),size(sz){}

	bool read_data(void* data,size_t dsz);

	size_t offset() const {return off; }
	
	size_t tot_len() const { return size; }

	size_t leftbytes() const { return size-off; }

	template<class T>
	bool read_data(T& obj) {
		return read_data(&obj,sizeof(obj));
	}

	CharSeqReader& operator >> (std::string& str) {
		int strsz=0;
		if(!read_data(strsz)) throw GeneralException2(ERROR_BYTEBUFFER_READ_STRSZ).format_errmsg("CharSeqReader: read strsz failed,left size=%ld",size-off);
		if(strsz<0||strsz>10000000000)
			throw GeneralException2(ERROR_BYTEBUFFER_INVALID_STRSZ,"byte_buffer: invalid string size!");
		str.resize(strsz);
		if(!read_data((char*)str.data(),strsz)) 
			throw GeneralException2(ERROR_BYTEBUFFER_READ_STR).format_errmsg("CharSeqReader: read str failed,require size=%ld but left size=%ld",strsz,size-off);
		return *this;
	}

	template<class T>
	CharSeqReader& operator >> (T& obj) {
		if(!read_data(&obj,sizeof(obj))) {
			throw GeneralException2(ERROR_BYTEBUFFER_INVALID_OP).format_errmsg("CharSeqReader: invalid op >>, require size=%ld but left size=%ld",sizeof(obj),size-off);
		}
		return *this;
	}

	const char* cur_ptr() const {
		return ptr + off;
	}

	void skip_bytes(size_t bn) {
		off += bn;
		if (off > size) off = size;
	}
};





#endif /* BYTE_BUFFER_H_ */
