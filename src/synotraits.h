#ifndef SYNOTRAITS_H
#define SYNOTRAITS_H

#include <functional>
#include <type_traits>
#include <utility>

#include <QDebug>
#include <QJSValue>
#include <QList>
#include <QString>

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


/*!
 *  \brief This function executes JS callback with provided arguments
 *
 *  \returns True on successful execution, false otherwise
 */
template <typename... Args>
bool executeJSCallback(QJSValue callback, Args... args) {
    if (!callback.isCallable()) {
        qWarning() << __FUNCTION__ << QStringLiteral("JS callback is not callable.");
        return false;
    }

    QJSValue result = callback.call(QList<QJSValue>{ std::forward<Args>(args)... });
    if (result.isError()) {
        qWarning() << __FUNCTION__
                   << QStringLiteral("Exception on JS callback. ")
                   << result.property(QStringLiteral("fileName")).toString()
                   << QStringLiteral(":")
                   << result.property(QStringLiteral("lineNumber")).toInt()
                   << result.property(QStringLiteral("message")).toString()
                   << QStringLiteral("Stack: [")
                   << result.property(QStringLiteral("stack")).toString()
                   << QStringLiteral("] ")
                   << result.toString();
        return false;
    }

    return true;
}

#endif // SYNOTRAITS_H
