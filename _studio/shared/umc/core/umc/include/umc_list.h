/* ****************************************************************************** *\

Copyright (C) 2002-2013 Intel Corporation.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
- Neither the name of Intel Corporation nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL INTEL CORPORATION BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

File Name: umc_list.h

\* ****************************************************************************** */

#ifndef __UMC_LIST_H__
#define __UMC_LIST_H__

#if defined(_WIN32) || defined(_WIN64)

namespace UMC
{

template<class T> class List
{
protected:
    class Node
    {
    public:
        Node()
        {
            m_value = 0;
            m_prev  = this;
            m_next  = this;
        }

        Node(Node *prev, Node *next)
        : m_prev(prev), m_next(next)
        {
            m_value  = new T;
        }

        ~Node() {  if(m_value) delete m_value; }

        Node *m_prev;
        Node *m_next;
        T    *m_value;
    private:
        Node(const Node&) {}
        const Node& operator=(const Node&) { return *this; }
    };

public:
    class Iterator
    {
        friend class List;

    public:
        Iterator()                     : m_curr(0)           {}
        Iterator(const Node *node)     : m_curr(node)        {}
        Iterator(const Iterator &iter) : m_curr(iter.m_curr) {}

        Iterator& operator=(const Iterator &iter)
        {
            if(this != &iter)
            {
                m_curr = iter.m_curr;
            }
            return *this;
        }

        operator T*()   const { return (m_curr)?m_curr->m_value:0; }
        T* operator->() const { return (&**this);       }


        Iterator& operator++()
        {
            m_curr = m_curr->m_next;
            return *this;
        }

        Iterator& operator--()
        {
            m_curr = m_curr->m_prev;
            return *this;
        }

        bool operator == (const Iterator& iter) const
        {
            return m_curr == iter.m_curr;
        }

        bool operator != (const Iterator& iter) const
        {
            return !(*this == iter);
        }

    protected:
        const Node *m_curr;
    };


    List() {}

    ~List() { Clear(); }

    void Clear () { while(!IsEmpty()) PopBack(); }

    bool IsEmpty() { return m_keystone.m_next == &m_keystone; }

    void PushBack()
    {
        if(IsEmpty())
        {
            m_keystone.m_prev = new Node(&m_keystone, &m_keystone);
            m_keystone.m_next = m_keystone.m_prev;
        }
        else
        {
            Node* last   = m_keystone.m_prev;
            m_keystone.m_prev = new Node(m_keystone.m_prev, &m_keystone);
            last->m_next =  m_keystone.m_prev;
        }
    }

    void PopBack()
    {
        Node *last = m_keystone.m_prev->m_prev;
        delete m_keystone.m_prev;
        m_keystone.m_prev = last;
        last->m_next = &m_keystone;
    }

    void PopFront()
    {
        Node *first = m_keystone.m_next->m_next;
        delete m_keystone.m_next;
        m_keystone.m_next = first;
        first->m_prev = &m_keystone;
    }

    void Remove(Iterator *iter)
    {
        Node *node = (Node*)iter->m_curr;
        node->m_prev->m_next = node->m_next;
        node->m_next->m_prev = node->m_prev;
        iter->m_curr = node->m_prev;
        delete node;
    }

    Iterator ItrFront()      const { return Iterator(m_keystone.m_next); }
    Iterator ItrBack ()      const { return Iterator(m_keystone.m_prev); }

    Iterator ItrBackBound () const { return Iterator(&m_keystone); }
    Iterator ItrFrontBound() const { return Iterator(&m_keystone); }

    const T& Front() const { return *ItrFront(); }
    T&       Front()       { return *ItrFront(); }

    const T& Back () const { return *ItrBack (); }
    T&       Back ()       { return *ItrBack (); }

protected:
    Node m_keystone;
};

}
#endif
#endif
