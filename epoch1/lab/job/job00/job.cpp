#include <iostream>

#ifndef FOOBAR
#define FOOBAR 42
#endif

int main(void) {
  std::cerr << "HELLO WORLD " << FOOBAR << std::endl;
}
