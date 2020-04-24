#include <iostream>
#include <list>
#include <vector>
#include <regex>
#include <iterator>
#include <string>
#include <stack>
#include <list>
#include <memory>
#include "minisat/core/SolverTypes.h" // Defines Var and Lit
#include "minisat/core/Solver.h" // Defines Solver

using namespace std; 

// Creates an adjacency list (ie. array of arrays)
void add_edge(vector<int> adj_list[], int u, int v){ 
    adj_list[u].push_back(v); //add v into list u
    adj_list[v].push_back(u); //add u into list v
}

void minisat_reduction(int V, vector<int> adj_list[]){
    std::unique_ptr<Minisat::Solver> solver(new Minisat::Solver());
    bool res = false; // Initializing solver to be false
    int k = 1; // Initialize minimum vertex cover as one

    while(k < V){

        // Initialize atomic proposition matrix sized Vxk
        Minisat::Lit matrix[V][k];

        // Populate each element in the atomic proposition matrix as a literal
        for (int x = 0; x < V; x++)
            for (int y = 0; y < k; y++){
                Minisat::Lit literal;
                literal = Minisat::mkLit(solver->newVar());
                matrix[x][y] = literal; 
            }
        

        // Clause 1: At least one vertex is theith vertex in the vertex cover
        for (unsigned int i = 0; i < k; i++){ //for all i which exists from k=1 to k 

            // MiniSAT only accepts 4 arguments max for each addClause! If multiple literals exist, use one vector of literals
            Minisat::vec<Minisat::Lit> clause_1;
            
            for (unsigned int j = 0; j < V; j++){
                clause_1.push(matrix[j][i]);
            }

            solver->addClause(clause_1);
        }
            
        // Clause 2: No one vertex can appear twice in a vertex cover
        for (unsigned int m = 0; m < V; m++){
            for (unsigned int p = 0; p < k; p++){
                for (unsigned int q = 0; q < k; q++){
                    if (p<q){
                        solver->addClause(~matrix[m][p],~matrix[m][q]);
                    }
                }
            }
        }

        
        // Clause 3: No more than one vertex appears in themth position of the vertex cover
        for (unsigned int m = 0; m < k; m++){
            for (unsigned int p = 0; p < V; p++){
                for (unsigned int q = 0; q < V; q++){
                    if (p<q){
                        solver->addClause(~matrix[p][m],~matrix[q][m]);
                    }
                }
            }
        }
    
        // Clause 4: Every edge is incident to at least one vertex in the vertex cover
        for (unsigned int i = 0; i < V; i++){
            for (auto m: adj_list[i]){
                
                if (m < i)
                    continue;

                
                Minisat::vec<Minisat::Lit> clause_2;
                                
                for (unsigned int x = 0; x < k; x++){
                    clause_2.push(matrix[i][x]);
                    clause_2.push(matrix[m][x]);
                }

                solver->addClause(clause_2);
    
            }
            
        }


        //Use SAT solver for the current minimum vertex cover value, k
        res = solver->solve();

        if (res == true){
        
            string minimum = "";

            for (int x = 0; x < V; x++){
                for (int y = 0; y < k; y++){
                    if (Minisat::toInt(solver->modelValue(matrix[x][y])) == 0){
                        minimum.append(to_string(x) + " ");

                    }
                }
            }

            // Outputs the Minimum Vertex Cover for the graph
            cout << minimum << endl;
        

            break;

        }

        else{
            solver.reset(new Minisat::Solver());
            k++;

        }
    }
}

int main(int argc, char* argv[]){

   smatch m;
   string input;
   int a,b,c,V = 1;
   vector <int> *adj_list = NULL; //initializing adjacency list

    // Matches command input to appropriate loop
    regex vert_rx("^[V]"); 
    regex edge_rx("^[E]");
    regex short_rx("^[s]");

    // Extract relevant info from input
    const regex numbers("([0-9]+)+");
    const regex coordinates("([0-9]+[,][0-9]+)+");

    while (!cin.eof()){ //eof allows program to exit gracefully 
        getline(cin, input);

        if (cin.eof())
            break;

        // Executes this block of code if the number of vertices is specifed through the "V" command
        if (regex_search(input,m,vert_rx)){
            regex_search(input, m, numbers);

            string temp = m[0]; // Stores the match object in string "temp"
            a = stoi(temp); // Converts the string to an integer

            // Error case --> user specifies a graph with zero vertices
            if (a == 0) 
                cout << "Error: Graph cannot have zero vertices!" << endl;
            else
                V = a;
        }

        getline(cin, input);

        // Executes this block of code if the edge list is specifed through the "E" command
        if (regex_search(input,m,edge_rx)){
            adj_list = new vector<int>[V];

            // Creates a match object, "m", with all edges parsed from user input using regex "coordinates"
            while (regex_search(input,m,coordinates)){
                string s = m.str(0); // Parses one edge in the match object at a time
                
                // Extracts vertices from the edge coordinate
                string delimiter_1 = ",";
                string delimiter_2 = ">";
                string token_1 = s.substr(0, s.find(delimiter_1));
                string token_2 = s.substr(s.find(",")+1, s.find(delimiter_2));
                b = stoi(token_1);
                c = stoi(token_2);
                
                // Error case --> edge specification includes the same vertice twice
                if (b == c){ 
                    cout << "Error: Cannot have an edge between a vertice and itself!" << endl;
                    break;
                }

                // Error case --> edge specification includes a vertex higher than the total number of vertices in the graph
                if (b >= V || c >= V){ 
                    cout << "Error: Cannot have an edge between non-existant vertices." << endl;
                    break;
                }

                bool duplication = false;
                bool inverse_duplication = false;

                // Error case --> duplicate edges
                for (unsigned int i = 0; i < adj_list[b].size(); i++){ 
                    if (adj_list[b][i] == c){
                        cout << "Error: System does not allow duplicate edges." << endl;
                        duplication = true;
                        break;
                    }
                }

                if (duplication == true)
                    break;

                // Error case --> inverse duplicate edges
                for (unsigned int i = 0; i < adj_list[c].size(); i++){ 
                    if (adj_list[c][i] == b){
                        cout << "Error: System does not allow duplicate edges." << endl;
                        inverse_duplication = true;
                        break;
                    }
                }

                if (inverse_duplication == true)
                    break;
                
                // If the edge specification is valid, add it to the adjacency list
                if (duplication == false && inverse_duplication == false) 
                    add_edge(adj_list, b ,c);

                input = m.suffix(); //Used to iterate through remaining edges
            }
        }

        if (adj_list->size() == 0)
             cout << endl;
        else{
            minisat_reduction(V, adj_list);
        }        
    }
   return 0;
} 
    
