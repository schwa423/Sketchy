//
//  ThreadOwner.cpp
//  Sketchy
//
//  Created by Joshua Gargus on 12/21/11.
//  Copyright (c) 2011 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//

#include "ThreadOwner.h"

// We use this pthread stuff because Apple C++11
// doesn't yet support thread-local storage.
#include <pthread.h>
static pthread_key_t tls_loop_key;
static pthread_once_t tls_once_control = PTHREAD_ONCE_INIT;
static void tls_loop_make_key() {
    (void) pthread_key_create(&tls_loop_key, NULL);
}

bool
ThreadOwner::isCurrent() {
	return this == (ThreadOwner*)pthread_getspecific(tls_loop_key);
}

void
ThreadOwner::setAsThreadOwner() {
	// Set up thread-local storage, so that anyone can 
	// check whether we're the currently-running loop...
	// see isCurrent().
	pthread_once(&tls_once_control, tls_loop_make_key);
	pthread_setspecific(tls_loop_key, this);
}