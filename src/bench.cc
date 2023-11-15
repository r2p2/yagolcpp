#include "gol.h"

#include <random>

int main()
{
  GOL gol(4000, 4000);

  auto const seed = 1337; // deterministic
  { // initial randomization
    std::mt19937 rng_gen(seed);
    std::uniform_int_distribution<> distrib(0, 100);
    auto const& size = gol.array().size();
    for (std::size_t i = 0; i < size; ++i) {
      if (distrib(rng_gen) <= 50 /* percent */) {
        gol.set(i);
      }
    }
  }

  for (int i = 0; i < 5000; ++i) {
    gol.iterate();
  }

  return 0;
}
