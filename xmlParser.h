#ifndef XML_PARSER_H
#define XML_PARSER_H

#include <set>

#include "Trie/trieNode.h"

class XMLParser
{
private:
    TrieNode<std::string> *xmlTrie;

public:
    XMLParser();

	// Parsing XML text/files
    void parseText(const std::string &text);
    bool parseFile(const std::string &filename);

	// Retrieving info from XML Trie
	std::set<std::string> getAllTags();
	std::set<std::string> getValues(const std::string &tag);
	std::set<std::string> getValues(const std::vector<std::string> &tags);

	// Display
	friend std::ostream &operator<<(std::ostream &outputStream, XMLParser &parser);

	~XMLParser();
};

#endif // XML_PARSER_H

