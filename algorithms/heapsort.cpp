#include<iostream>
#include<vector>

using namespace std;

void heapify(vector<int> &arr, int n, int idx) {
	int l = idx * 2 + 1;
	int r = l + 1;
	int m = idx;
	int tmp;
	if(l < n && arr[idx] < arr[l]) {
		m = l;
	}
	if(r < n && arr[m] < arr[r]) {
		m = r;
	}
	if(m != idx) {
		//Swap (p, ch)
		tmp = arr[m];
		arr[m] = arr[idx];
		arr[idx] = tmp;
		// Shift down to child at index [m]
		heapify(arr, n, m);
	}
}

void heapsort(vector<int> &arr) {
	int n = arr.size();
	int m = (n-1)/2;
	// Build heap
	for(int i=m; i>=0; i--) {
		heapify(arr, n, i);
	}
	int tmp;
	int j = n-1;
	while(j>0) {
		//Swap (p, ch)
		tmp = arr[0];
		arr[0] = arr[j];
		arr[j] = tmp;
		n = n - 1;
		heapify(arr, n, 0);
		j--;
	}
}

int main(int argc, char* args[]) {
	// vector<int> arr = {12, 10, 8, 33, 54, 28, 30, 5, 18, 22};
	vector<int> arr = {6, 12, 10, 8, 2, 33, 54, 28, 30, 5, 18, 22, 1, 90};
	heapsort(arr);
	cout << endl;
	for(int i=0; i<arr.size(); i++) {
		cout << arr[i] << " ";
	}
	cout << endl;
	return 0;
}

