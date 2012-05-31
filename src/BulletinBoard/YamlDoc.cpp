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

#include "../Debug.h"
#include "YamlDoc.h"

#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <cerrno>

using namespace YAML;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Doc::Doc (Node::Type type): Node(&document, init(&document, type))
{
}

/////////////////////////////////////////////////////////////////////////////
Doc::Doc (const std::string& file): Node(&document, load(&document, file))
{
}

/////////////////////////////////////////////////////////////////////////////
Doc::~Doc ()
{
    yaml_document_delete(&document);
}

/////////////////////////////////////////////////////////////////////////////
int Doc::init(yaml_document_t *doc, Node::Type type)
{
    ::memset(doc, 0, sizeof(*doc));
    if (!yaml_document_initialize(doc, NULL, NULL, NULL, 0, 1))
        throw std::runtime_error("Failed to initialize YAML document");

    int nodeId = 0;

    switch (type) {
        case Node::Sequence:
            nodeId = yaml_document_add_sequence(doc,
                    NULL, YAML_BLOCK_SEQUENCE_STYLE);
            break;

        case Node::Map:
            nodeId = yaml_document_add_mapping(doc,
                    NULL, YAML_BLOCK_MAPPING_STYLE);
            break;

        default:
            throw std::invalid_argument(
                    "Invalid type to initialize YAML document");
    }

    return nodeId;
}

/////////////////////////////////////////////////////////////////////////////
int Doc::load(yaml_document_t* doc, const std::string& file)
{
    ::memset(doc, 0, sizeof(*doc));
    if (!yaml_document_initialize(doc, NULL, NULL, NULL, 0, 1))
        throw std::runtime_error("Failed to initialize YAML document");

    FILE *fh = ::fopen(file.c_str(), "r");
    if (!fh)
        throw std::runtime_error("Could not open config file");

    /* Initialize parser */
    yaml_parser_t parser;
    if(!yaml_parser_initialize(&parser))
        throw std::runtime_error(parser.problem);

    /* Set input file */
    yaml_parser_set_input_file(&parser, fh);

    /* START new code */
    if (!yaml_parser_load(&parser, doc))
        throw std::runtime_error(parser.problem);

    // Now finished with the file
    ::fclose(fh);

    if (!yaml_document_get_root_node(doc))
        throw std::runtime_error("Document empty");

    return 1;
}

/////////////////////////////////////////////////////////////////////////////
int Doc::save(const std::string& file) const
{
    FILE *f = fopen(file.c_str(), "w");
    if (!f)
        throw std::runtime_error(strerror(errno));

    yaml_emitter_t emitter;

    ::memset(&emitter, 0, sizeof(emitter));
    
    if (!yaml_emitter_initialize(&emitter))
        goto emitter_error;
    yaml_emitter_set_output_file(&emitter, f);
    yaml_emitter_set_canonical(&emitter, 0);
    yaml_emitter_set_unicode(&emitter, 1);
    
    if (!yaml_emitter_open(&emitter))
        goto emitter_error;
    
    if (!yaml_emitter_dump(&emitter, doc))
        goto emitter_error;
    if (!yaml_emitter_close(&emitter))
        goto emitter_error;
    
    yaml_emitter_delete(&emitter);
    fclose(f);

    return 0;

emitter_error:

    fclose(f);

    throw std::runtime_error(emitter.problem);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Map::Map (const Node& other): Node(other)
{
    if (type() != Node::Map) {
        throw std::invalid_argument("Node is not a mapping");
    }

    yaml_node_t *node = yaml_document_get_node(doc, nodeId);
    for (yaml_node_pair_t *pair = node->data.mapping.pairs.start;
            pair != node->data.mapping.pairs.top; ++pair) {
        Node n(doc, pair->key);
        if (n.type() != Scalar)
            continue;

        nodeMap[Node(doc, pair->key)] = pair->value;
    }
}

/////////////////////////////////////////////////////////////////////////////
Node Map::appendNode (const std::string& key, Node::Type type)
{
    int keyId = createScalar(key);
    int valueId = createNode(type);

    yaml_document_append_mapping_pair(doc, nodeId, keyId, valueId);
    nodeMap[key] = valueId;

    return Node(doc, valueId);
}

/////////////////////////////////////////////////////////////////////////////
void Map::append (const std::string& key,
        const std::ostringstream& value, char quote)
{
    int keyId = createScalar(key);
    int valueId = createScalar(value, quote);

    nodeMap[key] = valueId;

    yaml_document_append_mapping_pair(doc, nodeId, keyId, valueId);
}

/////////////////////////////////////////////////////////////////////////////
Node Map::operator[] (const std::string& key)
{
    return Node(doc, nodeMap[key]);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Sequence::Sequence (const Node& other): Node(other)
{
    if (type() != Node::Sequence) {
        throw std::invalid_argument("Node is not a sequence");
    }
}

/////////////////////////////////////////////////////////////////////////////
Sequence::Iterator::Iterator (const Iterator& other):
    seq(other.seq), it(other.it)
{
}

/////////////////////////////////////////////////////////////////////////////
Sequence::Iterator::Iterator (const Sequence& other):
    seq(other)
{
    yaml_node_t *node = yaml_document_get_node(other.doc, other.nodeId);
    it = node->data.sequence.items.start;
}

/////////////////////////////////////////////////////////////////////////////
Node Sequence::Iterator::operator* () const
{
    return Node(seq.doc, *it);
}

/////////////////////////////////////////////////////////////////////////////
void Sequence::Iterator::operator++ ()
{
    ++it;
}

/////////////////////////////////////////////////////////////////////////////
Sequence::Iterator::operator bool () const
{
    yaml_node_t *node = yaml_document_get_node(seq.doc, seq.nodeId);
    return it != node->data.sequence.items.top;
}

/////////////////////////////////////////////////////////////////////////////
size_t Sequence::size () const
{
    yaml_node_t *node = yaml_document_get_node(doc, nodeId);

    return node->data.sequence.items.top - node->data.sequence.items.start;
}

/////////////////////////////////////////////////////////////////////////////
Node Sequence::operator[] (size_t n) const
{
    yaml_node_t *node = yaml_document_get_node(doc, nodeId);

    return Node(doc, node->data.sequence.items.start[n]);
}

/////////////////////////////////////////////////////////////////////////////
void Sequence::append (const std::ostringstream& value)
{
    int id = createScalar(value);

    yaml_document_append_sequence_item(doc, nodeId, id);
}

/////////////////////////////////////////////////////////////////////////////
Node Sequence::appendNode(Node::Type type)
{
    int newNode = Node::createNode(type);
    yaml_document_append_sequence_item(doc, nodeId, newNode);

    return Node(doc, newNode);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Node::Node (yaml_document_t* doc, int nodeId):
    doc(doc), nodeId(nodeId)
{
    if (!nodeId)
        throw std::runtime_error("Invalid node id 0");
}

/////////////////////////////////////////////////////////////////////////////
Node::Node (const Node& other):
    doc(other.doc), nodeId(other.nodeId)
{
}

/////////////////////////////////////////////////////////////////////////////
int Node::createScalar(const std::ostringstream& value, char quote)
{
    std::string s = value.str();
    yaml_scalar_style_t scalar_style;

    switch (quote) {
        case '\'':
            scalar_style = YAML_SINGLE_QUOTED_SCALAR_STYLE;
            break;

        case '"':
            scalar_style = YAML_DOUBLE_QUOTED_SCALAR_STYLE;
            break;

        default:
            scalar_style = YAML_PLAIN_SCALAR_STYLE;
    }

    return yaml_document_add_scalar(doc, NULL,
            (yaml_char_t*)s.c_str(), s.size(), scalar_style);
}

/////////////////////////////////////////////////////////////////////////////
int Node::createNode(Node::Type type)
{
    int nodeId = 0;

    switch (type) {
        case Sequence:
            nodeId = yaml_document_add_sequence(doc,
                    NULL, YAML_BLOCK_SEQUENCE_STYLE);
            break;

        case Map:
            nodeId = yaml_document_add_mapping(doc,
                    NULL, YAML_BLOCK_MAPPING_STYLE);
            break;

        default:
            break;
    }

    return nodeId;
}

/////////////////////////////////////////////////////////////////////////////
Node::Type Node::type() const
{
    switch (yaml_document_get_node(doc, nodeId)->type) {
        case YAML_SCALAR_NODE:
            return Scalar;

        case YAML_SEQUENCE_NODE:
            return Sequence;

        case YAML_MAPPING_NODE:
            return Map;

        default:
            return AUTO;
    }
}

/////////////////////////////////////////////////////////////////////////////
std::string Node::toString() const
{
    yaml_node_t *node = yaml_document_get_node(doc, nodeId);
    return std::string((const char*)node->data.scalar.value,
            node->data.scalar.length);
}
