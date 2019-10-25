/*
 * GNU General Public License (GPL)
 * Copyright (c) 2016 The Qt Company Ltd.
 * Copyright (c) 2019 by Aleksei Ilin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CACHE_H
#define CACHE_H

#include <QHash>
#include <utility>

/*!
 * \brief QCache version adapted to use default-constructed objects instead of pointers
 *
 * For objects supporting implicit sharing the usage of this container
 * has a performance advantage over QCache, as it does not require heap allocation.
 *
 * Additionally this cache version supports limit by maximum item count.
 * Item count is not limited in case of max count is set to 0.
 */

template <class Key, class T>
class Cache
{
    struct Node
    {
        inline Node() : keyPtr(0) {}
        inline Node(const T& data, int cost)
            : keyPtr(0), p(0), n(0), t(data), c(cost) {}

        const Key *keyPtr;
        Node *p;
        Node *n;
        T t;
        int c;
    };

    Node *f, *l;
    QHash<Key, Node> hash;
    int mx, mc, total;

    inline void unlink(Node &n)
    {
        if (n.p) n.p->n = n.n;
        if (n.n) n.n->p = n.p;
        if (l == &n) l = n.p;
        if (f == &n) f = n.n;
        total -= n.c;
        hash.remove(*n.keyPtr);
    }

    inline T relink(const Key& key, const T& def)
    {
        typename QHash<Key, Node>::iterator i = hash.find(key);
        if (typename QHash<Key, Node>::const_iterator(i) == hash.constEnd())
        {
            return def;
        }

        Node &n = *i;
        if (f != &n)
        {
            if (n.p) n.p->n = n.n;
            if (n.n) n.n->p = n.p;
            if (l == &n) l = n.p;
            n.p = 0;
            n.n = f;
            f->p = &n;
            f = &n;
        }
        return n.t;
    }

public:
    // STL compatibility
    typedef T mapped_type;
    typedef Key key_type;
    typedef int size_type;

public:
    inline Cache();
    inline Cache(int maxCost, int maxCount = 0);
    inline Cache(const Cache &o);
    inline Cache(Cache &&o) = default;
    inline ~Cache() { clear(); }

    inline int maxCost() const { return mx; }
    void setMaxCost(int m);
    inline int maxCount() const { return mc; }
    void setMaxCount(int m);
    inline int totalCost() const { return total; }

    inline int size() const { return hash.size(); }
    inline int count() const { return hash.size(); }
    inline bool isEmpty() const { return hash.isEmpty(); }
    inline QList<Key> keys() const { return hash.keys(); }

    void clear();

    bool insert(const Key &key, const T& object, int cost);
    T object(const Key& key, const T& def = T());

    inline bool contains(const Key &key) const { return hash.contains(key); }

    T operator[](const Key &key);

    bool remove(const Key &key);
    T take(const Key &key);

    inline Cache& operator=(const Cache &o);
    inline Cache& operator=(Cache &&o) = default;

private:
    void trimCost(int m);
    void trimCount(int m);
};

template <class Key, class T>
inline Cache<Key, T>::Cache()
    : f(0)
    , l(0)
    , mx(0)
    , mc(0)
    , total(0)
{
}

template <class Key, class T>
inline Cache<Key, T>::Cache(int amaxCost, int amaxCount)
    : f(0)
    , l(0)
    , total(0)
{
    setMaxCount(amaxCount);
    setMaxCost(amaxCost);
}

template <class Key, class T>
inline Cache<Key, T>::Cache(const Cache &o)
{
    *this = o;
}

template <class Key, class T>
inline void Cache<Key,T>::clear()
{
    hash.clear();
    f = 0;
    l = 0;
    total = 0;
}

template <class Key, class T>
inline void Cache<Key,T>::setMaxCost(int m)
{
    mx = m;
    trimCost(mx);
}

template <class Key, class T>
inline void Cache<Key,T>::setMaxCount(int m)
{
    Q_ASSERT(m >= 0);

    mc = m;
    if (m > 0) {
        trimCount(m);
        hash.reserve(m);
    }
}

template <class Key, class T>
inline T Cache<Key, T>::object(const Key &key, const T& def)
{ return const_cast<Cache<Key,T>*>(this)->relink(key, def); }

template <class Key, class T>
inline T Cache<Key,T>::operator[](const Key &key)
{ return object(key); }

template <class Key, class T>
inline bool Cache<Key,T>::remove(const Key &key)
{
    typename QHash<Key, Node>::iterator i = hash.find(key);
    if (typename QHash<Key, Node>::const_iterator(i) == hash.constEnd()) {
        return false;
    } else {
        unlink(*i);
        return true;
    }
}

template <class Key, class T>
inline T Cache<Key,T>::take(const Key &key)
{
    typename QHash<Key, Node>::iterator i = hash.find(key);
    if (i == hash.end())
    {
        return T();
    }

    Node &n = *i;
    T t(std::move(n.t));
    unlink(n);
    return std::move(t);
}

template <class Key, class T>
inline Cache<Key,T>& Cache<Key,T>::operator=(const Cache<Key,T> &o)
{
    clear();
    setMaxCount(o.maxCount());
    setMaxCost(o.maxCost());

    Node *on = o.l;
    while (on) {
        insert(*on->keyPtr, on->t, on->c);
        on = on->p;
    }

    return *this;
}

template <class Key, class T>
inline bool Cache<Key,T>::insert(const Key &akey, const T& aobject, int acost)
{
    Q_ASSERT(acost > 0);

    remove(akey);
    if (acost > mx)
    {
        return false;
    }

    trimCost(mx - acost);

    if (mc > 0) {
        trimCount(maxCount() - 1);
    }

    Node sn(aobject, acost);
    typename QHash<Key, Node>::iterator i = hash.insert(akey, sn);
    total += acost;
    Node *n = &i.value();
    n->keyPtr = &i.key();
    if (f) f->p = n;
    n->n = f;
    f = n;
    if (!l) l = f;
    return true;
}

template <class Key, class T>
inline void Cache<Key,T>::trimCost(int m)
{
    Node *n = l;
    while (n && total > m) {
        Node *u = n;
        n = n->p;
        unlink(*u);
    }
}

template <class Key, class T>
inline void Cache<Key,T>::trimCount(int m)
{
    Node *n = l;
    while (n && count() > m) {
        Node *u = n;
        n = n->p;
        unlink(*u);
    }
}

#endif // CACHE_H
