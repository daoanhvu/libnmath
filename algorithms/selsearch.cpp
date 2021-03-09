void quickSort(vector<int> &arr, int left, int right) {
	int l = left;
	int r = right;
	int pivot = arr[(left + right) / 2];
	int temp;
	while(l <= r) {
		while(arr[l] < pivot) {
			l++;
		}
		while(arr[r] > pivot) {
			r--;
		}

		if(l <= r) {
			// Swap elements at left and right to each other
			temp = arr[l];
			arr[l] = arr[r];
			arr[r] = temp;
			l++;
			r--;
		}
	}
	quickSort(arr, left, l);
	quickSort(arr, r, right);
}

void insertionSort(vector<int> &arr) {
	int n = arr.size();
	int val;
	int j;
	for(int i=0; i<n; i++) {
		val = arr[i];
		j = i - 1;
		while(j>=0 && arr[j] > val) {
			arr[j+1] = arr[j];
			j--;
		}
		arr[j+1] = val;
	}
}

void selectionSort(vector<int> &arr) {
	int n = arr.size();
	int minIdx;
	for(int i=0; i<n; i++) {
		minIdx = i;
		for(int j=i+1; j<n; j++) {
			if(arr[j] < arr[minIdx]) {
				minIdx = j;
			}
		}
		if(i != minIdx) {
			// Swap element at i by the minimum element 
			int temp = arr[i];
			arr[i] = arr[minIdx];
			arr[minIdx] = temp;
		}
	}
}

int main(int argc, char *args[]) {

	return 0;
}
