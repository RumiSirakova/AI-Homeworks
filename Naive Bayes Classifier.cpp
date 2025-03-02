#include<iostream>
#include<vector>
#include<string>
#include<math.h>
#include <unordered_map> 
#include <cmath>
#include <fstream>
#include <random>
#include <utility> 
#include <time.h> 
#include <chrono> 
#include<iomanip>
#include<limits>
#include<cstddef>
using namespace std;

std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());


void get_prob_per_class(vector<vector<string>> data, unordered_map< string, long double>& classes_prob, string class_)
{
	long double frequency = 0;
	for (int i = 0; i < data.size(); ++i)
	{
		if (data[i][0] == class_)
		{
			frequency++;
		}
	}
	//cout << class_ << " " << frequency << endl;
	long double value = (frequency / data.size());
	long double rounded_down = floorf(value * 100) / 100;
	long double nearest = roundf(value * 100) / 100;
	long double rounded_up = ceilf(value * 100) / 100;
	classes_prob[class_] = nearest;
}
void get_prob_class(vector<vector<string>> data, unordered_map< string, long double>& classes_prob, vector<string> classes)
{
	for (int i = 0; i < classes.size(); ++i)
	{
		get_prob_per_class(data, classes_prob, classes[i]);
	}
}


void get_probability_per_attribute_per_value(vector<vector<string>> data, unordered_map<string, unordered_map<string, long double>>& attributes, unordered_map< string, long double>& classes_prob, string classes, string attribute, string valueAttribute, int index)
{

	long double frequency = 0;
	long double total = 0;
	for (int i = 0; i < data.size(); ++i)
	{
		if (data[i][0] == classes)
		{

			total++;
			if (data[i][index] == valueAttribute)
			{
				frequency++;
			}
		}
	}

	string att = (attribute + " " + valueAttribute);
	unordered_map<string, long double> m;
	long double value = frequency / total;
	long double rounded_down = floorf(value * 100) / 100;
	long double nearest = roundf(value * 100) / 100;
	long double rounded_up = ceilf(value * 100) / 100;
	m[att] = nearest;
	attributes[classes + " " + attribute + " " + valueAttribute] = m;

}

void get_prob_per_attributes(vector<vector<string>> data, unordered_map<string, unordered_map<string, long double>>& attributes, unordered_map< string, long double>& classes_prob, string classes, string attribute, int index, vector<string> attributesValues)
{
	for (string value : attributesValues)
	{
		get_probability_per_attribute_per_value(data, attributes, classes_prob, classes, attribute, value, index);
	}
}

void get_prob_per_all_attributes(vector<vector<string>> data, unordered_map<string, unordered_map<string, long double>>& attributes_prob, unordered_map< string, long double>& classes_prob, string classes, vector<vector<string>> attibutes, vector<string> attributeLabel)
{
	int index = 1;
	for (int i = 0; i < attributeLabel.size(); ++i)
	{
		get_prob_per_attributes(data, attributes_prob, classes_prob, classes, attributeLabel[i], index, attibutes[i]);
		index++;
	}
}

void get_probabilirty(vector<vector<string>> data, unordered_map<string, unordered_map<string, long double>>& attributes_prob, unordered_map< string, long double>& classes_prob, vector<string>& classes, vector<vector<string>> attibutes, vector<string> attributeLabel)
{

	for (int i = 0; i < classes.size(); ++i)
	{
		get_prob_per_all_attributes(data, attributes_prob, classes_prob, classes[i], attibutes, attributeLabel);
	}
}

long double predict_with_given_class_for_one_attribute(unordered_map<string, unordered_map<string, long double>>& att, string line)
{
	unordered_map<string, long double> current;
	current = att[line];
	long double probability = 1;

	for (auto x : current)
	{
		probability = x.second;
	}
	long double nearest = roundf(probability * 100) / 100;
	return nearest;
}
long double predict_for_all_attributes(long double zero_valuee, unordered_map<string, unordered_map<string, long double>>& att, vector<string> line, string class_)
{
	long double probability = 0;
	for (int i = 0; i < line.size(); ++i)
	{
		line[i] = class_ + " " + line[i];

		long double value = predict_with_given_class_for_one_attribute(att, line[i]);
		if (value == 0)
		{
			value = zero_valuee;
		}

		probability += log(value);

		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!cout <<"log:"<< probability << endl;
	}
	return probability;
}
long double predict_for_all_classes(long double zero_value, unordered_map<string, long double>& classes_prob, unordered_map<string, unordered_map<string, long double>>& att, vector<string> line, vector<string> classes, string& predictedClass)
{
	long double predict_for_attribute = predict_for_all_attributes(zero_value, att, line, "democrat");
	long double predict_for_class = log(classes_prob["democrat"]);
	long double x = predict_for_attribute + predict_for_class;


	predict_for_attribute = predict_for_all_attributes(zero_value, att, line, "republican");
	predict_for_class = log(classes_prob["republican"]);
	long double y = predict_for_attribute + predict_for_class;


	if (x - y > zero_value)
	{
		predictedClass = "democrat";
		//!! cout << "predictedClass: " << predict_for_class << " predictedAttribute: " << predict_for_attribute << endl;
		return x;
	}
	else
	{
		predictedClass = "republican";
		//!! cout << "predictedClass: " << predict_for_class << " predictedAttribute: " << predict_for_attribute << endl;
		return y;
	}

}

void create_line(vector<string> attributesLabel, string line, vector<string>& result)
{
	int j = 0;
	string currentLine;
	for (int i = 0; i < line.size(); i++)
	{
		currentLine.clear();
		currentLine = attributesLabel[j] + " ";
		while (line[i] != ',' && i < line.size())
		{

			currentLine += line[i];
			i++;
		}
		result.push_back(currentLine);
		++j;
	}
}

void read_from_file(vector<vector<string>>& data)
{
	// Create a text string, which is used to output the text file
	string line;

	// Read from the text file
	ifstream file("house-votes-84.data");

	vector<string> row;
	string s;
	while (getline(file, s))
	{
		int i = 0;
		while (i < s.size())
		{
			if (s[i] != ',')
			{
				string w = "";
				while (i < s.size() && s[i] != ',')
				{
					w += s[i];
					++i;
				}
				row.push_back(w);
			}
			else
			{
				++i;
			}
		}
		data.push_back(row);
		row.clear();
	}
	file.close();
}

void get_ten_subset(vector<vector<string>>& data, vector<vector<string>>& subset_data, bool* visited, int size)
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

void remove_class_name_from_test_set(string& actualClass, vector<string>& test_set, string& test_line)
{
	test_line = "";
	actualClass.clear();
	actualClass = test_set[0];
	for (int i = 1; i < test_set.size(); ++i)
	{
		test_line += test_set[i];
		if (i != test_set.size())
			test_line += ",";
	}
}
//Accuracy=правилно изчислените/всички
double countCourrect = 0;


void task(vector<vector<string>>& data, string& test_line, string actualClass)
{

	vector<string> attributesLabel;
	attributesLabel.push_back("handicapped-infants");
	attributesLabel.push_back("water-project-cost-sharing");
	attributesLabel.push_back("adoption-of-the-budget-resolution");
	attributesLabel.push_back("physician-fee-freeze");
	attributesLabel.push_back("el-salvador-aid");
	attributesLabel.push_back("religious-groups-in-schools");
	attributesLabel.push_back("anti-satellite-test-ban");
	attributesLabel.push_back("aid-to-nicaraguan-contras");
	attributesLabel.push_back("mx-missile");
	attributesLabel.push_back("immigration");
	attributesLabel.push_back("synfuels-corporation-cutback");
	attributesLabel.push_back("education-spending");
	attributesLabel.push_back("superfund-right-to-sue");
	attributesLabel.push_back("crime");
	attributesLabel.push_back("duty-free-exports");
	attributesLabel.push_back("export-administration-act-south-africa");

	vector<string> classes;
	classes.push_back("democrat");
	classes.push_back("republican");

	vector<vector<string>> attributes;
	vector<string> a1;
	vector<string> a2;
	vector<string> a3;
	vector<string> a4;
	vector<string> a5;
	vector<string> a6;
	vector<string> a7;
	vector<string> a8;
	vector<string> a9;
	vector<string> a10;
	vector<string> a11;
	vector<string> a12;
	vector<string> a13;
	vector<string> a14;
	vector<string> a15;
	vector<string> a16;
	attributes.push_back(a1);
	attributes.push_back(a2);
	attributes.push_back(a3);
	attributes.push_back(a4);
	attributes.push_back(a5);
	attributes.push_back(a6);
	attributes.push_back(a7);
	attributes.push_back(a8);
	attributes.push_back(a9);
	attributes.push_back(a10);
	attributes.push_back(a11);
	attributes.push_back(a12);
	attributes.push_back(a13);
	attributes.push_back(a14);
	attributes.push_back(a15);
	attributes.push_back(a16);

	for (int i = 0; i < 16; ++i)
	{
		attributes[i].push_back("y");
		attributes[i].push_back("n");
		attributes[i].push_back("?");
	}


	unordered_map< string, long double> classes_prob;
	unordered_map< string, unordered_map<string, long double>> att;
	get_probabilirty(data, att, classes_prob, classes, attributes, attributesLabel);



	vector<string> result;
	string predicted_class;
	create_line(attributesLabel, test_line, result);
	get_prob_per_class(data, classes_prob, "democrat");
	get_prob_per_class(data, classes_prob, "republican");

	long double zero_values = (double)1 / data.size();

	predict_for_all_classes(zero_values, classes_prob, att, result, classes, predicted_class);




	//cout << "Predicted class for " << test_line << " is: " << predicted_class <<"  " << actualClass<< endl;
	if (predicted_class == actualClass)
	{
		countCourrect++;
	}
}
int main() {

	vector<vector<string>> data;
	read_from_file(data);
	vector<vector<string>> subset_of_data;
	bool* visited = new bool[data.size()];
	for (int i = 0; i < data.size(); ++i)
	{
		visited[i] = false;
	}

	int size = data.size() / 10;
	int	ost = data.size() % 10;

	int less_than;
	vector<string> test_subset;
	string test_line;

	string actualClass;
	double totalAccurancy = 0.0;
	int i;
	for (i = 0; i < 10; ++i)
	{
		size = data.size() / 10;
		if (ost > 0)
		{
			size = size + 1;
			ost--;
		}
		get_ten_subset(data, subset_of_data, visited, size);
		//множеството от данни без - това за което искаме да изчислим

		vector<vector<string>> subset_of_data_without_test;

		for (int j = 0; j < subset_of_data.size(); ++j)
		{
			for (int k = 0; k < subset_of_data.size(); ++k)
			{
				if (j != k)
				{
					subset_of_data_without_test.push_back(subset_of_data[k]);
				}
			}
			test_subset = subset_of_data[j];
			remove_class_name_from_test_set(actualClass, test_subset, test_line);
			task(subset_of_data, test_line, actualClass);
			subset_of_data_without_test.clear();
		}

		double accurancy = countCourrect / subset_of_data.size();
		cout << "Accuracy: " << accurancy << endl;
		countCourrect = 0;
		subset_of_data.clear();
		totalAccurancy += accurancy;
	}
	double averageAccurancy = totalAccurancy / i;
	cout << "AVERAGE Accuracy: " << averageAccurancy << endl;

	delete visited;

	return 0;
}
