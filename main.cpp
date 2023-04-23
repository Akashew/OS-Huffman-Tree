#include "Huffman.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <sstream>
#include <string>
#include <vector>

struct threadinfo {
  std::vector<int> positions;
  std::string *message;
  node *treenode;
  std::string trav;
};

//----------------------------------------------------------------------------------------------

char traverse(node *tree, std::string tra, int index) {

  if (tree == nullptr) {
    return 1;
  }

  if (tree->right == nullptr && tree->left == nullptr) {
    return tree->c;
  } else if (tra[index] == '0') {
    return traverse(tree->left, tra, index + 1);

  } else if (tra[index] == '1') {
    return traverse(tree->right, tra, index + 1);
  }
}

//----------------------------------------------------------------------------------------------

void *decode(void *void_ptr) {

  threadinfo *arg = (threadinfo *)void_ptr;

  for (int i = 0; i < arg->positions.size(); i++) {

    int pos = arg->positions.at(i);

    char letter = traverse(arg->treenode, arg->trav, 0);

    (*arg->message)[pos] = letter;
  }

  pthread_exit(NULL);
}

//----------------------------------------------------------------------------------------------

int main() {

  //----------------Input File-----------------------------------

  std::string filename;
  getline(std::cin, filename);
  std::ifstream fin(filename);

  if (fin.peek() == EOF) {
    std::cout << "Input file not found" << std::endl;
    fin.close();
    exit(0);
  }

  std::string compressed;
  getline(std::cin, compressed);
  std::ifstream cmpr(compressed);

  if (cmpr.peek() == EOF) {
    std::cout << "Compressed file not found" << std::endl;
    cmpr.close();
    exit(0);
  }

  if (!cmpr.is_open()) {
    std::cout << "Failed to open compressed file" << std::endl;
    return 1;
  }

  std::string line;
  std::vector<char> chars;
  std::vector<int> freqs;

  int Symcount = 0;

  while (getline(fin, line)) {
    char c = line.at(0);
    chars.push_back(c);

    std::string l = line.substr(2, line.length());
    int f = stoi(l);
    freqs.push_back(f);

    Symcount++;
  }

  node *root = huffman(chars, freqs);

  printCodes(root, "");

  //------------Compressed File---------------------------------

  std::string cline;

  pthread_t *threadid = new pthread_t[Symcount]; // pthread array
  threadinfo *data = new threadinfo[Symcount];   // struct array

  int totfreq = 0;

  for (int i = 0; i < freqs.size(); i++) { // size of the message
    totfreq += freqs.at(i);
  }

  std::string message(totfreq, '_');

  int j = 0;
  while (getline(cmpr, cline)) {

    std::string s1 = cline.substr(0, cline.find(' '));

    data[j].trav = s1;
    data[j].treenode = root;
    data[j].message = &message;

    int e = cline.find(' ');

    std::stringstream ss(cline.substr(e, cline.length()));
    int n;

    while (ss >> n) {
      data[j].positions.push_back(n);
    }

    if (pthread_create(&threadid[j], NULL, decode, &data[j])) {
      fprintf(stderr, "Error creating thread\n");
      return 1;
    }

    j++;
  }

  for (int i = 0; i < Symcount; i++) {
    pthread_join(threadid[i], NULL);
  }

  //------------Message-------------------

  std::cout << "Original message: ";

  for (int i = 0; i < totfreq; i++) {
    std::cout << message[i];
  }

  delete[] threadid;
  delete[] data;

  return 0;
}
