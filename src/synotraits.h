#ifndef SYNOTRAITS_H
#define SYNOTRAITS_H

#include <type_traits>
#include <utility>

#if __cplusplus < 201402L

namespace std
{

template< class T >
using add_const_t    = typename add_const<T>::type;

}

#endif

#if __cplusplus < 201703L

namespace std
{

template <class T>
constexpr add_const_t<T>& as_const(T& t) noexcept
{
    return t;
}

}
#endif

#endif // SYNOTRAITS_H
