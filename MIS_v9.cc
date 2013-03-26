#include <stdlib.h>
#include <fstream>
#include <iostream>
using namespace std;

int nverts = 0;
class SetOfSets;

// Just a list of vertices that correspond to a cover set.
// Objects of this type can be placed into a linked list via the 'next'
// field (this is managed by a object - see below)
class CoverSet {
   friend class SetOfSets;
   friend ostream &operator<<(ostream&, SetOfSets*);
   friend ostream &operator<<(ostream&, CoverSet*);
   int *vertex, nvertices;
   CoverSet *next; 

public:
   CoverSet () { vertex = new int[nverts];  nvertices = 0;  next = NULL; } 

   // Add a vertex to the cover    
   CoverSet *add (int i) { vertex[nvertices++] = i; return this; }
   
   //New 
    CoverSet *add2 (int i,int j) { vertex[nvertices++] = i; vertex[nvertices++] = j;return this; }

   // Return a copy of this cover
   CoverSet *getCopy () {
      CoverSet *newset = new CoverSet();
      for (int i=0 ; i < nvertices ; i++) newset->add(vertex[i]);
      return newset;
   }

   // Return true if and only if vertex v is in this cover
   bool member (int v) {
      for (int i=0 ; i < nvertices ; i++) if (vertex[i] == v) return true;
      return false;
   }
};

ostream &operator<<(ostream &out, CoverSet *cs) {
   if (cs->nvertices == 0)
      out << "{ }";
   else {
      out << "{";
      for (int i=0 ; i < cs->nvertices ; i++) out << cs->vertex[i] << ",";
      out << "\b}";
   }
   return out;
}

// Stores a linked list of cover sets - a SetOfSets object is an entry 
// in the dynamic programming table
class SetOfSets {
   friend ostream &operator<<(ostream&, SetOfSets*);
   CoverSet *set, *cursor;
   
public:
   SetOfSets () { set = cursor = NULL; }

   // Return true iff there are no covers stored in this object
   bool empty () { return set == NULL; }

   // Add a cover to this list of covers
   void add (CoverSet *s) {
      if (s == NULL) return;
      s->next = set; set = s;
   }

   // Move the cursor to the first cover, return the first cover.
   // Used to begin iterating though the stored list of covers.
   CoverSet *getFirst() { cursor = set; return set; }

   // Move the cursor to the next cover, return the next cover.
   // Used to continue iterating though the stored list of covers
   // and to identify the end of the list of covers.
   CoverSet *getNext() { 
      if (cursor == NULL) return NULL;
      return cursor = cursor->next;
   }
};

ostream &operator<<(ostream &out, SetOfSets *set) {
   out << " ";
   for (CoverSet *s=set->set ; s != NULL ; s=s->next) out << s << ",";
   out << "\b ";
   return out;
}

// Holds a graph for which the minimum vertex cover is to be found
class Graph {
   friend ostream& operator<<(ostream&, Graph*);
   friend class SetOfSets;
   int **vertex, *nvertex, nedges;

public:
   Graph () { vertex = NULL; }

   // Read the input file representing a graph - create an adjacency 
   // list for each vertex
   // File format:
   //   <number-of-vertices> <number-of-edges>
   //   <vertex-1> <vertex-2>
   //   ...
   //   <vertex-1> <vertex-2>
   void readFile (char *filename) {
      int v1, v2;
      fstream fin(filename, ios::in);
      if (fin.fail()) {
         cout << "Zowie\n";
         exit(0);
      }
      fin >> nverts >> nedges;
      vertex = new int*[nverts];
      nvertex = new int[nverts];
      for (int i=0 ; i < nverts ; i++) {
         vertex[i] = new int[nedges];
         nvertex[i] = 0;
      }
      for (int i=0 ; i < nedges ; i++) {
         fin >> v1 >> v2;
         vertex[v1][nvertex[v1]++] = v2;
         vertex[v2][nvertex[v2]++] = v1;
      }
   }

   /*
      Returns true if k vertex is not adjecent to any of the vertices in cover cs. In other words,
      it returns true if cs + vertex k are and independent set
   */
   bool allIndySets (int k, CoverSet *cs){ 
     for (int i=0 ; i < nvertex[k] ; i++){
	if (cs -> member(vertex[k][i])) return false;
	}
         return true;
   }
   
      
   /*
      Given a vertex v, it finds all vertices from 0-(k-1) that are not adjecent to k and creates MIS of size 2
   
   */
   
  void findNonAdjecent (int v, SetOfSets ***DP) {
      int counter=0;
      int edge_count =0;
      
      CoverSet *adjSet = new CoverSet();  //This set will just hold all adjecent vertices to v
      for (int i =0; i < nvertex[v];i++) { adjSet->add(vertex[v][i]);}

      while (counter<v){
	  if(!adjSet->member(counter)){ DP[v][2]->add((new CoverSet())->add2(counter,v));}
	  counter++;
      }
   } 
};

ostream &operator<<(ostream &out, Graph *g) {
   out << "Graph, adjacency lists:\n";
   for (int i=0 ; i < nverts ; i++) {
      out << "\t" << i << ": ";
      for (int j=0 ; j < g->nvertex[i] ; j++) out << g->vertex[i][j] << ",";
      out << "\b \n";
   }
   out << "\n";
   return out;
}

// Display sets of covers in row k of the table DP. I added a counter to check number of entries
void showRow(SetOfSets ***DP, int k, int &entries_count) {
   for (int i=0 ; i <= nverts ; i++) {
      if (DP[k][i]->empty()) continue;
      else {
	for (CoverSet *c=DP[k][i]->getFirst(); c != NULL; c=DP[k][i]->getNext()){
	  entries_count++;
	}
      }
      
    cout << "[" << k << "," << i << "]:\n" << DP[k][i] << "\n";
   }
   cout << "----------------------------------------------\n";
}

int main (int argc, char **argv) {
   if (argc < 2) {
      cout << "Zowie\n";
      exit(0);
   }

   bool showrows = (argc > 2) ? true : false;


   int entries_count =0;

   // Grab a graph and read edges from file
   Graph *graph = new Graph();
   graph->readFile(argv[1]);
   if (showrows) cout << graph;

   // Initialize the table
   // DP[i][j] - covers of size j for subgraph induced by vertices 0...i
   SetOfSets ***DP = new SetOfSets**[nverts];
   for (int i=0 ; i < nverts ; i++) {
      DP[i] = new SetOfSets*[nverts+1];
      for (int j=0 ; j <= nverts ; j++) DP[i][j] = new SetOfSets();
   }
           
   // Complete the 0th row - vertex 0 - { }, {0} for all graphs
   DP[0][0]->add(new CoverSet());
   DP[0][1]->add(new CoverSet());

   if (showrows) showRow(DP, 0, entries_count);

    
	 
   
   
   
   CoverSet *cs;
   // Complete all remaining rows - 
   for (int k=2 ; k < nverts ; k++) { //rows
      for (int i=2 ; i < nverts ; i++) { //columns
	
	if (i ==2){
	   graph->findNonAdjecent (k, DP); 
	}
         else if (!DP[k-1][i-1]->empty()) {
	  for(int row = k-1; row>=2; row--){
	    for (cs=DP[row][i-1]->getFirst(); cs != NULL; cs=DP[row][i-1]->getNext()) {
		if (graph->allIndySets(k,cs)){
		  DP[k][i]->add((cs->getCopy())->add(k));
		}
	      }
	    }
	 }
      }
      if (showrows) showRow(DP, k, entries_count);
   }

   // Walk through top row from column 0 until some sets are found
   // then output those sets
   cout<<"----------NUMBER OF ENTRIES="<<entries_count<<"----------------"<<endl;

    cout << "Independent Sets:";
    bool MIS_found =false;
   for (int column = nverts-1; column>=2; column--){
     for(int row = nverts -1; row>=2;row--){
       if(!DP[row][column]->empty()){
	  cout<<DP[row][column]<<",";
	  MIS_found =true;
	}
     }
     if(MIS_found) break;
   }

   cout<<"\n"<<endl;
}


