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

#include <cstring>
#include <cstdio>
#include <cerrno>

#include "Config.h"
#include "../Debug.h"

#include <yaml.h>
using namespace BulletinBoard;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// static char error[100];

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Config::Config (const std::string& file)
{
    if (file.empty()) {
        if (!yaml_document_initialize(&document, NULL, NULL, NULL, 0, 1));
        return;
    }

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
}

/////////////////////////////////////////////////////////////////////////////
Config::~Config ()
{
    yaml_document_delete(&document);
}

/////////////////////////////////////////////////////////////////////////////
int Config::save(const std::string& file)
{
     return 0;

     FILE *f = fopen(file.c_str(), "w");
     yaml_emitter_t emitter;

     if (!yaml_emitter_initialize(&emitter))
         goto emitter_error;
     yaml_emitter_set_output_file(&emitter, f);
     yaml_emitter_set_canonical(&emitter, 0);
     yaml_emitter_set_unicode(&emitter, 1);
 
     if (!yaml_emitter_open(&emitter))
         goto emitter_error;
 
     if (!yaml_emitter_dump(&emitter, &document))
         goto emitter_error;
     if (!yaml_emitter_close(&emitter))
         goto emitter_error;
 
     yaml_emitter_delete(&emitter);
 
     fclose(f);

     return 0;

emitter_error:
     return -1;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Config::Node::Group::Group(const Config::Node& other): document (other.document)
{
    yaml_node_t *node = yaml_document_get_node(&document, other.nodeId);

    if (!node or node->type != YAML_MAPPING_NODE) {
        throw exception();
    }

    for (yaml_node_pair_t *pair = node->data.mapping.pairs.start;
            pair != node->data.mapping.pairs.top; ++pair) {
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

    return Config::Node(document, it->second);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Config::Node::List::List(const Config::Node& other): node (other)
{
    yaml_node_t *node = yaml_document_get_node(&other.document, other.nodeId);

    if (!node or node->type != YAML_SEQUENCE_NODE) {
        throw exception();
    }

    it = node->data.sequence.items.start;
}

// /////////////////////////////////////////////////////////////////////////////
// Config::Node Config::Node::List::createNode()
// {
//     return Config::Node(*this);
// }

/////////////////////////////////////////////////////////////////////////////
Config::Node Config::Node::List::operator* () const
{
    return Config::Node(node.document, *it);
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
    yaml_node_t *n = yaml_document_get_node(&node.document, node.nodeId);
    return it != n->data.sequence.items.top;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Config::Node::Node (yaml_document_t& doc, int nodeId):
    document(doc), nodeId(nodeId)
{
}

/////////////////////////////////////////////////////////////////////////////
Config::Node::Node (const Config::Node& other):
    document(other.document), nodeId(other.nodeId)
{
}

/////////////////////////////////////////////////////////////////////////////
Config::Node::Node (Config& other):
    document(other.document), nodeId(1)
{
    log_debug("%i", nodeId);
}

/////////////////////////////////////////////////////////////////////////////
Config::Node::operator std::string () const
{
    yaml_node_t *n = yaml_document_get_node(&document, nodeId);

    if (n->type != YAML_SCALAR_NODE)
        throw exception();

    return std::string((char*)n->data.scalar.value,
            n->data.scalar.length);
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

/////////////////////////////////////////////////////////////////////////////
Config::Node Config::Node::createSequence()
{
    int n = yaml_document_add_sequence( &document, NULL,
            YAML_BLOCK_SEQUENCE_STYLE);

    if (nodeId) {
        yaml_document_add_sequence(&);
    }

    //log_debug("%i %p", n, node);
    return Config::Node(document, n);
}
