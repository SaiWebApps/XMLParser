#!/usr/bin/python

import sys
import subprocess
import xml.dom.minidom

def _runCppParser(input_xml_file):
	'''
		Compile and execute the XMLParser C++ project, using
		"input_xml_file" as an argument.
	'''
	subprocess.call(['make'])
	output = subprocess.check_output(['./xmlParser.out', input_xml_file])
	subprocess.call(['make', 'clean'])
	return output


def getCppOutput(input_xml_file):
	'''
		Return a dict where the keys are the XML tags and values
		are each tag's corresponding list of values.
		Use the C++ parser's output to build this dict for the
		"input_xml_file."
	'''
	output = _runCppParser(input_xml_file)
	lines = output.split('\n')
	tagsToValues = {}

	for line in lines:
		tokens = line.split('|')
		tag = tokens[0].strip('<').strip('>')
		if len(tag) == 0:
			continue
		tagsToValues[tag] = tokens[1:]

	return tagsToValues


def getPythonOutput(input_xml_file, cppMap):
	domTree = xml.dom.minidom.parse(input_xml_file)
	xmlDoc = domTree.documentElement
	pythonTagMap = {}

	for xmlTag in cppMap:
		pythonXMLElems = xmlDoc.getElementsByTagName(xmlTag)
		if xmlTag not in pythonTagMap:
			pythonTagMap[xmlTag] = []
		for elem in pythonXMLElems:
			pythonTagMap[xmlTag].append(elem.childNodes[0].data)

	return pythonTagMap


def verify(pythonMap, cppMap):
	'''
		Return True if the Python and C++ XML Parsers yielded
		the same results (stored in pythonMap and cppMap resp.),
		False otherwise.
	'''
	for tag in pythonMap:
		# Make sure that all tags detected by the Python parser
		# were also detected by the C++ parser.
		if tag not in cppMap:
			return False
	
		# Now, verify that each tag corresponds to the same set
		# of values under both the C++ and Python parsers.	
		actualValues = cppMap[tag]
		correctValues = pythonMap[tag]
		for value in correctValues:
			# Python parser does not return any values for tags
			# that have only other tags as values. C++ does, so
			# ignore those.
			if len(value.strip()) == 0:
				continue
			if value not in actualValues:
				return False

	return True


def main(argv):
	if len(argv) == 1:
		print('Usage: ' + argv[0] + ' [INPUT-XML-FILES]')
		return

	cppTagMap = getCppOutput(argv[1])
	pythonTagMap = getPythonOutput(argv[1], cppTagMap)
	if verify(pythonTagMap, cppTagMap):
		print('Parser Works!')
	else:
		print('Parser Does Not Work!')

if __name__ == "__main__":
	main(sys.argv)
