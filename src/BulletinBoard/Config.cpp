/*****************************************************************************
 *
 *  $Id$
 *
 *  Copyright 2010 Richard Hacker (lerichi at gmx dot net)
 *
 *  This file is part of the pdserv library.
 *
 *  The pdserv library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or (at
 *  your option) any later version.
 *
 *  The pdserv library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the pdserv library. If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#include <cstring>
#include <cstdio>
#include <cerrno>

#include "Config.h"

#include <yaml.h>
using namespace BulletinBoard;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// static char error[100];

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Config::Config (const std::string& file)
{
    yaml_parser_t parser;
    FILE *fh;

    /* Initialize parser */
    if(!yaml_parser_initialize(&parser)) {
        //::snprintf(error, sizeof(error), "Could not initialize YAML parser");
        //return error;
    }

    fh = ::fopen(file.c_str(), "r");
    if (!fh) {
        //::snprintf(error, sizeof(error), "Could not open config file %s: %s",
                //file, strerror(errno));
        //return error;
    }

    /* Set input file */
    yaml_parser_set_input_file(&parser, fh);

    /* START new code */
    if (!yaml_parser_load(&parser, &document)) {
        //snprintf(error, sizeof(error), "YAML parser failure at line %zu: %s",
            //parser.problem_mark.line, parser.problem);
        //return error;
    }

    // Now finished with the file
    ::fclose(fh);

    //node = yaml_document_get_root_node(&document);

    //return 0;
}

/////////////////////////////////////////////////////////////////////////////
Config::~Config ()
{
    yaml_document_delete(&document);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Config::Node::Group::Group(const Config::Node& other): document (other.document)
{
    if (!other.node or other.node->type != YAML_MAPPING_NODE) {
        throw exception();
    }

    for (yaml_node_pair_t *pair = other.node->data.mapping.pairs.start;
            pair != other.node->data.mapping.pairs.top; ++pair) {
        yaml_node_t *n = yaml_document_get_node(&document, pair->key);
        if (n->type == YAML_SCALAR_NODE) {
            std::string key = std::string((const char *)n->data.scalar.value,
                    n->data.scalar.length);
            map[key] = pair->value;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
Config::Node Config::Node::Group::operator[] (const char *key) const
{
    Map::const_iterator it = map.find(key);
    if (it == map.end())
        throw exception();

    return Config::Node(document,
            yaml_document_get_node(&document, it->second));
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Config::Node::List::List(const Config::Node& other): node (other)
{
    if (!other.node or other.node->type != YAML_SEQUENCE_NODE) {
        throw exception();
    }

    it = node.node->data.sequence.items.start;
}

/////////////////////////////////////////////////////////////////////////////
Config::Node Config::Node::List::operator* () const
{
    return Config::Node(node.document,
            yaml_document_get_node(&node.document, *it));
}

/////////////////////////////////////////////////////////////////////////////
Config::Node::List& Config::Node::List::operator++ ()
{
    it++;
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
Config::Node::List::operator bool () const
{
    return it != node.node->data.sequence.items.top;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Config::Node::Node (yaml_document_t& doc, yaml_node_t *node):
    document(doc), node(node)
{
}

/////////////////////////////////////////////////////////////////////////////
Config::Node::Node (const Config::Node& other):
    document(other.document), node(other.node)
{
}

/////////////////////////////////////////////////////////////////////////////
Config::Node::Node (Config& other):
    document(other.document),
    node(yaml_document_get_root_node(&document))
{
}

/////////////////////////////////////////////////////////////////////////////
Config::Node::operator std::string () const
{
    if (node->type != YAML_SCALAR_NODE)
        throw exception();

    return std::string((char*)node->data.scalar.value,
            node->data.scalar.length);
}

// /////////////////////////////////////////////////////////////////////////////
// namespace RtIPC {
//     template <typename T>
//         bool Config::get(T &value) const
//         {
//             if (!*this)
//                 return false;
// 
//             value = *this;
//             return true;
//         }
// 
//     template bool Config::get(int& value) const;
// }

/////////////////////////////////////////////////////////////////////////////
Config::Node::operator double () const
{
    std::string sval(*this);
    if (sval.empty())
        throw exception();

    return strtod(sval.c_str(), 0);
}

/////////////////////////////////////////////////////////////////////////////
Config::Node::operator int () const
{
    std::string sval(*this);
    if (sval.empty())
        throw exception();

    return strtol(sval.c_str(), 0, 0);
}

/////////////////////////////////////////////////////////////////////////////
Config::Node::operator unsigned int () const
{
    std::string sval(*this);
    if (sval.empty())
        throw exception();

    return strtoul(sval.c_str(), 0, 0);
}
