#ifndef MIN_HEAP_H
#define MIN_HEAP_H

#include <iostream>

/*
相当于一个完全二叉树，其中每个节点都比其左、右孩子节点小
存放在数组中，则若父节点下标为i，则其左、右孩子节点在数组中的下标分别为(2*i+1)、(2*i+2)
*/
template<class T>
class MinHeap{
public:
	MinHeap():_curSize(0){
		_heap=(T *)::malloc(sizeof(T)*MAX_SIZE);
	}
	~MinHeap(){
		::free(_heap);
		_heap=NULL;
	}
	bool InsertNUM(const T& num){
		if(!IsFull()){
			_heap[_curSize]=num;
			ShiftUp(_curSize);
			++_curSize;
			return true;
		}
		else
			return false;
	}
	void DeleteMin(){
		if(_curSize>1){
		_heap[0]=_heap[_curSize-1];
		--_curSize;
		ShiftDown(0,_curSize-1);
		}
		else
			--_curSize;
	}
	void HeapSort(){
		if(_curSize<=1)
			return;
		for(int i=_curSize-1;i>0;--i){	//将首尾节点交换，再对前n-1个调整
			T tmp=_heap[0];
			_heap[0]=_heap[i];
			_heap[i]=tmp;
			ShiftDown(0,i-1);
		}
	}
	T& GetMin() const{
			return _heap[0];
	}
	bool IsEmpty() const{
		return _curSize==0;
	}
	bool IsFull() const{
		return _curSize==MAX_SIZE;
	}
	void Print(){
		for(int i=0;i<_curSize;++i)
			std::cout<<_heap[i]<<std::endl;
	}

private:
	void ShiftUp(int pos){
		int i=pos;	//current
		int j=(i-1)/2;	//father
		while(j>=0&&i!=0){
			if(_heap[i]<_heap[j]){
				T tmp=_heap[i];
				_heap[i]=_heap[j];
				_heap[j]=tmp;
			}
			i=j;
			j=(j-1)/2;
		}
	}
	void ShiftDown(int start,int end){
		int i=start,j=2*i+1;
		T tmp=_heap[i];
		while(j<=end){
			if(j<end&&(_heap[j]>_heap[j+1]))
				++j;
			if(tmp<=_heap[j])
				break;
			else{
				_heap[i]=_heap[j];
				i=j;
				j=2*i+1;
			}
		}
		_heap[i]=tmp;
	}

private:
	static const int MAX_SIZE=100;
	int _curSize;
	T *_heap;
};


/*
相当于一个完全二叉树，其中每个节点都比其左、右孩子节点大
存放在数组中，则若父节点下标为i，则其左、右孩子节点在数组中的下标分别为(2*i+1)、(2*i+2)
*/
template<class T>
class MaxHeap{
public:
	MaxHeap():_curSize(0){
		_heap=(T *)::malloc(sizeof(T)*MAX_SIZE);
	}
	~MaxHeap(){
		::free(_heap);
		_heap=NULL;
	}
	bool InsertNUM(const T& num){
		if(!IsFull()){
			_heap[_curSize]=num;
			ShiftUp(_curSize);
			++_curSize;
			return true;
		}
		else
			return false;
	}
	void DeleteMax(){
		if(_curSize>1){
		_heap[0]=_heap[_curSize-1];
		--_curSize;
		ShiftDown(0,_curSize-1);
		}
		else
			--_curSize;
	}
	void HeapSort(){
		if(_curSize<=1)
			return;
		for(int i=_curSize-1;i>0;--i){
			T tmp=_heap[0];
			_heap[0]=_heap[i];
			_heap[i]=tmp;
			ShiftDown(0,i-1);
		}
	}
	T& GetMax() const{
			return _heap[0];
	}
	bool IsEmpty() const{
		return _curSize==0;
	}
	bool IsFull() const{
		return _curSize==MAX_SIZE;
	}
	void Print(){
		for(int i=0;i<_curSize;++i)
			std::cout<<_heap[i]<<std::endl;
	}

private:
	void ShiftUp(int pos){
		int i=pos;	//current
		int j=(i-1)/2;	//father
		while(j>=0&&i!=0){
			if(_heap[i]>_heap[j]){
				T tmp=_heap[i];
				_heap[i]=_heap[j];
				_heap[j]=tmp;
			}
			i=j;
			j=(j-1)/2;
		}
	}
	void ShiftDown(int start,int end){
		int i=start,j=2*i+1;
		T tmp=_heap[i];
		while(j<=end){
			if(j<end&&(_heap[j]<_heap[j+1]))
				++j;
			if(tmp>=_heap[j])
				break;
			else{
				_heap[i]=_heap[j];
				i=j;
				j=2*i+1;
			}
		}
		_heap[i]=tmp;
	}

private:
	static const int MAX_SIZE=100;
	int _curSize;
	T *_heap;
};








#endif