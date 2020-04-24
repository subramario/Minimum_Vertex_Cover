#include <iostream>
#include <list>
#include <vector>
#include <regex>
#include <iterator>
#include <string>
#include <stack>
#include <list>
#include <memory>
// defines Var and Lit
#include "minisat/core/SolverTypes.h"
// defines Solver
#include "minisat/core/Solver.h"

using namespace std; 

void add_edge(vector<int> adj_list[], int u, int v){ //creates an adjacency list (ie. array of arrays)
    adj_list[u].push_back(v); //add v into list u
    adj_list[v].push_back(u); //add u into list v
}

void minisat_reduction(int V, vector<int> adj_list[]){
    std::unique_ptr<Minisat::Solver> solver(new Minisat::Solver());
    bool res = false; //initializing solver to be false
    int k = 1; //Initialize minimum vertex cover as one

    while(k < V){
        //need to initialize my NxK atomic proposition matrix filled with literals
        Minisat::Lit matrix[V][k];

        for (int x = 0; x < V; x++)
            for (int y = 0; y < k; y++){
                Minisat::Lit literal;
                literal = Minisat::mkLit(solver->newVar());
                matrix[x][y] = literal; 
            }
        

        //case 1
        for (unsigned int i = 0; i < k; i++){ //for all i which exists from k=1 to k 
            //minisat only accepts 4 arguments max for each addClause! If multiple literals exist, use one vector of literals
            Minisat::vec<Minisat::Lit> clause_1;
            
            for (unsigned int j = 0; j < V; j++){
                clause_1.push(matrix[j][i]);
            }

            solver->addClause(clause_1);
        }
            
        //case 2
        for (unsigned int m = 0; m < V; m++){
            for (unsigned int p = 0; p < k; p++){
                for (unsigned int q = 0; q < k; q++){
                    if (p<q){
                        solver->addClause(~matrix[m][p],~matrix[m][q]);
                    }
                }
            }
        }

        
        //case 3
        for (unsigned int m = 0; m < k; m++){
            for (unsigned int p = 0; p < V; p++){
                for (unsigned int q = 0; q < V; q++){
                    if (p<q){
                        solver->addClause(~matrix[p][m],~matrix[q][m]);
                    }
                }
            }
        }
    
        //case 4
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

                        //cout << x << " ";
                    }
                }
            }

            // std::cout << "K value - " << k << endl;
            cout << minimum << endl;
        

            break;

        }

        else{
            solver.reset(new Minisat::Solver());
            k++;

        }
    }
}

//main function
int main(int argc, char* argv[]){
    //local variables declared here
   smatch m;
   string input;
   int a,b,c,V = 1;
   vector <int> *adj_list = NULL; //initializing adjacency list

    //matches command input to appropriate loop
   regex vert_rx("^[V]"); 
   regex edge_rx("^[E]");
   regex short_rx("^[s]");

   //extract relevant info from input
   const regex numbers("([0-9]+)+");
   const regex coordinates("([0-9]+[,][0-9]+)+");

   while (!cin.eof()){ //eof allows program to exit gracefully 
        getline(cin, input);

        if (cin.eof())
            break;

        if (regex_search(input,m,vert_rx)){
            regex_search(input, m, numbers);

            string temp = m[0];
            a = stoi(temp);

            if (a == 0) //Error case for if vertice = 0
                cout << "Error: Graph cannot have zero vertices!" << endl;
            else
                V = a;
        }

        getline(cin, input);

        if (regex_search(input,m,edge_rx)){
            adj_list = new vector<int>[V];
            while (regex_search(input,m,coordinates)){
                string s = m.str(0);
                
                string delimiter_1 = ",";
                string delimiter_2 = ">";
                string token_1 = s.substr(0, s.find(delimiter_1));
                string token_2 = s.substr(s.find(",")+1, s.find(delimiter_2));
                b = stoi(token_1);
                c = stoi(token_2);
                
                if (b == c){ //Error case when an edge spec includes the same vertice twice
                    cout << "Error: Cannot have an edge between a vertice and itself!" << endl;
                    break;
                }

                if (b >= V || c >= V){ //Error case when an edge spec includes a vertex higher than total number of vertices
                    cout << "Error: Cannot have an edge between non-existant vertices." << endl;
                    break;
                }

                bool flag_1 = false;
                bool flag_2 = false;


                for (unsigned int i = 0; i < adj_list[b].size(); i++){ //Error case for duplicate edges
                    if (adj_list[b][i] == c){
                        cout << "Error: System does not allow duplicate edges." << endl;
                        flag_1 = true;
                        break;
                    }
                }

                if (flag_1 == true)
                    break;

                for (unsigned int i = 0; i < adj_list[c].size(); i++){ //Error case for duplicate edges
                    if (adj_list[c][i] == b){
                        cout << "Error: System does not allow duplicate edges." << endl;
                        flag_2 = true;
                        break;
                    }
                }

                if (flag_2 == true)
                    break;
                
                if (flag_1 == false && flag_2 == false) //If the edge spec is valid, add it to the adjacency list
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
    
