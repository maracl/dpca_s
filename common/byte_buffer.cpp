/*
 * byte_buffer.cpp
 *
 *  Created on: 2013-10-9
 *      Author: ltmit
 */
//#include "stdafx.h"
#include "byte_buffer.h"
#include <string.h>
#include <stdlib.h>

void* g_malloc(size_t sz)  {
	return malloc(sz); //malloc(sz);
}

void* g_realloc(void* old,size_t sz) {
	return realloc(old,sz);//realloc(old,sz);
}

void g_free(void* p) {
	free(p);//free(p);
}

//[------------------------Pointer wrapper--------------------------------

_bb_ptr_wrap BBPW(const char* str)
{
	_bb_ptr_wrap pw(str,strlen(str));
	return pw;
}

_bb_ptr_wrap BBPW(const void* dt,size_t sz)
{
	_bb_ptr_wrap pw(dt,sz);
	return pw;
}

//[----------------------------------------------------------------------
byte_buffer::byte_buffer():loff(0),roff(0) {
	_ptr= (char*)g_malloc(_bufsz=64);
}

byte_buffer::byte_buffer(size_t init_sz):loff(0),roff(0) {
	for(_bufsz=8;_bufsz<init_sz;_bufsz<<=1) ;
	_ptr= (char*)g_malloc(_bufsz);
}

byte_buffer::~byte_buffer() {
	if(_ptr)
		g_free(_ptr);
}

//when detached, the buffer cannot be used!
void* byte_buffer::detach()
{
	char* op= _ptr;
	loff=roff=_bufsz=0;
	_ptr=0;
	return op;
}

void byte_buffer::check_memsize(size_t newsz)
{
	if(newsz<=_bufsz) return ;
	if(_bufsz==0) _bufsz=64;
	for(;_bufsz<newsz;_bufsz<<=1) ;
	_ptr=(char*)(_ptr==0?g_malloc(_bufsz):g_realloc(_ptr,_bufsz));
}

void byte_buffer::pre_alloc(size_t sz)
{
	check_memsize(sz);
}

byte_buffer& byte_buffer::push_data(const void* ptr,size_t sz)
{
	if(sz>0) {
		check_memsize(roff+sz);
		memcpy(_ptr+roff,ptr,sz);
		roff+=sz;
	}
	return *this;
}

bool byte_buffer::pop_data(void* buf,size_t bufsz) {
	if(bufsz>0) {
		if(data_size()<bufsz) return false;
		memcpy(buf,_ptr+loff,bufsz);
		loff+=bufsz;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
bool CharSeqReader::read_data(void* data,size_t dsz)
{
	if(off+dsz>size) return false;
	memcpy(data,ptr+off,dsz);
	off+=dsz;
	return true;
}
