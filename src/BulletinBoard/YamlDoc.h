/*****************************************************************************
 *
 *  $Id$
 *
 *  Copyright 2012 Richard Hacker (lerichi at gmx dot net)
 *
 *  This file is part of the rtipc library.
 *
 *  The rtipc library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or (at
 *  your option) any later version.
 *
 *  The rtipc library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the rtipc library. If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#ifndef BB_YAML_H
#define BB_YAML_H

#include <yaml.h>
#include <string>
#include <map>
#include <sstream>

namespace YAML {

/////////////////////////////////////////////////////////////////////////////
class Node {
    public:
        enum Type {AUTO, Scalar, Sequence, Map};

        Node(const Node&);
        Node(yaml_document_t*, int nodeId);
        Type type() const;

        std::string toString() const;

        template<class T>
            operator T() const {
                std::istringstream is(this->toString());
                T val;
                is >> val;
                return val;
            };

    protected:
        int createScalar(const std::ostringstream& value, char quote = 0);
        template<class T>
            int createScalar(const T& value) {
                std::ostringstream v;
                v << value;
                return createScalar(v);
            }


        int createNode(Type type);

        yaml_document_t * const doc;
        int const nodeId;
};

/////////////////////////////////////////////////////////////////////////////
class Sequence: public Node {
    public:
        class Iterator {
            public:
                Iterator(const Iterator& other);
                Iterator(const Sequence& seq);

                Node operator*() const;
                void operator++();

                operator bool() const;

            private:
                const Sequence& seq;
                yaml_node_item_t *it;
        };

        Sequence(const Node&);

        Node operator[] (size_t) const;
        size_t size() const;

        Node appendNode(Node::Type type);

        void append(const std::ostringstream& value);
        template<class T>
            void append(const T& value) {
                std::ostringstream v;
                v << value;
                append(v);
            }

    private:
};

/////////////////////////////////////////////////////////////////////////////
class Map: public Node {
    public:
        Map(const Node&);

        Node operator[] (const std::string& key);

        Node appendNode(const std::string& key, Node::Type type);

        void append(const std::string& key,
                const std::ostringstream& value, char quote = 0);
        template<class T>
            void append(const std::string& key,
                    const T& value, char quote = 0) {
                std::ostringstream v;
                v << value;
                append(key, v, quote);
            }

    private:
        std::map<std::string, int> nodeMap;
};

/////////////////////////////////////////////////////////////////////////////
class Doc: public Node {
    public:
        Doc(Node::Type node = Node::AUTO);
        Doc(const std::string& file);
        ~Doc();

        int save(const std::string& file) const;

    private:
        yaml_document_t document;

        static int init(yaml_document_t *, Node::Type type);
        static int load(yaml_document_t *, const std::string& file);
};

}

#endif // BB_YAML_H
