using namespace std;
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <map>
#include <random>
#include <utility> 
#include <time.h> 
#include <chrono> 
#include<iomanip>
#include<limits>
#include<cstddef>
std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
class DataStorage
{
public:
	vector<string> attrName;
	vector<vector<string> > data;
	vector<vector<string> > attrValueList;
	void print();
	void getAttibuteValue();
};

class readFromFile
{
private:
	ifstream fin;
	DataStorage dataStorage;
public:
	readFromFile(string filename);
	void read();
	DataStorage getTable();
};

class Node
{
public:
	int bestAttrIndex;
	string attrValue;
	int treeIndex;
	bool isLeaf;
	string label;
	vector<int > children;
	Node();
};

class Tree {
public:
	DataStorage init;
	vector<Node> tree;

	Tree(DataStorage dataStorage);
	string predict(vector<string> row);
	int DFS(vector<string>& row, int currentNode);
	void generateDT(DataStorage dataStorage, int nodeIndex);
	pair<string, int> getDominateLabel(DataStorage dataStorage);
	bool isLeafNode(DataStorage dataStorage);
	int getSelectedAttribute(DataStorage dataStorage);
	double getGainAll(DataStorage dataStorage, int attrIndex);
	double getInfo(DataStorage dataStorage);
	double getInfoAttr(DataStorage dataStorage, int attrIndex);
	double getGain(DataStorage dataStorage, int attrIndex);
	double partitionInfo(DataStorage dataStorage, int attrIndex);
	void printTree(int nodeIndex, string edgeLabel);
};

void getSubset(vector<vector<string>>& data, vector<vector<string>>& subset_data, bool* visited, int size)
{

	for (int i = 0; i < size; ++i)
	{
		int index = rng() % data.size();
		while (visited[index] != false)
		{
			index = rng() % data.size();
		}
		visited[index] = 1;
		subset_data.push_back(data[index]);
	}
}


string getMode(vector<vector<string>>& data, int index)
{
	vector<string> row;

	for (int i = 0; i < data.size(); ++i)
	{
		row.push_back(data[i][index]);
	}
	sort(row.begin(), row.end());

	string number = row[0];
	string mode = number;
	int count = 1;
	int countMode = 1;

	for (int i = 1; i < row.size(); i++)
	{
		if (row[i] == number)
		{
			++count;
		}
		else
		{
			if (count > countMode)
			{
				countMode = count;
				mode = number;
			}
			count = 1;
			number = row[i];
		}
	}
	return mode;
}


void withoutQuestion(vector<vector<string>>& data)
{
	for (int i = 1; i < data.size(); ++i)
	{

		for (int j = 0; j < data[i].size(); ++j)
		{
			if (data[i][j] == "?")
			{
				string mode = getMode(data, j - 1);
				data[i][j] = mode;
			}
		}
	}
}

double countCourrect = 0;
int main()
{
	string filename = "breast-cancer.data";
	readFromFile reader(filename);
	vector<string> test_set;
	vector<vector<string>> subsets;
	DataStorage t = reader.getTable();
	withoutQuestion(t.data);
	DataStorage t1 = t;

	bool* visited = new bool[t.data.size()];
	for (int i = 0; i < t.data.size(); ++i)
	{
		visited[i] = false;
	}
	int size = t.data.size() / 10;
	int	ost = t.data.size() % 10;

	int less_than;
	vector<string> test_subset;
	string test_line;

	string actualClass;
	double totalAccurancy = 0.0;
	int i;
	for (i = 0; i < 10; ++i)
	{
		size = t1.data.size() / 10;
		if (ost > 0)
		{
			size = size + 1;
			ost--;
		}
		getSubset(t1.data, subsets, visited, size);
		t.data = subsets;
		vector<vector<string>> subset_of_data_without_test;
		for (int j = 0; j < subsets.size(); ++j)
		{
			for (int k = 0; k < subsets.size(); ++k)
			{
				if (j != k)
				{
					subset_of_data_without_test.push_back(subsets[k]);
				}
			}
			test_set = subsets[j];
			t.data = subset_of_data_without_test;
			Tree d(t);
			string actualClass = test_set.back();
			test_set.pop_back();
			if (actualClass == d.predict(test_set))
			{
				countCourrect++;
			}
			subset_of_data_without_test.clear();
		}
		double accurancy = countCourrect / subsets.size();
		cout << "Accuracy: " << accurancy << endl;
		countCourrect = 0;
		subsets.clear();
		totalAccurancy += accurancy;
	}
	double averageAccurancy = totalAccurancy / i;
	cout << "AVERAGE Accuracy: " << averageAccurancy << endl;
	delete visited;
}

readFromFile::readFromFile(string filename)
{
	fin.open(filename);
	if (!fin) {
		cout << filename << " file could not be opened" << endl;
		exit(0);
	}
	read();
}

void readFromFile::read()
{
	string str;

	bool isAttrName = true;
	while (!getline(fin, str).eof()) {
		vector<string> row;
		int pre = 0;
		for (int i = 0; i < str.size(); i++) {
			if (str[i] == ',') {
				string col = str.substr(pre, i - pre);
				row.push_back(col);
				pre = i + 1;
			}
		}
		string col = str.substr(pre, str.size() - pre);
		row.push_back(col);
		if (isAttrName) {
			dataStorage.attrName = row;
			isAttrName = false;
		}
		else {
			dataStorage.data.push_back(row);
		}
	}
}

DataStorage readFromFile::getTable()
{
	return dataStorage;
}

Node::Node()
{
	isLeaf = false;
}

Tree::Tree(DataStorage dataStorage)
{
	init = dataStorage;
	init.getAttibuteValue();
	Node root;
	root.treeIndex = 0;
	tree.push_back(root);
	generateDT(init, 0);
}

string Tree::predict(vector<string> row)
{
	string label = "";
	int leafNode = DFS(row, 0);
	if (leafNode == -1) {
		return "ERROR";
	}
	label = tree[leafNode].label;
	return label;
}

int Tree::DFS(vector<string>& row, int currentNode)
{
	if (tree[currentNode].isLeaf) {
		return currentNode;
	}

	int bestAttrIndex = tree[currentNode].bestAttrIndex;

	for (int i = 0; i < tree[currentNode].children.size(); i++) {
		int next = tree[currentNode].children[i];

		if (row[bestAttrIndex] == tree[next].attrValue) {
			return DFS(row, next);
		}
	}
	return -1;
}

void Tree::generateDT(DataStorage dataStorage, int nodeIndex)
{

	if (isLeafNode(dataStorage) == true) {
		tree[nodeIndex].isLeaf = true;
		tree[nodeIndex].label = dataStorage.data.back().back();
		return;
	}

	int selectedAttrIndex = getSelectedAttribute(dataStorage);
	if (selectedAttrIndex > -1)
	{
		map<string, vector<int> > attrValueMap;
		for (int i = 0; i < dataStorage.data.size(); i++) {
			attrValueMap[dataStorage.data[i][selectedAttrIndex]].push_back(i);
		}

		tree[nodeIndex].bestAttrIndex = selectedAttrIndex;

		pair<string, int> dominate = getDominateLabel(dataStorage);
		if ((double)dominate.second / dataStorage.data.size() > 0.85) {
			tree[nodeIndex].isLeaf = true;
			tree[nodeIndex].label = dominate.first;
			return;
		}

		for (int i = 0; i < init.attrValueList[selectedAttrIndex].size(); i++) {
			string attrValue = init.attrValueList[selectedAttrIndex][i];
			DataStorage nextState;
			vector<int> candi = attrValueMap[attrValue];
			for (int i = 0; i < candi.size(); i++) {
				nextState.data.push_back(dataStorage.data[candi[i]]);
			}

			Node nextNode;
			nextNode.attrValue = attrValue;
			nextNode.treeIndex = (int)tree.size();
			tree[nodeIndex].children.push_back(nextNode.treeIndex);
			tree.push_back(nextNode);

			// при празно състояние
			if (nextState.data.size() == 0) {
				nextNode.isLeaf = true;
				nextNode.label = getDominateLabel(dataStorage).first;
				tree[nextNode.treeIndex] = nextNode;
			}
			else {
				generateDT(nextState, nextNode.treeIndex);
			}
		}
	}
}

pair<string, int> Tree::getDominateLabel(DataStorage dataStorage)
{
	string dominateLabel = "";
	int dominateCount = 0;
	map<string, int> labelCount;
	for (int i = 0; i < dataStorage.data.size(); i++) {
		labelCount[dataStorage.data[i].back()]++;

		if (labelCount[dataStorage.data[i].back()] > dominateCount) {
			dominateCount = labelCount[dataStorage.data[i].back()];
			dominateLabel = dataStorage.data[i].back();
		}
	}

	return { dominateLabel, dominateCount };
}
bool Tree::isLeafNode(DataStorage dataStorage)
{
	for (int i = 1; i < dataStorage.data.size(); i++) {
		if (dataStorage.data[0].back() != dataStorage.data[i].back()) {
			return false;
		}
	}
	return true;
}
int Tree::getSelectedAttribute(DataStorage dataStorage)
{
	int maxAttrIndex = -1;
	double maxAttrValue = 0.0;


	// вземане на характеристиките на атрибут
	for (int i = 0; i < init.attrName.size() - 1; i++) {
		if (maxAttrValue < getGainAll(dataStorage, i)) {
			maxAttrValue = getGainAll(dataStorage, i);
			maxAttrIndex = i;
		}
	}
	return maxAttrIndex;
}
double Tree::getGainAll(DataStorage dataStorage, int attrIndex)
{
	return getGain(dataStorage, attrIndex) / partitionInfo(dataStorage, attrIndex);
}
double Tree::getInfo(DataStorage dataStorage)
{
	double result = 0.0;

	int itemCount = (int)dataStorage.data.size();
	map<string, int> labelCount;

	for (int i = 0; i < dataStorage.data.size(); i++) {
		labelCount[dataStorage.data[i].back()]++;
	}
	for (auto iter = labelCount.begin(); iter != labelCount.end(); iter++) {
		double p = (double)iter->second / itemCount;

		result += -1.0 * p * log(p) / log(2);
	}
	return result;
}
double Tree::getInfoAttr(DataStorage dataStorage, int attrIndex)
{
	double infoForAttribute = 0.0;
	int itemCount = (int)dataStorage.data.size();

	map<string, vector<int> > attrValueMap;
	for (int i = 0; i < dataStorage.data.size(); i++) {
		attrValueMap[dataStorage.data[i][attrIndex]].push_back(i);
	}

	for (auto iter = attrValueMap.begin(); iter != attrValueMap.end(); iter++) {
		DataStorage nextState;
		for (int i = 0; i < iter->second.size(); i++) {
			nextState.data.push_back(dataStorage.data[iter->second[i]]);
		}
		int nextItemCount = (int)nextState.data.size();

		infoForAttribute += (double)nextItemCount / itemCount * getInfo(nextState);
	}

	return infoForAttribute;
}
double Tree::getGain(DataStorage dataStorage, int attrIndex)
{
	return getInfo(dataStorage) - getInfoAttr(dataStorage, attrIndex);
}
double Tree::partitionInfo(DataStorage dataStorage, int attrIndex)
{
	double result = 0.0;

	int itemCount = (int)dataStorage.data.size();

	map<string, vector<int> > attrValueMap;
	for (int i = 0; i < dataStorage.data.size(); i++) {
		attrValueMap[dataStorage.data[i][attrIndex]].push_back(i);
	}

	for (auto iter = attrValueMap.begin(); iter != attrValueMap.end(); iter++) {
		DataStorage nextState;
		for (int i = 0; i < iter->second.size(); i++) {
			nextState.data.push_back(dataStorage.data[iter->second[i]]);
		}
		int nextItemCount = (int)nextState.data.size();

		double d = (double)nextItemCount / itemCount;
		result += -1.0 * d * log(d) / log(2);
	}

	return result;
}
void Tree::printTree(int nodeIndex, string edgeLabel)
{
	if (tree[nodeIndex].isLeaf == true)
		cout << edgeLabel << "Label: " << tree[nodeIndex].label << endl;
	for (int i = 0; i < tree[nodeIndex].children.size(); i++) {
		int childIndex = tree[nodeIndex].children[i];

		string attributeName = init.attrName[tree[nodeIndex].bestAttrIndex];
		string attributeValue = tree[childIndex].attrValue;

		printTree(childIndex, edgeLabel + attributeName + " = " + attributeValue + ", ");
	}
}
void DataStorage::print() {
	cout << "DATA:" << endl;
	for (int i = 0; i < data.size(); ++i)
	{
		for (int j = 0; j < data[i].size(); ++j)
			cout << data[i][j] << ' ';

		cout << endl;
	}


	cout << "NAME:" << endl;
	for (int i = 0; i < attrName.size(); ++i)
	{
		cout << attrName[i] << ' ' << endl;
	}


	cout << endl;

	cout << "List Att-Value:" << endl;
	for (int i = 0; i < attrValueList.size(); ++i)
	{
		for (int j = 0; j < attrValueList[i].size(); ++j)
			cout << attrValueList[i][j] << ' ';
	}
	cout << endl;
}

void DataStorage::getAttibuteValue()
{
	attrValueList.resize(attrName.size());
	for (int j = 0; j < attrName.size(); j++) {
		map<string, int> value;
		for (int i = 0; i < data.size(); i++) {
			value[data[i][j]] = 1;
		}

		for (auto iter = value.begin(); iter != value.end(); iter++) {
			attrValueList[j].push_back(iter->first);
		}
	}
}
