#include<iostream>
#include<string>
#include<cstring>
#include<vector>

using namespace std;

void kmpTable(const char *p, int m, int *T);

int searchString(const char *s, int n, const char *p, int m) {
    int i = 0;
    int j;
    int l = n - m;
    while(i < l) {
        j = 0;
        while( (j < m) && (s[i + j] == p[j])) {
            j++;
        }
        if(j >= m) {
            // Found p at position i in S
            return i;
        }
        i++;
    }

    return -1;
}

// This section is for KPM algorithm
vector<int> kmp(const char *s, int n, const char *p, int m) {
    vector<int> found;
    int *T = new int[m];
    int i = 0;
    int j = 0;

    // Build prefix table
    kmpTable(p, m, T);

    while(i < n) {
        if(s[i] == p[j]) {
            i++;
            j++;
            if(j == m) {
                // Found P at (i-m) in S
                found.push_back(i-j);
                j = T[j];
            }
        } else {
            j = T[j];
            if(j < 0) {
                i++;
                j++;
            }
        }
    }
    delete[] T;
    return found;
}

void kmpTable(const char *p, int m, int *T) {
    int c = 0;
    int pos = 1;
    T[0] = -1;

    while(pos < m) {
        if(p[pos] == p[c]) {
            T[pos] = T[c];
        } else {
            T[pos] = c;
            while(c >= 0 && p[pos] != p[c]) {
                c = T[c];
            }
        }
        pos++;
        c++;
    }
    T[pos] = c;
}
// =================================

int testNaiveSearch() {
    string S = "This is an example for string matching algorithm!";
    string P = "example";
    const char *s_p = S.c_str();
    // int result = searchString(s_p, S.length(), P.c_str(), P.length());
    int result;


    if(result < 0) {
        cout << "P not found in S";
        return 0;
    }

    cout << "Found P in S at position: " << result << endl;
    return 0;
}

int testKMP() {
    string S = "This is an example for string matching algorithm, an awesome example!";
    string P = "example";
    const char *s_p = S.c_str();
    int m = P.length();
    int n = S.length();
    
    vector<int> found = kmp(S.c_str(), n, P.c_str(), m);
    cout << endl;
    for(int i=0; i<found.size(); i++) {
        cout << found[i] << " " ;
    }
    cout << endl;

    return 0;
}

int main(int argc, char* args[]) {
    string S = "This is an example for string matching algorithm!";
    string P = "example";
    const char *s_p = S.c_str();
    
    testKMP();

    return 0;
}