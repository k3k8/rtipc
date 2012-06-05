/*****************************************************************************
 *
 *  $Id$
 *
 *  Copyright 2010 Richard Hacker (lerichi at gmx dot net)
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

#ifndef BB_CONFIGFILE_H
#define BB_CONFIGFILE_H

#include <stdexcept>
#include <string>
#include <map>
#include <yaml.h>

namespace BulletinBoard {

// struct ConfigEmpty: public std::runtime_error {
//     ConfigEmpty();
// };
//
// struct ConfigError: public std::runtime_error {
//     ConfigError(yaml_document_t *document, yaml_node_t *node,
//             const std::string& reason);
// };

class Config {
    public:
        class exception: public std::exception {
        };

        class Node {
            public:

                class Group {
                    public:
                        Group(const Node&);

                        Node operator[](const char *) const;

                    private:
                        typedef std::map<std::string, yaml_node_item_t> Map;
                        Map map;

                        yaml_document_t& document;
                };

                class List {
                    public:
                        List(const Node&);

                        Config::Node createNode();

                        Node operator*() const;
                        List& operator++();
                        operator bool() const;

                    private:
                        const Node& node;

                        yaml_node_item_t *it;
                };

                Node(yaml_document_t& doc, int nodeId);
                Node(const Node&);
                Node(Config&);

                operator int() const;
                operator unsigned int() const;
                operator double() const;
                operator std::string() const;

                Node createSequence();

            private:
                yaml_document_t& document;
                int nodeId;
                //yaml_node_t *node;
        };

        Config(const std::string& file = std::string());
        ~Config();

        int save(const std::string& file);

    protected:
        Config(yaml_document_t *document, yaml_node_t *node);

    private:
        yaml_document_t document;
};

};
#endif // BB_CONFIGFILE_H
