// 1. Never use constexpr and noexcept.
//
// 2. If C++ compiler can't deduce template argument,
//    then we just need to pass the argument manually instead of playing with
//    template tricks.