#include<iostream>
#include<vector>
#include<fstream>
#include<sstream>
#include<queue>
#include<string>

using namespace std;

#define IntPair std::pair<int, int>
#define UNDEF_LABEL -1

typedef struct tagEdge {
    int fromNode;
    int toNode;
    int d;
} Edge;

typedef struct tagVertex {
    int id;
    // List of connection from this vertex
    vector<Edge*> edges;
    // Shortest distance from the source node to this node
    int dist;
    int prev;
    bool visited;
} Vertex;

void readGraph(string filename, vector<Vertex*> &g);
void releaseGraph(vector<Vertex*> &g);

/**
	STL creates a max-heap for priority_queue but we need min-heap, for this reason we
	need this class to compare the elements in queue
*/
class CompareVertex {
	public:
		bool operator() (const Vertex *a, const Vertex *b) {
			return a->dist > b->dist;
		}
};

void descreaseKey(std::vector<Vertex*> &q, int id, int updatedDist, int prevId, const CompareVertex &compare) {
    int n = q.size();
    int idx = 0;
    while(idx < n && q[idx]->id != id) {
        idx++;
    }

    if(idx < n) {
        Vertex *pUpdated = q[idx];
        auto rmIter = q.begin() + idx;
        q.erase(rmIter);
        std::make_heap(q.begin(), q.end(), compare);
        pUpdated->dist = updatedDist;
        pUpdated->prev = prevId;
        q.push_back(pUpdated);   
        std::push_heap(q.begin(), q.end(), compare);     
    }
}

void pqDijkstra(vector<Vertex*> &g, int src) {
	CompareVertex compare;
    std::vector<Vertex*> Q;
    int V = g.size();
	for(int i=0; i<V; i++) {
        g[i]->dist = INT_MAX;
        g[i]->prev = UNDEF_LABEL;
		Q.push_back(g[i]);
	}

    std::make_heap(Q.begin(), Q.end(), compare);

	while(!Q.empty()) {
        std::pop_heap(Q.begin(), Q.end(), compare);
		auto u = Q.back();
		Q.pop_back();
		
        vector<Edge*> edges = u->edges;
        int E = edges.size();
        for(int i=0; i<E; i++) {
            int v = edges[i]->toNode;
            if(g[v]->dist > u->dist + edges[i]->d) {
                descreaseKey(Q, g[v]->id, u->dist + edges[i]->d, u->id, compare);
            }
        }
	}

}



int main(int argc, char *args[]) {
    vector<Vertex*> graph;
    // readGraph(args[1], graph);

    // This is for testing the descreaseKey from queue
    std::vector<Vertex*> Q;
    Vertex *pv1 = new Vertex();
    pv1->dist = 70;
    Q.push_back(pv1);


    Vertex *pv2 = new Vertex();
    pv2->dist = 10;
    Q.push_back(pv2);

    Vertex *pv3 = new Vertex();
    pv3->dist = 45;
    Q.push_back(pv3);

    std::make_heap(Q.begin(), Q.end(), CompareVertex());

    cout << "Size of queue:" << Q.size() << endl;
    std::pop_heap(Q.begin(), Q.end(), CompareVertex());
    auto top = Q.back();
    cout << "Top item:" << top->dist << endl;

    pv1->dist = 5;
    Q.push_back(pv1);
    cout << "Size of queue:" << Q.size() << endl;
    std::push_heap(Q.begin(), Q.end(), CompareVertex());
    auto top1 = Q.back();
    cout << "Top item:" << top1->dist << endl;

    delete pv1;
    delete pv2;
    delete pv3;
    // End testing

    // releaseGraph(graph);
	return 0;
}

void readGraph(string filename, vector<Vertex*> &g) {
    ifstream file(filename, std::ios::binary);
    string line;
    int n;

    if(!file.is_open()) {
        cout << "Opening file failed." << endl;
        return;
    }

    // read the number of vertex
    std::getline(file, line);
    istringstream ss(line);

    ss >> n;
    int v;

    cout << "Number of vertex: " << n << endl;

    for(int i=0; i<n; i++) {
        std::getline(file, line);
        istringstream strline(line);
        Vertex *pVertex = new Vertex();
        pVertex->id = i;
        pVertex->dist = INT_MAX;
        pVertex->prev = UNDEF_LABEL;

        for(int j=0; j<n; j++) {
            strline >> v;
            if(v > 0) {
                Edge *pEd = new Edge();
                pEd->fromNode = i;
                pEd->toNode = j;
                pVertex->edges.push_back(pEd);

            }
        }
        g.push_back(pVertex);
    }

    file.close(); 
}

void releaseGraph(vector<Vertex*> &g) {
    int n = g.size();

    for(int i=0; i<n; i++) {
        Vertex *pV = g[i];
        if(pV != NULL) {
            int e = pV->edges.size();
            for(int j=0; j<e; j++) {
                if(pV->edges[j] != NULL) {
                    delete pV->edges[j];
                }
            }
        }
        delete pV;
    }
}
