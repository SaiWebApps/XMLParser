#include "../xmlParser.h"

using namespace std;

int main(int argc, char *argv[])
{
	if (argc < 2) {
		cout << "Usage: " << argv[0] << " [INPUT-XML-FILES]" << endl;
		return -1;
	}

	XMLParser parser;
	for (int i = 1; i < argc; i++) {
		if (!parser.parseFile(argv[i])) {
			cout << "File " << argv[i] << " does not exist." << endl;
		}
	}

	set<string> tags = parser.getAllTags();
	for (set<string>::iterator it = tags.begin(); it != tags.end(); it++) {
		cout << *it << "|";
		set<string> childValues = parser.getValues(*it);
		int childValuesSizeMinus1 = childValues.size() - 1;
		int count = 0;

		for (set<string>::iterator cit = childValues.begin(); cit != childValues.end(); cit++) {
			cout << *cit;
			if (count < childValuesSizeMinus1) {
				cout << "|";
			}
			count++;
		}
		cout << endl;
	}
	return 0;
}

