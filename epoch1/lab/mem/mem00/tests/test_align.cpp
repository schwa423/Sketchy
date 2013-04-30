//
//  test_align.cpp
//  schwa::mem00
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//  

#include "mem00/align.h"

#include <iostream>
using namespace std;

#include <assert.h>


struct CACHE_ALIGNED Foo {

	int a;
	int alignas(16) b;
};

struct FooHolder {
	int fooCount;
	Foo foo1;
	Foo foo2;
};

struct FooHolderHolder {
	int padder;
	FooHolder holder;
};

struct FooHolderHolderHolder {
	int padder;
	FooHolderHolder holderHolder;
};


int main(void) {

	Foo foos[2];

	assert(alignment_of<Foo>::value == CACHE_LINE_SIZE);

	assert(reinterpret_cast<uint64_t>(&(foos[0])) % CACHE_LINE_SIZE == 0);
	assert(reinterpret_cast<uint64_t>(&(foos[0].b)) % 16 == 0);
	assert(reinterpret_cast<uint64_t>(&(foos[1])) % CACHE_LINE_SIZE == 0);
	assert(reinterpret_cast<uint64_t>(&(foos[1].b)) % 16 == 0);

	assert(sizeof(Foo) > sizeof(int) + sizeof(int));

	FooHolder CACHE_ALIGNED holder;

	assert(reinterpret_cast<uint64_t>(&holder) % CACHE_LINE_SIZE == 0);
	assert(reinterpret_cast<uint64_t>(&(holder.foo1)) % CACHE_LINE_SIZE == 0);
	assert(reinterpret_cast<uint64_t>(&(holder.foo2)) % CACHE_LINE_SIZE == 0);


	FooHolderHolder CACHE_ALIGNED holderHolder;
	assert(CACHE_LINE_SIZE == reinterpret_cast<uint64_t>(&(holderHolder.holder)) - reinterpret_cast<uint64_t>(&holderHolder));
	assert(reinterpret_cast<uint64_t>(&(holderHolder.holder)) % CACHE_LINE_SIZE == 0);
	assert(reinterpret_cast<uint64_t>(&(holderHolder.holder.foo1)) % CACHE_LINE_SIZE == 0);
	assert(reinterpret_cast<uint64_t>(&(holderHolder.holder.foo2)) % CACHE_LINE_SIZE == 0);
	cerr << "ALIGNMENT:  " << reinterpret_cast<uint64_t>(&(holderHolder.holder.foo2.b)) % CACHE_LINE_SIZE << endl;
	assert(reinterpret_cast<uint64_t>(&(holderHolder.holder.foo2.b)) % CACHE_LINE_SIZE == 16);


	FooHolderHolderHolder holderHolderHolder;
	assert(reinterpret_cast<uint64_t>(&(holderHolderHolder.holderHolder.holder.foo1)) % CACHE_LINE_SIZE == 0);

    cerr << "mem00/align...  PASSED!" << endl << endl;
}
