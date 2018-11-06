
public class BinaryHeap {
	
	private int [] a;
	private int size;

	public BinaryHeap() {
		a = new int [10];
		size = 0;
	}

	public void add(int pri) {
		if (size == a.length){
			growArray();
		}
		a[size] = pri;
		++size;
		int index = size - 1;
		int parent = (index - 1)/2;
		while (index > 0 && a[index] < a[parent]){
			swap(a, index, parent);
			index = parent;
			parent = (index - 1)/2;
		}
	}

	public int remove(){
		int pri = a[0];
		a[0] = a[size - 1];
		--size;
		shiftdown(0);
		return pri;
	}

	public void growArray(){
		int [] newArr = new int [a.length * 2];
		for (int i = 0; i < a.length; i++){
			newArr[i] = a[i];
		}
		a = newArr;
	}

	public void swap(int [] arr, int index, int parent){
		int temp = arr[index];
		arr[index] = arr[parent];
		arr[parent] = temp;
	}

	public void shiftdown(int parent){
		int child = parent * 2 + 1;
		//base case
		if (child >= size){
			return;
		}
		if (a[child + 1] < a[child]){
			child++;
		}
		if (a[parent] > a[child]){
			swap(a, child, parent);
			shiftdown(child);
		}
	}
}