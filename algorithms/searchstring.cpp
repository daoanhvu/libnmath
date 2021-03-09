#include<iostream>
#include<string>
#include<cstring>

using namespace std;

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

int main(int argc, char* args[]) {
    string S = "This is an example for string matching algorithm!";
    string P = "example";
    const char *s_p = S.c_str();
    int result = searchString(s_p, S.length(), P.c_str(), P.length());
    if(result < 0) {
        cout << "P not found in S";
        return 0;
    }

    cout << "Found P in S at position: " << result << endl;
    return 0;
}