#include <fstream>

#include "xmlParser.h"

using namespace std;

/**
 * Initialize this XMLParser's XML Trie.
 */
XMLParser::XMLParser() : xmlTrie(new TrieNode<string>()) {}

/**
 * Return a version of input without leading/trailing whitespaces.
 */
static string trim(const string &input)
{
	string line = input;

    size_t idx = line.find_first_not_of(" \t\n");
    if (idx != string::npos) {
        line = line.erase(0, idx);
    }
    idx = line.find_last_not_of(" \t\n");
    if (idx != string::npos) {
        line = line.erase(idx + 1);
    }

	return line;
}

/**
 * Helper method to handle the occurrence of a start XML tag on the specified line.
 * EXAMPLE OF START TAG: <xml>
 */
static void handleStartTag(TrieNode<string> **tn, string &line)
{
    // line[0] = <, so find loc where startTag ends (first occurrence of >)
    size_t endOfStartTagIndex = line.find(">");
    if (endOfStartTagIndex == string::npos) {
        return;
    }

    // Extract the start tag from line.
    string startTag = line.substr(0, endOfStartTagIndex + 1);
    trim(startTag);
    **tn << startTag; // Add tag to XML Trie.
	// Move pointer to startTag node. Add children to this startTag node
	// (nested XML tags + content) until we encounter the corresponding
	// endTag. At that point, we will retreat back to this node's parent.
    *tn = (**tn)[startTag];

    // Done handling start tag, so look at string past end of start tag
    line = line.substr(endOfStartTagIndex + 1);
}

/**
 * Helper method to handle the occurrence of an end XML tag on the specified line.
 * EXAMPLE OF END TAG: </xml>
 */
static void handleEndTag(TrieNode<string> **tn, string &line)
{
    // line[0] = <, line[1] = /
    // Find the end of the end tag.
    size_t endOfEndTagIndex = line.find(">");
    if (endOfEndTagIndex == string::npos) {
        return;
    }
    
    // Since this tag just ended, keep moving up the Trie until the current
    // node contains the start tag value corresponding to this end tag.
    string tag = line.substr(0, endOfEndTagIndex + 1);
    tag = tag.erase(1,1); // Delete '/'.
    while ((*tn)->getValue() != tag && (*tn)->getParent() != NULL) {
        *tn = (*tn)->getParent();
    }
    // Move up 1 more time since no more values will be added under this tag.
    *tn = (*tn)->getParent();
    
	// Update line - start reading after the end tag.
    line = line.substr(endOfEndTagIndex + 1);
}

/**
 * Helper method to handle the occurrence of content (non-tag value inside XML tag)
 * on the given line.
 */
static void handleContent(TrieNode<string> **tn, string &line)
{
    // Find out where the next tag (start or end) begins after this content.
    size_t nextTagIndex = line.find("<");
	// If there is no tag after this content, then we are done processing this
	// line afterwards.
	if (nextTagIndex == string::npos) {
		nextTagIndex = line.size();
	}

    // Get the content. Add to XML Trie.
    string content = line.substr(0, nextTagIndex);
    content = trim(content);
    **tn << content;
	// As with start tag, move to new node. We will want to add to this node
	// until we encounter an end tag.
    *tn = (**tn)[content];

   	line = line.substr(nextTagIndex);
}

/**
 * Parse the given line, and store the data in the provided XML Trie.
 */
static void processLine(TrieNode<string> **triePtr, string &line)
{
    while (!line.empty()) {
		line = trim(line);
        char current = line[0];

        switch(current) {
		// If tag
        case '<':
			// If end tag
            if (line[1] == '/') {
                handleEndTag(triePtr, line);
                break;
            }
			// Otherwise, if start tag
            handleStartTag(triePtr, line);
            break;

		// If content (not a tag)
        default:
            handleContent(triePtr, line);
        }
    }
}

/**
 * Parse the given XML text, and build a XML Trie using the information.
 */
void XMLParser::parseText(const string &text)
{
    stringstream textStream(text.c_str());
    string line;
    TrieNode<string> *triePtr = xmlTrie;

	// Parse 1 line at a time.
    while (getline(textStream, line, '\n')) {
        processLine(&triePtr, line);
    }
}

/**
 * Read the contents of the specified XML file into a string,
 * and then pass the string to parseText to build the XML Trie.
 * Return true if the file exists and was parsed successfully,
 * false otherwise.
 */
bool XMLParser::parseFile(const string &filename)
{
	// Build string with file's contents.
	// Return false if the file does not exist.
    ifstream inputFileStream(filename.c_str());
	if (!inputFileStream.good()) {
		return false;
	}
    string xml, line;
    while (getline(inputFileStream, line)) {
        xml += line + "\n";
    }
    inputFileStream.close();

	// Pass string to parseText method.
    parseText(xml);
	return true;
}

static void getTags(TrieNode<string> *tn, set<string> &results)
{
	string tv = tn->getValue();
	size_t tvSizeMinus1 = tv.size() - 1;
	if (!tv.empty() && tv[0] == '<' && tv[tvSizeMinus1] == '>') {
		results.insert(tv);
	}

	for (int i = 0; i < tn->getNumChildren(); i++) {
		getTags(tn->getChildAtIndex(i), results);
	}
}

set<string> XMLParser::getAllTags()
{
	set<string> tags;
	getTags(xmlTrie, tags);
	return tags;
}

/**
 * Return a set with the values of the specified TrieNode's children.
 * Note that an empty set would indicate that this TrieNode is a leaf.
 * Also, this function returns a set since the child values of a given
 * TrieNode must be unique (no duplicate children).
 */
static set<string> getChildValues(TrieNode<string> *tn)
{
	set<string> childValues;
	
	for (int i = 0; i < tn->getNumChildren(); i++) {
		childValues.insert(tn->getChildAtIndex(i)->getValue());
	}
	return childValues;
}

/**
 * Populates "results" with all TrieNodes in "trie" that have value "value."
 * Does nothing if "trie" is NULL; results is empty if there are no matches.
 */
static void findAllMatches(vector<TrieNode<string> *> &results,
						TrieNode<string> *trie, const string &value)
{
	if (trie == NULL) {
		return;
	}
	if (trie->getValue() == value) {
		results.push_back(trie);
	}
	for (int i = 0; i < trie->getNumChildren(); i++) {
		TrieNode<string> *child = trie->getChildAtIndex(i);
		findAllMatches(results, child, value);
	}
}

/**
 * Return the set of child-values for all instances of the tag "tag."
 * Example:
 *		{<store> | <groceries>, <tech>, <outdoors>}
 *		{<groceries> | <item>}, where {<item> | X}
 *		{<tech> | <item>}, where {<item> | Y}
 *		{<outdoors> | <item}, where {<item> | Z}
 * 
 * getValues("<item>") = [X, Y, Z]
 */
set<string> XMLParser::getValues(const string &tag)
{
	set<string> results;
	vector<TrieNode<string> *> matches;

	// Fill matches with all TrieNodes in "xmlTrie" that have value "tag."
	findAllMatches(matches, xmlTrie, tag);
	// For each matching TrieNode, get the child values, and insert them into
	// the set "results."
	for (size_t i = 0; i < matches.size(); i++) {
		TrieNode<string> *tn = matches[i];
		set<string> childValues = getChildValues(tn);
		results.insert(childValues.begin(), childValues.end());
	}
	return results;
}

/**
 * Return a vector with all TrieNodes in "matches" that have a child 
 * with "childValue." All filtered TrieNodes will point to the matching
 * child node.
 * Note that if the result is empty, then there were no TrieNodes in "matches,"
 * that contained a child with value "childValue."
 */
static vector<TrieNode<string> *> filterAndReorient(
		const vector<TrieNode<string> *> &matches, const string &childValue)
{
	vector<TrieNode<string> *> filteredMatches;

	for (size_t i = 0; i < matches.size(); i++) {
		TrieNode<string> *match = matches[i];
		// If the current node has a child with "childValue," then
		// push the corresponding child node into filteredMatches.
		if (match->hasChild(childValue)) {
			filteredMatches.push_back((*match)[childValue]);
		}
	}
	return filteredMatches;
}

/**
 * Return the set of child-values for the given tag-sequence "tags."
 * Example:
 *		{<store> | <groceries>, <tech>, <outdoors>}
 *		{<groceries> | <item>}, where {<item> | X}
 *		{<tech> | <item>}, where {<item> | Y}
 *		{<outdoors> | <item}, where {<item> | Z}
 * 
 * getValues(["<groceries>", "<item>"]} = [X]
 */
set<string> XMLParser::getValues(const vector<string> &tags)
{
	set<string> results;
	if (tags.empty()) {
		return results;
	}

	// Find all matching TrieNodes for the first tag.
	vector<TrieNode<string> *> matches;
	findAllMatches(matches, xmlTrie, tags[0]);
	if (matches.empty()) {
		return results;
	}	

	// Now, filter out all first-tag (tags[0]) matches that don't
	// have a child with value tags[1]. Repeat until we are only left
	// with matches where the tags are organized according to the
	// specified tag-sequence in "tags."
	for (size_t i = 1; i < tags.size(); i++) {
		matches = filterAndReorient(matches, tags[i]);
		if (matches.empty()) {
			return results;
		}
	}
	// For each the final set of matches, get child values, and add to result. 
	for (size_t i = 0; i < matches.size(); i++) {
		set<string> individualResults = getChildValues(matches[i]);
		results.insert(individualResults.begin(), individualResults.end());
	}
	return results;
}

ostream &operator<<(ostream &outputStream, XMLParser &parser)
{
	return outputStream << *(parser.xmlTrie);
}

XMLParser::~XMLParser()
{
	delete xmlTrie;
}

