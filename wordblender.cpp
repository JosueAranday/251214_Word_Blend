#include <unordered_map>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include "wordblender.h"

string blending(string first_word, string last_word)
{
	if (last_word.length() < 2) return first_word + last_word; // Handle short words
	return first_word + last_word.substr(2);
}

void printTable(string** table, int max_word_count, size_t dict_size) {
	for (int i = 1; i <= max_word_count; ++i) {
		std::cout << "L" << i << ": ";
		for (size_t j = 1; j < dict_size; ++j) {
			if (!table[i][j].empty()) {
				std::cout << table[i][j] << " | ";
			}
			else {
				std::cout << "_______ | "; // Print placeholder and separator
			}
		}
		std::cout << std::endl;
	}
}

WordBlender::WordBlender(string filename, int max_word_count) {
	unordered_map<string, size_t> dictionary;
	dictionary.clear();

    ifstream f(filename);
	if (!f.is_open()) {
		throw runtime_error("Failed to open file: " + filename);
	}
    string line;
	size_t dict_size = 1;
	while (getline(f, line)) {
		dictionary[line] = dict_size;
		++dict_size;
    }
    f.close(); 

	// Print dictionary for debugging
	/*for (const auto& pair : dictionary) {
		cout << "Word: " << pair.first << ", ID: " << pair.second << endl;
	}*/

	this->max_word_count = max_word_count;
	size_t allocated_cols = dictionary.size() + 1;
	// Initialize table with empty strings
	table = new string * [max_word_count + 1];      // Rows: 0 to max_word_count
	for (int i = 0; i <= max_word_count; ++i) {
		table[i] = new string[allocated_cols];		// Columns: 0 to dict_size
		for (size_t j = 0; j < allocated_cols; ++j) {
			table[i][j] = "";                       // Initialize with empty strings
		}
	}
	//printTable(table, max_word_count, allocated_cols);

	for (const auto& pair : dictionary) {
		table[1][pair.second] = pair.first;			// Blend of length 1 is the word itself
		//cout << "length: 1, word: " << pair.first << endl;
	}
	//printTable(table, max_word_count, allocated_cols);

	// Filling out table for blends of length 2
	for (const auto& first_pair : dictionary) {
		const string& first_word = first_pair.first;
		size_t first_id = first_pair.second;
		string last2letters1 = first_word.substr(first_word.length() - 2, 2); // Last 2 chars of first_word
		for (const auto& second_pair : dictionary) {
			const string& second_word = second_pair.first;
			size_t second_id = second_pair.second;
			string first2letters2 = second_word.substr(0, 2); // First 2 chars of second_word
			if (last2letters1 == first2letters2) {
				string blended = blending(first_word, second_word);
				//std::cout << "length: 2, first_word: " << first_word << ", second_word: " << second_word << ", blended: " << blended << endl;
				table[2][first_id] = blended; // Store the blend in the table
			}
		}
	}	
	//printTable(table, max_word_count, allocated_cols);


	// Filling out table for blends of length 3 to max_word_count
	for (int length =3; length <= max_word_count; ++length) {
		for (int k = 0; k < dict_size; k++) {
			if (table[length - 1][k] != "") {
				string checkingWord = table[length - 1][k];
				//cout << "Checking word: " << checkingWord << endl;
				string last2letters = checkingWord.substr(checkingWord.length() - 2, 2); // Last 2 chars of checkingWord
				for (const auto& pair : dictionary) {
					const string& dict_word = pair.first;
					size_t dict_id = pair.second;
					string first2letters = dict_word.substr(0, 2); // First 2 chars of dict_word
					if (last2letters == first2letters) {
						string blended = blending(checkingWord, dict_word);
						//std::cout << "length: " << length << ", checkingWord: " << checkingWord << ", dict_word: " << dict_word << ", blended: " << blended << endl;
						table[length][k] = blended; // Store the blend in the table
					}
				}
			}
			else {
				//cout << "No checking word " << endl;
				continue;
			}
		}
		//printTable(table, max_word_count, allocated_cols);

	}
	//printTable(table, max_word_count, allocated_cols);

}

size_t getTableIndex(string** table, string word) {
	int dict_size = 1;
	while (!table[1][dict_size].empty()) {
		if (table[1][dict_size] == word) {
			return dict_size;
		}
		++dict_size;
	}
	return 0;
}

string WordBlender::blend(string first_word, string last_word, int word_count) {
	if (word_count < 1 || word_count > max_word_count) {
		return ""; // or throw std::out_of_range
	}

	if(word_count == 2){
		// Directly check for blends of length 2
		string last2letters1 = first_word.substr(first_word.length() - 2, 2); // Last 2 chars of first_word
		string first2letters2 = last_word.substr(0, 2); // First 2 chars of last_word
		if (last2letters1 == first2letters2) {
			return blending(first_word, last_word);
		}
		else {
			return "";
		}
	}

	size_t index = getTableIndex(table, first_word);
	cout << "Index for first_word '" << first_word << "': " << index << endl;
	cout << "Retrieving blend for length " << word_count << " and index " << index << endl;
	cout << "Blend found: " << table[word_count][index] << endl;



	// The result is stored at table[length][index]
	const string& blend_result = table[word_count][index];

	// CRITICAL LOGIC: The table stores all possible L-length blends ENDING at 'last_word',
	// but we must ensure it STARTS with 'first_word'.
	if (blend_result.empty() || blend_result.substr(0, first_word.length()) != first_word) {
		return "";
	}

	return blend_result; // Return the actual found blend.
}
//WordBlender::~WordBlender() {
//	// Check if the table was successfully allocated before trying to delete
//	if (table != nullptr) {
//		// 1. Delete each row (the inner arrays)
//		// We must loop up to max_word_count, as that was the row count used in allocation.
//		for (int i = 0; i <= this->max_word_count; ++i) {
//			// It's possible that the allocation loop for rows failed before finishing.
//			// A more robust check might involve ensuring table[i] is not null, 
//			// but for simplicity, we assume if table != nullptr, we try to clean all rows.
//			delete[] table[i];
//		}
//		// 2. Delete the array of pointers (the outer array)
//		delete[] table;
//		table = nullptr;
//	}
//}